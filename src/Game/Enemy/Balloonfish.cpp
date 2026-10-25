#include "Game/Util/Functor.hpp"
#include "Game/Enemy/Balloonfish.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"


namespace {
    static const s32 hWaitTime = 120;
    static const s32 hAttackBeginTime = 82;
    static const f32 hTargetPlayerVelocityMult = 10.0f;
    static const f32 hWaitMaxScale = 1.5f;
    static const s32 hDashTime = 180;
    static const f32 hDashVel = 10.0f;
    static const f32 hDashScale = 0.5f;
    static const s32 hDashEndTime = 95;
    static const f32 hRotateAngle = 0.0f;
};  // namespace

namespace NrvBalloonfish {
    NEW_NERVE(HostTypeNrvWait, Balloonfish, Wait);
    NEW_NERVE(HostTypeNrvDash, Balloonfish, Dash);
};  // namespace NrvBalloonfish

Balloonfish::Balloonfish(const char* pName)
    : LiveActor(pName), mAnimScaleController(), mSupportTicoStop(), mQuat(0.0f, 0.0f, 0.0f, 1.0f), mRotateAngle(::hRotateAngle), mWaitScale(1.0f), mNotBoundStep() {
}

void Balloonfish::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Balloonfish", false);
    MR::listenStageSwitchOnAppear(this, MR::FunctorV0M< Balloonfish*, void (Balloonfish::*)() >(this, &Balloonfish::appear));
    mQuat.setEuler(mRotation);
    initNerve(GET_NERVE(Balloonfish, HostTypeNrvWait));
    mAnimScaleController = new AnimScaleController(nullptr);
    mSupportTicoStop = new SupportTicoStop(this, mAnimScaleController);
    MR::invalidateClipping(this);
    makeActorDead();
}

void Balloonfish::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE(Balloonfish, HostTypeNrvWait));
}

void Balloonfish::appear() {
    LiveActor::appear();
    MR::emitEffect(this, "AppearFromBox");
    MR::startActionSound(this, "EmBlnfishAppear");
}

void Balloonfish::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmBlnfishExplosion");
}

void Balloonfish::control() {
    mSupportTicoStop->updateNerve();
    mAnimScaleController->updateNerve();
}

void Balloonfish::exeWait() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        mNotBoundStep = 0;
        MR::startAction(this, "appearance");
        MR::startActionSound(this, "EmBlnfishPreDash", -1, -1, -1);
    }

    if (mSupportTicoStop->tryAttachTouch()) {
        return;
    }

    TVec3f toPredictedPlayerPos(*MR::getPlayerCenterPos());
    toPredictedPlayerPos += *MR::getPlayerVelocity() * ::hTargetPlayerVelocityMult;
    toPredictedPlayerPos -= mPosition;

    f32 ratio = static_cast< f32 >(mNotBoundStep) / ::hWaitTime;
    mWaitScale = (1.0f - ratio) * 1.0f + ratio * ::hWaitMaxScale;
    MR::blendQuatFrontUp(&mQuat, -mGravity, toPredictedPlayerPos, 0.02f, 0.1f);

    if (mNotBoundStep > ::hWaitTime) {
        setNerve(GET_NERVE(Balloonfish, HostTypeNrvDash));
        return;
    }

    mNotBoundStep++;
}

void Balloonfish::exeDash() {
    if (MR::isFirstStep(this)) {
        mNotBoundStep = 0;
        if (isNerve(GET_NERVE(Balloonfish, HostTypeNrvDash))) {
            MR::startAction(this, "Attack");
            MR::emitEffect(this, "SwimBubble");
        } else {
            MR::startAction(this, "AttackEnd");
            MR::emitEffect(this, "SwimBubble");
        }

        TVec3f zDir;
        mQuat.getZDir(zDir);
        mVelocity.set(zDir * ::hDashVel);
    }

    MR::startActionSound(this, "EmLvBlnfishDash", -1, -1, -1);

    if (mSupportTicoStop->tryAttachTouch()) {
        return;
    }

    if (mSupportTicoStop->mIsTouching) {
        TVec3f zDir;
        mQuat.getZDir(zDir);
        mVelocity.set(zDir * ::hDashVel);
    }

    s32 nerveMaxDuration;
    if (isNerve(GET_NERVE(Balloonfish, HostTypeNrvDash))) {
        nerveMaxDuration = ::hDashTime;
        f32 ratio = MR::calcNerveRate(this, nerveMaxDuration);
        mWaitScale = (1.0f - ratio) * ::hWaitMaxScale + ratio * 1.0f;
    } else {
        nerveMaxDuration = ::hDashEndTime;
    }

    if (nerveMaxDuration < mNotBoundStep) {
        kill();
        return;
    }

    if (MR::isBindedWall(this)) {
        kill();
        return;
    }

    if (!MR::isInWater(*MR::getSensorPos(this, "Body"))) {
        kill();
        return;
    }

    mNotBoundStep++;
}

void Balloonfish::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
        return;
    }

    if (isNerve(GET_NERVE(Balloonfish, HostTypeNrvWait)) && MR::isLessStep(this, ::hAttackBeginTime)) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (mSupportTicoStop->isTouching()) {
        MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        return;
    }

    if (MR::sendMsgEnemyAttackMaximum(pReceiver, pSender) != true) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Balloonfish::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        if (!mSupportTicoStop->isTouching()) {
            mAnimScaleController->startHitReaction();
        }
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        MR::emitItemHit(this);
        kill();
        return true;
    }

    return false;
}

void Balloonfish::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.makeQuat(mQuat);

    TPos3f mtx2;
    mtx2.makeRotate(TVec3f(0.0f, 0.0f, 1.0f), mRotateAngle);
    mtx.concat(mtx, mtx2);

    mtx.setTrans(mPosition);
    MR::setBaseTRMtx(this, mtx);

    MR::updateBaseScale(this, mAnimScaleController);
    MR::setBaseScale(this, mScale * mWaitScale);
}
