#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>

class HitSensor;
class LiveActor;

// One row of the ActorHitReactionCtrl csv (0x20 bytes).
class ActorHitReactionCtrlEntry {
public:
    ActorHitReactionCtrlEntry();

    /* 0x00 */ const char* mReactionName;
    /* 0x04 */ const char* mEffectName;
    /* 0x08 */ f32 mEffectBetweenOffset;
    /* 0x0C */ const char* mSoundName;
    /* 0x10 */ const char* mPadRumbleName;
    /* 0x14 */ const char* mCameraShakeName;
    /* 0x18 */ s32 mStopSceneFrame;
    /* 0x1C */ bool mIsStopSceneForHitEffect;
};

class ActorHitReactionCtrl {
public:
    ActorHitReactionCtrl(LiveActor*);

    static ActorHitReactionCtrl* tryCreate(LiveActor*);

    void reactToHit(const char*, const TVec3f*, const HitSensor*, const HitSensor*);

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ s32 mEntryNum;
    /* 0x08 */ ActorHitReactionCtrlEntry* mEntries;
};
