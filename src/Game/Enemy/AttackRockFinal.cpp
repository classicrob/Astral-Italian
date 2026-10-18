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

extern "C" {
void fn_801521F0(AttackRockFinal*, f32, f32) NO_INLINE;
void fn_80152280(AttackRockFinal*) NO_INLINE;
void fn_801522F0(AttackRockFinal*) NO_INLINE;
void fn_80152400(AttackRockFinal*) NO_INLINE;
void fn_801524F0(AttackRockFinal*) NO_INLINE;
void fn_80152620(AttackRockFinal*) NO_INLINE;
}

AttackRockFinal::AttackRockFinal(const char* pName)
    : LiveActor(pName), mProjmapEffectMtxSetter(nullptr), mBreakModel(nullptr), mRide(nullptr), mStartPos(0.0f, 0.0f, 0.0f),
      mAxisF8(1.0f, 0.0f, 0.0f), mAxis104(0.0f, 1.0f, 0.0f), mWaitStep(0), mIsOnPlayer(false) {
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
    mRide = new AttackRockRide;
    fn_801528E0(mRide, this);
    BaseMatrixFollowValidater* pValidater = nullptr;
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    fn_800CB710(getSensor("Body"));
    initNerve(&NrvAttackRockFinal::AttackRockFinalNrvWait::sInstance, 0);
    makeActorDead();
}

void AttackRockFinal::makeActorDead() {
    LiveActor::makeActorDead();
    fn_80152F20(mRide);
    mIsOnPlayer = false;
}

// NONMATCHING: vector ops: target expands ps_sub inline, operator-/operator* inline decisions differ (separate TU).
void AttackRockFinal::control() {
    TVec3f dir;
    fn_800CB460(&dir);
    TVec3f toPlayer = dir - mPosition;
    MR::normalizeOrZero(&dir, toPlayer);
    MR::turnQuatZDirRate(&mQuatCC, mQuatCC, dir, 0.3f);
    TVec3f zdir;
    mQuatCC.getZDir(zdir);
    fn_801529E0(mRide, &zdir);
    fn_80152C00(mRide);
}

void AttackRockFinal::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    fn_80155540(mProjmapEffectMtxSetter, &mQuatCC, &mPosition);
}

void AttackRockFinal::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
        isNerve(&NrvAttackRockFinal::AttackRockFinalNrvBound::sInstance) && fn_800CC1F0(pReceiver, pSender)) {
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
    if (fn_800CC2B0(msg)) {
        makeActorDead();
        return true;
    }
    if (fn_80152E20(mRide, msg, pSender, pReceiver)) {
        return true;
    }
    if (MR::isMsgKoopaPunchAttack(msg) && MR::isEqualSensor(pReceiver, this, "Body") &&
        isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance)) {
        MR::appearCoinPop(this, mPosition, 1);
        setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBound::sInstance);
        MR::startBrk(mBreakModel, "Outer");
        return false;
    }
    return false;
}

// NONMATCHING: operator*/add inline decisions differ (target was a separate TU).
extern "C" void fn_801521F0(AttackRockFinal* pActor, f32 a, f32 b) {
    pActor->mPosition.set(pActor->mStartPos);
    TVec3f va = pActor->mAxisF8 * a;
    pActor->mPosition.add(va);
    TVec3f vb = pActor->mAxis104 * b;
    pActor->mPosition.add(vb);
}

extern "C" void fn_80152280(AttackRockFinal* pActor) {
    if (MR::isFirstStep(pActor)) {
        fn_801521F0(pActor, 800.0f, 2000.0f);
    }
    pActor->mIsOnPlayer = false;
    if (MR::isGreaterStep(pActor, pActor->mWaitStep)) {
        pActor->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvFall::sInstance);
    }
}

