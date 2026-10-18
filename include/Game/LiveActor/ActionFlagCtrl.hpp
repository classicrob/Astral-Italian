#pragma once

#include "Game/Util/ObjUtil.hpp"

class ActorJointCtrl;
class HitSensorKeeper;
class LiveActor;

class ActionFlagSensorInfo {
public:
    ActionFlagSensorInfo();
    const char* mName;
    u16 mFlag;
    s16 mStartFrame;
    s16 mEndFrame;
};

struct ActionFlagInfo {
    const char* mName;
    u16* mFlags;
    ActionFlagSensorInfo* mSensors;
};

class ActionFlagCtrl {
public:
    ActionFlagCtrl(LiveActor* pHost);

    void init();
    void setJointCtrl(ActorJointCtrl*);
    void startAction(const char*);
    void update();
    void updateFlags();
    void updateFaceCtrl();
    void initSensors();
    void updateSensors();
    bool isFlagOn(s32, bool) const;
    bool isFlagOff(s32, bool) const;
    ActionFlagInfo* findInfo(const char*) const;

    static ActionFlagCtrl* tryCreate(LiveActor* pHost);

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ HitSensorKeeper* mHitSensorKeeper;
    /* 0x08 */ ActorJointCtrl* mJointCtrl;
    /* 0x0C */ s32 mNumEntries;
    /* 0x10 */ ActionFlagInfo* mEntries;
    /* 0x14 */ ActionFlagInfo* mFlagData;
    /* 0x18 */ bool mIsActive;
};
