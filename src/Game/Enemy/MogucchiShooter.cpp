#include "Game/Enemy/MogucchiShooter.hpp"
#include "Game/Util.hpp"
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
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE9F0(); }
        static Nrv2C00 sInstance;
    };
    Nrv2C00 Nrv2C00::sInstance ATTRIBUTE_WEAK;

    class Nrv2C04 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE8C0(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C04 sInstance;
    };
    Nrv2C04 Nrv2C04::sInstance ATTRIBUTE_WEAK;

    class Nrv2C08 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE820(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C08 sInstance;
    };
    Nrv2C08 Nrv2C08::sInstance ATTRIBUTE_WEAK;

    class Nrv2C0C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEB60(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C0C sInstance;
    };
    Nrv2C0C Nrv2C0C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C10 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE710(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C10 sInstance;
    };
    Nrv2C10 Nrv2C10::sInstance ATTRIBUTE_WEAK;

    class Nrv2C14 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CE7C0(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C14 sInstance;
    };
    Nrv2C14 Nrv2C14::sInstance ATTRIBUTE_WEAK;

    class Nrv2C18 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEA90(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C18 sInstance;
    };
    Nrv2C18 Nrv2C18::sInstance ATTRIBUTE_WEAK;

    class Nrv2C1C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->fn_801CEB10(static_cast< MogucchiShooter* >(pSpine->mExecutor)); }
        static Nrv2C1C sInstance;
    };
    Nrv2C1C Nrv2C1C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C20 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MogucchiShooter* pActor = static_cast< MogucchiShooter* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(pActor, pActor->mBindStarPointer, pActor->mRestoreNerve);
        }
        static Nrv2C20 sInstance;
    };
    Nrv2C20 Nrv2C20::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMogucchiShooter

#define MS_NRV(name) (&NrvMogucchiShooter::name::sInstance)

namespace {
    const f32 sTurnRate = 1.0f;
};

MogucchiShooter::MogucchiShooter(const char* pName) : LiveActor(pName) {
    mHomePos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    _A0 = 0.0f;
    _A4 = 1.0f;
    mUpVec.set(0.0f, 0.0f, 0.0f);
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
    mJointCtrl = nullptr;
    mHomingKiller = nullptr;
    mBindStarPointer = nullptr;
    mRestoreNerve = nullptr;
}

MogucchiShooter::~MogucchiShooter() {}

void MogucchiShooter::init(const JMapInfoIter& rIter) {
    MR::initRotation(&mRot, rIter);
    MR::initActor(this, rIter, "GliderBazooka", nullptr, false);
    s32 switchArg = 0;
    MR::getJMapInfoArg0NoInit(rIter, &switchArg);
    mLimit = switchArg;
    if (MR::isEqualObjectName(rIter, "KillerShooter")) {
        mSwitchKind = 1;
        mHomingKiller = new HomingKiller("ホーミングキラー");
        mHomingKiller->initWithoutIter();
        if (MR::isValidSwitchA(this)) {
            MR::listenStageSwitchOnOffA(this, MR::Functor(this, &MogucchiShooter::onStageSwitchOn), MR::Functor(this, &MogucchiShooter::onStageSwitchOff));
        }
    } else {
        mSwitchKind = 0;
        MR::createSceneObj(0x8D);
    }
    mBindStarPointer = nullptr;
    JointControlDelegator< MogucchiShooter >* pJoint = new JointControlDelegator< MogucchiShooter >(this, nullptr, nullptr);
    MR::setJointControllerParam(pJoint, this, "Propeller");
    mJointCtrl = pJoint;
    mSpike = new MogucchiSpike(this, "狙撃手", "MogucchiSpike", MR::getJointMtx(this, "Cockpit"), 0x13, false);
    mSpike->initWithoutIter();
    mSpike->makeActorDead();
    mBreakModel = MR::createModelObjEnemy("壊れモデル", "GliderBazookaBreak", getBaseMtx());
    if (MR::isValidSwitchA(this) && MR::isDead(mBreakModel)) {
        initNerve(MS_NRV(Nrv2C00), 1);
    } else {
        initNerve(MS_NRV(Nrv2C10), 1);
    }
    mBindStarPointer = new WalkerStateBindStarPointer(this, mSpike->mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, MS_NRV(Nrv2C20), "SupportFreeze");
    mLimit -= MR::getBckFrameMax(this);
    mLimit -= MR::getBckFrameMax(this);
    MR::startAction(this, "ShotWait");
    MR::startAction(mSpike, "ShotWait");
    makeActorAppeared();
    MR::extractMtxYDir(getBaseMtx(), &mUpVec);
    MR::extractMtxZDir(getBaseMtx(), &mFront);
    mHomePos.set(mPosition);
}

