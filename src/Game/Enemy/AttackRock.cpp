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

namespace NrvAttackRock {
    class AttackRockNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockNrvFall sInstance;
    };
    class AttackRockNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockNrvWait sInstance;
    };
    class AttackRockNrvHold : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockNrvHold sInstance;
    };
    class AttackRockNrvBound : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockNrvBound sInstance;
    };
    class AttackRockNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockNrvBreak sInstance;
    };
    AttackRockNrvFall AttackRockNrvFall::sInstance ATTRIBUTE_WEAK;
    AttackRockNrvWait AttackRockNrvWait::sInstance ATTRIBUTE_WEAK;
    AttackRockNrvHold AttackRockNrvHold::sInstance ATTRIBUTE_WEAK;
    AttackRockNrvBound AttackRockNrvBound::sInstance ATTRIBUTE_WEAK;
    AttackRockNrvBreak AttackRockNrvBreak::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRock


AttackRock::AttackRock(const char* pName)
    : LiveActor(pName), mQuatF0(0.0f, 0.0f, 0.0f, 1.0f), mQuat100(0.0f, 0.0f, 0.0f, 1.0f),
      mBreakModel(nullptr), mDemoObj(nullptr), mProjmapEffectMtxSetter(nullptr), mRockNum(1), mIsOnPlayer(false) {
    mMtx90.identity();
    mMtxC0.identity();
    mQuat100.set(0.0f, 0.0f, 0.0f, 1.0f);
}

// 80150xxx
void AttackRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatF0, this);
    mProjmapEffectMtxSetter = createProjmapEffectMtxSetter(this);
    MR::setEffectHostMtx(this, "Attack", mMtxC0.toMtxPtr());
    MR::addKoopaBattleReceiverSensor(getSensor("Body"));
    BaseMatrixFollowValidater* pValidater = MR::createBaseMatrixFollowValidateDelegator(this, &AttackRock::isRockFollowValid);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    setupRockBinder(this);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    mBreakModel = MR::createBreakModel(this, getBaseMtx());
    mDemoObj = reinterpret_cast< LiveActor* >(MR::createDemoObj("\220\267\202\350\223y\203\202\203f\203\213", "AttackRockHill", nullptr));
    mDemoObj->makeActorDead();
    mRockNum = 1;
    MR::getJMapInfoArg0WithInit(rIter, &mRockNum);
    if (mRockNum < 1) {
        mRockNum = 1;
    }
    initNerve(&NrvAttackRock::AttackRockNrvWait::sInstance, 2);
    mStateFall = new AttackRockStateFall(this, mRockNum);
    MR::initActorState(this, mStateFall, &NrvAttackRock::AttackRockNrvFall::sInstance, "KoopaFall");
    mStateHold = new AttackRockStateHold(this, mRockNum);
    MR::initActorState(this, mStateHold, &NrvAttackRock::AttackRockNrvHold::sInstance, "Float");
    MR::invalidateClipping(this);
    makeActorDead();
}

void AttackRock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
}

void AttackRock::makeActorDead() {
    LiveActor::makeActorDead();
    mDemoObj->makeActorDead();
}

void AttackRock::control() {
    if (MR::isOnGroundPlayer()) {
        if (MR::isActorOnPlayer(this)) {
            mIsOnPlayer = true;
        } else {
            mIsOnPlayer = false;
        }
    }
    updateBaseMtxFromVelocity(&mMtxC0, this);
}

void AttackRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatF0);
    updateProjmapEffectMtx(mProjmapEffectMtxSetter, &mQuat100, &mPosition);
}

void AttackRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
        isNerve(&NrvAttackRock::AttackRockNrvBound::sInstance)) {
        MR::sendMsgKoopaPunchAttack(pReceiver, pSender);
        if (MR::sendMsgAttackRockReflect(pReceiver, pSender)) {
            this->startRockBreak();
            return;
        }
    }
    if (isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        mStateFall->attackSensor(pSender, pReceiver);
    }
}

f32 AttackRock::getRockBaseRadius() const {
    return 300.0f;
}

bool AttackRock::isOuterState() const {
    const AttackRock* pActor = this;
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance) || pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return true;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && !(pActor->mStateHold)->isPowerOrFloatEnd()) {
        return true;
    }
    return false;
}

void AttackRock::startRockBreak() {
    bool outer = isOuterState();
    const char* pName = "Inner";
    if (outer) {
        pName = "Outer";
    }
    MR::startBrk(this->mBreakModel, pName);
    this->setNerve(&NrvAttackRock::AttackRockNrvBreak::sInstance);
}

