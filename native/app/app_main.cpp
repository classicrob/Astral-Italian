#include <petari/gpu_trace.hpp>
#include "editor_m4_smoke.hpp"
#include "display_settings.hpp"
#include "display_window.hpp"
#include <petari/ubershader_mode.hpp>
extern "C" bool aurora_imgui_update_texture(unsigned long long, unsigned, unsigned, const void*);
// Native entry point: window, platform start-up, then the game's main loop
// on this (the macOS main) thread. See docs/dev/INTEGRATION_PLAN.md.

#include <aurora/aurora.h>
#include <aurora/main.h>
#include <aurora/event.h>
#include <aurora/gfx.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_hints.h>
#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <pthread/qos.h>
#include <petari/thread_priority.hpp>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#ifdef PETARI_EDITOR_UI
#include <aurora/imgui.h>
#include <petari/editor/ui.hpp>
extern "C" int petari_present_take_readback(const void* copy, unsigned char* rgba, unsigned capacity, unsigned* width, unsigned* height);
#endif
#include <petari/test_fixture.hpp>
#include <petari/unlocked_save.hpp>
#include <petari/pipeline_startup.hpp>
#include <petari/mods.hpp>
#ifdef PETARI_EDITOR_UI
#include <petari/editor/terrain/bake.hpp>
#endif
#ifdef PETARI_EDITOR_UI
#include <petari/editor/import_assets.hpp>
#endif
#include <petari/platform/mod_folder.hpp>
#include <petari/platform/mod_galaxy.hpp>
#include <petari/progress.hpp>
#include <petari/camera_input.h>
#include <petari/camera_settings.hpp>
#ifdef PETARI_EDITOR_UI
#include <petari/editor/edit_api.hpp>
#endif
#include <petari/editor_pick.hpp>
#include <petari/editor_settings.hpp>
#include <petari/efb_dump_mark.hpp>
#include <petari/input.hpp>
#include <petari/home_menu.hpp>

#include "host.hpp"
#include "app_locations.hpp"
#include "first_run.hpp"
#include "smoke_background.hpp"
#include "smoke_storage.hpp"
#include "pipeline_cache_dir.hpp"
#include "display_link.hpp"
#include "latency_report.hpp"
#include "../gx/boot_splash.h"
#include <petari/launch_stage.hpp>
#include "launch_stage.hpp"
#include "tape_play.hpp"
#include "tape_scene.hpp"
#include "tape_record.hpp"
#include <petari/player_launch.hpp>
#include <petari/host_allocation.hpp>

extern "C" void petari_gx_pipeline_background_begin();
extern "C" void petari_efb_snapshot_prewarm();
extern "C" void petari_gx_pipeline_set_gameplay(bool gameplay);
extern "C" void petari_gx_pipeline_set_complete_scenes(bool complete);

namespace App = PetariNative::App;

namespace {

// A game never restores its windows. Without this, after Petari crashed (a test sweep crashes it on
// purpose) macOS asks "reopen windows?" in a modal alert at the next launch, which blocks the game
// (and background test runs) before the first frame. Registration domain only: nothing is persisted.
void disableWindowRestoration() {
    const void* keys[2] = {CFSTR("ApplePersistenceIgnoreState"), CFSTR("NSQuitAlwaysKeepsWindows")};
    const void* values[2] = {kCFBooleanTrue, kCFBooleanFalse};
    CFDictionaryRef defaults = CFDictionaryCreate(nullptr, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);
    const auto sendObject = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
    const id standard = sendObject(reinterpret_cast<id>(objc_getClass("NSUserDefaults")), sel_registerName("standardUserDefaults"));
    reinterpret_cast<void (*)(id, SEL, CFDictionaryRef)>(objc_msgSend)(standard, sel_registerName("registerDefaults:"), defaults);
    CFRelease(defaults);
}


void logMessage(AuroraLogLevel level, const char* module, const char* message, unsigned int length) {
    std::fprintf(stderr, "[%s] %.*s\n", module, static_cast<int>(length), message);
    if (level == LOG_FATAL) {
        std::abort();
    }
}

void usage() {
    std::fputs("Usage: petari [--disc DIR] [--user DIR] [--stage GALAXY [--scenario N]]\n"
               "  --disc DIR  extracted disc (containing files/); default: $PETARI_GAME_DIR,\n"
               "              remembered app.cfg choice, dev build/game-data/SB4E01, then first-run setup\n"
               "  --setup     choose a game disc again before starting\n"
               "  --user DIR  saves, settings, controls and crash reports;\n"
               "              default: ~/Library/Application Support/Astral Italian\n"
               "  --stage GALAXY [--scenario N]  one normal file-load stage request, scenario default 1.\n"
               "              GALAXY is an SMG2 internal name or English-name alias; actual ScenarioNo membership is checked.\n"
               "              --stage list prints the SMG2 catalog after the game's metadata loads.\n"
               "  --make-unlocked-save all-missions|complete-luigi|grand-finale  unlock file 1 of an isolated\n"
               "              --user holding .petari-make-unlocked-save, save, reload and verify it, then exit\n"
               "              (native/SAVES.md; use native/tools/make_unlocked_save.py)\n"
               "  --make-unlocked-save feed-galaxy-lumas  in every file of an isolated --user, feed only the\n"
               "              in-galaxy Hungry Lumas whose mission star the file holds; save, reload, verify, exit\n",
               stderr);
}

bool setupQuit = false;

bool initializeSetupUI() {
    // SDL creates its NSApplication subclass and standard menus only when NSApp is nil.
    // First-run AppKit dialogs must not create a plain NSApplication ahead of SDL.
    if (SDL_WasInit(SDL_INIT_VIDEO)) return true;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO)) return true;
    std::fprintf(stderr, "petari: could not open game-disc setup: %s\n", SDL_GetError());
    return false;
}

