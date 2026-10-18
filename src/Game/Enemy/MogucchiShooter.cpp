#include "Game/Enemy/MogucchiShooter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"

// Nerves of the shooter: SMG2 target instances 0x807D2C00..0x807D2C20. Each execute runs the
// fn_ body below (binding decoded from the setNerve calls; see renames file for the names).
namespace NrvMogucchiShooter {
    class Nrv2C00 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE710(); }
        static Nrv2C00 sInstance;
    };
    Nrv2C00 Nrv2C00::sInstance ATTRIBUTE_WEAK;

    class Nrv2C04 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE820(); }
        static Nrv2C04 sInstance;
    };
    Nrv2C04 Nrv2C04::sInstance ATTRIBUTE_WEAK;

    class Nrv2C08 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE7C0(); }
        static Nrv2C08 sInstance;
    };
    Nrv2C08 Nrv2C08::sInstance ATTRIBUTE_WEAK;

    class Nrv2C0C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE8C0(); }
        static Nrv2C0C sInstance;
    };
    Nrv2C0C Nrv2C0C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C10 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE9F0(); }
        static Nrv2C10 sInstance;
    };
    Nrv2C10 Nrv2C10::sInstance ATTRIBUTE_WEAK;

    class Nrv2C14 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEA90(); }
        static Nrv2C14 sInstance;
    };
    Nrv2C14 Nrv2C14::sInstance ATTRIBUTE_WEAK;

    class Nrv2C18 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEB10(); }
        static Nrv2C18 sInstance;
    };
    Nrv2C18 Nrv2C18::sInstance ATTRIBUTE_WEAK;

    class Nrv2C1C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEB60(); }
        static Nrv2C1C sInstance;
    };
    Nrv2C1C Nrv2C1C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C20 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MogucchiShooter* pActor = static_cast< MogucchiShooter* >(pSpine->mExecutor);
            if (MR::updateActorState(pActor) && !pActor->fn_801CF070()) {
                pActor->setNerve(&Nrv2C10::sInstance);
            }
        }
        static Nrv2C20 sInstance;
    };
    Nrv2C20 Nrv2C20::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMogucchiShooter

#define MS_NRV(name) (&NrvMogucchiShooter::name::sInstance)

namespace {
    TVec3f sPropellerAxis(1.0f, 0.0f, 0.0f);
    TVec3f sShotOffset(-100.0f, 5.0f, 0.0f);
    s32 sSwingPeriod = 240;
    extern const f32 sViewAngle;
    extern const f32 sViewHalf;
};

MogucchiShooter::MogucchiShooter(const char* pName) : LiveActor(pName) {
    mHomePos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mFront.z = 1.0f;
    mRot.set(0.0f, 0.0f, 1.0f);
    mTimer = 0;
    mCounter = 0;
    mStep = 0;
    mLimit = 90;
    mSwitchKind = 0;
    mFlag = 1;
    mDelay = 0;
    mSpike = nullptr;
    mBreakModel = nullptr;
    mHomingKiller = nullptr;
    mBindStarPointer = nullptr;
}

MogucchiShooter::~MogucchiShooter() {}

// NONMATCHING: callback stack placement and string-pool address-load scheduling; calls, literals and field offsets verified.
void MogucchiShooter::init(const JMapInfoIter& rIter) {
    MR::initRotation(&mRot, rIter);
    MR::initActor(this, rIter, "GliderBazooka", false);
    const char* objectName;
    MR::getObjectName(&objectName, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mLimit);
    if (MR::isEqualObjectName(rIter, "KillerShooter")) {
        mSwitchKind = 1;
        mHomingKiller = new HomingKiller("ホーミングキラー");
        mHomingKiller->initWithoutIter();
        mHomingKiller->makeActorDead();
        if (MR::isValidSwitchA(this)) {
            MR::listenStageSwitchOnOffA(this, MR::Functor(this, &MogucchiShooter::fn_801CE460), MR::Functor(this, &MogucchiShooter::fn_801CE470));
        }
    } else {
        mSwitchKind = 0;
        MR::createSceneObj(0x8D);
    }
    bool (MogucchiShooter::*callback)(TPos3f*, const JointControllerInfo&) = &MogucchiShooter::fn_801CE3A0;
    const char* jointName = "Propeller";
    JointControlDelegator< MogucchiShooter >* pJoint = new JointControlDelegator< MogucchiShooter >(this, callback, nullptr);
    MR::setJointControllerParam(pJoint, this, jointName);
    mJointCtrl = pJoint;
    MogucchiSpike* spike = new MogucchiSpike(this, MR::getJointMtx(this, "Cockpit"));
    spike->initWithoutIter();
    reinterpret_cast<u8*>(spike)[0x9D] = 1;
    mSpike = spike;
    mBreakModel = MR::createModelObjEnemy("壊れモデル", "GliderBazookaBreak", getBaseMtx());
    mBreakModel->makeActorDead();
    if (MR::isValidSwitchA(this)) {
        initNerve(MS_NRV(Nrv2C00), 1);
    } else {
        initNerve(MS_NRV(Nrv2C10), 1);
    }
    mBindStarPointer = new WalkerStateBindStarPointer(this, mSpike->mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, MS_NRV(Nrv2C20), "SupportFreeze");
    MR::startAction(this, "ShotSign");
    mLimit -= MR::getBckFrameMax(this);
    MR::startAction(this, "ShotStart");
    mLimit -= MR::getBckFrameMax(this);
    MR::startAction(this, "ShotWait");
    MR::startAction(mSpike, "ShotWait");
    makeActorAppeared();
    MR::extractMtxYDir(getBaseMtx(), &mUpVec);
    MR::extractMtxZDir(getBaseMtx(), &mFront);
    mHomePos.set(mPosition);
}

