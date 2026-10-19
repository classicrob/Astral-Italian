#pragma auto_inline off
#define JGEOMETRY_VEC3_WMR_OUTLINE

#include "Game/Util/YoshiUtil.hpp"
#include "Game/MapObj/BalloonFruit.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>

// Float constants (.sdata2) used by BalloonFruit.

#include "Game/Player/YoshiFunction.hpp"


namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
    bool isMsgYoshiAutoEatStart(u32);
    bool isMsgYoshiAutoEatEnd(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgYoshiProjectile(u32);
    bool isMsgRushEnd(u32);
    bool isMsgSupportTicoSpin(u32);
    bool isMsgPlayerSpinAttackOrSupportTico(u32);
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiCancelEaten(u32);
    bool isMsgYoshiReleaseEaten(u32);
    bool isMsgJetTurtleAttack(u32);
    bool isMsgStarPieceReflect(u32);
    bool isMsgYoshiEat(u32);
    bool isMsgPlayerTrample(u32);
    bool isMsgPlayerHipDrop(u32);
    bool isPlayerElementModeYoshi();
    bool isValidSwitchA(const LiveActor*);
    bool isOnSwitchA(const LiveActor*);
    void enablePlayerResourceYoshiBalloon();
    void showModel(LiveActor*);
    void validateClipping(LiveActor*);
    void invalidateClipping(LiveActor*);
    bool isSensorPlayerOrRide(const HitSensor*);
    bool isStarPointerPointing2PAttachTarget(const LiveActor*);
    void makeMtxTR(MtxPtr, const TVec3f&, const TVec3f&);
    void onCalcGravity(LiveActor*);
    void connectToSceneItem(LiveActor*);
    void initDefaultPos(LiveActor*, const JMapInfoIter&);
    void initStarPointerTarget(LiveActor*, f32, const TVec3f&);
    void initShadowFromCSVWithInitShadowVolumeSphere(LiveActor*, const char*);
}

// Unnamed SMG2 helpers, C linkage so the target reloc names match.

namespace { extern const f32 sSpinDegrees; }

namespace NrvBalloonFruit {
    NEW_NERVE(BalloonFruitNrvAppear, BalloonFruit, Appear);
    NEW_NERVE(BalloonFruitNrvWait, BalloonFruit, Wait);
    NEW_NERVE(BalloonFruitNrvTrampled, BalloonFruit, Trampled);
    NEW_NERVE(BalloonFruitNrvSpin, BalloonFruit, Spin);
    NEW_NERVE(BalloonFruitNrvShake, BalloonFruit, Shake);

    // Pop: execute body is inline in the target (no exePop symbol).
    class BalloonFruitNrvPop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Get");
            }
        }
        static BalloonFruitNrvPop sInstance;
    };
    BalloonFruitNrvPop BalloonFruitNrvPop::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BalloonFruitNrvHide, BalloonFruit, Hide);
    NEW_NERVE(BalloonFruitNrvBreak, BalloonFruit, Break);
    NEW_NERVE(BalloonFruitNrvHipDropCrash, BalloonFruit, HipDropCrash);
    NEW_NERVE(BalloonFruitNrvDead, BalloonFruit, Dead);
}

BalloonFruit::BalloonFruit(const char* pName) : LiveActor(pName) {
    f32 zero = 0.0f;
    mHomePos.x = zero;
    mHomePos.y = zero;
    mHomePos.z = zero;
    mHomeRot.x = zero;
    mHomeRot.y = zero;
    mHomeRot.z = zero;
    mBaseScale = zero;
    mYoshiLockOn = nullptr;
    mTimerE0 = 0;
    mAngleE4 = zero;
}

BalloonFruit::~BalloonFruit() {
}

