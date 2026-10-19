#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include <revolution/mtx.h>


namespace MR {
    s32 clamp(s32, s32, s32) NO_INLINE;

    void startSoundInPlayerDistance(LiveActor* pActor, const char* pSound, s32 pitch, s32 param, f32 distance) {
        s32 volume = clamp(static_cast<s32>(1000.0f * ((distance - calcDistanceToPlayer(pActor)) / distance)), 0, 1000);
        startActionSound(pActor, pSound, pitch, volume, param);
    }

    void startSoundInCameraDistance(LiveActor* pActor, const char* pSound, s32 pitch, s32 param, f32 distance) {
        s32 volume = clamp(static_cast<s32>(1000.0f * ((distance - PSVECDistance(&pActor->mPosition, &getMicPos())) / distance)), 0, 1000);
        startActionSound(pActor, pSound, pitch, volume, param);
    }
}
