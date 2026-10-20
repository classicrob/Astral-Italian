#include "launch_stage.hpp"
#include "launch_stage_messages.hpp"

#include "Game/Scene/GameScene.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <petari/host_allocation.hpp>

#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstring>
#include <limits>
#include <set>
#include <utility>

namespace PetariNative::App::LaunchStage {
namespace {
EnglishNames sEnglish;
bool sConfigured = false;
bool sPrintList = false;

bool fail(std::string* error, const std::string& message) {
    if (error) *error = message;
    failStartupRequest(message);
    return false;
}

std::string optionalString(const ScenarioData& data, const char* key, int number) {
    const char* text = nullptr;
    return data.getValueString(key, number, &text) && text ? text : "";
}

bool dispatch(const Request& request, int* placedScenario, std::string* error) {
    // Mirror ScenarioSelectScene::trySetCurrentScenarioNo: selected nonnormal
    // stars can be placed in another scenario. Never assume row-count == ID.
    int placed = request.scenario;
    {
        HostAllocationScope host;
        const GalaxyStatusAccessor status = MR::makeGalaxyStatusAccessor(request.stage.c_str());
        if (!status.mScenarioData) return fail(error, "stage disappeared from ScenarioDataParser");
        if (status.isNotNormalStar(request.scenario)) {
            // The canonical placement helper uses a u32 star mask with (1 << (id-1)).
            if (request.scenario > 32) return fail(error, "nonnormal star ID exceeds the engine's 32-bit placement mask");
            placed = MR::getPlacedHiddenStarScenarioNo(request.stage.c_str(), request.scenario);
            if (!scenario(request.stage, placed) || !status.isExistPowerStarAtScenario(placed, request.scenario))
                return fail(error, "no verified placed-scenario membership for the requested hidden/green star");
        }
    }
    // Engine transition allocations retain their normal JKR routing.
    // Also puts the in-game sequence into InStage (a bare requestChangeStage left it in its Starship state).
    GameSequenceFunction::requestChangeStageInStageScenario(request.stage.c_str(), placed, request.scenario);
    *placedScenario = placed;
    return true;
}
}  // namespace

// Test-only driver, shared by GX and headless. This seam runs on the game
// thread before its host-allocation scope; transition allocations keep normal
// game routing. Nothing is forced except the canonical scene-change request.
void testReturnToShipFrame() {
    struct Settings {
        unsigned long frames = 0;
        int startIndex = 0;
    };
    static const Settings settings = [] {
        Settings value;
        const char* text = std::getenv("PETARI_TEST_RETURN_TO_SHIP");
        if (!text || !*text) return value;
        char* end = nullptr;
        errno = 0;
        if (*text < '0' || *text > '9') end = const_cast<char*>(text);
        else value.frames = std::strtoul(text, &end, 10);
        bool valid = !errno && value.frames > 0 && end != text;
        if (valid && *end == ':') {
            const char* index = end + 1;
            errno = 0;
            const long parsed = std::strtol(index, &end, 10);
            valid = *index >= '0' && *index <= '9' && end != index && !errno && parsed <= INT_MAX;
            if (valid) value.startIndex = static_cast<int>(parsed);
        }
        if (!valid || *end) {
            std::fputs("[return-to-ship-test] invalid PETARI_TEST_RETURN_TO_SHIP; disabled\n", stderr);
            value.frames = 0;
        }
        return value;
    }();
    static bool fired = false;
    static unsigned long steadyFrames = 0;
    if (!settings.frames || fired) return;
    // Only the explicitly launched target counts, never FileSelect or the ship
    // visited during boot. Placement can differ from the selected star number.
    const Request* target = resolvedStartupRequest();
    const auto* system = SingletonHolder<GameSystem>::get();
    const auto* controller = system ? system->mSceneController : nullptr;
    bool ready = startupStatus() == StartupStatus::Applied && target && controller &&
        controller->isSceneInitializeState(SceneInitializeState_End) && controller->mScene &&
        std::strcmp(controller->mCurrSceneControlInfo.mScene, "Game") == 0 &&
        std::strcmp(controller->mCurrSceneControlInfo.mStage, target->stage.c_str()) == 0 &&
        controller->getCurrentScenarioNo() == startupPlacedScenario() && !controller->isExistRequest();
    if (ready) {
        const auto* scene = static_cast<const GameScene*>(controller->mScene);
        ready = scene->isExecAction() && scene->permitsStartPauseMenu() && !MR::isDemoActive();
    }
    if (!ready) { steadyFrames = 0; return; }
    if (++steadyFrames != settings.frames) return;
    fired = true;
    // The game's first argument is a spawn/start index, despite older test
    // recipes calling this optional field a scenario. -1 selects ship scenario2.
    GameSequenceFunction::requestChangeStageMarioFaceShip(settings.startIndex, -1);
    std::fprintf(stderr, "[return-to-ship-test] fired stage=%s scenario=%d steady_frames=%lu start_index=%d request=GameSequenceFunction::requestChangeStageMarioFaceShip(%d,-1)\n",
        target->stage.c_str(), startupPlacedScenario(), steadyFrames, settings.startIndex, settings.startIndex);
}

bool configureCatalog(const std::filesystem::path& disc, std::string* error) {
    HostAllocationScope host;
    EnglishNames names;
    if (!loadEnglishGalaxyNames(disc, &names, error)) {
        if (error) failStartupRequest(*error);
        return false;
    }
    sEnglish.swap(names);
    sConfigured = true;
    if (error) error->clear();
    return true;
}

bool catalogConfigured() { return sConfigured; }
void requestCatalogListing() { sPrintList = true; }
bool catalogListingRequested() { return sPrintList; }

bool onGameDataReady(const ScenarioDataParser& parser, const JMapInfo& worldOrder, std::string* error) {
    HostAllocationScope host;
    if (!sConfigured) return true;  // No catalog consumer was requested by this frontend.
    if (!worldOrder.dataExists()) return fail(error, "SMG2 GalaxyWorldOrderList is not loaded");
    std::vector<CatalogGalaxy> records;
    std::set<std::string> stages;
    for (int row = 0; row < worldOrder.getNumEntries(); ++row) {
        const char* stage = nullptr;
        u32 orderedWorld = 0;
        if (!worldOrder.getValue<const char*>(row, "EnName", &stage) || !stage ||
            !worldOrder.getValue<u32>(row, "WorldNo", &orderedWorld))
            return fail(error, "GalaxyWorldOrderList lacks EnName/WorldNo");
        // ScenarioDataIter uses WorldNo>0 to identify galaxy records; zero-world
        // non-galaxy records are not fabricated as level-select destinations.
        if (orderedWorld == 0) continue;
        if (!stages.insert(stage).second) return fail(error, "duplicate EnName in GalaxyWorldOrderList");
        const ScenarioData* data = parser.getScenarioData(stage);
        if (!data || !data->mScenarioData || !data->mScenarioData->dataExists() || !data->mGalaxyInfo || !data->mGalaxyInfo->dataExists())
            return fail(error, std::string("missing ScenarioData/GalaxyInfo for ") + stage);
        const GalaxyStatusAccessor status(data);
        u32 dataWorld = 0;
        if (!data->mGalaxyInfo->getValue<u32>(0, "WorldNo", &dataWorld) || dataWorld == 0 ||
            dataWorld > static_cast<u32>(std::numeric_limits<int>::max()))
            return fail(error, std::string("invalid GalaxyInfo.WorldNo for ") + stage);
        const int world = status.getWorldNo();
        if (world <= 0 || orderedWorld > static_cast<u32>(std::numeric_limits<int>::max()) || world != static_cast<int>(orderedWorld))
            return fail(error, std::string("world-number provenance mismatch for ") + stage);
        // Some world-ordered records (world maps such as WorldMap08Galaxy) have no GalaxyName label: they keep
        // their internal name in listings and are still addressable by it.
        const auto english = sEnglish.find(stage);
        const bool hasEnglish = english != sEnglish.end();
        CatalogGalaxy galaxy;
        galaxy.stage = stage;
        galaxy.englishName = hasEnglish ? english->second.text : std::string();
        galaxy.world = world;
        galaxy.provenance = "ObjectData/SystemDataTable.arc/GalaxyWorldOrderList.bcsv:row=" + std::to_string(row) +
                            ";StageData/" + stage + "/" + stage + "Scenario.arc/{GalaxyInfo,ScenarioData}.bcsv;" +
                            (hasEnglish ? english->second.provenance : std::string("no GalaxyName label"));
        for (int index = 0; index < data->mScenarioData->getNumEntries(); ++index) {
            s32 number = 0;
            u32 mask = 0;
            if (!data->mScenarioData->getValue<s32>(index, "ScenarioNo", &number) || number <= 0 ||
                !data->mScenarioData->getValue<u32>(index, "PowerStarId", &mask))
                return fail(error, std::string("invalid ScenarioNo/PowerStarId in ") + stage);
            // Look up optional fields through ScenarioData's actual-number API,
            // never substitute the physical row index for ScenarioNo.
            Scenario s;
            s.number = number;
            s.powerStarMask = mask;
            s.powerStarType = optionalString(*data, "PowerStarType", number);
            const char* comet = status.getCometName(number);
            s.comet = comet ? comet : "";
            s.name = optionalString(*data, "ScenarioName", number);
            galaxy.scenarios.push_back(std::move(s));
        }
        records.push_back(std::move(galaxy));
    }
    if (!installCatalog(std::move(records), error)) { if (error) failStartupRequest(*error); return false; }
    if (startupStatus() == StartupStatus::Failed) return fail(error, startupError());
    if (sPrintList) {
        printList(stdout);
        std::fflush(stdout);
        std::_Exit(0);  // Metadata-only CLI: no user file has been selected/loaded.
    }
    return true;
}

bool onFileLoaded(bool canOverride) {
    const Request* pending = pendingStartupRequest();
    if (!pending) {
        if (startupStatus() == StartupStatus::PendingCatalog) failStartupRequest("file loaded before the SMG2 catalog was published");
        return false;
    }
    if (!canOverride) {
        finishStartupRequest(false, "normal prologue/tutorial destination retained; startup jump refused");
        std::fprintf(stderr, "stage launch: %s\n", startupError().c_str());
        return false;
    }
    const Request& request = *pending;
    if (!GameDataFunction::isOnGameEventFlagWorld1Entered())
        std::fputs("stage launch: explicit request bypasses an unfinished prologue/tutorial; progress flags are unchanged\n", stderr);
    std::string error;
    int placed = 0;
    if (!dispatch(request, &placed, &error)) { failStartupRequest(error); return false; }
    finishStartupRequest(true, {}, placed);
    std::fprintf(stderr, "stage launch: requested %s selected scenario %d through GameSequenceFunction\n", request.stage.c_str(), request.scenario);
    return true;
}
}  // namespace PetariNative::App::LaunchStage
