#pragma once

#include <revolution.h>

class ActionSoundInfo;
class SyncBckEffectInfo;
class XanimePlayer;
class JAISoundHandle;
class AudSoundObject;

class ActionSound {
public:
    ActionSound(const ActionSoundInfo* pInfo, AudSoundObject* pSoundObject);

    void fn_8021F970(XanimePlayer*, s32);
    void fn_8021FA40();
    void fn_8021FA80();
    void fn_8021FAE0(u32);
    void fn_8021FB80();
    void fn_8021FC10(f32);
    void fn_8021FD50(f32);
    void fn_8021FF00(s32, s32);
    f32 fn_8021F910(s32) const;
    f32 fn_8021F930(s32) const;
    s32 getId() const;
    const char* getName() const;
    bool play(s32 mode, s32 pitch, s32 velocity, s32 volume);

    const ActionSoundInfo* mInfo;
    SyncBckEffectInfo* mParam;
    AudSoundObject* mSoundObject;
    JAISoundHandle* mHandle;
    bool mUnk10;
};

JAISoundHandle* fn_8021F6F0(AudSoundObject*, s32, u32, s32, s32);
JAISoundHandle* fn_8021F7E0(AudSoundObject*, s32, u32, s32, s32, s32);
void fn_8021F8C0(f32*, f32, f32);