void MogucchiShooter::control() {
    mCounter += 1;
    if (!isNerve(MS_NRV(Nrv2C08)) && !isNerve(MS_NRV(Nrv2C0C))) {
        mTimer += 1;
    }
    f32 phase = (90.0f * (f32)(mTimer % 120)) / 120.0f;
    f32 swing;
    if (phase < 45.0f) {
        swing = -0.5f;
    } else {
        swing = 0.5f;
    }
    TVec3f sp8(mHomePos);
    sp8.scale(swing * 0.0f);
    TVec3f sp14(mPosition);
    sp14.add(sp8);
    bool v1 = true;
    bool v2 = true;
    bool v3 = true;
    if (!isNerve(MS_NRV(Nrv2C00)) && !isNerve(MS_NRV(Nrv2C08))) {
        v2 = false;
    }
    if (!v2 && !isNerve(MS_NRV(Nrv2C04))) {
        v3 = false;
    }
    if (!v3 && !isNerve(MS_NRV(Nrv2C10))) {
        v1 = false;
    }
    if (v1 && mBindStarPointer->tryStartPointBind()) {
        setNerve(MS_NRV(Nrv2C20));
    }
}

void MogucchiShooter::endClipped() {
    LiveActor::endClipped();
    if (mSwitchKind != 1 && !isNerve(MS_NRV(Nrv2C0C))) {
        fn_801CE480(&mFront, nullptr);
        mFront.normalize();
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

s32 MogucchiShooter::fn_801CE3A0(TPos3f* pMtx) {
    TVec3f ang(0.0f, 0.0f, 0.0f);
    ang.x = (sTurnRate * (f32)(mCounter % 20)) / 20.0f;
    TPos3f rot;
    rot.identity();
    PSMTXConcat(pMtx->mMtx, rot.mMtx, pMtx->mMtx);
    return 1;
}

void MogucchiShooter::fn_801CE460(LiveActor* pActor) {
    MR::invalidateClipping(pActor);
}

void MogucchiShooter::fn_801CE470(LiveActor* pActor) {
    MR::validateClipping(pActor);
}

void MogucchiShooter::fn_801CE480(TVec3f* pPos, TVec3f* pVel) {
    LiveActor* pRush = MR::getCurrentRushActor();
    if (pRush != nullptr) {
        if (pPos != nullptr) {
            pPos->set(pRush->mPosition);
        }
        if (pVel != nullptr) {
            pVel->set(pRush->mVelocity);
        }
    } else {
        if (pPos != nullptr) {
            pPos->set(*MR::getPlayerPos());
        }
        if (pVel != nullptr) {
            pVel->set(*MR::getPlayerVelocity());
        }
    }
}

void MogucchiShooter::fn_801CE530(TVec3f* pFront, const TVec3f& rPos) {
    TVec3f origin(0.0f, 0.0f, 0.0f);
    TVec3f target(0.0f, 0.0f, 0.0f);
    fn_801CE480(&origin, nullptr);
    TVec3f best(rPos);
    f32 bestDist = 1000.0f;
    TVec3f dirMul(0.0f, 0.0f, 0.0f);
    dirMul.scale(0.0f);
    for (s32 i = 0; i < 150; i++) {
        TVec3f cand(origin);
        cand.add(TVec3f(0.0f, 0.0f, 0.0f));
        f32 d = PSVECDistance(&cand, &rPos);
        if (d < bestDist) {
            best.set(cand);
            bestDist = d;
        }
    }
    TVec3f dir(best);
    dir.sub(rPos);
    MR::normalize(&dir);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::turnVecToVecDegree(pFront, *pFront, dir, 0.0f, up);
}

void MogucchiShooter::fn_801CE710(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mSpike, "Wait");
    }
    if (fn_801CEF20()) {
        fn_801CE530(&mFront, mPosition);
    }
    if (MR::isOnSwitchA(this)) {
        if (mSwitchKind == 1) {
            MR::validateClipping(this);
        }
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CE7C0(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "ShotSign");
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CE820(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        const char* pName = MR::getPlayingBckName(mSpike);
        MR::setBckFrameEndAndStop(this);
        if (!MR::isEqualString(pName, "Hide") && !MR::isEqualString(pName, "HideAttack")) {
            MR::startAction(mSpike, "Hide");
        }
    }
    fn_801CF120();
}

void MogucchiShooter::fn_801CE8C0(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        MR::tryHitReactionBreak(this);
        mBreakModel->makeActorAppeared();
        MR::startAction(mBreakModel, "Break");
        MR::startAction(mSpike, "Damage");
    }
    if (!MR::isDead(mBreakModel) && MR::isActionEnd(mBreakModel)) {
        mBreakModel->kill();
    }
    if (MR::isDead(mBreakModel) && MR::isDead(mSpike)) {
        kill();
    }
}

