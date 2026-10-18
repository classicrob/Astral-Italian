#pragma auto_inline off

#include "Game/MapObj/BalloonFruit.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>

// Float constants (.sdata2) used by BalloonFruit.
extern "C" f32 lbl_807DCBE8;  // 1.0
extern "C" f32 lbl_807DCBEC;  // 0.0
extern "C" f32 lbl_807DCBF0;
extern "C" f32 lbl_807DCBF4;
extern "C" f32 lbl_807DCBF8;  // pi
extern "C" f32 lbl_807DCBFC;  // pi / 2
extern "C" f32 lbl_807DCC00;  // -pi / 2
extern "C" f32 lbl_807DCC04;  // 1.0
extern "C" f32 lbl_807DCC08;  // 75.0
extern "C" f32 lbl_807DCC0C;  // 80.0
extern "C" f32 lbl_807DCC10;  // 100.0
extern "C" f32 lbl_807DCC14;  // 180 / pi
extern "C" f32 lbl_807DCC18;  // 180.0
extern "C" f32 lbl_807DCC1C;  // 2 pi

class YoshiLockOnTarget;

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
    bool tryYoshiPushEat(YoshiLockOnTarget*);
}

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
extern "C" s32 fn_804225C0();
extern "C" void fn_80050DB0(const char*, s32, s32);
extern "C" void fn_802730E0_unused();
extern "C" void fn_80422580();
extern "C" s32 fn_80422880(u32 msg);
extern "C" u8 lbl_807CFBC0;  // sdata string used by the Pop nerve (action name)

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
                MR::startAction(actor, reinterpret_cast< const char* >(&lbl_807CFBC0));
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
    f32 zero = lbl_807DCBEC;
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

void BalloonFruit::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "BalloonFruit", false);
    MR::connectToSceneItem(this);
    MR::initLightCtrl(this);

    TVec3f sp2C;
    sp2C.setAll<f32>(lbl_807DCBEC);
    initSound(4, nullptr, nullptr, sp2C);
    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x4E, 0x20, lbl_807DCC0C, TVec3f(lbl_807DCBEC, lbl_807DCC0C, lbl_807DCBEC));
    initEffectKeeper(0, nullptr, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::onCalcGravity(this);
    MR::initStarPointerTarget(this, lbl_807DCC10, TVec3f(lbl_807DCBEC, lbl_807DCC10, lbl_807DCBEC));

    mHomePos.set(mPosition);
    mHomeRot.set(mRotation);
    mBaseScale = mScale.x;
    mMtxAC.identity();
    TVec3f sp8;
    sp8.setAll<f32>(lbl_807DCBEC);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, mMtxAC, sp8);
    initNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait), 0);
    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);
    makeActorAppeared();
}

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeTrampled() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Trampled");
    }
    if (MR::isActionEnd(this) || MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
    }
    if (MR::isActionEnd(this) || MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shake");
    }
    if (MR::isActionEnd(this) || MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvWait));
    }
}

void BalloonFruit::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
    }
    if (fn_804225C0() == 0 && MR::isGreaterEqualStep(this, 60)) {
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

// NONMATCHING: isActionEnd/isStarPointer and step-check shapes are approximate (m2c passes the star-pointer result as the actor argument).
void BalloonFruit::exeHipDropCrash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropCrash");
    }
    TVec3f toPlayer(*MR::getPlayerPos());
    toPlayer.sub(mPosition);
    f32 d = toPlayer.dot(*MR::getPlayerGravity());
    if (d > lbl_807DCC08) {
        TVec3f sp8(*MR::getPlayerGravity());
        sp8.scale(d);
        toPlayer.add(sp8);
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
    mAngleE4 = lbl_807DCBEC;
    setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear));
}

