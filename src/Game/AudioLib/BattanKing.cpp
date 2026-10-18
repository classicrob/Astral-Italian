#include "Game/Enemy/BattanKing.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/Pattan.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/TriangleFilter.hpp"

namespace {
    // (actor name, parts/arc name, joint name) of the five collision parts.
    struct CollisionPartsInfo {
        const char* mActorName;
        const char* mPartsName;
        const char* mJointName;
    };

    const CollisionPartsInfo sCollisionPartsInfo[5] = {
        {"コリジョン[体　]", "BattanKing", "BattanKingBody"},
        {"コリジョン[左腕]", "BattanKingArmL", "ArmL"},
        {"コリジョン[右腕]", "BattanKingArmR", "ArmR"},
        {"コリジョン[左足]", "BattanKingLegL", "LegL"},
        {"コリジョン[右足]", "BattanKingLegR", "LegR"},
    };
}

BattanKing::BattanKing(const char* pName) : LiveActor(pName) {
    mCollisions = NULL;
    mCollisionPresses = NULL;
    mBreakModel = NULL;
    mVolumeDrawers = NULL;
    mUnkD0 = 0.0f;
    mUnkD4 = 0.0f;
    mUnkD8 = 0.0f;
    mVolumeMtxs = NULL;
    mEarthquakeDrawer = NULL;
    mPattanHolder = NULL;
    mCameraInfo = NULL;
    mTalkStateA = NULL;
    mTalkStateB = NULL;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mPhase = 0;
    mAttackType = -1;
    mUnk1B0.x = 0.0f;
    mUnk1B0.y = 0.0f;
    mUnk1B0.z = 0.0f;
    mUnk1BC.x = 0.0f;
    mUnk1BC.y = 0.0f;
    mUnk1BC.z = 0.0f;
    mArg7 = -1;
    mFlagA = 0;
    mFlagB = 0;
    mFlagC = 0;
    mMtxFx.identity();
    mMtxBreak.identity();
    mMtxFollow.identity();
}

void BattanKing::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BattanKing", false);

    mCollisions = new BattanKingCollision*[5];
    mCollisionPresses = new BattanKingCollisionPress*[5];
    for (s32 i = 0; i < 5; i++) {
        const CollisionPartsInfo& info = sCollisionPartsInfo[i];
        BattanKingCollision* pCollision = new BattanKingCollision(this, info.mActorName, info.mPartsName, NULL,
                                                                  MR::getJointMtx(this, info.mJointName), 0x1E);
        mCollisions[i] = pCollision;
        pCollision->initWithoutIter();

        BattanKingCollisionPress* pPress = new BattanKingCollisionPress(this, info.mActorName, info.mPartsName, NULL,
                                                                        MR::getJointMtx(this, info.mJointName), 0x1E);
        mCollisionPresses[i] = pPress;
        pPress->initWithoutIter();
    }

    // Result of this allocation is not stored in the target.
    new u8[0x160];
    mVolumeMtxs = new TMtx34f[7];

    mVolumeDrawers = new VolumeDrawer*[7];
    mVolumeDrawers[0] = MR::createVolumeDrawer("Body", "VolumeBox", mVolumeMtxs[0]);
    mVolumeDrawers[1] = MR::createVolumeDrawer("ArmL", "VolumeBox", mVolumeMtxs[1]);
    mVolumeDrawers[2] = MR::createVolumeDrawer("ArmR", "VolumeBox", mVolumeMtxs[2]);
    mVolumeDrawers[3] = MR::createVolumeDrawer("HandL", "VolumeCylinder", mVolumeMtxs[3]);
    mVolumeDrawers[4] = MR::createVolumeDrawer("HandR", "VolumeBox", mVolumeMtxs[4]);
    mVolumeDrawers[5] = MR::createVolumeDrawer("LegL", "VolumeBox", mVolumeMtxs[5]);
    mVolumeDrawers[6] = MR::createVolumeDrawer("LegR", "VolumeBox", mVolumeMtxs[6]);
    mEarthquakeDrawer = MR::createVolumeDrawer("地震範囲描画", "VolumeBox", mEarthquakeMtx);

    PattanHolder* pHolder = new PattanHolder("パタン保持", 10);
    mPattanHolder = pHolder;
    for (s32 i = 0; i < pHolder->mMaxCount; i++) {
        Pattan* pPattan = new Pattan("パタン");
        pPattan->initWithoutIter();
        pHolder->registerActor(pPattan);
    }
    pHolder->makeAllDead();

    mBreakModel = new BreakModel(this, "バッタンキング(Break)", "BattanKingBreak", getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();

    MR::getJMapInfoArg7WithInit(rIter, &mArg7);
    if (mArg7 != -1) {
        MR::declareCameraRegisterVec(this, mArg7, &mUnk1B0);
        MR::declareCameraRegisterVec(this, mArg7 + 1, &mUnk1BC);
    }

    mCameraInfo = new ActorCameraInfo(rIter);

    // NOTE: target calls setBinderCollisionPartsFilter with this TriangleFilter object (see NOTES); keep the
    // triangle-filter binder until the parameter type is resolved.
    MR::setBindTriangleFilter(this, new TriangleFilterDelegator<BattanKing>(this, &BattanKing::isInvalidTriangle));

    MR::tryRegisterNamePosLinkObj(this, rIter, "バッタンキング基準位置");
    MR::tryRegisterNamePosLinkObj(this, rIter, "マリオ位置");
    MR::findLinkNamePos(this, "バッタンキング基準位置", mMtxFx);
    MR::setEffectHostMtx(this, "AttackShockWave", mMtxFx);
    MR::setEffectHostMtx(this, "QuakeShockWave", mMtxFx);
    MR::setEffectHostMtx(this, "AttackSign", mMtxFx);
    MR::setEffectHostMtx(this, "QuakeSign", mMtxFx);

    MR::calcFrontVec(&mFront, this);
    mUnk180 = mPosition;
    mUnk18C = mFront;
    MR::declarePowerStar(this);
    MR::makeMtxTR(mMtxFollow, this);

    MR::addBaseMatrixFollowTarget(this, rIter, &mMtxFollow,
                                  new BaseMatrixFollowValidateDelegator<BattanKing>(this, &BattanKing::isValidCollisionState));

    initNerve(&NrvBattanKing::NrvE70::sInstance, 2);

    mTalkStateA = new BattanKingTalkState(this, rIter, "バッタンキング登場", "DemoBattanKingOpening", "BattanKing000");
    mTalkStateB = new BattanKingTalkState(this, rIter, "バッタンキングダウン", "DemoBattanKingDown", "BattanKing001");
    mTalkStateB->mFlag18 = 1;

    for (s32 i = 0; i < 5; i++) {
        MR::registerDemoCast(mCollisions[i], "バッタンキング登場", rIter);
        MR::registerDemoCast(mCollisions[i], "バッタンキングダウン", rIter);
        MR::registerDemoCast(mCollisionPresses[i], "バッタンキング登場", rIter);
        MR::registerDemoCast(mCollisionPresses[i], "バッタンキングダウン", rIter);
    }
    MR::registerDemoCast(mBreakModel, "バッタンキングダウン", rIter);

    MR::initActorState(this, mTalkStateA, &NrvBattanKing::NrvE6C::sInstance, NULL);
    MR::initActorState(this, mTalkStateB, &NrvBattanKing::NrvEAC::sInstance, NULL);

    makeActorAppeared();
    MR::validateClipping(this);
}
