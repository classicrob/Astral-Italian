#pragma once

#include "Game/LiveActor/LiveActor.hpp"


class LayoutActor;

class ActionSoundSystem;

namespace MR {
    void fn_80008560(const LiveActor*, const char*);
    ActionSoundSystem* getActionSoundData(const char* pName);
    void startActionSoundAuto(const LayoutActor*, const char*, s32 = -1, s32 = -1);
    void startActionSoundOneShot(const LayoutActor*, const char*, s32 = -1, s32 = -1);
    void startActionSound(const LayoutActor*, const char*, s32 = -1, s32 = -1);
    const ActionSoundInfo* getActionSoundInfo(const LiveActor* pActor) NO_INLINE;
    const ActionSoundInfo* fn_80008370(const LiveActor* pActor) NO_INLINE;
    void startActionSound(const LiveActor* pActor, const char* pName, s32 pitch = -1, s32 velocity = -1, s32 volume = -1);
};  // namespace MR

namespace MR {
    void startActionSoundAuto(const LiveActor*, const char*, s32 = -1, s32 = -1, s32 = -1);
}

namespace MR {
    void fn_80008510(const LiveActor*, const char*, s32);
}