// NONMATCHING: the atan2/asin decomposition and fmod-free angle wrap are approximations of the target matrix math.
void BalloonFruit::control() {
    if (mTimerE0 != 0) {
        mTimerE0 = mTimerE0 - 1;
    }
    if (isNerveSpinOrStill()) {
        // NONMATCHING: rotation math is a Rodrigues matrix about the gravity axis, decomposed back
        // into yaw/pitch/roll; target uses the same formulas but the matrix/concat order is not verified.
        TVec3f axis(lbl_807DCBEC, lbl_807DCBE8, lbl_807DCBEC);
        TVec3f n = axis;
        PSVECNormalize(&n, &n);
        f32 s = (f32)sin(mAngleE4);
        f32 c = (f32)cos(mAngleE4);
        f32 t = lbl_807DCBE8 - c;
        f32 nx = n.x;
        f32 ny = n.y;
        f32 nz = n.z;
        TPos3f rot;
        rot[0][0] = c + t * (nx * nx);
        rot[0][1] = t * nx * ny - s * nz;
        rot[0][2] = t * nx * nz + s * ny;
        rot[1][0] = t * nx * ny + s * nz;
        rot[1][1] = c + t * (ny * ny);
        rot[1][2] = t * ny * nz - s * nx;
        rot[2][0] = t * nx * nz - s * ny;
        rot[2][1] = t * ny * nz + s * nx;
        rot[2][2] = c + t * (nz * nz);
        TPos3f mtx;
        TVec3f zero(lbl_807DCBEC, lbl_807DCBEC, lbl_807DCBEC);
        MR::makeMtxTR(mtx, zero, mHomeRot);
        mtx.concat(rot);

        if ((rot[2][0] - lbl_807DCBE8) == lbl_807DCBF4) {
            mRotation.x = JMath::sAtanTable.atan2_(-mtx[1][2], mtx[1][1]);
            mRotation.y = lbl_807DCC00;
            mRotation.z = lbl_807DCBEC;
        } else if ((rot[2][0] + lbl_807DCBE8) == lbl_807DCBF0) {
            mRotation.x = JMath::sAtanTable.atan2_(mtx[1][2], mtx[1][1]);
            mRotation.y = lbl_807DCBFC;
            mRotation.z = lbl_807DCBEC;
        } else {
            mRotation.x = JMath::sAtanTable.atan2_(mRotation.x, mtx[0][0]);
            f32 yaw = JMath::sAtanTable.atan2_(mtx[2][1], mtx[2][2]);
            mRotation.z = yaw;
            mRotation.y = JGeometry::TUtil< f32 >::asin(-rot[2][0]);
        }

        f32 rx = mRotation.x * lbl_807DCC14;
        f32 ry = mRotation.y * lbl_807DCC14;
        f32 rz = mRotation.z * lbl_807DCC14;
        mAngleE4 = mAngleE4 + (lbl_807DCBF8 * lbl_807DCC04) / lbl_807DCC18;
        mRotation.x = rx;
        mRotation.y = ry;
        mRotation.z = rz;
        if (mAngleE4 >= lbl_807DCC1C) {
            mAngleE4 -= lbl_807DCC1C;
        }
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
        if (isNerveYoshiAttackable() && YoshiFunction::tryYoshiPushEat(mYoshiLockOn) == 0) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

// NONMATCHING: target body is a bare blr (void-like); the LiveActor base returns bool here.
bool BalloonFruit::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    tryStartShake();
    return false;
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
    if (YoshiFunction::tryYoshiPushEat(mYoshiLockOn)) {
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
        if (fn_80422880(msg)) {
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
            fn_80050DB0("Swallow", -1, -1);
            fn_80422580();
            setNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHide));
            return true;
        }
        if (MR::isMsgYoshiAutoEatEnd(msg)) {
            MR::hideModel(this);
            fn_80050DB0("Swallow", -1, -1);
            fn_80422580();
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
    // NONMATCHING: remaining nerve checks (Appear/Pop/Hide/Break/...) not all verified.
    return !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvAppear)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvPop)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvHide)) && !isNerve(GET_NERVE(BalloonFruit, BalloonFruitNrvBreak));
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

