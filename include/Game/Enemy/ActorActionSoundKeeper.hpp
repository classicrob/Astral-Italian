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
    void updateSounds();
    void updateSoundState(ActionSound*);
    void playActionSoundKeeper(const char*, s32, s32, s32) const;
    void playSoundWithMode(s32, const char*, s32, s32, s32) const;
    void startSoundByName(const char*) const;
    bool tryPlaySound(s32, s32, const char*, s32, s32, s32) const;
    void stopActionSoundKeeper(const char*, u32);
    void releaseActionSoundKeeper(const char*);
    void resetChecker();
    void updateChecker();
    LiveActor* mActor;
    const ActionSoundData* mData;
    s32 _8;
    ActionSound** mSounds;
    SyncBckEffectChecker* mChecker;
    s32 mSoundCount;
    s32 mJointCount;
    JointSoundInfo* mJoints;
};