extern "C" void fn_801522F0(AttackRockFinal* pActor) {
    f32 y = MR::calcNerveEaseOutValue(pActor, 100, 2000.0f, -1000.0f);
    f32 x = MR::calcNerveEaseInValue(pActor, 100, 800.0f, 400.0f);
    fn_801521F0(pActor, x, y);
    if (MR::isFirstStep(pActor)) {
        MR::offBind(pActor);
        MR::startAction(pActor, "FinalWait");
        MR::showModel(pActor);
        MR::validateCollisionParts(pActor);
        MR::validateHitSensors(pActor);
        MR::resetPosition(pActor);
    }
    if (MR::isOnGroundPlayer()) {
        pActor->mIsOnPlayer = MR::isActorOnPlayer(pActor);
    }
    MR::startActionSound(pActor, "FinalFall", -1, -1, -1);
    if (MR::isGreaterEqualStep(pActor, 99)) {
        pActor->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvFloat::sInstance);
    }
}

extern "C" void fn_80152400(AttackRockFinal* pActor) {
    f32 y = MR::calcNerveEaseInOutValue(pActor, 150, -1000.0f, 0.0f);
    f32 x = MR::calcNerveEaseOutValue(pActor, 150, 400.0f, 0.0f);
    fn_801521F0(pActor, x, y);
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "FinalFloat");
        fn_80152F10(pActor->mRide);
    }
    if (MR::isOnGroundPlayer()) {
        pActor->mIsOnPlayer = MR::isActorOnPlayer(pActor);
    }
    MR::startActionSound(pActor, "FinalPower", -1, -1, 18);
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(pActor)) {
        fn_80152F10(pActor->mRide);
    }
}

extern "C" void fn_801524F0(AttackRockFinal* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Attack");
        MR::onBind(pActor);
        fn_80152F20(pActor->mRide);
        pActor->mIsOnPlayer = false;
    }
    fn_801554D0(&pActor->mMtx9C, pActor);
    if (MR::isBinded(pActor)) {
        fn_800CC220(pActor, pActor->getSensor("Body"));
    }
    if (MR::reboundVelocityFromCollision(pActor, 0.8f, 0.0f, 1.0f)) {
        MR::emitEffectHit(pActor, MR::getBindedHitPos(pActor), MR::getBindedFixReactionVector(pActor), "GroundHit");
        MR::startActionSound(pActor, "Bound", -1, -1, -1);
        MR::attenuateVelocity(pActor, 0.7f);
    }
    if (MR::isGreaterStep(pActor, 90)) {
        pActor->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvBreak::sInstance);
    }
}

extern "C" void fn_80152620(AttackRockFinal* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Break");
        MR::startActionSound(pActor, "FinalBreak", -1, -1, -1);
        MR::startBrk(pActor->mBreakModel, "Inner");
        MR::hideModelAndOnCalcAnim(pActor);
        MR::invalidateCollisionParts(pActor);
        MR::invalidateHitSensors(pActor);
        MR::zeroVelocity(pActor);
        MR::offBind(pActor);
        pActor->mBreakModel->appear();
        fn_80152F20(pActor->mRide);
    }
    if (MR::isDead(pActor->mBreakModel)) {
        pActor->setNerve(&NrvAttackRockFinal::AttackRockFinalNrvWait::sInstance);
    }
}

extern "C" bool fn_80152700(LiveActor* pActor) {
    if (pActor->isNerve(&NrvAttackRockFinal::AttackRockFinalNrvWait::sInstance) ||
        pActor->isNerve(&NrvAttackRockFinal::AttackRockFinalNrvFall::sInstance)) {
        return true;
    }
    return false;
}

AttackRockFinal::~AttackRockFinal() {
}

namespace NrvAttackRockFinal {
    inline void AttackRockFinalNrvWait::execute(Spine* pSpine) const {
        fn_80152280(reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor));
    }
    inline void AttackRockFinalNrvFall::execute(Spine* pSpine) const {
        fn_801522F0(reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor));
    }
    inline void AttackRockFinalNrvFloat::execute(Spine* pSpine) const {
        fn_80152400(reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor));
    }
    inline void AttackRockFinalNrvBound::execute(Spine* pSpine) const {
        fn_801524F0(reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor));
    }
    inline void AttackRockFinalNrvBreak::execute(Spine* pSpine) const {
        fn_80152620(reinterpret_cast< AttackRockFinal* >(pSpine->mExecutor));
    }
};  // namespace NrvAttackRockFinal

// ---- AttackRockRide: lets the player ride the rock (rush handling) ----