bool resolvePaths(int argc, char** argv, App::Paths* paths, const App::AppLocations& locations) {
    bool setup = false;
    bool userGiven = false;
    bool fixture = false;
    std::string unlockedSave;
    std::string launchStage;
    bool launchStageGiven = false;
    int launchScenario = 1;
    bool launchScenarioGiven = false;
    std::string replayTape;  // --replay-tape <dir> / PETARI_TAPE_PLAY=<dir>: a recorded tape replayed in this window
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--stage") == 0 && i + 1 < argc) {
            if (launchStageGiven) { std::fputs("petari: duplicate --stage\n", stderr); return false; }
            launchStageGiven = true;
            launchStage = argv[++i];
            if (launchStage.empty()) { std::fputs("petari: --stage requires a name or list\n", stderr); return false; }
        } else if (std::strcmp(argv[i], "--scenario") == 0 && i + 1 < argc) {
            const char* text = argv[++i];
            int value = 0;
            const auto parsed = std::from_chars(text, text + std::strlen(text), value);
            if (launchScenarioGiven || parsed.ec != std::errc() || parsed.ptr != text + std::strlen(text) || value < 1) {
                std::fprintf(stderr, "petari: --scenario expects a mission number, got %s\n", argv[i]);
                return false;
            }
            launchScenario = value;
            launchScenarioGiven = true;
        } else if (std::strcmp(argv[i], "--replay-tape") == 0 && i + 1 < argc) {
            replayTape = argv[++i];
        } else if (std::strcmp(argv[i], "--disc") == 0 && i + 1 < argc) {
            paths->disc = argv[++i];
        } else if (std::strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            paths->user = argv[++i];
            userGiven = true;
        } else if (std::strcmp(argv[i], "--setup") == 0) {
            setup = true;
        } else if (std::strcmp(argv[i], "--test-fixture") == 0 && i + 1 < argc &&
                   std::strcmp(argv[i + 1], "observatory") == 0) {
            ++i;
            fixture = true;
        } else if (std::strcmp(argv[i], "--test-fixture") == 0 && i + 1 < argc &&
                   std::strcmp(argv[i + 1], "stage") == 0) {
            ++i;
            fixture = true;
        } else if (std::strcmp(argv[i], "--make-unlocked-save") == 0 && i + 1 < argc &&
                   (std::strcmp(argv[i + 1], "all-missions") == 0 || std::strcmp(argv[i + 1], "complete-luigi") == 0 ||
                    std::strcmp(argv[i + 1], "grand-finale") == 0 ||
                    std::strcmp(argv[i + 1], "feed-galaxy-lumas") == 0)) {
            unlockedSave = argv[++i];
        } else {
            return false;
        }
    }
    if (replayTape.empty()) {
        if (const char* env = std::getenv("PETARI_TAPE_PLAY"); env != nullptr && env[0] != '\0') replayTape = env;
    }
    if (!replayTape.empty()) {
        // The tape's first segment is entered like a normal --stage request (a save is still the player's own: use --user);
        // later segments are reached by the level-select warp (native/PLAYTEST.md "Replaying your tape").
        const char* sessionMode = std::getenv("PETARI_TAPE_PLAY_SESSION");
        const bool wholeSession = sessionMode && std::strcmp(sessionMode, "1") == 0;
        if ((!launchStage.empty() && !wholeSession) || fixture) {
            std::fputs("petari: --replay-tape replaces --stage and cannot be combined with it\n", stderr);
            return false;
        }
        setenv("ASTRAL_TAPE", "off", 1);  // a replay is not recorded again (the recorder is for a person's own session)
        std::string error;
        if (!App::TapePlay::configure(replayTape, &error)) {
            std::fprintf(stderr, "petari: --replay-tape: %s\n", error.c_str());
            return false;
        }
        const auto* first = App::TapePlay::firstSegment();
        if (wholeSession && App::TapePlay::meta()->schema==2) {
            const auto& recorded=*App::TapePlay::meta();
            if ((!launchStage.empty() && launchStage!=recorded.launchStage) ||
                (launchScenarioGiven && launchScenario!=recorded.launchScenario)) {
                std::fputs("petari: explicit launch disagrees with schema2 tape\n",stderr);
                return false;
            }
            launchStage=recorded.launchStage; launchScenario=recorded.launchScenario;
            launchScenarioGiven=!launchStage.empty();
        }
        if (first) {  // whole-session mode boots normally into the title, then drives FileSelect from the tape
            launchStage = first->stage;
            launchScenario = first->scenario > 0 ? first->scenario : 1;
            launchScenarioGiven = true;
        }
        std::fprintf(stderr, "PETARI TAPE PLAY: %s: %zu segment(s), mods [", replayTape.c_str(), App::TapePlay::meta()->segments.size());
        for (std::size_t i = 0; i < App::TapePlay::meta()->mods.size(); ++i)
            std::fprintf(stderr, "%s%s", i ? "," : "", App::TapePlay::meta()->mods[i].c_str());
        std::fputs("]; physical input is ignored\n", stderr);
    }
    if (fixture) {
        std::fputs("petari: legacy SMG1 --test-fixture modes are unavailable in SMG2; use --stage/--scenario for a normal stage request\n", stderr);
        return false;
    }
    // A restart into a new galaxy (the level editor's New Galaxy): read once, then cleared so a later
    // restart doesn't repeat it.
    if (const char* open = std::getenv("PETARI_OPEN_STAGE"); open != nullptr && open[0] != '\0' && launchStage.empty()) {
        const char* edit = std::getenv("PETARI_OPEN_EDIT");
        const char* title = std::getenv("PETARI_OPEN_TITLE");
        // A test fixture's stage checks need play, so it only logs the Edit request.
        PetariNative::PlayerLaunch::openEdit = edit != nullptr && edit[0] == '1' && !fixture;
        PetariNative::PlayerLaunch::openTitle = title != nullptr && title[0] != '\0' ? title : open;
        if (!fixture) {
            const char* file = std::getenv("PETARI_OPEN_FILE");
            PetariNative::PlayerLaunch::openFile = file != nullptr ? std::atoi(file) : 0;
            launchStage = open;
            launchScenario = 1;
        }
        std::fprintf(stderr, "PETARI OPEN: %s (file %d, edit %d)\n", open, PetariNative::PlayerLaunch::openFile,
                     edit != nullptr && edit[0] == '1' ? 1 : 0);
    }
    for (const char* name : {"PETARI_OPEN_STAGE", "PETARI_OPEN_FILE", "PETARI_OPEN_EDIT", "PETARI_OPEN_TITLE"}) {
        unsetenv(name);
    }
    if (launchScenarioGiven && launchStage.empty()) {
        std::fputs("petari: --scenario needs --stage\n", stderr);
        return false;
    }
    if (launchStage == "list" && launchScenarioGiven) {
        std::fputs("petari: --stage list does not accept --scenario\n", stderr);
        return false;
    }
    if (!launchStage.empty()) {
        if (fixture || !unlockedSave.empty()) {
            std::fputs("petari: --stage cannot be combined with save mutation or legacy fixtures\n", stderr);
            return false;
        }
    }
    if (!unlockedSave.empty()) {
        // Rewrites a saved file: only an explicit, marked, isolated directory.
        const char* home = std::getenv("HOME");
        const bool normalUser = home && !paths->user.empty() &&
            std::filesystem::weakly_canonical(paths->user) ==
            std::filesystem::weakly_canonical(App::AppLocations::defaultRoot(home));
        if (paths->user.empty() || normalUser || fixture ||
            !std::filesystem::is_regular_file(paths->user / ".petari-make-unlocked-save")) {
            std::fputs("petari: --make-unlocked-save requires an explicit isolated --user holding "
                       ".petari-make-unlocked-save, and no --test-fixture\n", stderr);
            return false;
        }
        PetariNative::UnlockedSave::variant = unlockedSave;
        std::fprintf(stderr, "PETARI UNLOCKED SAVE: requested %s for file %d in %s\n", unlockedSave.c_str(),
                     PetariNative::UnlockedSave::slot, paths->user.string().c_str());
    }
    // The stage smoke script needs the stage fixture's entry.
    const char* smoke = std::getenv("PETARI_SMOKE");
    if (smoke != nullptr && std::strcmp(smoke, "stage") == 0) {
        std::fputs("petari: the legacy stage smoke fixture is unavailable in SMG2; use --stage/--scenario\n", stderr);
        return false;
    }
    if (!userGiven) paths->user = locations.root;
    if (const char* home = std::getenv("HOME"); home != nullptr && home[0] != '\0' &&
        App::isOtherGameUserPath(paths->user, home)) {
        std::fputs("petari: refusing MacioGalaxy's user folder (SMG1 saves); SMG2 uses ~/Library/Application Support/Astral Italian\n", stderr);
        return false;
    }
    if (!userGiven) {
        const char* home = std::getenv("HOME");
        const auto source = App::migrationSource(locations, home ? std::filesystem::path(home) : std::filesystem::path{},
                                                  App::readAppConfig(locations));
        if (!source.empty() && (!initializeSetupUI() || !App::FirstRun::offerMigration(locations))) return false;
    }
    const char* gameDir = std::getenv("PETARI_GAME_DIR");
    const auto config = !paths->disc.empty() || (gameDir && *gameDir) || setup
        ? App::AppConfig{} : App::readAppConfig(locations);
    const auto resolution = App::resolveDisc(paths->disc, gameDir, config, std::filesystem::current_path());
    if (!resolution.problem.empty()) {
        if (!initializeSetupUI()) return false;
        App::FirstRun::showProblem(resolution.problem);
    }
    paths->disc = resolution.disc;
    if (setup || paths->disc.empty()) {
        if (!initializeSetupUI()) return false;
        if (!App::FirstRun::chooseDisc(locations, &paths->disc)) { setupQuit = true; return false; }
    }
    std::string catalogError;
    if (!App::LaunchStage::configureCatalog(paths->disc, &catalogError)) {
        std::fprintf(stderr, "petari: SMG2 level select unavailable: %s\n", catalogError.c_str());
        if (!launchStage.empty() || !PetariNative::UnlockedSave::variant.empty()) return false;
    }
    if (launchStage == "list") {
        App::LaunchStage::requestCatalogListing();
    } else if (!launchStage.empty()) {
        std::string error;
        if (!App::LaunchStage::configureStartupRequest(launchStage, launchScenario, &error)) {
            std::fprintf(stderr, "petari: --stage: %s\n", error.c_str());
            return false;
        }
        std::fprintf(stderr, "PETARI LAUNCH: queued SMG2 request %s scenario %d; awaiting real catalog/file load\n", launchStage.c_str(), launchScenario);
    }
    App::prepareRunArtifacts(locations);
    // The canonical catalog is published only after game boot. A pending
    // resolved request therefore does not exist yet here or at Seam::attach.
    // Retain the exact accepted launch spelling/scenario, which replay passes
    // through the same catalog resolution instead of silently booting title.
    if (App::TapeRecord::routePath()) App::TapeRecord::setLaunch(launchStage, launchScenario);
    return true;
}

