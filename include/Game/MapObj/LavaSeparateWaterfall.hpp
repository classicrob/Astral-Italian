#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"
class CollisionParts;
class MapPartsRailMover;
class LavaSeparateWaterfall : public LiveActor {
public:
    LavaSeparateWaterfall(const char*);
    virtual ~LavaSeparateWaterfall();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void calcAnim();
    void exeWait();
    void exeMove();
    JointController* createJointController(const char*, JointControlDelegator<LavaSeparateWaterfall>::func);
    bool calcLeftMtx(TPos3f*, const JointControllerInfo&);
    bool calcRightMtx(TPos3f*, const JointControllerInfo&);
    CollisionParts* mLeftCollision;
    CollisionParts* mRightCollision;
    LiveActor* mLeftRail;
    LiveActor* mRightRail;
    MapPartsRailMover* mLeftMover;
    MapPartsRailMover* mRightMover;
    JointController* mLeftController;
    JointController* mRightController;
    f32 mReactionDistance;
    bool mIsMoving;
};