#pragma auto_inline on
// NONMATCHING: integer and boolean register allocation differs; branch structure matches.
void MogucchiShooter::control() {
    mStep++;
    if (!isNerve(MS_NRV(Nrv2C14)) && !isNerve(MS_NRV(Nrv2C18)) && !isNerve(MS_NRV(Nrv2C1C)) && !isNerve(MS_NRV(Nrv2C0C))) {
        mTimer++;
    }
    f32 phase = 360.0f * (mTimer % sSwingPeriod) / sSwingPeriod;
    mPosition = mHomePos + mUpVec * (20.0f * MR::sinDegree(phase));
    bool v1 = true;
    bool v2 = true;
    bool v3 = true;
    if (!isNerve(MS_NRV(Nrv2C00)) && !isNerve(MS_NRV(Nrv2C08))) { v2 = false; }
    if (!v2 && !isNerve(MS_NRV(Nrv2C04))) { v3 = false; }
    if (!v3 && !isNerve(MS_NRV(Nrv2C10))) { v1 = false; }
    if (v1 && mBindStarPointer->tryStartPointBind()) setNerve(MS_NRV(Nrv2C20));

}

#pragma auto_inline off

void MogucchiShooter::endClipped() {
    LiveActor::endClipped();
    if (mSwitchKind != 1 && !isNerve(MS_NRV(Nrv2C0C))) {
        fn_801CE480(&mRot, nullptr);
        mRot.sub(mPosition);
        MR::normalize(&mRot);
    }
}

void MogucchiShooter::calcAndSetBaseMtx() {
    mJointCtrl->registerCallBack();
    MR::fn_8000F880(this, &mRot);
}

void MogucchiShooter::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool MogucchiShooter::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiProjectileOrEat(msg)) {
        return fn_801CED80(pSender->mPosition);
    }
    if (MR::isMsgStarPieceReflect(msg) || MR::isMsgStarPieceAttack(msg)) {
        fn_801CEFC0();
        return true;
    }
    return false;
}

bool MogucchiShooter::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return fn_801CED80(pSender->mPosition);
    }
    return false;
}

bool MogucchiShooter::fn_801CE3A0(TPos3f* pMtx, const JointControllerInfo&) {
    s32 period = 20;
    TVec3f angle(sPropellerAxis * (360.0f * (mStep % period) / period));
    TPos3f rot;
    PSMTXIdentity(rot.mMtx);
    MR::makeMtxRotate(rot.mMtx, angle);
    PSMTXConcat(pMtx->mMtx, rot.mMtx, pMtx->mMtx);
    return true;

}

void MogucchiShooter::fn_801CE460() {
    MR::invalidateClipping(this);
}

void MogucchiShooter::fn_801CE470() {
    MR::validateClipping(this);
}

void MogucchiShooter::fn_801CE480(TVec3f* pPos, TVec3f* pVel) {
    if (MR::getCurrentRushActor()) {
        if (pPos) pPos->set(MR::getCurrentRushActor()->mPosition);
        if (pVel) pVel->set(MR::getCurrentRushActor()->mVelocity);
    } else {
        if (pPos) pPos->set(*MR::getPlayerPos());
        if (pVel) pVel->set(*MR::getPlayerVelocity());
    }

}

