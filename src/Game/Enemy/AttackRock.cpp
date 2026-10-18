#include "Game/Enemy/AttackRock.hpp"
f32 fn_801510C0();
bool fn_801510D0(LiveActor* pActor);
bool fn_801517C0(LiveActor* pActor, s32 arg);
void fn_801552A0(LiveActor* pActor);
f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat);
void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801554D0(TPos3f* pMtx, LiveActor* pActor);
void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos);
ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor);
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

class BaseMatrixFollowValidater;
class KoopaBattleAccessor {
public:
    void addAttackRockFinal(AttackRockFinal*);
};

namespace MR {
    bool isMsgKoopaCastAttackRockStart(u32);
    bool isMsgKoopaCastAttackRockEnd(u32);
    bool isMsgKoopaPunchAttack(u32);
    bool isMsgKoopaReleaseAttackRock(u32);
    bool isMsgKoopaFinalBattleStart(u32);
    bool isMsgAutoRushBegin(u32);
    bool isMsgUpdateBaseMtx(u32);
    bool isMsgRushCancel(u32);
    bool isMsgFloorTouch(u32);
    f32 calcNerveEaseOutValue(const NerveExecutor*, s32, s32, f32, f32);
    void createKoopaBattleAccessor();
    KoopaBattleAccessor* getKoopaBattleAccessor();
};  // namespace MR

// Unnamed SMG2 helpers outside this unit (Koopa battle functions).

// Unit-local helpers (unnamed in the target).

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


// 80150xxx
void AttackRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatF0, this);
    mProjmapEffectMtxSetter = fn_80155660(this);
    MR::setEffectHostMtx(this, "Attack", mMtxC0.toMtxPtr());
    BaseMatrixFollowValidater* pValidater = nullptr;
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    fn_801552A0(this);
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
    fn_801554D0(&mMtxC0, this);
}

void AttackRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatF0);
    fn_80155540(mProjmapEffectMtxSetter, &mQuat100, &mPosition);
}

void AttackRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
        isNerve(&NrvAttackRock::AttackRockNrvBound::sInstance)) {
        if (MR::sendMsgAttackRockReflect(pReceiver, pSender)) {
            this->fn_80151160();
            return;
        }
    }
    if (isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        mStateFall->attackSensor(pSender, pReceiver);
    }
}

f32 fn_801510C0() {
    return 300.0f;
}

bool fn_801510D0(LiveActor* pActor) {
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance) || pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return true;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && !(static_cast< AttackRock* >(pActor)->mStateHold)->fn_80153400()) {
        return true;
    }
    return false;
}

// NONMATCHING: string literal base register (separate TU data pool) and register allocation.
void AttackRock::fn_80151160() {
    const char* pName = "Inner";
    if (fn_801510D0(this)) {
        pName = "Outer";
    }
    MR::startBrk(this->mBreakModel, pName);
    this->setNerve(&NrvAttackRock::AttackRockNrvBreak::sInstance);
}

bool AttackRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && mStateHold->fn_80153400()) {
        fn_80155360(&mVelocity, &mQuat100, pSender, pReceiver, 120.0f);
        setNerve(&NrvAttackRock::AttackRockNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaCastAttackRockStart(msg)) {
        if (MR::isDead(this)) {
            MR::resetPosition(this);
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
        this->fn_80151160();
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
        this->fn_80151160();
        return true;
    }
    if (MR::isMsgKoopaReleaseAttackRock(msg) && isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        setNerve(&NrvAttackRock::AttackRockNrvHold::sInstance);
        return true;
    }
    return false;
}

void AttackRock::fn_80151510() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::onBind(this);
        MR::invalidateCollisionParts(this);
    }
    if (MR::isBinded(this)) {
        if (!MR::sendMsgToBindedKoopaC7(this, this->getSensor("Body"))) {
            this->fn_80151160();
            return;
        }
    }
    f32 dist = MR::calcKoopaBattleDistance(this->mPosition);
    f32 scale = this->mScale.x;
    MR::setShadowVolumeSphereRadius(this, nullptr, MR::getLinerValueFromMinMax(dist, 700.0f, 2100.0f, 285.0f * scale, 15.0f * scale));
    TVec3f up;
    TVec3f pos;
    MR::setShadowProjection(this, nullptr, up, pos, dist < 2100.0f);
    if (MR::reboundVelocityFromCollision(this, 0.8f, 0.0f, 1.0f)) {
        MR::emitEffectHit(this, MR::getBindedHitPos(this), MR::getBindedFixReactionVector(this), "GroundHit");
        MR::startActionSound(this, "Bound", -1, -1, -1);
        MR::attenuateVelocity(this, 0.7f);
    }
    if (MR::isGreaterStep(this, 120)) {
        this->fn_80151160();
    }
}

void AttackRock::fn_801516E0() {
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

bool fn_801517C0(LiveActor* pActor, s32 arg) {
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return arg != 0;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance)) {
        AttackRock* pRock = static_cast< AttackRock* >(pActor);
        if (arg == 0 && !pRock->mIsOnPlayer) {
            return false;
        }
        return pRock->mStateHold->fn_80153460();
    }
    return false;
}

AttackRock::~AttackRock() {
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
    inline void AttackRockNrvHold::execute(Spine* pSpine) const {
        AttackRock* pActor = reinterpret_cast< AttackRock* >(pSpine->mExecutor);
        if (MR::updateActorState(pActor)) {
            pActor->kill();
        }
    }
    inline void AttackRockNrvBound::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRock* >(pSpine->mExecutor))->fn_80151510();
    }
    inline void AttackRockNrvBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRock* >(pSpine->mExecutor))->fn_801516E0();
    }
};  // namespace NrvAttackRock