bool prepareKnownPipelines(SDL_Window* window) {
    // The blocking preparation screen is opt-in (PETARI_PIPELINE_GLOBAL_PRECOMPILE=1).
    // By default the game starts at once; the scene being loaded compiles first and
    // everything else compiles in the background while playing.
    if (!petari_gx_pipeline_full_preparation()) return true;
    return App::preparePipelines(window);
}

// Frame classification also tells the shader pool when speculative compiles
// must yield to gameplay (menus, file select, loading and pause run them at full speed).
App::FrameStats::Phase framePhaseForPipelines() {
    const App::FrameStats::Phase phase = App::Host::framePhase();
    const bool gameplay = phase == App::FrameStats::Phase::Gameplay && !App::Host::pauseMenuActive();
    petari_gx_pipeline_set_gameplay(gameplay);
    // No partial scenes outside active play: intros, menus, loading and pause wait for
    // their pipelines (the fade holds) instead of skipping draws (pipeline_runtime.inc).
    petari_gx_pipeline_set_complete_scenes(!(gameplay && App::Host::playerInPlay()));
    App::LatencyReport::frame(gameplay);
    // Opt-in image dumps of the whole session (PETARI_XFB_DUMP with _LABELS=boot) count from the first frame.
    static bool bootMarked = false;
    if (!bootMarked) {
        bootMarked = true;
        PetariNative::EfbDump::mark("boot");
    }
    // Test driver (PETARI_MODS_AUTOPRESS=<frames>): during gameplay, press the
    // collect mod's button every <frames> frames and the shoot mod's halfway
    // between. The mods still decide whether a press does anything.
    static const long period = [] {
        const char* value = std::getenv("PETARI_MODS_AUTOPRESS");
        return value != nullptr ? std::strtol(value, nullptr, 10) : 0L;
    }();
    // Test driver (PETARI_TEST_WARP=<galaxy>:<mission>): once, 30 gameplay frames
    // in, queue the warp the Home menu's Level Select Go would (its UI is
    // covered by native_home_menu); the game takes it at its next permitted frame.
    static std::string testWarp = [] {
        const char* value = std::getenv("PETARI_TEST_WARP");
        return std::string(value != nullptr ? value : "");
    }();
    static long warpFrames = 0;
    if (!testWarp.empty() && gameplay && ++warpFrames == 30) {
        const auto colon = testWarp.find(':');
        std::string stage, error;
        const int mission = colon == std::string::npos ? 1 : std::atoi(testWarp.c_str() + colon + 1);
        if (App::LaunchStage::resolve(testWarp.substr(0, colon), mission, &stage, &error)) {
            std::snprintf(PetariNative::PlayerLaunch::warpStage, sizeof(PetariNative::PlayerLaunch::warpStage), "%s", stage.c_str());
            PetariNative::PlayerLaunch::warpScenario = mission;
            PetariNative::PlayerLaunch::warpPending.store(true);
            std::fprintf(stderr, "[level-select-test] queued warp to %s mission %d\n", stage.c_str(), mission);
        } else {
            std::fprintf(stderr, "[level-select-test] %s\n", error.c_str());
        }
        testWarp.clear();
    }
#ifdef PETARI_EDITOR_UI
    // A new galaxy's first start (PETARI_OPEN_EDIT=1, after New galaxy's restart): into Edit on the first playable
    // frames, as a Tab tap would; the "Creating <name>..." banner until then (UX section 14).
    static long openEditFrames = 0;
    if (PetariNative::PlayerLaunch::openEdit) {
        if (PetariNative::EditorSettings::active()) {
            std::fprintf(stderr, "[editor] opened %s in Edit\n", PetariNative::PlayerLaunch::openTitle.c_str());
            PetariNative::PlayerLaunch::openEdit = false;
        } else {
            PetariEditor::Ui::showOpening(PetariNative::PlayerLaunch::openTitle.c_str());
            if (gameplay && (++openEditFrames == 10 || (openEditFrames > 10 && openEditFrames % 60 == 0 && openEditFrames < 600))) {
                PetariNative::EditorSettings::toggle();
            }
        }
    }
#endif
    // Test driver (PETARI_CAMERA_TEST=1, with the Odyssey camera mod on), counted only
    // while the orbit is active (scripted cameras leave it alone): first a collision
    // sweep (farthest zoom, camera below Mario, one full turn: the line of sight must
    // hit the floor and nearby walls; XFB label "camera-sweep"), then right-stick orbit,
    // pitch, zoom, mouse drag and recentre. PETARI_CAMERA_TEST_DELAY=N waits N orbit
    // frames first (e.g. until a smoke's movement checks, which read the camera, are over).
    static const bool cameraTest = std::getenv("PETARI_CAMERA_TEST") != nullptr;
    static const long cameraDelay = std::getenv("PETARI_CAMERA_TEST_DELAY") ? std::atol(std::getenv("PETARI_CAMERA_TEST_DELAY")) : 0;
    static long cameraFrames = 0;
    if (cameraTest && gameplay && PetariNative::CameraSettings::enabled() && petari_camera_orbit_active() &&
        ++cameraFrames > cameraDelay) {
        namespace Camera = PetariNative::CameraSettings;
        const long f = (cameraFrames - cameraDelay) % 900;
        if (f == 30) {
            std::fputs("[camera-test] collision sweep\n", stderr);
            PetariNative::EfbDump::mark("camera-sweep");
            Camera::zoomSteps(20.0f);
        }
        if (f == 300) std::fputs("[camera-test] orbit right\n", stderr);
        if (f == 540) std::fputs("[camera-test] pitch, zoom, drag, recentre\n", stderr);
        const float sweepX = f >= 80 && f < 200 ? 1.0f : 0.0f, sweepY = f >= 30 && f < 80 ? -1.0f : 0.0f;
        Camera::stick(f >= 300 && f < 420 ? 0.7f : sweepX, f >= 480 && f < 540 ? 0.6f : sweepY);
        if (f == 240) Camera::recenter();
        if (f == 560) Camera::zoomSteps(4.0f);
        if (f == 620) Camera::zoomSteps(-6.0f);
        if (f >= 660 && f < 690) Camera::mouseDrag(-8.0f, 2.0f);
        if (f == 840) Camera::recenter();
    } else if (cameraTest) {
        PetariNative::CameraSettings::stick(0.0f, 0.0f);
    }
    // Photo mode test driver (PETARI_PHOTO_TEST=N, with PETARI_PHOTO_MODE=1): after N
    // gameplay frames, press PhotoMode (retried until the game takes it), fly forward
    // and up, look round, narrow the view, take two screenshots, and leave.
    static const long photoTest = std::getenv("PETARI_PHOTO_TEST") ? std::atol(std::getenv("PETARI_PHOTO_TEST")) : -1;
    static long photoFrames = 0, photoStep = -1;
    // (Once in photo mode the frame no longer counts as gameplay: keep driving it.)
    if (photoTest >= 0 && (gameplay || PetariNative::CameraSettings::photoActive()) && PetariNative::CameraSettings::photoEnabled() &&
        photoStep < 400) {
        namespace Camera = PetariNative::CameraSettings;
        if (++photoFrames > photoTest) {
            if (photoStep < 0) {
                if ((photoFrames - photoTest) % 30 == 1 && !Camera::photoActive()) {
                    std::fputs("[photo-test] PhotoMode press\n", stderr);
                    Camera::photoToggle();
                }
                if (Camera::photoActive()) {
                    photoStep = 0;
                    PetariNative::EfbDump::mark("photo");
                }
            } else {
                const long t = ++photoStep;
                Camera::photoMove(0.0f, t < 90 ? 1.0f : 0.0f, t < 60 ? 0.5f : 0.0f);
                Camera::photoLookHold(t >= 100 && t < 172 ? 1.0f : 0.0f, t >= 100 && t < 120 ? -0.5f : 0.0f);
                if (t == 180) Camera::photoFov(-4.0f);
                if (t == 200 || t == 300) {
                    std::fputs("[photo-test] PhotoShot press\n", stderr);
                    Camera::photoShot();
                }
                if (t == 220) Camera::photoSpeed(true, false);
                if (t >= 220 && t < 260) Camera::photoMove(-1.0f, 0.0f, 0.0f);
                if (t == 260) Camera::photoSpeed(false, false);
                if (t == 360) {
                    std::fputs("[photo-test] PhotoMode press (leave)\n", stderr);
                    Camera::photoMove(0.0f, 0.0f, 0.0f);
                    Camera::photoToggle();
                    photoStep = 400;
                }
            }
        }
    }
#ifdef PETARI_EDITOR_UI
    // Level editor test driver (PETARI_EDITOR_TEST=N, with the LevelEditor mod on, e.g.
    // PETARI_MODS=LevelEditor): after N gameplay frames, tap the editor key (retried until the game
    // takes it), orbit, zoom toward the centre, pan, frame Mario, and leave.
    static const long editorTest = std::getenv("PETARI_EDITOR_TEST") ? std::atol(std::getenv("PETARI_EDITOR_TEST")) : -1;
    static long editorFrames = 0, editorStep = -1;
    if (editorTest >= 0 && (gameplay || PetariNative::EditorSettings::active()) && PetariNative::EditorSettings::enabled() &&
        editorStep < 400) {
        namespace Editor = PetariNative::EditorSettings;
        if (++editorFrames > editorTest) {
            if (editorStep < 0) {
                if ((editorFrames - editorTest) % 30 == 1 && !Editor::active()) {
                    std::fputs("[editor-test] LevelEditor tap\n", stderr);
                    Editor::toggle();
                }
                if (Editor::active()) {
                    editorStep = 0;
                    PetariNative::EfbDump::mark("editor");
                }
            } else if (const char* gizmoTarget = std::getenv("PETARI_EDITOR_TEST_GIZMO")) {
                // The gizmo, end to end: select an object by name, frame it, grab the move arrow along
                // X where the game drew it, drag it, let go (one op), then drop it to the ground (End).
                namespace Gizmo = PetariNative::EditorGizmo;
                namespace Pick = PetariNative::EditorPick;
                static float tipX = 0.0f, tipY = 0.0f, dirX = 0.0f, dirY = 0.0f;
                const long t = ++editorStep;
                if (t == 5) Pick::selectByName(gizmoTarget);
                if (t == 15) Editor::frame();
                if (t == 60) {
                    Gizmo::setMode(Gizmo::Mode::Move);
                    const Gizmo::View view = Gizmo::view();
                    if (!view.lines.empty()) {  // the X arrow's shaft: origin -> tip
                        const Gizmo::Line& shaft = view.lines[0];
                        tipX = shaft.x0 + (shaft.x1 - shaft.x0) * 0.8f;
                        tipY = shaft.y0 + (shaft.y1 - shaft.y0) * 0.8f;
                        const float len = std::sqrt((shaft.x1 - shaft.x0) * (shaft.x1 - shaft.x0) + (shaft.y1 - shaft.y0) * (shaft.y1 - shaft.y0));
                        dirX = len > 0.0f ? (shaft.x1 - shaft.x0) / len : 1.0f;
                        dirY = len > 0.0f ? (shaft.y1 - shaft.y0) / len : 0.0f;
                    }
                    std::fprintf(stderr, "[editor-test] gizmo: %d lines, grab at (%.3f, %.3f)\n", static_cast<int>(view.lines.size()), tipX, tipY);
                    Editor::cursor(tipX, tipY);
                }
                if (t == 62) Gizmo::press();
                if (t > 62 && t <= 82) Editor::cursor(tipX + dirX * 0.012f * static_cast<float>(t - 62), tipY + dirY * 0.012f * static_cast<float>(t - 62));
                if (t == 84) Gizmo::release();
                if (t == 120) Gizmo::ground(Gizmo::Ground::Drop);
                // The play loop: the marker under the centre, Tab plays (Runtime reloads the edited layout and
                // puts Mario at the marker), Tab again freezes back into Edit (at t 250, below), and leaves at 330.
                if (t == 140) {
                    Editor::cursor(0.0f, -0.2f);
                    Pick::placeMarker();
                }
                if (t == 170) {
                    std::fputs("[editor-test] LevelEditor tap (play)\n", stderr);
                    Editor::toggle();
                }
                // PETARI_EDITOR_TEST_PLACE_CLASS=<class>: place a new one of a class with no example (EP-005), as the Browser's
                // Place does for a catalog-only card; Obj_arg0 = 0 (a brush's box preset).
                if (t == 147 && std::getenv("PETARI_EDITOR_TEST_PLACE_CLASS") != nullptr) {
                    Editor::cursor(0.0f, -0.3f);
#ifdef PETARI_EDITOR_UI
                    PetariEditor::Ui::placeClassForTest(std::getenv("PETARI_EDITOR_TEST_PLACE_CLASS"), 0);  // the Browser's path
#endif
                }
                // PETARI_EDITOR_TEST_PLACE=1: place a copy of the selection on the ground at the view's centre (the
                // Browser's Place path).
                // ...then duplicate the placed copy (Cmd+D) and delete the duplicate (Delete).
                static std::uint64_t sPlaceExample = 0;
                if (t == 141 && std::getenv("PETARI_EDITOR_TEST_PLACE") != nullptr) {
                    const std::vector<Pick::Item> selected = Pick::selection();
                    sPlaceExample = !selected.empty() ? selected.front().row : 0;
                }
                // PETARI_EDITOR_TEST_PLACE_OFFMISSION=1 (EP-008): the example is a row from another mission's layer.
                if (t == 141 && std::getenv("PETARI_EDITOR_TEST_PLACE_OFFMISSION") != nullptr) {
                    if (const PetariEditor::ModelView* model = PetariEditor::model()) {
                        const std::uint32_t bit = 1u << (model->scenario() - 1);
                        for (int zone = 0; zone < model->zoneCount() && sPlaceExample == 0; zone++) {
                            for (int table = 0; table < model->tableCount(zone) && sPlaceExample == 0; table++) {
                                if (std::strstr(model->tablePath(zone, table), "placement/layer") == nullptr) continue;
                                if (std::strstr(model->tablePath(zone, table), "/objinfo") == nullptr) continue;
                                model->forEachRow(zone, table, [&](PetariEditor::RowId row) {
                                    if (sPlaceExample == 0 && (model->rowMissions(row) & bit) == 0) sPlaceExample = row.v;
                                });
                            }
                        }
                        std::fprintf(stderr, "[editor-test] place: off-mission example %s\n",
                                     sPlaceExample != 0 && model->objectName(PetariEditor::RowId{sPlaceExample}) ? model->objectName(PetariEditor::RowId{sPlaceExample}) : "(none)");
                    }
                }
                if (sPlaceExample != 0) {
                    if (t == 142) {
                        Editor::cursor(0.0f, -0.3f);
                        Pick::place(sPlaceExample);
                    }
                    if (t == 150) Pick::duplicate();
                    if (t == 158) {
                        const PetariEditor::ModelView* model = PetariEditor::model();
                        const std::vector<Pick::Item> selected = Pick::selection();
                        if (model != nullptr && !selected.empty() && selected.front().row != 0) {
                            PetariEditor::Command remove{PetariEditor::Command::Kind::Apply};
                            remove.op.label = "Deleted the duplicate";
                            remove.op.deletes.push_back(PetariEditor::RowId{selected.front().row});
                            PetariEditor::enqueue(std::move(remove));
                            std::fputs("[editor-test] delete the selection\n", stderr);
                        }
                    }
                }
#ifdef PETARI_EDITOR_UI
                // PETARI_EDITOR_TEST_SAVE=1: open the Save sheet, then press Save (the mod goes to this --user folder).
                if (std::getenv("PETARI_EDITOR_TEST_SAVE") != nullptr) {
                    if (t == 147) PetariEditor::Ui::openSaveSheet();
                    if (t == 164) {
                        std::fputs("[editor-test] save: press Save\n", stderr);
                        PetariEditor::Ui::submitSaveSheet();
                    }
                }
#endif
                // PETARI_EDITOR_TEST_CHECK=1: give the selection two problems for the Checks tab (a switch out of
                // range, a path that doesn't exist).
                if (t == 145 && std::getenv("PETARI_EDITOR_TEST_CHECK") != nullptr) {
                    const std::vector<Pick::Item> selected = Pick::selection();
                    if (!selected.empty() && selected.front().row != 0) {
                        PetariEditor::Command broken{PetariEditor::Command::Kind::Apply};
                        broken.op.label = "Test: break the selection";
                        PetariEditor::Cell sw, path;
                        sw.i = 500;
                        path.i = 77;
                        broken.op.sets.push_back({PetariEditor::RowId{selected.front().row}, PetariEditor::columnHash("SW_APPEAR"), sw});
                        broken.op.sets.push_back({PetariEditor::RowId{selected.front().row}, PetariEditor::columnHash("CommonPath_ID"), path});
                        PetariEditor::enqueue(std::move(broken));
                        std::fputs("[editor-test] check: SW_APPEAR 500, CommonPath_ID 77\n", stderr);
                    }
                }
                if (t == 160) {
                    const PetariEditor::ModelView* model = PetariEditor::model();
                    std::fprintf(stderr, "[editor-test] gizmo: history %d (%s, %s)\n", model != nullptr ? model->historySize() : -1,
                                 model != nullptr && model->historySize() > 0 ? model->historyLabel(0) : "-",
                                 model != nullptr && model->historySize() > 1 ? model->historyLabel(1) : "-");
                }
                if (t == 250) {
                    std::fputs("[editor-test] LevelEditor tap (back to Edit)\n", stderr);
                    Editor::toggle();
                }
                if (t == 330) {
                    std::fputs("[editor-test] LevelEditor tap (leave)\n", stderr);
                    Editor::toggle();
                    editorStep = 400;
                }
            } else {
                const long t = ++editorStep;
                // The view's centre, except while the SDL pick test lets the UI's mouse drive the cursor.
                if (t < 196 || std::getenv("PETARI_EDITOR_TEST_SDL_PICK") == nullptr) Editor::cursor(0.0f, 0.0f);
                if (t == 2) Editor::repivot();
                if (t >= 2 && t < 62) Editor::orbit(1.5f, 0.0f);    // 90 degrees about the pivot
                if (t >= 62 && t < 92) Editor::orbit(0.0f, -0.5f);  // and 15 degrees higher (drag down: the camera rises)
                if (t >= 100 && t < 120) Editor::dolly(0.15f);      // closer
                // Hover the centre (the ground the zoom went toward), click it, frame the selection.
                if (t >= 120 && t < (std::getenv("PETARI_EDITOR_TEST_SDL_PICK") != nullptr ? 196 : 250)) PetariNative::EditorPick::setHoverWanted(true);
                if (t == 125) PetariNative::EditorPick::click(PetariNative::EditorPick::Click::Replace);
                if (t == 140) Editor::frame();
                if (t >= 170 && t < 190) Editor::pan(0.01f, 0.0f);  // a fifth of the view sideways
#ifdef PETARI_EDITOR_UI
                // PETARI_EDITOR_TEST_SDL_CLICK=1 (EP-001): real SDL clicks with no motion before them, through Aurora and
                // ImGui's SDL backend, on the Browser tab, then on the Save sheet's Cancel. The log shows "[editor] tab
                // Browser" and "[editor] save sheet cancelled" when they land.
                static const bool sSdlClick = std::getenv("PETARI_EDITOR_TEST_SDL_CLICK") != nullptr;
                if (sSdlClick) {
                    auto click = [](const char* target, bool down) {
                        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
                        SDL_Window* window = SDL_GetWindows(nullptr) != nullptr ? SDL_GetWindows(nullptr)[0] : nullptr;
                        if (std::strcmp(target, "viewport") == 0) {
                            PetariEditor::Ui::viewportRect(&x, &y, &w, &h);
                            // the centre: the driver framed the ground there
                        } else if (!PetariEditor::Ui::testTargetRect(target, &x, &y, &w, &h) || window == nullptr) {
                            std::fprintf(stderr, "[editor-test] sdl click: no %s\n", target);
                            return;
                        }
                        SDL_Event event{};
                        event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
                        event.button.windowID = SDL_GetWindowID(window);
                        event.button.which = App::SmokeBackground::kTestMouseId;
                        event.button.button = SDL_BUTTON_LEFT;
                        event.button.down = down;
                        event.button.clicks = 1;
                        event.button.x = x + w * 0.5f;
                        event.button.y = y + h * 0.5f;
                        const bool pushed = SDL_PushEvent(&event);
                        std::fprintf(stderr, "[editor-test] sdl click %s %s at (%.0f, %.0f) display scale %.1f: %s\n", target, down ? "down" : "up",
                                     event.button.x, event.button.y, SDL_GetWindowDisplayScale(window), pushed ? "pushed" : SDL_GetError());
                    };
                    // PETARI_EDITOR_TEST_SDL_MOTION=1: a motion event to the target first, as a real mouse sends.
                    if (t == 148 && std::getenv("PETARI_EDITOR_TEST_SDL_MOTION") != nullptr) {
                        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
                        SDL_Window* window = SDL_GetWindows(nullptr) != nullptr ? SDL_GetWindows(nullptr)[0] : nullptr;
                        if (window != nullptr && PetariEditor::Ui::testTargetRect("tab:Browser", &x, &y, &w, &h)) {
                            SDL_Event motion{};
                            motion.type = SDL_EVENT_MOUSE_MOTION;
                            motion.motion.windowID = SDL_GetWindowID(window);
                            motion.motion.which = App::SmokeBackground::kTestMouseId;
                            motion.motion.x = x + w * 0.5f;
                            motion.motion.y = y + h * 0.5f;
                            SDL_PushEvent(&motion);
                            std::fputs("[editor-test] sdl motion to tab:Browser\n", stderr);
                        }
                    }
                    if (t == 150) click("tab:Browser", true);
                    if (t == 152) click("tab:Browser", false);
                    if (t == 160) PetariEditor::Ui::openSaveSheet();
                    if (t == 175) click("button:Cancel", true);
                    if (t == 177) click("button:Cancel", false);
                    const bool helpTest = std::getenv("PETARI_EDITOR_TEST_HELP") != nullptr;
                    if (t == 185 && !helpTest) click("button:View", true);
                    if (t == 187 && !helpTest) click("button:View", false);
                    // Card-to-arm (friction #13): one click on a Browser card, one on the ground, and it's placed.
                    if (helpTest) {
                        if (t == 205) click("button:Help", true);
                        if (t == 207) click("button:Help", false);
                        if (t == 215) {
                            std::fprintf(stderr, "[editor-help] toolbar opens overlay: %s\n", PetariEditor::Ui::helpVisible() ? "PASS" : "FAIL");
                            PetariNative::EfbDump::mark("editor-help");
                        }
                        if (t == 240) PetariEditor::Ui::toggleHelp();
                        if (t == 245) {
                            std::fprintf(stderr, "[editor-help] closes overlay: %s\n", !PetariEditor::Ui::helpVisible() ? "PASS" : "FAIL");
                            PetariNative::EfbDump::mark("editor-help-toolbar");
                        }
                    } else {
                        if (t == 205) click("tab:Browser", true);
                        if (t == 207) click("tab:Browser", false);
                        if (t == 215) click("card:first", true);
                        if (t == 217) click("card:first", false);
                        if (t == 225) click("viewport", true);
                        if (t == 227) click("viewport", false);
                    }
                }
#endif
#ifdef PETARI_EDITOR_UI
                // PETARI_EDITOR_TEST_SDL_PICK=1 (EP-001 follow-up): find a small object the game draws off the view's centre
                // (trying the galaxy's objinfo rows in turn), note where, clear, then click that spot with a real SDL click;
                // the same object must come back.
                static const bool sSdlPick = std::getenv("PETARI_EDITOR_TEST_SDL_PICK") != nullptr;
                if (sSdlPick && t >= 196 && t < 600) {
                    namespace Pick = PetariNative::EditorPick;
                    static std::vector<std::uint64_t> sCandidates;
                    static size_t sNext = 0;
                    static Pick::Item sTarget;
                    static long sClickAt = 0;
                    static long sVerify = 0;
                    const PetariEditor::ModelView* model = PetariEditor::model();
                    if (t == 196 && model != nullptr) {
                        for (int zone = 0; zone < model->zoneCount(); zone++) {
                            for (int table = 0; table < model->tableCount(zone); table++) {
                                if (std::strstr(model->tablePath(zone, table), "/objinfo") == nullptr) continue;
                                model->forEachRow(zone, table, [](PetariEditor::RowId row) { sCandidates.push_back(row.v); });
                            }
                        }
                        std::fprintf(stderr, "[editor-test] sdl pick: %zu candidates\n", sCandidates.size());
                    }
                    // A candidate is used only if the game's cursor placed on it hovers it (not hidden behind terrain).
                    if (sVerify != 0 && t >= sVerify) {
                        if (t < sVerify + 4) {
                            Editor::cursor(sTarget.screenX, sTarget.screenY);
                            Pick::setHoverWanted(true);
                        } else {
                            Pick::Item hover;
                            const bool hit = Pick::hover(&hover) && hover.row == sTarget.row;
                            Pick::setHoverWanted(false);
                            if (hit) sClickAt = t + 2;
                            sVerify = 0;
                        }
                    }
                    if (sClickAt == 0 && sVerify == 0 && t > 196 && sNext < sCandidates.size()) {
                        if (t % 2 == 1) {
                            Pick::selectRow(sCandidates[sNext], Pick::Click::Replace);
                        } else {
                            const std::vector<Pick::Item> selected = Pick::selection();
                            if (!selected.empty() && selected.front().row == sCandidates[sNext] && selected.front().onScreen &&
                                std::fabs(selected.front().screenX) > 0.2f && std::fabs(selected.front().screenX) < 0.85f &&
                                std::fabs(selected.front().screenY) < 0.85f && selected.front().screenRadius < 0.08f) {
                                sTarget = selected.front();
                                sVerify = t + 20;  // then: does the game's own cursor there hover it (visible, pickable), once settled?
                                Pick::clearSelection();
                            }
                            sNext++;
                        }
                    }
                    SDL_Window* window = SDL_GetWindows(nullptr) != nullptr ? SDL_GetWindows(nullptr)[0] : nullptr;
                    if (sClickAt != 0 && window != nullptr && (t == sClickAt || t == sClickAt + 2)) {
                        float vx = 0.0f, vy = 0.0f, vw = 0.0f, vh = 0.0f;
                        PetariEditor::Ui::viewportRect(&vx, &vy, &vw, &vh);
                        SDL_Event event{};
                        event.type = t == sClickAt ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
                        event.button.windowID = SDL_GetWindowID(window);
                        event.button.which = App::SmokeBackground::kTestMouseId;
                        event.button.button = SDL_BUTTON_LEFT;
                        event.button.down = t == sClickAt;
                        event.button.clicks = 1;
                        event.button.x = vx + (sTarget.screenX + 1.0f) * 0.5f * vw;
                        event.button.y = vy + (1.0f - sTarget.screenY) * 0.5f * vh;
                        SDL_PushEvent(&event);
                        if (t == sClickAt) {
                            std::fprintf(stderr, "[editor-test] sdl pick: %s drawn at viewport (%.2f, %.2f) r %.3f -> window (%.0f, %.0f) of viewport %.0fx%.0f\n",
                                         sTarget.name, sTarget.screenX, sTarget.screenY, sTarget.screenRadius, event.button.x, event.button.y, vw, vh);
                        }
                    }
                    if (sClickAt != 0 && t == sClickAt + 14) {
                        const std::vector<Pick::Item> selected = Pick::selection();
                        const bool same = !selected.empty() && selected.front().row == sTarget.row;
                        std::fprintf(stderr, "[editor-test] sdl pick: clicked %s, selected %s: %s\n", sTarget.name,
                                     !selected.empty() ? selected.front().name : "(nothing)", same ? "PASS" : "FAIL");
                        sClickAt = -1;  // done
                    }
                    if (t == 599 && sClickAt == 0) std::fputs("[editor-test] sdl pick: no off-centre object on screen\n", stderr);
                }
#endif
#ifdef PETARI_EDITOR_UI
                // PETARI_EDITOR_TEST_NEWGALAXY=1: open the New Galaxy sheet (not Create: that restarts the app).
                if (t == 196 && std::getenv("PETARI_EDITOR_TEST_NEWGALAXY") != nullptr) PetariEditor::Ui::openNewGalaxySheetForTest();
                // PETARI_EDITOR_TEST_SHARE=1 (UX section 17): save the mod, export it with the Share sheet into
                // PETARI_EDITOR_SHARE_DIR, then install that zip through the Install sheet (it is installed already,
                // so the second press replaces it). The log shows each step.
                if (std::getenv("PETARI_EDITOR_TEST_SHARE") != nullptr) {
                    if (t == 196) PetariEditor::Ui::openSaveSheet();
                    if (t == 200) PetariEditor::Ui::submitSaveSheet();
                    if (t == 206) PetariEditor::Ui::openShareSheetForTest();
                    if (t == 212) PetariEditor::Ui::submitShareSheet();
                    if (t == 222) {
                        std::string zip;
                        std::error_code ec;
                        if (const char* dir = std::getenv("PETARI_EDITOR_SHARE_DIR")) {
                            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                                if (entry.path().extension() == ".zip") zip = entry.path().string();
                            }
                        }
                        std::fprintf(stderr, "[editor-test] share: zip %s\n", zip.empty() ? "(none)" : zip.c_str());
                        if (!zip.empty()) PetariEditor::Ui::openInstallSheet(zip.c_str());
                    }
                    if (t == 228) PetariEditor::Ui::submitInstallSheet();
                    if (t == 234) PetariEditor::Ui::submitInstallSheet();  // "Replace it"
                }
#endif
                // PETARI_EDITOR_TEST_MISSION=<n>: switch to mission n as the toolbar's chip does, then leave later.
                static const int sMission = std::getenv("PETARI_EDITOR_TEST_MISSION") != nullptr ? std::atoi(std::getenv("PETARI_EDITOR_TEST_MISSION")) : 0;
#ifdef PETARI_EDITOR_UI
                if (t == 195 && sMission > 0) {
                    std::fprintf(stderr, "[editor-test] switch to mission %d\n", sMission);
                    PetariEditor::Ui::switchMission(sMission);
                }
#endif
                if (t == (sMission > 0 ? 420 : std::getenv("PETARI_EDITOR_TEST_SDL_PICK") != nullptr ? 610 : 250)) {
                    PetariNative::EditorPick::setHoverWanted(false);
                    std::fputs("[editor-test] LevelEditor tap (leave)\n", stderr);
                    Editor::toggle();
                    editorStep = 400;
                }
            }
        }
    }