// NONMATCHING: vector temporary stack slots differ.
void BalloonFruit::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "BalloonFruit", false);
    MR::connectToSceneItem(this);
    MR::initLightCtrl(this);

    TVec3f sp2C;
    sp2C.setAll<f32>(0.0f);
    initSound(4, nullptr, nullptr, sp2C);
    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x4E, 0x20, 80.0f, TVec3f(0.0f, 80.0f, 0.0f));
    initEffectKeeper(0, nullptr, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::initStarPointerTarget(this, 100.0f, TVec3f(0.0f, 100.0f, 0.0f));
    MR::onCalcGravity(this);

    mHomePos = mPosition;
    mHomeRot = mRotation;
    mBaseScale = mScale.x;
    mMtxAC.identity();
    TVec3f sp8;
    sp8.setAll<f32>(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, mMtxAC, sp8);
    initNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait), 0);
    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);
    makeActorAppeared();
}

void BalloonFruit::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    mMtxAC.set(getBaseMtx());
    MR::addTransMtxLocalY(mMtxAC, 100.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

// NONMATCHING: epilogue restores r0/r31 in the opposite order.
void BalloonFruit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    mMtxAC.set(getBaseMtx());
    MR::addTransMtxLocalY(mMtxAC, 100.0f);
    tryStartBind();
}

void BalloonFruit::exeTrampled() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Trampled");
    }
    MR::isStarPointerPointing2PAttachTarget(this);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

void BalloonFruit::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
    }
    MR::isStarPointerPointing2PAttachTarget(this);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

void BalloonFruit::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shake");
    }
    MR::isStarPointerPointing2PAttachTarget(this);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

void BalloonFruit::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
    }
    if (MR::fn_804225C0() == 0 && MR::isGreaterEqualStep(this, 60)) {
        reset();
    }
}

void BalloonFruit::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvDead));
    }
}

// NONMATCHING: vector temporary address and paired-single load scheduling differ.
void BalloonFruit::exeHipDropCrash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropCrash");
    }
    TVec3f toPlayer(*MR::getPlayerPos());
    toPlayer.sub(toPlayer, mPosition);
    f32 d = MR::getPlayerGravity()->dot(toPlayer);
    if (d > 75.0f) {
        TVec3f sp8(*MR::getPlayerGravity());
        sp8.scale(d);
        mPosition.add(mPosition, sp8);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvDead));
    }
}

void BalloonFruit::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::hideModel(this);
    }
    if (MR::isGreaterEqualStep(this, 60)) {
        reset();
    }
}

void BalloonFruit::reset() {
    MR::showModel(this);
    MR::validateClipping(this);
    mPosition.set(mHomePos);
    mRotation.set(mHomeRot);
    mScale.setAll<f32>(mBaseScale);
    mAngleE4 = 0.0f;
    setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear));
}

static inline f32 wrapAngle(f32 v, f32 base) {
    return base + (f32)fmod(6.2831855f + (v - base), 6.2831855f);
}

static inline f32 degToRad(f32 deg) {
    return (3.1415927f * deg) / 180.0f;
}

// NONMATCHING: vector temporary stack slots and arithmetic register scheduling differ.
void BalloonFruit::control() {
    if (mTimerE0 != 0) {
        mTimerE0 = mTimerE0 - 1;
    }
    if (isNerveSpinOrStill()) {
        TPos3f rot;
        TPos3f mtx;
        TVec3f zero;
        rot.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), mAngleE4);
        zero.setAll< f32 >(0.0f);
        MR::makeMtxTR(mtx, zero, mHomeRot);
        mtx.concat(rot);

        if (mtx[2][0] - 1.0f >= -3.81469727e-06f) {
            mRotation.x = JMath::sAtanTable.atan2_(-mtx[0][1], mtx[1][1]);
            mRotation.y = -1.5707964f;
            mRotation.z = 0.0f;
        } else if (mtx[2][0] + 1.0f <= 3.81469727e-06f) {
            mRotation.x = JMath::sAtanTable.atan2_(mtx[0][1], mtx[1][1]);
            mRotation.y = 1.5707964f;
            mRotation.z = 0.0f;
        } else {
            mRotation.x = JMath::sAtanTable.atan2_(mtx[2][1], mtx[2][2]);
            mRotation.z = JMath::sAtanTable.atan2_(mtx[1][0], mtx[0][0]);
            mRotation.y = JGeometry::TUtil< f32 >::asin(-mtx[2][0]);
        }
        mAngleE4 = mAngleE4 + sSpinDegrees * MR::pi() / 180.0f;
        mRotation.x *= 57.295776f;
        mRotation.y *= 57.295776f;
        mRotation.z *= 57.295776f;
        mAngleE4 = wrapAngle(mAngleE4, 0.0f);
    }
    if (isNerveNoYoshiEat()) {
        if (MR::isPlayerElementModeYoshi()) {
            if (isSwitchA() == 0) {
                YoshiFunction::tryYoshiEat(mYoshiLockOn);
            }
        }
    }
}

