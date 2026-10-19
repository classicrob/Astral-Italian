#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class DinoPackun;
template < typename T >
class JointControlDelegator;
class JointController;
class JointControllerInfo;

class DinoPackunTailNode : public LiveActor {
public:
    DinoPackunTailNode(const char*, DinoPackun*);

    virtual const TVec3f* getNodeDirection() const;
    virtual void requestLockPosition();
    virtual void requestUnLockPosition();
    virtual void addNodeVelocity(const TVec3f&);
    virtual JointController* createJointControllerOwn(LiveActor*, const char*) = 0;

    void createJointController(LiveActor*, const char*);
    void resetJoint();
    f32 getLinkLength() const;
    f32 getKeepBendPower() const;
    bool preCalcJoint(TPos3f*, const JointControllerInfo&);
    bool turnJointLocalXDir(TPos3f*, const JointControllerInfo&);
    bool calcJointScale(TPos3f*, const JointControllerInfo&);
    void registerPreCalcJointCallBack();
    void registerJointCallBack();
    void lockPosition();
    void unLockPosition();
    void addNodeVelocityHost(const TVec3f&);

    /* 0x90 */ DinoPackun* mParent;
    /* 0x94 */ TVec3f mNodeDirection;
    /* 0xA0 */ TVec3f mAnimatedPos;
    /* 0xAC */ TVec3f mJointFront;
    /* 0xB8 */ DinoPackunTailNode* mPrevNode;
    /* 0xBC */ DinoPackunTailNode* mNextNode;
    /* 0xC0 */ MtxPtr mJointMtx;
    /* 0xC4 */ JointControlDelegator< DinoPackunTailNode >* mPreCalcJointCtrl;
    /* 0xC8 */ JointControlDelegator< DinoPackunTailNode >* mJointCtrl;
    /* 0xCC */ f32 mLinkLength;
    /* 0xD0 */ f32 mKeepBendPower;
    /* 0xD4 */ u8 mIsPositionLocked;
};