#endif
    static long gameplayFrames = 0;
    if (period > 1 && gameplay) {
        ++gameplayFrames;
        if (gameplayFrames % period == 0) {
            PetariNative::Input::injectActionPress(PetariNative::Input::Action::ModCollectStarBits);
            std::fprintf(stderr, "[mods-test] collect press at gameplay frame %ld\n", gameplayFrames);
        } else if (gameplayFrames % period == period / 2) {
            PetariNative::Input::injectActionPress(PetariNative::Input::Action::ModShootEnemy);
            std::fprintf(stderr, "[mods-test] shoot press at gameplay frame %ld\n", gameplayFrames);
        }
    }
#ifdef PETARI_EDITOR_UI
    App::editorM4Smoke(gameplay);  // M4: opt-in SDL regression driver.
#endif
    return phase;
}

}  // namespace

int main(int argc, char** argv) {
    PetariNative::GpuTrace::configure(std::getenv("PETARI_PIPELINE_DIAG"), std::getenv("PETARI_GPU_TRACE"),
        std::getenv("PETARI_GPU_TRACE_BINDING_SHA256"));
    App::AppLocations locations;
    App::Paths paths;
    try {
        locations = App::AppLocations::fromEnvironment();
        App::redirectAppLog(locations);
        // Before SDL starts AppKit; Finder logs also include startup diagnostics.
        disableWindowRestoration();
        // Build the allocation-site symbol table before any game thread runs.
        PetariNative::prepareAllocationSymbols();
        if (!resolvePaths(argc, argv, &paths, locations)) {
            if (setupQuit) return 0;
            usage();
            return 2;
        }
    } catch (const std::exception& exception) {
        App::FirstRun::showProblem(exception.what());
        return 1;
    }

    std::string error;
    App::SmokeBackground::enabled = App::SmokeBackground::requested(
        std::getenv("PETARI_SMOKE"), PetariNative::TestFixture::observatory,
        std::getenv("PETARI_SMOKE_BACKGROUND"));
    App::SmokeStorage smokeStorage;
    if (App::SmokeBackground::enabled && !smokeStorage.prepare(paths.user, std::getenv("PETARI_CACHE_DIR"), &error)) {
        std::fprintf(stderr, "petari: background storage: %s\n", error.c_str());
        return 1;
    }
    // mods.txt before the disc is mounted: the level editor's switch decides how patch mods are built.
    if (!PetariNative::Mods::load(paths.user / "mods.txt", &error)) {
        std::fprintf(stderr, "petari: mods: %s\n", error.c_str());
        return 1;
    }
#ifdef PETARI_EDITOR_UI
    // Terrain brushes in patch mods (docs/dev/level-editor/TECH.md 6.5): the mod loader bakes them into
    // planets, unless the level editor is on, which keeps them as rows to edit (shown live; the save
    // bakes or patches them).
    {
        const char* editorEnv = std::getenv("PETARI_EDITOR");
        if (!PetariNative::Mods::enabled(PetariNative::Mods::Mod::LevelEditor) && !(editorEnv != nullptr && editorEnv[0] == '1')) {
            PetariEditor::Terrain::registerPatchBuilder();
            PetariEditor::Imports::registerPatchBuilder();  // M6 import: neutral assets become native planets.
        }
    }
#endif
    if (!App::Host::preparePlatform(paths, &error)) {
        std::fprintf(stderr, "petari: %s\n", error.c_str());
        return 1;
    }
    // controls.txt: never fatal. An invalid file leaves the default controls in use.
    if (!App::Events::loadControls(paths.user / "controls.txt", &error) || !error.empty()) {
        std::fprintf(stderr, "petari: controls: %s\n", error.c_str());
    }
    error.clear();
    // The personal progress record (native/PROGRESS.md): beside the save, never inside it. A damaged
    // file is reported and left alone; progress is then not recorded this run.
    if (!PetariNative::Progress::load(paths.user / "progress.json", &error)) {
        std::fprintf(stderr, "petari: progress: %s (not recording progress this run)\n", error.c_str());
    }
    if (!PetariNative::CameraSettings::load(paths.user / "camera.txt", &error)) {
        std::fprintf(stderr, "petari: camera: %s\n", error.c_str());
        return 1;
    }
    if (PetariNative::CameraSettings::enabled()) std::fputs("PETARI MODS: OdysseyCamera on\n", stderr);
    for (int m = 0; m < static_cast<int>(PetariNative::Mods::Mod::Count); ++m) {
        const auto mod = static_cast<PetariNative::Mods::Mod>(m);
        if (PetariNative::Mods::enabled(mod)) std::fprintf(stderr, "PETARI MODS: %s on\n", PetariNative::Mods::name(mod));
    }

    SDL_SetHintWithPriority("PETARI_SMOKE_BACKGROUND", App::SmokeBackground::enabled ? "1" : "0", SDL_HINT_OVERRIDE);
    App::SmokeBackground::ignorePhysical = !App::SmokeBackground::enabled &&
        App::SmokeBackground::ignorePhysicalRequested(std::getenv("PETARI_SMOKE"), PetariNative::TestFixture::observatory,
                                                      std::getenv("PETARI_SMOKE_IGNORE_PHYSICAL"));
    if (App::SmokeBackground::ignorePhysical) {
        SDL_SetEventFilter(App::SmokeBackground::eventFilter, nullptr);
        std::fputs("PETARI SMOKE INPUT: physical input ignored (PETARI_SMOKE_IGNORE_PHYSICAL=1); visible focused window\n",
                   stderr);
    }
    if (App::SmokeBackground::enabled) {
        // A muted smoke has no use for a CoreAudio device (coreaudiod sat at ~35% of a core per smoke): SDL's dummy driver
        // keeps the callback timing, so the mixer and the game logic run as before. PETARI_SMOKE_REAL_AUDIO=1 opens the
        // real device (audio timing/choppiness evidence needs it).
        if (const char* real = std::getenv("PETARI_SMOKE_REAL_AUDIO"); real == nullptr || real[0] != '1') {
            SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE);
            std::fputs("PETARI SMOKE AUDIO DRIVER: dummy (PETARI_SMOKE_REAL_AUDIO=1 opens the real device)\n", stderr);
        }
        SDL_SetHintWithPriority(SDL_HINT_MAC_BACKGROUND_APP, "1", SDL_HINT_OVERRIDE);
        SDL_SetHintWithPriority(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0", SDL_HINT_OVERRIDE);
        SDL_SetHintWithPriority(SDL_HINT_WINDOW_ACTIVATE_WHEN_RAISED, "0", SDL_HINT_OVERRIDE);
        SDL_SetEventFilter(App::SmokeBackground::eventFilter, nullptr);
        // Let SDL install its Cocoa application/delegate first, before creating
        // any window. Accessory policy avoids normal foreground application activation.
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            std::fprintf(stderr, "petari: background video initialization: %s\n", SDL_GetError());
            return 1;
        }
        const auto sendObject = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
        const id application = sendObject(reinterpret_cast<id>(objc_getClass("NSApplication")),
                                          sel_registerName("sharedApplication"));
        constexpr long backgroundActivation = 1; // NSApplicationActivationPolicyAccessory
        const auto setPolicy = reinterpret_cast<BOOL (*)(id, SEL, long)>(objc_msgSend);
        if (!application || !setPolicy(application, sel_registerName("setActivationPolicy:"), backgroundActivation)) {
            std::fputs("petari: could not set background app activation policy\n", stderr);
            return 1;
        }
        // Opt-in (PETARI_SMOKE_QOS=1): declare the hidden process user-initiated and latency
        // critical (no App Nap) and raise this thread's QoS (later threads inherit it). The A/B
        // on EggStarGalaxy s1 (qos-on-v2 / qos-off-v2) showed no benefit once machine load was
        // controlled, so it is not the default.
        if (const char* qosEnv = std::getenv("PETARI_SMOKE_QOS"); !qosEnv || qosEnv[0] != '1') {
            std::fputs("PETARI SMOKE QOS: default (PETARI_SMOKE_QOS=1 enables activity + QoS boost)\n", stderr);
        } else {
            const auto sendClass = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
            const auto makeString = reinterpret_cast<id (*)(id, SEL, const char*)>(objc_msgSend);
            const auto beginActivity = reinterpret_cast<id (*)(id, SEL, unsigned long long, id)>(objc_msgSend);
            const id processInfo = sendClass(reinterpret_cast<id>(objc_getClass("NSProcessInfo")), sel_registerName("processInfo"));
            const id reason = makeString(reinterpret_cast<id>(objc_getClass("NSString")), sel_registerName("stringWithUTF8String:"),
                                         "Petari background smoke test");
            constexpr unsigned long long userInitiated = 0x00FFFFFFULL, latencyCritical = 0xFF00000000ULL;
            id activity = processInfo ? beginActivity(processInfo, sel_registerName("beginActivityWithOptions:reason:"),
                                                      userInitiated | latencyCritical, reason) : nullptr;
            if (activity) sendClass(activity, sel_registerName("retain")); // held until process exit
            const int qos = pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
            std::fprintf(stderr, "PETARI SMOKE QOS: activity=%s thread_qos=%s\n", activity ? "user-initiated+latency-critical" : "unavailable",
                         qos == 0 ? "user-interactive" : "unavailable");
        }
        std::fputs("PETARI SMOKE BACKGROUND: enabled; physical input isolated; nonfocusable background window; VI pacing unchanged\n", stderr);
    }

    const std::string userPath = paths.user.string();
    AuroraConfig config{};
    config.appName = "Astral Italian";  // the window title (Aurora uses it for the SDL window)
    config.userPath = userPath.c_str();
    App::PipelineCacheDir pipelineCache;
    if (!App::SmokeBackground::enabled) {
        const char* smoke = std::getenv("PETARI_SMOKE");
        std::error_code markerError;
        const bool testRun = PetariNative::TestFixture::observatory || (smoke && smoke[0] && smoke[0] != '0') ||
            !PetariNative::UnlockedSave::variant.empty() ||
            std::filesystem::exists(paths.user / ".petari-test-fixture", markerError);
        pipelineCache.prepare(paths.user, testRun, std::getenv("PETARI_CACHE_DIR"));
    }
    const std::string cachePath = App::SmokeBackground::enabled ? smokeStorage.cache.string()
                                                                : pipelineCache.path.string();
    config.cachePath = cachePath.c_str();
    config.desiredBackend = BACKEND_METAL;
    // Keep VI/display-link pacing unchanged; VSync changes apply at launch only.
    if (!App::DisplaySettings::controller().configure(paths.user / "display.txt", &error)) {
        std::fprintf(stderr, "petari: %s\n", error.c_str());
    }
    const auto display = App::DisplaySettings::controller().settings();
    const auto uberMode = PetariNative::Ubershaders::resolve(std::getenv("PETARI_UBERSHADER"),
        display.ubershadersSaved ? std::optional<bool>(display.ubershaders) : std::nullopt);
    // Resolve before Aurora initializes its device, uniform buffers or workers.
    // Never replace an explicit environment choice, including zero.
    if (setenv("PETARI_UBERSHADER", PetariNative::Ubershaders::environmentValue(uberMode.mode), 0) != 0) {
        std::fputs("petari: could not configure ubershaders at launch\n", stderr);
        return 1;
    }
    std::fprintf(stderr, "[gx uber] mode=%s source=%s\n", PetariNative::Ubershaders::name(uberMode.mode), uberMode.source);
    // VSync Auto (default): Fifo in a window, Immediate in fullscreen; DisplayWindow::update switches it live afterwards.
    config.vsync = display.vsyncAuto ? !display.fullscreen : display.vsync;
    std::fprintf(stderr, "[display] vsync %s: starting %s\n", display.vsyncAuto ? "auto" : display.vsync ? "on" : "off",
                 config.vsync ? "with VSync (Fifo)" : "without VSync (Immediate)");
    App::DisplayWindow::setStartupVsync(config.vsync);
    config.startFullscreen = display.fullscreen;
    // The game keeps running without focus, as on the console. Input releases
    // held keys on focus loss.
    config.pauseOnFocusLost = false;
    config.windowWidth = PetariNative::HomeMenu::kDisplaySizes[display.size].width;
    config.windowHeight = PetariNative::HomeMenu::kDisplaySizes[display.size].height;
    config.logCallback = logMessage;
    config.logLevel = LOG_INFO;
    PetariBootSplash::deferWindowShow = !App::SmokeBackground::enabled;
    const AuroraInfo info = aurora_initialize(1, argv, &config);
    if (info.window == nullptr || aurora_get_backend() != BACKEND_METAL) {
        std::fputs("petari: Metal initialization failed\n", stderr);
        aurora_shutdown();
        return 1;
    }
    // Aurora has just given this thread (the main game thread) an explicit SDL policy; see petari/thread_priority.hpp.
    PetariNative::ThreadPriority::raiseToSchedOtherMax("main thread");
    PetariBootSplash::startupEvent("app_menu_setup_enter");
    App::FirstRun::installMenu(locations);
    PetariBootSplash::startupEvent("app_menu_setup_exit");
    PetariNative::HomeMenu::setPreparationProgressSource(&petari_gx_pipeline_prewarm_progress);
    if (App::SmokeBackground::enabled) {
        auto* window = static_cast<SDL_Window*>(info.window);
        if (!SDL_SetWindowFocusable(window, false)) {
            std::fprintf(stderr, "petari: could not isolate background window focus: %s\n", SDL_GetError());
            aurora_shutdown();
            return 1;
        }
        if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN)) {
            std::fputs("petari: background window unexpectedly visible\n", stderr);
            aurora_shutdown();
            return 1;
        }
        int x = 0, y = 0;
        SDL_GetWindowPosition(window, &x, &y);
        std::fprintf(stderr, "PETARI SMOKE BACKGROUND: accessory activation policy; hidden window at %d,%d; flags=0x%llx\n",
                     x, y, static_cast<unsigned long long>(SDL_GetWindowFlags(window)));
        App::Events::assertFocus();
    }
    // The native menus draw with the game's own fonts from the disc; without them
    // (a disc without LayoutData/Font.arc) they keep ImGui's built-in font.
    {
        std::string fontError;
        if (!PetariNative::HomeMenu::installGalaxyFonts((paths.disc / "files/LayoutData/Font.arc").string().c_str(), &fontError)) {
            std::fprintf(stderr, "petari: menus use the built-in font: %s\n", fontError.c_str());
        }
    }