void BalloonFruit::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (isNerveYoshiAttackable() && YoshiFunction::tryYoshiPushEat() == 0) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool BalloonFruit::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    return tryStartShake();
}

bool BalloonFruit::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerveDamage()) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        tryStartSpin();
        return true;
    }
    if (MR::isMsgYoshiEat(msg)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak));
        return true;
    }
    if (MR::isMsgYoshiProjectile(msg)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak));
        return true;
    }
    if (MR::isMsgJetTurtleAttack(msg)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak));
        return true;
    }
    if (YoshiFunction::tryYoshiPushEat()) {
        if (MR::isMsgSupportTicoSpin(msg)) {
            return tryStartSpin();
        }
        return false;
    }
    if (MR::isMsgPlayerTrample(msg)) {
        mTimerE0 = 30;
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvTrampled));
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return tryStartSpin();
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHipDropCrash));
        return true;
    }
    return false;
}

bool BalloonFruit::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerveDamage() && MR::isMsgRushEnd(msg)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak));
        return true;
    }
    if (isNerveNoYoshiEat()) {
        if (MR::isMsgYoshiAutoEatQuery(msg)) {
            return true;
        }
        if (MR::isMsgYoshiAutoEatStart(msg)) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvPop));
            return true;
        }
        if (MR::isMsgYoshiStartTongue(msg)) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvPop));
            return true;
        }
    }
    if (isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvPop))) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            MR::hideModel(this);
            MR::startActionSoundPlayer("Swallow", -1, -1);
            MR::resetYoshiTongueTarget();
            setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHide));
            return true;
        }
        if (MR::isMsgYoshiAutoEatEnd(msg)) {
            MR::hideModel(this);
            MR::startActionSoundPlayer("Swallow", -1, -1);
            MR::resetYoshiTongueTarget();
            setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHide));
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            reset();
            return true;
        }
    }
    return false;
}

bool BalloonFruit::isNerveNoYoshiEat() const {
    if (isSwitchA()) {
        return false;
    }
    return !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvPop)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHide)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHipDropCrash)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvDead));
}

bool BalloonFruit::isNerveYoshiAttackable() const {
    return isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvTrampled)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvSpin)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvShake));
}

bool BalloonFruit::isNerveSpinOrStill() const {
    return isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvTrampled)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvSpin)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvShake));
}

bool BalloonFruit::isNerveDamage() const {
    if (isSwitchA()) {
        return false;
    }
    return isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvTrampled)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvSpin)) || isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvShake));
}

bool BalloonFruit::tryStartSpin() {
    if (!isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvSpin))) {
        mTimerE0 = 30;
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvSpin));
        return true;
    }
    return false;
}

bool BalloonFruit::tryStartShake() {
    if (isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait)) && mTimerE0 == 0) {
        mTimerE0 = 30;
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvShake));
        return true;
    }
    mTimerE0 = 30;
    return false;
}

bool BalloonFruit::tryStartBind() {
    if (isSwitchA()) {
        return false;
    }
    if (MR::isStarPointerPointing2PAttachTarget(this) && tryStartShake()) {
        return true;
    }
    return false;
}

bool BalloonFruit::isSwitchA() const {
    if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
        return true;
    }
    return false;
}

void BalloonFruit::enableYoshiBalloonResource() {
    MR::enablePlayerResourceYoshiBalloon();
}


namespace { const f32 sSpinDegrees = 1.0f; }
