#pragma once
#include <JSystem/JGeometry.hpp>
#include <revolution/types.h>
class JointController;
struct JointControllerInfo;
class JointCtrlRate;
class LiveActor;

struct FaceJointCtrlParam {
    FaceJointCtrlParam();
    f32 mDegreeLeft;
    f32 mDegreeRight;
    f32 mDegreeVertical;
};

class FaceJointCtrl {
public:
    FaceJointCtrl(LiveActor*, const char*, FaceJointCtrlParam*);
    void startCtrl(s32);
    void endCtrl(s32);
    void update() NO_INLINE;
    void setCallBackFunction();
    bool updateJointMtxCallBack(TPos3f*, const JointControllerInfo&);
    /* 0x00 */ LiveActor* mHostActor;
    /* 0x04 */ const char* mJointName;
    /* 0x08 */ FaceJointCtrlParam* mParam;
    /* 0x0C */ TVec3f mDirection;
    /* 0x18 */ JointController* mJointController;
    /* 0x1C */ JointCtrlRate* mJointCtrlRate;
    /* 0x20 */ const TVec3f* mTargetPosition;
    /* 0x24 */ TPos3f mRotation;
};

class FaceJointCtrlKeeper {
public:
    static FaceJointCtrlKeeper* tryCreate(LiveActor*);
    FaceJointCtrlKeeper(LiveActor*);
    void update();
    void setCallBackFunction();
    void startCtrl(s32);
    void endCtrl(s32);
    LiveActor* mHost;
    s32 mCount;
    FaceJointCtrl** mControllers;
};
