#include "Game/Enemy/AttackRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"

#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

namespace NrvAttackRockFinal {
    class AttackRockFinalNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockFinalNrvWait sInstance;
    };
    class AttackRockFinalNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockFinalNrvFall sInstance;
    };
    class AttackRockFinalNrvFloat : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockFinalNrvFloat sInstance;
    };
    class AttackRockFinalNrvBound : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockFinalNrvBound sInstance;
    };
    class AttackRockFinalNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockFinalNrvBreak sInstance;
    };
    AttackRockFinalNrvWait AttackRockFinalNrvWait::sInstance ATTRIBUTE_WEAK;
    AttackRockFinalNrvFall AttackRockFinalNrvFall::sInstance ATTRIBUTE_WEAK;
    AttackRockFinalNrvFloat AttackRockFinalNrvFloat::sInstance ATTRIBUTE_WEAK;
    AttackRockFinalNrvBound AttackRockFinalNrvBound::sInstance ATTRIBUTE_WEAK;
    AttackRockFinalNrvBreak AttackRockFinalNrvBreak::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRockFinal


AttackRockFinal::AttackRockFinal(const char* pName)
    : LiveActor(pName), mProjmapEffectMtxSetter(nullptr), mBreakModel(nullptr), mRide(nullptr), mStartPos(0.0f, 0.0f, 0.0f),
      mAxisF8(1.0f, 0.0f, 0.0f), mAxis104(0.0f, -1.0f, 0.0f), mWaitStep(0), mIsOnPlayer(false) {
    mQuatDC.set(0.0f, 0.0f, 0.0f, 1.0f);
    mQuatCC.set(0.0f, 0.0f, 0.0f, 1.0f);
    mMtx9C.identity();
}

void AttackRockFinal::init(const JMapInfoIter& rIter) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->addAttackRockFinal(this);
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatCC, this);
    mQuatDC.x = mQuatCC.x;
    mQuatDC.y = mQuatCC.y;
    mQuatDC.z = mQuatCC.z;
    mQuatDC.w = mQuatCC.w;
    mStartPos.set(mPosition);
    mQuatDC.getXDir(mAxisF8);
    mQuatDC.getYDir(mAxis104);
    mProjmapEffectMtxSetter = fn_80155660(this);
    MR::invalidateShadowAll(this);
    fn_801552A0(this);
    MR::setEffectHostMtx(this, "Attack", mMtx9C.toMtxPtr());
    MR::setSensorFollowPos(this, "Land", &mPosition);
    mBreakModel = MR::createBreakModel(this, nullptr);
    MR::invalidateClipping(mBreakModel);
    MR::declareCoin(this, 16);
    MR::getJMapInfoArg0NoInit(rIter, &mWaitStep);
    mRide = new AttackRockRide(this);
    BaseMatrixFollowValidater* pValidater = MR::createBaseMatrixFollowValidateDelegator(this, &AttackRockFinal::fn_80152700);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    MR::addKoopaBattleReceiverSensor(getSensor("Body"));
    initNerve(&NrvAttackRockFinal::AttackRockFinalNrvWait::sInstance, 0);
    makeActorDead();
}

void AttackRockFinal::makeActorDead() {
    LiveActor::makeActorDead();
    mRide->fn_80152F20();
    mIsOnPlayer = false;
}

// NONMATCHING: Vector temporary stack slots and GPR allocation differ.
void AttackRockFinal::control() {
    TVec3f dir;
    MR::getKoopaBattleTargetPos(&dir);
    TVec3f toPlayer = mPosition - dir;
    MR::normalizeOrZero(&dir, toPlayer);
    MR::turnQuatZDirRate(&mQuatCC, mQuatCC, dir, 0.3f);
    TVec3f zdir;
    mQuatCC.getZDir(zdir);
    mRide->fn_801529E0(&zdir);
    mRide->fn_80152C00();
}

void AttackRockFinal::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    fn_80155540(mProjmapEffectMtxSetter, &mQuatCC, &mPosition);
}

void AttackRockFinal::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
        isNerve(&NrvAttackRockFinal::AttackRockFinalNrvBound::sInstance) && MR::sendMsgAttackRockReflect(pReceiver, pSender)) {
        setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBreak::sInstance);
        MR::startBrk(mBreakModel, "Inner");
    }
}

