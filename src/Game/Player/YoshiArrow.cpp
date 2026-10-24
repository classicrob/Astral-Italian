#include "Game/Player/YoshiArrow.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/JMath/JMath.hpp>

YoshiArrow::YoshiArrow(const char* name) : LiveActor(name), mTargetLength(0.0f), mLength(0.0f), mArrowJoint(nullptr), mVolume(nullptr), mUp(0.0f,0.0f,0.0f) {
    mArrowMtx.identity();
    mVolumeMtx.identity();
}
YoshiArrow::~YoshiArrow() {}
void YoshiArrow::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "YoshiArrow", false);
    MR::invalidateClipping(this);
    mArrowJoint = MR::getJoint(this, "Arrow");
    MR::startBtk(this, "YoshiArrow");
    makeActorDead();
    mVolume = MR::createVolumeDrawer("ヨッシーショット矢印", "YoshiArrowShadow", mVolumeMtx.toMtxPtr());
    mVolume->setColor(Color8(0xFF,0x20,0,0x50));
    mVolume->setDrawPasses(5);
    mVolume->setScale(TVec3f(0.1f,0.0f,0.05f));
    MR::invalidateClipping(mVolume);
    mVolume->makeActorDead();
    MR::registerDemoSimpleCastAll(this);
}
// NONMATCHING: paired-single register allocation and instruction order only.
void YoshiArrow::fn_80413DC0(const TVec3f& start, const TVec3f& end, const TVec3f& up) {
    TVec3f direction(end);
    direction -= start;
    mTargetLength = PSVECMag(&direction);
    direction.scale(1.0f / mTargetLength);
    if (mTargetLength < 250.0f) mTargetLength = 250.0f;
    else if (mTargetLength > 600.0f) mTargetLength = 600.0f;
    mPosition.set(start);
    mPosition += up * 60.0f;
    TQuat4f rotation;
    rotation.setRotate(TVec3f(0.0f,0.0f,1.0f), direction);
    mArrowMtx.makeQuat(rotation);
    mArrowMtx.setTrans(mPosition);
    mUp = up;
}
void YoshiArrow::fn_80413FB0() { mLength = 0.0f; }
void YoshiArrow::appear() { LiveActor::appear(); mVolume->appear(); }
void YoshiArrow::kill() { LiveActor::kill(); mVolume->kill(); }
void YoshiArrow::fn_80414040() { MR::requestMovementOn(this); MR::requestMovementOn(mVolume); }
// NONMATCHING: scalar literal relocation labels.
void YoshiArrow::control() {
    if (MR::isPlayerDead()) {
        if (!MR::isHiddenModel(this)) { MR::hideModel(this); mVolume->setScaleDownRate(0); }
    } else if (MR::isPlayerInRushOrDemoOrTalk()) {
        if (!MR::isHiddenModel(this)) { MR::hideModel(this); mVolume->setScaleDownRate(0); }
    } else if (MR::isHiddenModel(this)) { MR::showModel(this); mVolume->setScaleUpRate(0); }
    mLength = mLength + 0.2f * (mTargetLength - mLength);
    mArrowJoint->getTransformInfo().mScale.x = mLength / 6.0f;
    TVec3f front;
    mArrowMtx.getZDir(front);
    JMAVECScaleAdd(&mUp, &front, &front, -mUp.dot(front));
    front.scale(0.00125f * mLength);
    TVec3f side;
    PSVECCrossProduct(&mUp, &front, &side);
    MR::normalizeOrZero(&side);
    mVolumeMtx.setXDir(side);
    mVolumeMtx.setYDir(mUp);
    mVolumeMtx.setZDir(front);
    mVolumeMtx.setTrans(mPosition);
}
void YoshiArrow::calcAndSetBaseMtx() { MR::setBaseTRMtx(this, mArrowMtx); }
