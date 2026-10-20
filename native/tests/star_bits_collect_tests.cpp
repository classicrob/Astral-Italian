#include <petari/star_bits_collect.hpp>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace PetariNative::StarBits;
namespace {
int checks;
void check(bool value, const char* description) {
    ++checks;
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", description);
        std::exit(1);
    }
}
void testContext() {
    Context context;
    check(canCollect(context), "ordinary gameplay allowed");
    context.demo = true;
    check(!canCollect(context), "demo blocks");
    context.demo = false; context.talking = true;
    check(!canCollect(context), "normal talking blocks");
    context.talking = false; context.pointerValid = false;
    check(!canCollect(context), "disconnected or disabled pointer blocks");
    context.pointerValid = true; context.pointer1PDisabled = true;
    check(!canCollect(context), "1P disabled / 2P transparency mode blocks");
    context.pointer1PDisabled = false; context.starBits = 9999;
    check(canCollect(context), "below vanilla cap allowed");
    context.starBits = 10000;
    check(!canCollect(context), "vanilla cap blocks");
    context.starBits = 10001;
    check(!canCollect(context), "over cap blocks");
}
void testEligibility() {
    PieceState state;
    for (auto motion : {Motion::Floating, Motion::Fall, Motion::FollowPlayer, Motion::RailMove}) {
        state.motion = motion;
        check(eligible(state), "each pointer-collectible nerve allowed");
    }
    state.motion = Motion::Other;
    check(!eligible(state), "Hop, reflected Fall, ammo, gifts, chase and collected nerves excluded");
    state.motion = Motion::Floating; state.alive = false;
    check(!eligible(state), "dead / awake-switch-disabled piece excluded");
    state.alive = true; state.movementEnabled = false;
    check(!eligible(state), "suspended piece excluded");
    state.movementEnabled = true; state.collectionEnabled = false;
    check(!eligible(state), "group switch-gated piece excluded");
    state.collectionEnabled = true; state.hasPointerTarget = false;
    check(!eligible(state), "uninitialized pointer target excluded");
    state.hasPointerTarget = true; state.pointerDelay = 25;
    check(!eligible(state), "launch pointer delay retained");
    state.pointerDelay = 0;
    check(!eligible(state), "delay zero still excluded as in tryGotJudge");
    state.pointerDelay = -1;
    check(eligible(state), "delay expiration enables collection");
    // No camera, distance or clipped field exists in this production policy.
    check(eligible(state), "eligible policy is independent of view and distance");
}
void testWindow() {
    CollectionWindow window;
    check(!window.beginFrame(true, false, true), "no spontaneous window");
    for (int frame = 0; frame < 4; ++frame)
        check(window.beginFrame(true, frame == 0, true), "press frame plus three frames");
    check(!window.beginFrame(true, false, true), "window expires after four frames");
    check(!window.beginFrame(true, true, false), "blocked press discarded");
    check(!window.beginFrame(true, false, true), "blocked press cannot replay later");
    check(window.beginFrame(true, true, true), "new valid press starts window");
    check(!window.beginFrame(false, false, true), "toggle off cancels active window");
    check(!window.beginFrame(true, false, true), "reenabling does not restore old window");
    check(window.beginFrame(true, true, true), "new press after toggle works");
    check(!window.beginFrame(true, false, false), "demo/talk/cap cancels active window");
    check(!window.beginFrame(true, false, true), "unblocking cannot resume canceled window");
    window.beginFrame(true, true, true); window.reset();
    check(!window.beginFrame(true, false, true), "scene reset cancels window");
}
struct Piece { PieceState state; int starts = 0; };
void testRegistryAndLifecycle() {
    SceneRegistry<Piece> registry;
    CollectionWindow window;
    int sceneA, sceneB;
    Piece pooled, placed, group, dead, delayed, ammo;
    for (Piece* piece : {&pooled, &placed, &group, &dead, &delayed, &ammo})
        piece->state.motion = Motion::Floating;
    dead.state.alive = false; delayed.state.pointerDelay = 0; ammo.state.motion = Motion::Other;
    registry.add(&sceneA, &pooled);
    int visits = 0;
    registry.forEach(&sceneA, [&](Piece*) { ++visits; });
    check(visits == 0, "registration requires an active scene owner");
    registry.beginScene(&sceneA);
    for (Piece* piece : {&pooled, &placed, &group, &dead, &delayed, &ammo}) registry.add(&sceneA, piece);
    registry.add(&sceneA, &pooled); registry.add(&sceneA, nullptr);
    registry.forEach(&sceneA, [&](Piece*) { ++visits; });
    check(visits == 6, "all construction categories registered once, no pool-only restriction");
    for (int frame = 0; frame < 5; ++frame) {
        if (!window.beginFrame(true, frame == 0, true)) continue;
        registry.forEach(&sceneA, [](Piece* piece) {
            if (!eligible(piece->state)) return;
            ++piece->starts;
            // The production callback enters ToTarget immediately. It does not
            // award counters: the game's later touchPlayer path does that.
            piece->state.motion = Motion::Other;
        });
        if (frame == 0) delayed.state.pointerDelay = -1;
    }
    check(pooled.starts == 1 && placed.starts == 1 && group.starts == 1,
          "pooled, placed and group bits each start once across the window");
    check(delayed.starts == 1, "piece becoming eligible within the window starts once");
    check(dead.starts == 0 && ammo.starts == 0, "dead/ammo never start");
    registry.beginScene(&sceneB); window.reset();
    check(!registry.endScene(&sceneA), "late old-scene destructor cannot clear new scene");
    visits = 0;
    registry.forEach(&sceneB, [&](Piece*) { ++visits; });
    check(visits == 0, "replacement scene clears all old actor pointers before registration");
    Piece fresh; registry.add(&sceneA, &pooled); registry.add(&sceneB, &fresh);
    registry.forEach(&sceneB, [&](Piece* piece) { check(piece == &fresh, "only current scene visited"); });
    check(registry.endScene(&sceneB), "matching director teardown clears registry");
    check(!registry.endScene(&sceneB), "repeated teardown safe");
    visits = 0;
    registry.forEach(&sceneB, [&](Piece*) { ++visits; });
    check(visits == 0, "no actors reachable after teardown");
    registry.add(&sceneB, &fresh);
    registry.forEach(&sceneB, [&](Piece*) { ++visits; });
    check(visits == 0, "registration after teardown rejected");
}
}
int main() {
    testContext(); testEligibility(); testWindow(); testRegistryAndLifecycle();
    std::printf("%d Star Bit collection policy checks passed\n", checks);
}
