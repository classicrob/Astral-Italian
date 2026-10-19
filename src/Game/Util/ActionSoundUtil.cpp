#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace MR {
    ActorActionSoundKeeper* getActionSoundInfo(const LiveActor* pActor) NO_INLINE {
        const ActorSoundHolder* soundHolder = pActor->mSoundHolder;
        return soundHolder != nullptr ? soundHolder->mActionSoundKeeper : nullptr;
    }

    ActorActionSoundKeeper* getActorActionSoundKeeper(const LiveActor* pActor) NO_INLINE {
        const ActorSoundHolder* soundHolder = pActor->mSoundHolder;
        return soundHolder != nullptr ? soundHolder->mActionSoundKeeper : nullptr;
    }

    void startActionSound(const LiveActor* pActor, const char* pName, s32 pitch, s32 velocity, s32 volume) {
        ActorActionSoundKeeper* soundInfo = getActionSoundInfo(pActor);
        if (soundInfo != nullptr) {
            soundInfo->playActionSoundKeeper(pName, pitch, velocity, volume);
        }
    }
    void startActionSoundAuto(const LiveActor* actor, const char* name, s32 pitch, s32 velocity, s32 volume) {
        ActorActionSoundKeeper* keeper = getActorActionSoundKeeper(actor);
        if (keeper)
            keeper->playSoundWithMode(0, name, pitch, velocity, volume);
    }
    void startActionSoundOneShot(const LiveActor* actor, const char* name, s32 pitch, s32 velocity, s32 volume) {
        ActorActionSoundKeeper* keeper = getActorActionSoundKeeper(actor);
        if (keeper)
            keeper->playSoundWithMode(1, name, pitch, velocity, volume);
    }
    void startActionSoundLevel(const LiveActor* actor, const char* name, s32 pitch, s32 velocity, s32 volume) {
        ActorActionSoundKeeper* keeper = getActorActionSoundKeeper(actor);
        if (keeper)
            keeper->playSoundWithMode(2, name, pitch, velocity, volume);
    }
    void fn_80008510(const LiveActor* actor, const char* name, s32 frames) {
        ActorActionSoundKeeper* keeper = getActionSoundInfo(actor);
        if (keeper)
            keeper->stopActionSoundKeeper(name, frames);
    }
    void fn_80008560(const LiveActor* actor, const char* name) {
        ActorActionSoundKeeper* keeper = getActionSoundInfo(actor);
        if (keeper)
            keeper->releaseActionSoundKeeper(name);
    }
    void startActionSoundAuto(const LayoutActor* actor, const char* name, s32 pitch, s32 velocity) {
        if (actor->mActionSoundKeeper)
            actor->mActionSoundKeeper->startSoundAuto(name, pitch, velocity);
    }
    void startActionSound(const LayoutActor* actor, const char* name, s32 pitch, s32 velocity) {
        if (actor->mActionSoundKeeper)
            actor->mActionSoundKeeper->startSound(name, pitch, velocity);
    }
    void startActionSoundOneShot(const LayoutActor* actor, const char* name, s32 pitch, s32 velocity) {
        if (actor->mActionSoundKeeper)
            actor->mActionSoundKeeper->startSoundOneShot(name, pitch, velocity);
    }
    void startPlayerSoundForYoshiMode(const char* playerName, const char* yoshiName, s32 pitch) {
        if (isPlayerElementModeYoshi()) {
            if (yoshiName)
                startActionSoundPlayer(yoshiName, pitch, -1);
        } else {
            if (playerName)
                startLevelSoundPlayer(playerName, pitch, -1);
        }
    }
}  // namespace MR
