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
extern "C" void fn_800CB710(HitSensor*);
extern "C" void fn_800CB090(TVec3f*);
extern "C" void fn_800CB6D0(TVec3f*);
extern "C" f32 fn_800CB410(const TVec3f*);
extern "C" void fn_800CAF80(TVec3f*, const TVec3f*);
extern "C" void fn_800CAFE0(TVec3f*, const TVec3f*);
extern "C" void fn_800CB460(TVec3f*);
extern "C" void fn_800CC210(HitSensor*, HitSensor*);
extern "C" bool fn_800CC1F0(HitSensor*, HitSensor*);
extern "C" void fn_800CC220(LiveActor*, HitSensor*);
extern "C" bool fn_800CC230(LiveActor*, HitSensor*);
extern "C" bool fn_800CC270(u32);
extern "C" bool fn_800CC280(u32);
extern "C" bool fn_800CC290(u32);
extern "C" bool fn_800CC2B0(u32);
extern "C" void fn_800E4420(void*);
extern "C" void fn_800E4700(const TVec3f*);
extern "C" void fn_800513C0(TQuat4f*, TVec3f*);
extern "C" TVec3f* fn_800514B0(TQuat4f*, TVec3f*);

// Unit-local helpers (unnamed in the target).
extern "C" {
f32 fn_801510C0() NO_INLINE;
bool fn_801510D0(LiveActor*) NO_INLINE;
void fn_80151160(AttackRock*) NO_INLINE;
bool fn_801517C0(LiveActor*, s32) NO_INLINE;
void fn_80151850(void*) NO_INLINE;
void fn_801552A0(LiveActor*) NO_INLINE;
void fn_80155360(TVec3f*, TQuat4f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801553D0(TVec3f*, TVec3f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801554D0(TPos3f*, LiveActor*) NO_INLINE;
void fn_80155540(ProjmapEffectMtxSetter*, TQuat4f*, TVec3f*) NO_INLINE;
ProjmapEffectMtxSetter* fn_80155660(LiveActor*) NO_INLINE;
void fn_80152F10(AttackRockRide*) NO_INLINE;
void fn_80152F20(AttackRockRide*) NO_INLINE;
bool fn_80152E20(AttackRockRide*, u32, HitSensor*, HitSensor*) NO_INLINE;
void fn_80152A00(AttackRockRide*) NO_INLINE;
void fn_80152C00(AttackRockRide*) NO_INLINE;
void fn_801529E0(AttackRockRide*, const TVec3f*) NO_INLINE;
AttackRockRide* fn_801528E0(AttackRockRide*, LiveActor*) NO_INLINE;
bool fn_80153400(AttackRockStateHold*) NO_INLINE;
bool fn_80153460(AttackRockStateHold*) NO_INLINE;
f32 fn_801552E0(LiveActor*, TQuat4f*) NO_INLINE;
}

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

extern "C" {
void fn_80151510(AttackRock*) NO_INLINE;
void fn_801516E0(AttackRock*) NO_INLINE;
}

// 80150xxx
void AttackRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatF0, this);
    mProjmapEffectMtxSetter = fn_80155660(this);
    MR::setEffectHostMtx(this, "Attack", mMtxC0.toMtxPtr());
    fn_800CB710(getSensor("Body"));
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
        fn_800CC210(pReceiver, pSender);
        if (fn_800CC1F0(pReceiver, pSender)) {
            fn_80151160(this);
            return;
        }
    }
    if (isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        mStateFall->attackSensor(pSender, pReceiver);
    }
}

extern "C" f32 fn_801510C0() {
    return 300.0f;
}

extern "C" bool fn_801510D0(LiveActor* pActor) {
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance) || pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return true;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && !fn_80153400(static_cast< AttackRock* >(pActor)->mStateHold)) {
        return true;
    }
    return false;
}

// NONMATCHING: string literal base register (separate TU data pool) and register allocation.
extern "C" void fn_80151160(AttackRock* pActor) {
    const char* pName = "Inner";
    if (fn_801510D0(pActor)) {
        pName = "Outer";
    }
    MR::startBrk(pActor->mBreakModel, pName);
    pActor->setNerve(&NrvAttackRock::AttackRockNrvBreak::sInstance);
}

