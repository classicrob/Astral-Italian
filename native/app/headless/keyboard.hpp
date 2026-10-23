#pragma once
#include "replay.hpp"
#include "trace.hpp"
#include <petari/input.hpp>
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>

namespace PetariNative::Headless {
// Real keyboard -> WPAD reports -> SDK KPAD. This is separate from recorded
// KPAD replay: mod buttons such as Cappy are host actions, not Wii buttons.
class KeyboardScript {
public:
    // `playerMode` (sweep): one of bee hopper teresa ice fire yoshi cloud rock foo invincible lifeup. The game's own
    // MR::changePlayerMode* is called once when the script starts, as a power-up pickup would (Mario, not a state poke).
    //
    // Route directives (boot mode, `--boot`: the script starts at the first loop instead of waiting for Mario, and
    // is how a player-style path through title / file select / the ship / the world map is driven):
    //   wait <field>=<value> ... [timeout=<loops>]  hold nothing until the observed state matches (scene stage
    //                                               scenario init_state player_valid); a timeout fails the run
    //   atseen <stage> <offset>                     same, counted from the stage's first observation at any init_state
    //   at <stage|-> <offset> [timeout=<loops>]     hold nothing until <offset> loops after the stage was first ready (init_state 6)
    //                                               ("-" = since the script started); PORT's stage-relative route offsets
    //   viewport <w> <h>                            window size for pointer coordinates (16:9 image, letterboxed)
    //   ptr <x> <y>                                 move the Star Pointer (window coordinates), one loop
    // and the key names MouseLeft / MouseRight in an ordinary `<frames> <keys...>` line.
    explicit KeyboardScript(const std::string& path, const std::string& playerMode = {}, bool noPlayer = false, bool boot = false) : mPlayerMode(playerMode), mNoPlayer(noPlayer), mBoot(boot) {
        if (!playerMode.empty() && !applyPlayerMode(playerMode, false)) throw ReplayError("unknown --player-mode: " + playerMode);
        std::ifstream input(path);
        if (!input) throw ReplayError("cannot open keyboard script");
        std::string text;
        while (std::getline(input, text)) {
            std::istringstream words(text.substr(0, text.find('#')));
            if (!(words >> std::ws).eof()) {
                Line line{};
                std::string first;
                const auto mark = words.tellg();
                words >> first;
                if (first == "wait") {
                    line.kind = Line::Wait;
                    line.frames = 1;
                    std::string cond;
                    while (words >> cond) {
                        const auto eq = cond.find('=');
                        if (eq == std::string::npos || eq == 0) throw ReplayError("bad wait condition: " + cond);
                        if (cond.compare(0, eq, "timeout") == 0) line.timeout = std::stoi(cond.substr(eq + 1));
                        else line.conditions.emplace_back(cond.substr(0, eq), cond.substr(eq + 1));
                    }
                    if (line.conditions.empty()) throw ReplayError("wait needs at least one field=value");
                } else if (first == "at" || first == "atseen") {
                    line.kind = Line::At;
                    line.seen = first == "atseen";
                    line.frames = 1;
                    if (!(words >> line.stage >> line.offset) || line.offset < 0) throw ReplayError("at <stage|-> <offset> [timeout=N]");
                    if (line.stage == "-") line.stage.clear();
                    std::string opt;
                    while (words >> opt) {
                        if (opt.compare(0, 8, "timeout=") != 0) throw ReplayError("bad at option: " + opt);
                        line.timeout = std::stoi(opt.substr(8));
                    }
                } else if (first == "viewport") {
                    line.kind = Line::Viewport;
                    line.frames = 1;
                    if (!(words >> line.x >> line.y) || line.x <= 0 || line.y <= 0) throw ReplayError("viewport <w> <h>");
                } else if (first == "ptr") {
                    line.kind = Line::Pointer;
                    line.frames = 1;
                    if (!(words >> line.x >> line.y)) throw ReplayError("ptr <x> <y>");
                } else {
                    words.clear();
                    words.seekg(mark);
                    if (!(words >> line.frames) || line.frames <= 0 || line.frames > 100000)
                        throw ReplayError("invalid keyboard script duration");
                    std::string key;
                    while (words >> key) {
                        if (key == "MouseLeft") { line.mouse.push_back(Input::MouseButton::Left); continue; }
                        if (key == "MouseRight") { line.mouse.push_back(Input::MouseButton::Right); continue; }
                        Input::KeyCode code;
                        if (!Input::parseKeyName(key, &code)) throw ReplayError("unknown keyboard script key: " + key);
                        line.keys.push_back(code);
                    }
                }
                mLines.push_back(std::move(line));
            }
        }
        if (mLines.empty()) throw ReplayError("empty keyboard script");
    }
    bool started() const { return mStarted; }
    bool step(const State& state) {
        {   // `at` clocks: a stage counts from the first loop it is ready (init_state 6), the instant the GX app's
            // route recorder starts logging it; "-" counts from the script start.
            const auto stage = state.fields.find("stage");
            const auto init = state.fields.find("init_state");
            mFirstSeen.emplace(std::string(), 0);
            mFirstObserved.emplace(std::string(), 0);
            if (stage != state.fields.end() && !stage->second.empty()) mFirstObserved.emplace(stage->second, mLoop);
            if (stage != state.fields.end() && !stage->second.empty() && init != state.fields.end() && init->second == "6")
                mFirstSeen.emplace(stage->second, mLoop);
            ++mLoop;
        }
        if (!mStarted && mBoot) {
            mStarted = true;
            Input::focusChanged(true);
            std::fprintf(stderr, "[input-script] start (headless keyboard, boot route)\n");
        }
        if (!mStarted) {
            const auto is = [&](const char* key, const char* value) {
                auto it = state.fields.find(key);
                return it != state.fields.end() && it->second == value;
            };
            // --no-player (world-map UI stages): ready is an established Game scene; there is no Mario to wait for.
            const bool notReady = mNoPlayer ? !(is("scene", "Game") && is("init_state", "6")) :
                (!(is("scene", "Game") && is("init_state", "6") && is("player_valid", "1")) || MR::isDemoActive() ||
                 !static_cast<const GameScene*>(SingletonHolder<GameSystem>::get()->mSceneController->mScene)->permitsStartPauseMenu());
            if (notReady) {
                mSteady = 0;
                // A player advances an opening demo's dialogue / skips it with A: tap it while a demo runs in the stage.
                if (!mNoPlayer && is("scene", "Game") && is("init_state", "6") && MR::isDemoActive()) {
                    Input::KeyCode a;
                    if (Input::parseKeyName("Space", &a)) {
                        if (mDemoFrames == 0) { Input::focusChanged(true); std::fprintf(stderr, "[input-script] demo active before play-ready: tapping A\n"); }
                        const bool down = (mDemoFrames % 40) < 3;
                        if (down != mDemoA) { Input::keyEvent(a, down, false); mDemoA = down; }
                    }
                    ++mDemoFrames;
                }
                return false;
            }
            if (mDemoA) { Input::KeyCode a; if (Input::parseKeyName("Space", &a)) Input::keyEvent(a, false, false); mDemoA = false; }
            if (++mSteady < 60) return false;
            mStarted = true;
            if (!mPlayerMode.empty()) {
                applyPlayerMode(mPlayerMode, true);
                std::fprintf(stderr, "[input-script] player mode %s requested\n", mPlayerMode.c_str());
            }
            Input::focusChanged(true);
            std::fprintf(stderr, "[input-script] start (headless keyboard)\n");
        }
        if (mLine == mLines.size()) {
            hold({}, {});
            std::fprintf(stderr, "[input-script] done after %d frames\n", mFrame);
            return true;
        }
        auto& line = mLines[mLine];
        switch (line.kind) {
        case Line::Wait: {
            bool match = true, wantsControl = false;
            for (const auto& [key, value] : line.conditions) {
                std::string have;
                if (key == "controllable") {  // Mario can be steered: no demo running, pause menu allowed
                    wantsControl = true;
                    const auto scene = state.fields.find("scene"), init = state.fields.find("init_state"), valid = state.fields.find("player_valid");
                    have = (scene != state.fields.end() && scene->second == "Game" && init != state.fields.end() && init->second == "6" &&
                            valid != state.fields.end() && valid->second == "1" && !MR::isDemoActive() &&
                            static_cast<const GameScene*>(SingletonHolder<GameSystem>::get()->mSceneController->mScene)->permitsStartPauseMenu()) ? "1" : "0";
                } else {
                    const auto it = state.fields.find(key);
                    if (it != state.fields.end()) have = it->second;
                    else { match = false; break; }
                }
                if (have != value) { match = false; break; }
            }
            // A player advances an opening demo's dialogue with A while waiting for control.
            const auto sceneNow = state.fields.find("scene"), validNow = state.fields.find("player_valid");
            const bool tapping = !match && wantsControl && sceneNow != state.fields.end() && sceneNow->second == "Game" &&
                                 validNow != state.fields.end() && validNow->second == "1" && MR::isDemoActive();
            Input::KeyCode a;
            bool down = false;
            if (tapping && Input::parseKeyName("Space", &a)) {
                down = (mDemoFrames++ % 40) < 3;
            }
            hold(down ? std::vector<Input::KeyCode>{a} : std::vector<Input::KeyCode>{}, {});
            if (match) {
                std::fprintf(stderr, "[input-script] wait satisfied after %d loops (line %zu)\n", mLineFrame, mLine);
                ++mLine; mLineFrame = 0;
            } else if (line.timeout > 0 && ++mLineFrame > line.timeout) {
                std::string what;
                for (const auto& [key, value] : line.conditions) what += " " + key + "=" + value;
                std::string seen;
                for (const char* key : {"scene", "stage", "scenario", "init_state", "player_valid"}) {
                    const auto it = state.fields.find(key);
                    seen += std::string(" ") + key + "=" + (it == state.fields.end() ? "-" : it->second);
                }
                throw ReplayError("route wait timed out after " + std::to_string(line.timeout) + " loops for" + what + "; observed" + seen);
            }
            break;
        }
        case Line::At: {
            hold({}, {});
            const auto& clocks = line.seen ? mFirstObserved : mFirstSeen;
            const auto seen = clocks.find(line.stage);
            if (seen != clocks.end() && mLoop - seen->second >= static_cast<std::uint64_t>(line.offset)) {
                ++mLine; mLineFrame = 0;
            } else if (line.timeout > 0 && ++mLineFrame > line.timeout) {
                throw ReplayError("route 'at " + (line.stage.empty() ? std::string("-") : line.stage) + " " + std::to_string(line.offset) + "' timed out: stage " + (seen == clocks.end() ? "never observed" : "observed too late"));
            }
            break;
        }
        case Line::Viewport:
            Input::setViewport(Input::Viewport::letterbox(line.x, line.y, 16.0f / 9.0f));
            ++mLine;
            break;
        case Line::Pointer:
            Input::mouseMoved(line.x, line.y);
            ++mLine;
            break;
        default:
            hold(line.keys, line.mouse);
            if (++mLineFrame == line.frames) { ++mLine; mLineFrame = 0; }
        }
        ++mFrame;
        return false;
    }
private:
    static bool applyPlayerMode(const std::string& mode, bool apply) {
        struct Entry { const char* name; void (*change)(); };
        static const Entry table[] = {
            {"bee", MR::changePlayerModeBee},       {"hopper", MR::changePlayerModeHopper}, {"teresa", MR::changePlayerModeTeresa},
            {"ice", MR::changePlayerModeIce},       {"fire", MR::changePlayerModeFire},     {"yoshi", MR::changePlayerModeYoshi},
            {"cloud", MR::changePlayerModeCloud},   {"rock", MR::changePlayerModeRock},     {"foo", MR::changePlayerModeFoo},
            {"invincible", MR::changePlayerModeInvincible}, {"lifeup", MR::changePlayerItemLifeUp},
        };
        for (const auto& e : table)
            if (mode == e.name) { if (apply) e.change(); return true; }
        return false;
    }
    std::string mPlayerMode;
    bool mNoPlayer = false, mBoot = false;
    int mDemoFrames = 0;
    bool mDemoA = false;
    struct Line {
        enum Kind { Keys, Wait, At, Viewport, Pointer } kind = Keys;
        int frames = 0, timeout = 0, offset = 0;
        std::string stage;
        bool seen = false;  // atseen: count from the stage's first observation (any init_state), not from ready
        float x = 0.0f, y = 0.0f;
        std::vector<Input::KeyCode> keys;
        std::vector<Input::MouseButton> mouse;
        std::vector<std::pair<std::string, std::string>> conditions;
    };
    void hold(const std::vector<Input::KeyCode>& keys, const std::vector<Input::MouseButton>& mouse) {
        for (auto button : mHeldMouse)
            if (std::find(mouse.begin(), mouse.end(), button) == mouse.end()) Input::mouseButtonEvent(button, false);
        for (auto button : mouse)
            if (std::find(mHeldMouse.begin(), mHeldMouse.end(), button) == mHeldMouse.end()) Input::mouseButtonEvent(button, true);
        mHeldMouse = mouse;
        for (auto key : mHeld)
            if (std::find(keys.begin(), keys.end(), key) == keys.end()) Input::keyEvent(key, false, false);
        for (auto key : keys)
            if (std::find(mHeld.begin(), mHeld.end(), key) == mHeld.end()) Input::keyEvent(key, true, false);
        mHeld = keys;
    }
    std::vector<Line> mLines;
    std::vector<Input::KeyCode> mHeld;
    std::vector<Input::MouseButton> mHeldMouse;
    std::map<std::string, std::uint64_t> mFirstSeen, mFirstObserved;
    std::uint64_t mLoop = 0;
    std::size_t mLine = 0;
    int mFrame = 0, mLineFrame = 0, mSteady = 0;
    bool mStarted = false;
};
}