bool AttackRockFinal::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance)) {
        fn_80155360(&mVelocity, &mQuatCC, pSender, pReceiver, 250.0f);
        setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRockFinal::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaFinalBattleStart(msg)) {
        setNerve(&NrvAttackRockFinal::AttackRockFinalNrvWait::sInstance);
        makeActorAppeared();
        MR::hideModelAndOnCalcAnim(this);
        return true;
    }
    if (MR::isMsgKoopaC6(msg)) {
        makeActorDead();
        return true;
    }
    if (mRide->fn_80152E20(msg, pSender, pReceiver)) {
        return true;
    }
    if (MR::isMsgKoopaPunchAttack(msg) && MR::isEqualSensor(pReceiver, this, "Body") &&
        isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance)) {
        MR::appearCoinPop(this, mPosition, 1);
        setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBreak::sInstance);
        MR::startBrk(mBreakModel, "Outer");
        return false;
    }
    return false;
}

void AttackRockFinal::fn_801521F0(f32 a, f32 b) {
    this->mPosition.set(this->mStartPos);
    TVec3f va = this->mAxisF8 * a;
    this->mPosition.add(va);
    TVec3f vb = this->mAxis104 * b;
    this->mPosition.add(vb);
}

void AttackRockFinal::fn_80152280() {
    if (MR::isFirstStep(this)) {
        this->fn_801521F0(800.0f, 2000.0f);
    }
    this->mIsOnPlayer = false;
    if (MR::isGreaterStep(this, this->mWaitStep)) {
        this->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvFall::sInstance);
    }
}

void AttackRockFinal::fn_801522F0() {
    f32 y = MR::calcNerveEaseOutValue(this, 100, 2000.0f, -1000.0f);
    f32 x = MR::calcNerveEaseInValue(this, 100, 800.0f, 400.0f);
    this->fn_801521F0(x, y);
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::startAction(this, "FinalWait");
        MR::showModel(this);
        MR::validateCollisionParts(this);
        MR::validateHitSensors(this);
        MR::resetPosition(this);
    }
    if (MR::isOnGroundPlayer()) {
        this->mIsOnPlayer = MR::isActorOnPlayer(this);
    }
    MR::startActionSound(this, "FinalFall", -1, -1, -1);
    if (MR::isGreaterEqualStep(this, 99)) {
        this->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance);
    }
}

void AttackRockFinal::fn_80152400() {
    f32 y = MR::calcNerveEaseInOutValue(this, 150, -1000.0f, 0.0f);
    f32 x = MR::calcNerveEaseOutValue(this, 150, 400.0f, 0.0f);
    this->fn_801521F0(x, y);
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FinalFloat");
        this->mRide->fn_80152F10();
    }
    if (MR::isOnGroundPlayer()) {
        this->mIsOnPlayer = MR::isActorOnPlayer(this);
    }
    MR::startActionSound(this, "FinalPower", -1, -1, 18);
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(this)) {
        this->mRide->fn_80152F10();
    }
}

void AttackRockFinal::fn_801524F0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::onBind(this);
        this->mRide->fn_80152F20();
        this->mIsOnPlayer = false;
    }
    fn_801554D0(&this->mMtx9C, this);
    if (MR::isBinded(this)) {
        MR::sendMsgToBindedAttackRockReflect(this, getSensor("Body"));
    }
    if (MR::reboundVelocityFromCollision(this, 0.8f, 0.0f, 1.0f)) {
        MR::emitEffectHit(this, MR::getBindedHitPos(this), MR::getBindedFixReactionVector(this), "GroundHit");
        MR::startActionSound(this, "Bound", -1, -1, -1);
        MR::attenuateVelocity(this, 0.7f);
    }
    if (MR::isGreaterStep(this, 90)) {
        this->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBreak::sInstance);
    }
}

void AttackRockFinal::fn_80152620() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::startActionSound(this, "FinalBreak", -1, -1, -1);
        this->mBreakModel->appear();
        MR::startBrk(this->mBreakModel, "Inner");
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        MR::zeroVelocity(this);
        MR::offBind(this);
        this->mRide->fn_80152F20();
    }
    if (MR::isDead(this->mBreakModel)) {
        this->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvFall::sInstance);
    }
}

bool AttackRockFinal::fn_80152700(s32) const {
    const AttackRockFinal* pActor = this;
    if (pActor->isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFall::sInstance) ||
        pActor->isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance)) {
        return true;
    }
    return false;
}

AttackRockFinal::~AttackRockFinal() {
}

namespace NrvAttackRockFinal {
    inline void AttackRockFinalNrvWait::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor))->fn_80152280();
    }
    inline void AttackRockFinalNrvFall::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor))->fn_801522F0();
    }
    inline void AttackRockFinalNrvFloat::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor))->fn_80152400();
    }
    inline void AttackRockFinalNrvBound::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor))->fn_801524F0();
    }
    inline void AttackRockFinalNrvBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor))->fn_80152620();
    }
};  // namespace NrvAttackRockFinal

// ---- AttackRockRide: lets the player ride the rock (rush handling) ----