// NONMATCHING: vector temporary stack slots and f29/f30 allocation differ; calls, constants and branches match.
void MogucchiShooter::fn_801CE530(TVec3f* pFront, const TVec3f& rPos) {
    TVec3f origin(0.0f);
    TVec3f velocity(0.0f);
    fn_801CE480(&origin, &velocity);
    TVec3f best(rPos);
    TVec3f muzzle(rPos + *pFront * 800.0f);
    f32 bestDist = 3.4028234663852886e+38f;
    for (s32 i = 0; i < 150; i++) {
        TVec3f candidate(origin + velocity * i);
        TVec3f direction(candidate);
        direction.sub(muzzle);
        MR::normalize(&direction);
        TVec3f shot(muzzle + (direction * 22.0f) * i);
        f32 distance = PSVECDistance(&shot, &candidate);
        if (distance < bestDist) {
            best.set(candidate);
            bestDist = distance;
        }
    }
    TVec3f dir(best);
    dir.sub(rPos);
    MR::normalize(&dir);
    TVec3f up;
    up.set(0.0f, 1.0f, 0.0f);
    MR::turnVecToVecDegree(pFront, *pFront, dir, 0.3f, up);

}

void MogucchiShooter::fn_801CE710() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mSpike, "Wait");
    }
    if (fn_801CEF20()) {
        fn_801CE530(&mRot, mPosition);
    }
    if (MR::isOnSwitchA(this)) {
        if (mSwitchKind == 1) {
            MR::validateClipping(this);
        }
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CE7C0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "Appear");
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CE820() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::setBckFrameEndAndStop(this);
        const char* pName = MR::getPlayingBckName(mSpike);
        if (!MR::isEqualString(pName, "Hide") && !MR::isEqualString(pName, "HideAttack")) {
            MR::startAction(mSpike, "Hide");
        }
    }
    fn_801CF120();
}

void MogucchiShooter::fn_801CE8C0() {
    if (MR::isFirstStep(this)) {
        MR::emitItemHit(this);
        MR::tryHitReactionBreak(this);
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        mBreakModel->makeActorAppeared();
        MR::startAction(mBreakModel, "Break");
        MR::startAction(mSpike, "Damage");
    }
    if (!MR::isDead(mBreakModel) && MR::isActionEnd(mBreakModel)) {
        mBreakModel->makeActorDead();
    }
    if (!MR::isDead(mSpike) && MR::isActionEnd(mSpike)) {
        mSpike->makeActorDead();
    }
    if (MR::isDead(mBreakModel) && MR::isDead(mSpike)) {
        makeActorDead();
    }
}

void MogucchiShooter::fn_801CE9F0() {
    s32 fn = fn_801CEF20();
    if (fn != 0) {
        fn_801CE530(&mRot, mPosition);
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotWait");
        MR::startAction(mSpike, "ShotWait");
    }
    if (fn_801CF0C0() == 0 && fn_801CF070() == 0) {
        if (fn_801CF1A0()) return;
    }
}

void MogucchiShooter::fn_801CEA90() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotSign");
        MR::startAction(mSpike, "ShotSign");
        fn_801CEBE0();
    }
    if (MR::isActionEnd(this)) {
        setNerve(MS_NRV(Nrv2C18));
    }
}

// NONMATCHING: epilogue restores LR and r31 in reverse order.
void MogucchiShooter::fn_801CEB10() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "ShotSignWait");
    }
    fn_801CF220();
}

void MogucchiShooter::fn_801CEB60() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
        MR::startAction(mSpike, "ShotStart");
        fn_801CECD0();
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CEBE0() {
    if (mSwitchKind == 1) {
        TPos3f mtx;
        mtx.set(MR::getJointMtx(this, "Bazooka"));
        TVec3f direction;
        TVec3f trans;
        TVec3f offset;
        MR::extractMtxXDir(mtx.mMtx, &direction);
        MR::extractMtxTrans(mtx.mMtx, &trans);
        MR::setMtxTrans(mtx.mMtx, 0.0f, 0.0f, 0.0f);
        mtx.mult(sShotOffset, offset);
        MR::normalizeOrZero(&direction);
        mHomingKiller->appear(trans + offset, direction);
    }

}

