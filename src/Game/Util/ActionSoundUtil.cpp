#include "Game/Util/ActionSoundUtil.hpp"

namespace MR {
    const ActorActionSoundKeeper* getActionSoundInfo(const LiveActor* pActor) NO_INLINE {
        const ActorSoundHolder* soundHolder = pActor->mSoundHolder;
        return soundHolder != nullptr ? soundHolder->mActionSoundKeeper : nullptr;
    }

    // Does any code reference this? Assuming it's a copy for now...
    const ActorActionSoundKeeper* fn_80008370(const LiveActor* pActor) NO_INLINE {
        const ActorSoundHolder* soundHolder = pActor->mSoundHolder;
        return soundHolder != nullptr ? soundHolder->mActionSoundKeeper : nullptr;
    }

    void startActionSound(const LiveActor* pActor, const char* pName, s32 pitch, s32 velocity, s32 volume) {
        const ActorActionSoundKeeper* soundInfo = getActionSoundInfo(pActor);
        if (soundInfo != nullptr) {
            soundInfo->playActionSoundKeeper(pName, pitch, velocity, volume);
        }
    }
}  // namespace MR