#ifdef PETARI_EDITOR_UI
    // The level editor's data font: macOS's own UI font, read from the system (not shipped).
    if (!PetariEditor::Ui::installFonts(SDL_GetWindowDisplayScale(static_cast<SDL_Window*>(info.window)))) {
        std::fprintf(stderr, "petari: editor panels use the built-in font (no system UI font)\n");
    }
    PetariEditor::Ui::setSettingsFile((paths.user / "editor" / "ui-settings.txt").string().c_str());
    PetariEditor::Ui::setThumbnailHost([](const void* copy, unsigned char* rgba, unsigned capacity, unsigned* width,
                                          unsigned* height) { return petari_present_take_readback(copy, rgba, capacity, width, height); },
                                       [](unsigned width, unsigned height, const void* rgba) {
                                           return static_cast< unsigned long long >(aurora_imgui_add_texture(width, height, rgba));
                                       },
                                       &aurora_imgui_update_texture);
    // Native file panels for the editor's sheets (Share's save panel, Import's open panel): SDL3's dialogs.
    static SDL_Window* sDialogWindow = static_cast<SDL_Window*>(info.window);
    PetariEditor::Ui::setFileDialogHost([](bool save, const char* defaultPath, const char* filterName, const char* filterPattern,
                                           PetariEditor::Ui::FileDialogDone done, void* user) {
        struct Pending {
            PetariEditor::Ui::FileDialogDone done;
            void* user;
            std::string name, pattern;
            SDL_DialogFileFilter filter;
        };
        auto* pending = new Pending{done, user, filterName != nullptr ? filterName : "", filterPattern != nullptr ? filterPattern : "", {}};
        pending->filter = {pending->name.c_str(), pending->pattern.c_str()};
        auto callback = [](void* data, const char* const* files, int) {
            auto* p = static_cast<Pending*>(data);
            p->done(p->user, files != nullptr && files[0] != nullptr ? files[0] : "");
            delete p;
        };
        const bool filtered = filterPattern != nullptr && filterPattern[0] != '\0';
        if (save) {
            SDL_ShowSaveFileDialog(callback, pending, sDialogWindow, filtered ? &pending->filter : nullptr, filtered ? 1 : 0, defaultPath);
        } else {
            SDL_ShowOpenFileDialog(callback, pending, sDialogWindow, filtered ? &pending->filter : nullptr, filtered ? 1 : 0, defaultPath, false);
        }
    });
    // Development: the editor's docked layout from the start (the Edit mode shows it normally).
    if (const char* dock = std::getenv("PETARI_EDITOR_DOCK"); dock != nullptr && dock[0] == '1') {
        PetariEditor::Ui::setVisible(true);
    }
