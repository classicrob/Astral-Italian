#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class ActionSound;
class ActionSoundData;
class AudSoundObject;
class LiveActor;
class SyncBckEffectChecker;
struct JointSoundInfo {
    JointSoundInfo();
    const char* mName;
    MtxPtr mJointMtx;
    TVec3f mPosition;
    AudSoundObject* mSoundObject;
};
class ActorActionSoundKeeper {
public:
    ActorActionSoundKeeper(LiveActor*, AudSoundObject*, const ActionSoundData*);
    void fn_8021F2D0();
    void fn_8021F3B0(ActionSound*);
    void fn_8021F470(const char*, s32, s32, s32) const;
    void fn_8021F490(s32, const char*, s32, s32, s32) const;
    void fn_8021F4B0(const char*) const;
    bool fn_8021F4D0(s32, s32, const char*, s32, s32, s32) const;
    void fn_8021F5A0(const char*, u32);
    void fn_8021F630(const char*);
    void fn_8021F6B0();
    void fn_8021F6D0();
    LiveActor* mActor;
    const ActionSoundData* mData;
    s32 _8;
    ActionSound** mSounds;
    SyncBckEffectChecker* mChecker;
    s32 mSoundCount;
    s32 mJointCount;
    JointSoundInfo* mJoints;
};