void MogucchiShooter::fn_801CE9F0() {
    s32 fn = fn_801CEF20();
    if (fn != 0) {
        fn_801CE530(&mFront, mPosition);
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Bazooka");
        MR::startAction(mSpike, "Bazooka");
    }
    if (fn_801CF0C0() == 0 && fn_801CF070(this) == 0) {
        fn_801CF1A0();
    }
}

void MogucchiShooter::fn_801CEA90(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
        MR::startAction(mSpike, "ShotStart");
        fn_801CEBE0("Bazooka");
    }
    if (MR::isActionEnd(this)) {
        setNerve(MS_NRV(Nrv2C18));
    }
}

void MogucchiShooter::fn_801CEB10(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "ShotSignWait");
    }
    fn_801CF220(pActor);
}

void MogucchiShooter::fn_801CEB60(LiveActor* pActor) {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
        MR::startAction(mSpike, "ShotStart");
        fn_801CECD0("Bazooka");
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::fn_801CEBE0(const char* pJointName) {
    if (mSwitchKind == 1) {
        TPos3f mtx(MR::getJointMtx(this, pJointName));
        TVec3f xDir;
        mtx.getXDir(xDir);
        TVec3f trans;
        mtx.getTrans(trans);
        MR::normalizeOrZero(&xDir);
        TVec3f pos(trans);
        pos.add(mSpike->mPosition);
        mHomingKiller->appear(pos, xDir);
    }
}

void MogucchiShooter::fn_801CECD0(const char* pJointName) {
    if (mSwitchKind == 0) {
        TVec3f xDir;
        MR::extractMtxXDir(MR::getJointMtx(this, pJointName), &xDir);
        MR::normalizeOrZero(&xDir);
        LiveActorGroup* pGroup = static_cast< LiveActorGroup* >(MR::getSceneObjHolder()->getObj(0x8D));
        LiveActor* pDead = pGroup->getDeadActor();
        if (pDead != nullptr) {
            MR::setPosition(pDead, mPosition);
            TVec3f vel(xDir);
            vel.scale(10.0f);
            MR::setVelocity(pDead, vel);
            pDead->makeActorAppeared();
        }
    }
}

s32 MogucchiShooter::fn_801CED80(const TVec3f& rPos) {
    TVec3f up;
    MR::extractMtxYDir(getBaseMtx(), &up);
    TVec3f sp8(rPos);
    sp8.sub(mPosition);
    TVec3f dir;
    MR::normalizeOrZero(&dir, sp8);
    TPos3f mtx;
    MR::makeMtxUpFront(&mtx, up, dir);
    f32 angleH = 0.0f;
    f32 angleV = 0.0f;
    angleH = MR::atan2(dir.x, dir.z);
    angleV = MR::atan2(dir.y, dir.z);
    mSpike->mPosition.set(mPosition);
    MR::setRotation(mSpike, TVec3f(angleV, angleH, 0.0f));
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
    return MR::isNearAngleDegreeHV(sp8, mFront, up, 30.0f, 30.0f);
}

s32 MogucchiShooter::fn_801CEFC0() {
    if (isNerve(MS_NRV(Nrv2C00)) || isNerve(MS_NRV(Nrv2C08)) || isNerve(MS_NRV(Nrv2C0C))) {
        mDelay = 70;
        setNerve(MS_NRV(Nrv2C04));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::fn_801CF070(LiveActor* pActor) {
    if (MR::isNearPlayer(pActor, 500.0f)) {
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
    if (!MR::isNearPlayer(this, 1000.0f)) {
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

s32 MogucchiShooter::fn_801CF220(LiveActor* pActor) {
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

void MogucchiShooter::onStageSwitchOn() {
    if (mSwitchKind == 1) {
        MR::validateClipping(this);
    }
    setNerve(MS_NRV(Nrv2C10));
}

void MogucchiShooter::onStageSwitchOff() {
    setNerve(MS_NRV(Nrv2C00));
}

// ---- MogucchiSpike ----

MogucchiSpike::MogucchiSpike(LiveActor* pActor, const char* pModel, const char* pName, MtxPtr pMtx, s32 drawType, bool isBaseMtx)
    : PartsModel(pActor, pModel, pName, pMtx, drawType, isBaseMtx) {
    mScaleCtrl = new AnimScaleController(nullptr);
}

void MogucchiSpike::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mScaleCtrl);
    PartsModel::calcAndSetBaseMtx();
}

void MogucchiSpike::control() {
    mScaleCtrl->updateNerve();
}

void MogucchiSpike::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::isSensorEnemyAttack(pSender)) {
            const char* pName = MR::getPlayingBckName(this);
            if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                if (MR::isEqualString(pName, "Bazooka") || MR::isEqualString(pName, "ShotStart")) {
                    MR::startAction(this, "Damage");
                }
            }
            return;
        }
    }
    MR::sendMsgPush(pReceiver, pSender);
}

bool MogucchiSpike::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return MR::sendMsgStarPieceAttack(pSender, pReceiver);
    }
    return false;
}
