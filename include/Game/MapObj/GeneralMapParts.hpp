#pragma once

#include "Game/MapObj/MapParts.hpp"
#include "Game/Util/Array.hpp"

class ActorCameraInfo;
class MapPartsAppearController;
class MapPartsFunction;
class MapPartsRailGuideDrawer;
class MapPartsRailMover;
class MapPartsRailPosture;
class MapPartsRailRotator;
class MapPartsRotator;

class GeneralMapParts : public MapParts {
public:
    GeneralMapParts(const char* pName);

    virtual ~GeneralMapParts() {
    }

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initMapPartsFunction(const JMapInfoIter& rIter);
    void initSensorType(const JMapInfoIter& rIter);
    void initGravity(const JMapInfoIter& rIter);
    void receiveMsgSwitchBOn();
    void receiveMsgSwitchBOff();
    void broadcastMsgToAllFunctions(u32 msg);
    bool isFixed() const;
    void startMove();
    void exeWait();
    void exeWaitForPlayerOn();
    void exeMoveStart();

    /* 0xA8 */ ActorCameraInfo* mCameraInfo;
    /* 0xAC */ s32 mMoveConditionType;
    /* 0xB0 */ s32 mSignMotionType;
    /* 0xB4 */ s32 mShadowType;
    /* 0xB8 */ MR::Vector< MR::FixedArray< MapPartsFunction*, 8 > > mFunctionArray;
    /* 0xDC */ MapPartsAppearController* mAppearController;
    /* 0xE0 */ u32 _E0;
    /* 0xE4 */ MapPartsRailRotator* mRailRotator;
    /* 0xE8 */ MapPartsRailMover* mRailMover;
    /* 0xEC */ MapPartsRotator* mRotator;
    /* 0xF0 */ MapPartsRailGuideDrawer* mGuideDrawer;
    /* 0xF4 */ MapPartsRailPosture* mRailPosture;
    /* 0xF8 */ u8 mIsCameraEnded;
    /* 0xF9 */ u8 mIsPaused;
};