bool AttackRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && mStateHold->isPowerOrFloatEnd()) {
        calcBoundVelocity(&mVelocity, &mQuat100, pSender, pReceiver, 120.0f);
        setNerve(&NrvAttackRock::AttackRockNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaCastAttackRockStart(msg)) {
        if (MR::isDead(this)) {
            MR::getKoopaBattlePos(&mPosition);
            MR::resetPosition(this);
            MR::zeroVelocity(this);
            setNerve(&NrvAttackRock::AttackRockNrvFall::sInstance);
            MR::offBind(this);
            MR::hideModel(this);
            appear();
            return true;
        }
        return false;
    }
    if (MR::isMsgKoopaC8(msg)) {
        return !MR::isDead(this);
    }
    if (MR::isMsgKoopaBA(msg)) {
        mStateFall->receiveOtherMsg(msg, pSender, pReceiver);
        mStateHold->receiveOtherMsg(msg, pSender, pReceiver);
        mBreakModel->makeActorDead();
        mDemoObj->makeActorDead();
        makeActorDead();
        return true;
    }
    if (MR::isMsgKoopaBB(msg) && isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        this->startRockBreak();
        return true;
    }
    if (isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        return mStateFall->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance)) {
        return mStateHold->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (MR::isMsgKoopaPunchAttack(msg) && MR::isEqualSensor(pReceiver, this, "Body") &&
        !isNerve(&NrvAttackRock::AttackRockNrvBreak::sInstance)) {
        MR::appearCoinPopKoopaBattle(mPosition);
        this->startRockBreak();
        return true;
    }
    if (MR::isMsgKoopaReleaseAttackRock(msg) && isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        setNerve(&NrvAttackRock::AttackRockNrvHold::sInstance);
        return true;
    }
    return false;
}

namespace NrvAttackRock {
    inline void AttackRockNrvFall::execute(Spine* pSpine) const {
        AttackRock* pActor = reinterpret_cast< AttackRock* >(pSpine->mExecutor);
        MR::startActionSound(pActor, "PgFall", -1, -1, -1);
        MR::updateActorStateAndNextNerve(pActor, &AttackRockNrvWait::sInstance);
    }
    inline void AttackRockNrvWait::execute(Spine* pSpine) const {
        AttackRock* pActor = reinterpret_cast< AttackRock* >(pSpine->mExecutor);
        if (MR::isFirstStep(pActor)) {
            MR::startAction(pActor, "Wait");
            MR::invalidateShadow(pActor, nullptr);
        }
    }
}

// NONMATCHING: String-pool addends differ; resolved strings and instructions otherwise agree.
void AttackRock::exeRockBound() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::onBind(this);
        MR::invalidateCollisionParts(this);
    }
    if (MR::isBinded(this)) {
        MR::sendMsgToBindedAttackRockReflect(this, getSensor("Body"));
        if (!MR::sendMsgToBindedKoopaC7(this, this->getSensor("Body"))) {
            MR::appearCoinPopKoopaBattle(mPosition);
            this->startRockBreak();
            return;
        }
    }
    f32 dist = MR::calcKoopaBattleDistance(this->mPosition);
    MR::setShadowVolumeSphereRadius(this, nullptr, MR::getLinerValueFromMinMax(dist, 700.0f, 2100.0f, 285.0f * mScale.y, 15.0f * mScale.y));
    TVec3f up;
    TVec3f pos;
    MR::calcKoopaBattleDirFromBase(&up, mPosition);
    MR::calcKoopaBattleEdgePos(&pos, mPosition);
    MR::setShadowProjection(this, nullptr, pos, up, dist < 2100.0f);
    if (MR::reboundVelocityFromCollision(this, 0.8f, 0.0f, 1.0f)) {
        MR::emitEffectHit(this, MR::getBindedHitPos(this), MR::getBindedFixReactionVector(this), "GroundHit");
        MR::startActionSound(this, "Bound", -1, -1, -1);
        MR::attenuateVelocity(this, 0.7f);
    }
    if (MR::isGreaterStep(this, 120)) {
        this->startRockBreak();
    }
}

void AttackRock::exeRockBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        this->mBreakModel->appear();
        this->mDemoObj->kill();
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        MR::invalidateShadow(this, nullptr);
        MR::zeroVelocity(this);
    }
    if (MR::isStep(this, 1)) {
        MR::hideModelAndOnCalcAnim(this);
    }
    if (MR::isGreaterStep(this, 10)) {
        this->kill();
    }
}

bool AttackRock::isRockFollowValid(s32 arg) const {
    const AttackRock* pActor = this;
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return arg != 0;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance)) {
        const AttackRock* pRock = pActor;
        if (arg == 0 && !pRock->mIsOnPlayer) {
            return false;
        }
        return pRock->mStateHold->isFloating();
    }
    return false;
}

AttackRock::~AttackRock() {
}

namespace NrvAttackRock {
    inline void AttackRockNrvHold::execute(Spine* pSpine) const {
        AttackRock* pActor = reinterpret_cast< AttackRock* >(pSpine->mExecutor);
        if (MR::updateActorState(pActor)) {
            pActor->kill();
        }
    }
    inline void AttackRockNrvBound::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRock* >(pSpine->mExecutor))->exeRockBound();
    }
    inline void AttackRockNrvBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRock* >(pSpine->mExecutor))->exeRockBreak();
    }
};  // namespace NrvAttackRock