bool AttackRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance) && fn_80153400(mStateHold)) {
        fn_80155360(&mVelocity, &mQuat100, pSender, pReceiver, 120.0f);
        setNerve(&NrvAttackRock::AttackRockNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaCastAttackRockStart(msg)) {
        if (MR::isDead(this)) {
            fn_800CB090(&mPosition);
            MR::resetPosition(this);
            setNerve(&NrvAttackRock::AttackRockNrvFall::sInstance);
            MR::offBind(this);
            MR::hideModel(this);
            appear();
            return true;
        }
        return false;
    }
    if (fn_800CC270(msg)) {
        return !MR::isDead(this);
    }
    if (fn_800CC280(msg)) {
        mStateFall->receiveOtherMsg(msg, pSender, pReceiver);
        mStateHold->receiveOtherMsg(msg, pSender, pReceiver);
        mBreakModel->makeActorDead();
        mDemoObj->makeActorDead();
        makeActorDead();
        return true;
    }
    if (fn_800CC290(msg) && isNerve(&NrvAttackRock::AttackRockNrvFall::sInstance)) {
        fn_80151160(this);
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
        fn_800CB6D0(&mPosition);
        fn_80151160(this);
        return true;
    }
    if (MR::isMsgKoopaReleaseAttackRock(msg) && isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        setNerve(&NrvAttackRock::AttackRockNrvHold::sInstance);
        return true;
    }
    return false;
}

extern "C" void fn_80151510(AttackRock* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Attack");
        MR::onBind(pActor);
        MR::invalidateCollisionParts(pActor);
    }
    if (MR::isBinded(pActor)) {
        fn_800CC220(pActor, pActor->getSensor("Body"));
        if (!fn_800CC230(pActor, pActor->getSensor("Body"))) {
            fn_800CB6D0(&pActor->mPosition);
            fn_80151160(pActor);
            return;
        }
    }
    f32 dist = fn_800CB410(&pActor->mPosition);
    f32 scale = pActor->mScale.x;
    MR::setShadowVolumeSphereRadius(pActor, nullptr, MR::getLinerValueFromMinMax(dist, 700.0f, 2100.0f, 285.0f * scale, 15.0f * scale));
    TVec3f up;
    TVec3f pos;
    fn_800CAF80(&pos, &pActor->mPosition);
    fn_800CAFE0(&up, &pActor->mPosition);
    MR::setShadowProjection(pActor, nullptr, up, pos, dist < 2100.0f);
    if (MR::reboundVelocityFromCollision(pActor, 0.8f, 0.0f, 1.0f)) {
        MR::emitEffectHit(pActor, MR::getBindedHitPos(pActor), MR::getBindedFixReactionVector(pActor), "GroundHit");
        MR::startActionSound(pActor, "Bound", -1, -1, -1);
        MR::attenuateVelocity(pActor, 0.7f);
    }
    if (MR::isGreaterStep(pActor, 120)) {
        fn_80151160(pActor);
    }
}

extern "C" void fn_801516E0(AttackRock* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Break");
        pActor->mBreakModel->appear();
        pActor->mDemoObj->kill();
        MR::invalidateCollisionParts(pActor);
        MR::invalidateHitSensors(pActor);
        MR::invalidateShadow(pActor, nullptr);
        MR::zeroVelocity(pActor);
    }
    if (MR::isStep(pActor, 1)) {
        MR::hideModelAndOnCalcAnim(pActor);
    }
    if (MR::isGreaterStep(pActor, 10)) {
        pActor->kill();
    }
}

extern "C" bool fn_801517C0(LiveActor* pActor, s32 arg) {
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvWait::sInstance)) {
        return arg != 0;
    }
    if (pActor->isNerve(&NrvAttackRock::AttackRockNrvHold::sInstance)) {
        AttackRock* pRock = static_cast< AttackRock* >(pActor);
        if (arg == 0 && !pRock->mIsOnPlayer) {
            return false;
        }
        return fn_80153460(pRock->mStateHold);
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
        fn_80151510(reinterpret_cast< AttackRock* >(pSpine->mExecutor));
    }
    inline void AttackRockNrvBreak::execute(Spine* pSpine) const {
        fn_801516E0(reinterpret_cast< AttackRock* >(pSpine->mExecutor));
    }
};  // namespace NrvAttackRock

