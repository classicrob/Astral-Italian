#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Array.hpp"

class StaffRoll;
class StaffRollDemoCamera;
class ActorCameraInfo;

class StaffRollDemoObj : public LiveActor {
public:
    StaffRollDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual ~StaffRollDemoObj();

    void initObjects(const JMapInfoIter&);
    void setAndShowPlayer();
    void updateActorVisibility();
    void exeStart();
    void exeB();
    void exeC();
    void exeD();
    void exeE();
    void exeF();
    void exeG();
    void exeH();

    /* 0x90 */ StaffRoll* mStaffRollObj;
    /* 0x94 */ LiveActor* mChildA;
    /* 0x98 */ s32 mStep98;
    /* 0x9C */ s32 mStep9C;
    /* 0xA0 */ ActorCameraInfo* mCameraInfo;
    /* 0xA4 */ MR::Vector< MR::AssignableArray< LiveActor* > > mActors;
    /* 0xB0 */ LiveActor* mChildB;
    /* 0xB4 */ LiveActor* mChildC;
    /* 0xB8 */ StaffRollDemoCamera* mRollA;
    /* 0xBC */ StaffRollDemoCamera* mRollB;
};