#endif
    App::LatencyReport::configureLayer(info.window);
    App::DisplayLink::start(info.window);  // the VI clock (default; PETARI_VI_CLOCK=timer opts out)
    App::Seam::attach(info.window);
    App::TapeScene::install();
    if (!App::prepareFrontend(static_cast<SDL_Window*>(info.window))) {
        App::Seam::reportFrameStats();
        aurora_shutdown();
        return 0;
    }
    if (!prepareKnownPipelines(static_cast<SDL_Window*>(info.window))) {
        aurora_shutdown();
        return 0;
    }
    // Default (unset or PETARI_PIPELINE_GLOBAL_PRECOMPILE=background): queue the global seed
    // at background priority so it compiles while the game plays. A no-op for 0 or 1.
    petari_gx_pipeline_background_begin();
    // The EFB capture compute pipeline would otherwise compile on the first capture
    // (file-select entry) and stall it by up to ~0.5 s on a cold Metal cache.
    petari_efb_snapshot_prewarm();
    // Audio opens on a game thread when the game starts AI DMA; initialize
    // SDL's audio subsystem here, on the main thread, first. The sink's own
    // initialization then only adds a reference.
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "petari: SDL audio: %s\n", SDL_GetError());
    }

    App::Seam::attach(info.window);
    App::Seam::setPhaseProbe(framePhaseForPipelines);
    App::LatencyReport::startTestDriver();
    App::Host::startOS();
    App::Seam::openFirstFrame();
    App::Host::runGame();
}
