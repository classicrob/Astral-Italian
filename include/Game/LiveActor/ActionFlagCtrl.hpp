#pragma once

#include "Game/Util/ObjUtil.hpp"

class ActorJointCtrl;
class HitSensorKeeper;
class LiveActor;

class ActionFlagCtrl {
public:
    ActionFlagCtrl(LiveActor* pHost);

    static ActionFlagCtrl* tryCreate(LiveActor* pHost);

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ HitSensorKeeper* mHitSensorKeeper;
    /* 0x08 */ ActorJointCtrl* mJointCtrl;
    /* 0x0C */ s32 mNumEntries;
    /* 0x10 */ void* mEntries;
    /* 0x14 */ void* mFlagData;
    /* 0x18 */ bool mIsActive;
};
