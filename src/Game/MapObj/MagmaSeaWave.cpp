#pragma auto_inline off
#include "Game/MapObj/MagmaSeaWave.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
extern const f32 lbl_807DDBF8; // Shared positive half-width; the target negates it at runtime.
namespace NrvMagmaSeaWave {
NEW_NERVE(MagmaSeaWaveNrvMove, MagmaSeaWave, Move);
}
namespace {
TVec3f sInitialScale(0.1f, 0.1f, 0.1f);
TVec3f sVolumeScale(0.8f, 0.7f, 13.0f);
TVec3f sVolumeOffset(-50.0f, 100.0f, -4000.0f);
}
#pragma auto_inline on
MagmaSeaWave::MagmaSeaWave(const char* name) : LiveActor(name), mDrawer(0), mSensorPosition(0.0f, 0.0f, 0.0f), mRailClipPosition(0.0f, 0.0f, 0.0f), mSpeed(10.0f), mSoundFrame(0) { mVolumeMtx.identity(); }
#pragma auto_inline off
void MagmaSeaWave::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    MR::setSensorFollowPos(this, "Attack", &mSensorPosition);
    MR::initAndSetRailClipping(&mRailClipPosition, this, 100.0f, 500.0f);
    MR::moveCoordAndTransToNearestRailPos(this);
    mDrawer = MR::createVolumeDrawer("津波発熱領域", "VolumeCylinder", mVolumeMtx);
    GXColor color = {255, 0, 0, 255};
    mDrawer->setColor(Color8(color));
    mDrawer->setScale(sInitialScale);
    mDrawer->setDrawPasses(1);
    MR::getJMapInfoArg0NoInit(iter, &mSpeed);
    initNerve(&NrvMagmaSeaWave::MagmaSeaWaveNrvMove::sInstance);
    makeActorAppeared();
    mDrawer->makeActorAppeared();
    MR::setSoundPositionPtr(this, &mSensorPosition);
}
// NONMATCHING: component-load scheduling differs around the vector set call.
void MagmaSeaWave::control() {
    TPos3f mtx;
    JMath::gekko_ps_copy12(&mtx, MR::getJointMtx(this, "VolumeCenter"));
    MR::rotateMtxLocalYDegree(mtx, 90.0f);
    JMath::gekko_ps_copy12(&mVolumeMtx, &mtx);
    mVolumeMtx.getTrans(mSensorPosition);
    const f32 halfWidth = lbl_807DDBF8;
    MR::calcNearestPointToPlayerOnSideLine(&mSensorPosition, this, &mSensorPosition, -halfWidth, halfWidth);
    MR::addTransMtxLocal(mVolumeMtx, sVolumeOffset);
    MR::preScaleMtx(mVolumeMtx, sVolumeScale);
    MR::rotateMtxLocalX(mVolumeMtx, 1.5707963705062866f);
    GXColor color = {255, 0, 0, 255};
    mDrawer->setColor(Color8(color));
    mDrawer->setScale(sVolumeScale);
    mDrawer->setDrawPasses(1);
}
void MagmaSeaWave::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver) && MR::isEqualSensor(sender, this, "Attack")) MR::sendMsgEnemyAttackFire(receiver, sender);
}
void MagmaSeaWave::kill() { LiveActor::kill(); mDrawer->kill(); }
void MagmaSeaWave::exeMove() {
    if (MR::isRailReachedGoal(this)) {
        MR::moveCoordAndTransToStartPos(this);
        MR::calcAnimDirect(this);
        MR::resetAllCollisionMtx(this);
        MR::fn_80008510(this, "Move", 0);
        mSoundFrame = 0;
    } else {
        MR::moveCoordAndFollowTrans(this, mSpeed);
        MR::startActionSound(this, "Move");
    }
    if (mSoundFrame == 90) MR::startActionSound(this, "Start");
    ++mSoundFrame;
}
MagmaSeaWave::~MagmaSeaWave() {}

// Recovered target constant storage; keep definitions after their call sites.
// Positive half-width used as both signs for the lateral player projection segment.
extern const f32 lbl_807DDBF8 = 3250.0f;