void MogucchiShooter::fn_801CECD0() {
    if (mSwitchKind == 0) {
        MtxPtr mtx = MR::getJointMtx(this, "Bazooka");
        TVec3f direction;
        TVec3f trans;
        MR::extractMtxXDir(mtx, &direction);
        MR::extractMtxTrans(mtx, &trans);
        MR::normalizeOrZero(&direction);
        LiveActorGroup* group = static_cast<LiveActorGroup*>(MR::getSceneObjHolder()->getObj(0x8D));
        LiveActor* actor = group->getDeadActor();
        if (actor) {
            MR::setPosition(actor, trans);
            MR::setVelocity(actor, direction * 22.0f);
            actor->appear();
        }
    }

}

bool MogucchiShooter::fn_801CED80(const TVec3f& rPos) {
    TPos3f mtx;
    TVec3f dir;
    TVec3f up;
    TVec3f rotation;
    MR::extractMtxYDir(getBaseMtx(), &up);
    const TVec3f& spikePosition = mSpike->mPosition;
    TVec3f difference(rPos);
    difference.sub(spikePosition);
    MR::normalizeOrZero(&dir, difference);
    MR::makeMtxUpFront(&mtx, up, dir);
    mtx.getEuler(rotation);
    reinterpret_cast<u8*>(mSpike)[0x9C] = 0;
    MR::setRotation(mSpike, 57.295780181884766f * rotation.x, 57.295780181884766f * rotation.y, 57.295780181884766f * rotation.z);
    MR::invalidateHitSensors(mSpike);
    setNerve(MS_NRV(Nrv2C0C));
    return 1;

}

s32 MogucchiShooter::fn_801CEF20() {
    if (mSwitchKind == 0) {
        return 1;
    }
    TVec3f up;
    MR::extractMtxYDir(getBaseMtx(), &up);
    TVec3f origin;
    fn_801CE480(&origin, nullptr);
    TVec3f sp8(origin);
    sp8.sub(mPosition);
    return MR::isNearAngleDegreeHV(sp8, mFront, up, sViewAngle * sViewHalf, 360.0f);
}

s32 MogucchiShooter::fn_801CEFC0() {
    bool allowed = isNerve(MS_NRV(Nrv2C00)) || isNerve(MS_NRV(Nrv2C08)) || isNerve(MS_NRV(Nrv2C04)) || isNerve(MS_NRV(Nrv2C10));
    if (allowed) {
        mDelay = 70;
        setNerve(MS_NRV(Nrv2C04));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF070() {
    if (MR::isNearPlayer(this, 1000.0f)) {
        setNerve(MS_NRV(Nrv2C04));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF0C0() {
    bool isValid = MR::isValidSwitchA(this);
    if (isValid && !MR::isOnSwitchA(this)) {
        setNerve(MS_NRV(Nrv2C00));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF120() {
    if (mDelay > 0) {
        mDelay -= 1;
        if (mDelay > 0) {
            return 0;
        }
    }
    if (!MR::isNearPlayer(this, 1200.0f)) {
        setNerve(MS_NRV(Nrv2C08));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF1A0() {
    if (mSwitchKind != 1 || MR::isDead(mHomingKiller)) {
        mCounter += 1;
    }
    if (mCounter >= mLimit) {
        mCounter = 0;
        setNerve(MS_NRV(Nrv2C14));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF220() {
    bool done;
    if (mSwitchKind == 1) {
        done = !mHomingKiller->isNerveAppear();
    } else {
        done = MR::isBckLooped(mSpike);
    }
    if (done) {
        setNerve(MS_NRV(Nrv2C1C));
        return 1;
    }
    return 0;
}

// ---- MogucchiSpike ----


void MogucchiSpike::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mScaleCtrl);
    PartsModel::calcAndSetBaseMtx();
}

void MogucchiSpike::control() {
    mScaleCtrl->updateNerve();
}

#pragma pool_strings off
inline void MogucchiSpike::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        const char* name = MR::getPlayingBckName(this);
        if (MR::isEqualString(name, "Hide") || MR::isEqualString(name, "HideAttack")) {
            MR::startAction(this, "HideAttack");
        }
        return;
    }
    MR::sendMsgPush(pReceiver, pSender);
}

#pragma pool_strings reset

bool MogucchiSpike::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return MR::sendMsgStarPieceAttack(pSender, pReceiver);
    }
    return false;
}

namespace {
    const f32 sViewAngle = 90.0f;
    const f32 sViewHalf = 0.5f;
}

#pragma pool_strings off
inline MogucchiSpike::MogucchiSpike(LiveActor* pActor, MtxPtr pMtx)
    : PartsModel(pActor, "狙撃手", "MogucchiSpike", pMtx, 0x14, false) {
    mScaleCtrl = new AnimScaleController(nullptr);
}
#pragma pool_strings reset

