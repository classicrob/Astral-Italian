#pragma once

namespace PetariNative::App::Smoke {

// Game-thread lifetime gate. Ordinary scene changes clear Initialize_End before
// starting asynchronous deletion; reset uses mIsResetProcessing instead. Test
// that flag FIRST, without even reading the worker-owned mScene slot. Reset is
// monotonic until the controller restarts, so late worker completion cannot
// expose a frozen or already freed player through an observer.
template <class Controller, class InitializeState>
bool canObserveScene(const Controller* controller, InitializeState end) {
    return controller && !controller->mIsResetProcessing &&
           controller->isSceneInitializeState(end) && controller->mScene;
}

template <class Player>
bool canObservePlayer(const Player* player) {
    return player && player->getMario() &&
           (!player->_934 || (player->_924 && player->_924->mHost));
}

// Warp status counts as "in rush" without an actor-bound rush sensor.
// Unlike gameplay callers in race/bind states, observers run in every state.
template <class Player>
const char* observedRushActorName(const Player* player, bool inRush) {
    if (!player || !inRush || !player->_924 || !player->_924->mHost || !player->_924->mHost->mName) {
        return "";
    }
    return player->_924->mHost->mName;
}

} // namespace PetariNative::App::Smoke
