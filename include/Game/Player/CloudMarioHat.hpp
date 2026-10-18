#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class CloudHatPoint {
public:
    CloudHatPoint();
    void update();
    TVec3f mPosition;
    TVec3f mVelocity;
    TVec3f mAcceleration;
};

class CloudHatSpring {
public:
    CloudHatSpring(TPos3f*, CloudHatPoint*, CloudHatPoint*);
    void update(const TVec3f&);
    void reset();

    TPos3f* mMatrix;
    CloudHatPoint* mStart;
    CloudHatPoint* mEnd;
    TVec3f mRestOffset;
    f32 mLength;
    TVec3f mPreviousDirection;
    TVec3f mAnchor;
    TVec3f mDirection;
};

class CloudMarioHat : public LiveActor {
public:
    CloudMarioHat(const char*);
    virtual ~CloudMarioHat();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAnim();
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    bool calcJoint(TPos3f*, const JointControllerInfo&);
    void initDynamics();
    void resetDynamics();
    void updateDynamics();
    s32 findJoint(const JointControllerInfo&);
    void updateJoint(TPos3f*, s32);

    u8 _90;
    u8 _91;
    CloudHatPoint** mPoints;
    CloudHatSpring** mSprings;
    JointController** mControllers;
    J3DJoint** mJoints;
    TPos3f mHatMatrix;
};
