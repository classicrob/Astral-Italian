#include "Game/Boss/Yukkina.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/TriangleFilter.hpp"

extern u8 lbl_8067B8A8[];
extern u8 lbl_8067B8C0[];
extern u8 lbl_8067B8CC[];
extern u8 lbl_8067B8F4[];
extern u8 lbl_8067B8FC[];
extern u8 lbl_8067B930[];
extern u8 lbl_8067B97C[];
extern u8 lbl_8067B994[];
extern u8 lbl_8067B99C[];
extern u8 lbl_807CF678[];

extern s32 lbl_807D1D18;
extern s32 lbl_807D1D1C;
extern s32 lbl_807D1D20;
extern s32 lbl_807D1D24;
extern s32 lbl_807D1D28;
extern s32 lbl_807D1D2C;
extern s32 lbl_807D1D30;
extern s32 lbl_807D1D34;
extern s32 lbl_807D1D38;
extern s32 lbl_807D1D3C;
extern s32 lbl_807D1D40;
extern s32 lbl_807D1D44;
extern s32 lbl_807D1D48;


#define STR8A8(off) (reinterpret_cast< const char* >(lbl_8067B8A8 + (off)))
#define STRAT(sym) (reinterpret_cast< const char* >(sym))
#define NRV(sym) (reinterpret_cast< Nerve* >(&(sym)))
#define RAW(T, this_, off) (*reinterpret_cast< T* >(reinterpret_cast< u8* >(this_) + (off)))

// PLACEHOLDER(objects): unnamed target symbol fn_8001BA80, declared extern "C" so the reloc name matches; resolve when fn_8001BA80 gets its real name in symbols.txt
extern "C" void fn_8001BA80();
// PLACEHOLDER(objects): unnamed target symbol fn_8004FF90, declared extern "C" so the reloc name matches; resolve when fn_8004FF90 gets its real name in symbols.txt
extern "C" void fn_8004FF90();

Yukkina::Yukkina(const char* pName) : LiveActor(pName) {
    mCameraInfo = NULL;
    mQuatC4.set(0.0f, 0.0f, 0.0f, 1.0f);
    mQuatD4.set(0.0f, 0.0f, 0.0f, 1.0f);
    mVelE4.x = 0.0f;
    mVelE4.y = 0.0f;
    mVelE4.z = 1.0f;
    mQuatF0.set(0.0f, 0.0f, 0.0f, 1.0f);
    mPos100.x = 0.0f;
    mPos100.y = 0.0f;
    mPos100.z = 0.0f;
    mPos10C.x = 0.0f;
    mPos10C.y = 0.0f;
    mPos10C.z = 0.0f;
    mSwitch = (NameObj*)-1;
    mCounter = 0;
    mStep = 3;
    _124 = 0.0f;
    _128 = 0.0f;
    _12C = 0.0f;
    mLowAngle = 1.0f;
    mHighAngle = 3.0f;
    mRollSpeed = 1.5f;
    mIdx = 1;
    mHasPlayerLink = 0;
    mBaseMtx.identity();
}

// NONMATCHING: init() call signatures for addBaseMatrixFollowTarget, declareCameraRegisterVec and
// declarePowerStar are approximations of the target calls.
void Yukkina::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::makeQuatAndFrontFromRotate(&mQuatC4, &mVelE4, this);
    mQuatF0.set(mQuatC4);
    mPos100.set(RAW(TVec3f, this, 0x14));

    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::setAllEffectHostMtx(this, mBaseMtx);
    MR::initAnimCamera(this, mCameraInfo, STR8A8(0xC));
    MR::initAnimCamera(this, mCameraInfo, STR8A8(0x18));
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, STR8A8(0x24));
    MR::getJMapInfoArg7WithInit(rIter, reinterpret_cast< s32* >(&mSwitch));
    if (mSwitch != (NameObj*)-1) {
        MR::declareCameraRegisterVec(this, reinterpret_cast< u32 >(mSwitch), &mPos10C);
        MR::calcPositionUpOffset(&mPos10C, this, 250.0f);
    }

    TriangleFilterDelegator< Yukkina >* pFilter = new TriangleFilterDelegator< Yukkina >(this, &Yukkina::isInvalidTriangle);
    MR::addBaseMatrixFollowTarget(this, rIter, NULL, reinterpret_cast< BaseMatrixFollowValidater* >(pFilter));
    mHasPlayerLink = MR::tryRegisterNamePosLinkObj(this, rIter, STR8A8(0x40));
    initNerve(NRV(lbl_807D1D18), 0);
    MR::hideModel(this);
    MR::declarePowerStar(this);
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::startBva(this, STRAT(lbl_807CF678));
    MR::setBvaFrameAndStop(this, 0.0f);
}

void Yukkina::kill() {
    LiveActor::kill();
}

// NONMATCHING: JGeometry::negateInternal call is elided (no matching declaration in the repo).
void Yukkina::control() {
    if (mSwitch != (NameObj*)-1) {
        RAW(TVec3f, this, 0x44).set(RAW(TVec3f, this, 0x14));
        JMAVECScaleAdd(&RAW(Vec, this, 0x44), &mPos10C, &mPos10C, 250.0f);
    }
    if (MR::isBindedGround(this)) {
        mCounter = 0;
        return;
    }
    mCounter += 1;
}

void Yukkina::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatC4);
}

void Yukkina::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::isEqualSensor(pSender, this, STRAT(lbl_8067B8F4))) {
        if (isNerve(NRV(lbl_807D1D24)) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            setNerve(NRV(lbl_807D1D30));
            // NONMATCHING: target passes both sensors and two floats to setVelocitySeparateHV
            MR::setVelocitySeparateHV(this, mVelE4, 5.0f, 15.0f);
            return;
        }
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Yukkina::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isNerve(NRV(lbl_807D1D24))) {
        if (MR::isEqualSensor(pReceiver, this, STRAT(lbl_8067B8FC))) {
            mStep -= 1;
            if (mStep == 1) {
                MR::stopStageBGM(10);
            }
            if (mStep <= 0) {
                MR::zeroVelocity(this);
                setNerve(NRV(lbl_807D1D3C));
                return true;
            }
            mIdx += 1;
            TVec3f sp8;
            MR::calcSensorHorizonNormalize(&sp8, RAW(TVec3f, this, 0x44), pSender, pReceiver);
            mVelE4.set(sp8);
            // NONMATCHING: setVelocitySeparateHV argument list
            MR::setVelocitySeparateHV(this, mVelE4, 20.0f, 40.0f);
            setNerve(NRV(lbl_807D1D28));
            return true;
        }
    }
    return false;
}

// NONMATCHING: fn_8010A970 -> exeHidden. Call to resetPosition uses the 1-arg repo overload.
void Yukkina::exeHidden() {
    if (MR::isFirstStep(this)) {
        mQuatC4.set(mQuatF0);
        MR::zeroVelocity(this);
        MR::resetPosition(this);
        fn_8001BA80();
    }
    if (MR::isGreaterEqualStep(this, 4)) {
        if (MR::tryStartDemoMarioPuppetable(this, STR8A8(0x5C))) {
            MR::hidePlayer();
            if (mHasPlayerLink != 0) {
                MR::showPlayer();
                MR::setPlayerLinkPosAndWait(this, STR8A8(0x40), false);
                MR::startBckPlayer(STR8A8(0x6C), NULL);
            }
            fn_8004FF90();
            setNerve(NRV(lbl_807D1D1C));
        }
    }
}

// NONMATCHING: fn_8010AA60 -> exeAppear. endAnimCamera / startAnimCamera argument lists approximated.
void Yukkina::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, STR8A8(0xC));
    }
    if (MR::tryEndDemo(this, STR8A8(0x5C), MR::isActionEnd(this))) {
        MR::showPlayer();
        MR::endAnimCamera(this, mCameraInfo, STR8A8(0xC), -1, true);
        if (MR::isDemoEndWaitCameraInterpolating() != 0) {
            MR::tryForceDeleteEffectAll(this);
            MR::startMultiActorCameraTargetSelf(this, mCameraInfo, STR8A8(0x24), -1);
            setNerve(NRV(lbl_807D1D20));
            return;
        }
        setNerve(NRV(lbl_807D1D24));
    }
}

// NONMATCHING: fn_8010AB40 -> exeAppearEnd (endMultiActorCamera argument list approximated).
void Yukkina::exeAppearEnd() {
    if (MR::isGreaterStep(this, 3)) {
        MR::endMultiActorCamera(this, mCameraInfo, STRAT(lbl_8067B8CC), false, -1);
        setNerve(NRV(lbl_807D1D24));
    }
}

// NONMATCHING: fn_8010ABB0 -> exeWait. Body is a partial reconstruction (isNearPlayer/turn
// helpers have different signatures in the repo); the nerve transition logic is not reproduced.
void Yukkina::exeWait() {
    if (MR::isFirstStep(this)) {
        startWaitAction();
    }
    updateSound();
}

// NONMATCHING: fn_8010AD30 -> exeBreak. Partial reconstruction.
void Yukkina::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(5);
        MR::startAction(this, STR8A8(0x78));
        setTurnParam(mIdx);
        MR::calcVecToPlayerHorizon(NULL, this, 0);
    }
    if (MR::isGreaterStep(this, 0xA)) {
        setNerve(NRV(lbl_807D1D2C));
    }
}

// NONMATCHING: fn_8010AE80 -> exeBreakWait (sound stage check reconstructed partially).
void Yukkina::exeBreakWait() {
    MR::isFirstStep(this);
    if (MR::isGreaterStep(this, 0x3C)) {
        setNerve(NRV(lbl_807D1D24));
    }
}

// NONMATCHING: fn_8010AF00 -> exeJump.
void Yukkina::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, STR8A8(0x8));
    }
    if (MR::isGreaterStep(this, 0xA)) {
        setNerve(NRV(lbl_807D1D34));
    }
}

// NONMATCHING: fn_8010AFC0 -> exeJumpWait.
void Yukkina::exeJumpWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, STR8A8(0x18));
    }
    if (MR::isGreaterStep(this, 0x50)) {
        setNerve(NRV(lbl_807D1D38));
    }
}

// NONMATCHING: fn_8010B050 -> exeWaitTurn.
void Yukkina::exeWaitTurn() {
    if (MR::isFirstStep(this)) {
        updateSound();
    }
    if (MR::isGreaterStep(this, 0x5A)) {
        setNerve(NRV(lbl_807D1D24));
    }
}

// NONMATCHING: fn_8010B1D0 -> exeSwitchDead.
void Yukkina::exeSwitchDead() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0);
        MR::startActionSound(this, STR8A8(0x0), -1, -1, -1);
        MR::startAction(this, STR8A8(0x0));
    }
    if (MR::isActionEnd(this)) {
        setNerve(NRV(lbl_807D1D40));
    }
}

// NONMATCHING: fn_8010B2A0 -> exeDemoStart (demo start helper arguments approximated).
void Yukkina::exeDemoStart() {
    if (MR::tryStartDemoMarioPuppetable(this, STRAT(lbl_8067B97C))) {
        MR::showModel(this);
        MR::resetPosition(this);
        MR::startAnimCameraTargetSelf(this, mCameraInfo, STRAT(lbl_8067B8C0), false, 0, 1.0f);
        setNerve(NRV(lbl_807D1D44));
    }
}

// NONMATCHING: fn_8010B330 -> exeDemoEnd.
void Yukkina::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, STR8A8(0x18));
    }
    if (MR::isActionEnd(this)) {
        MR::endDemo(this, STR8A8(0xD4));
        if (mHasPlayerLink != 0) {
            MR::setPlayerLinkPosAndWait(this, STR8A8(0x40), false);
        }
        setNerve(NRV(lbl_807D1D48));
    }
}

// fn_8010B440
void Yukkina::setTurnParam(s32 arg) {
    switch (arg) {
    case 1:
        mHighAngle = 3.0f;
        mLowAngle = 1.0f;
        mRollSpeed = 1.5f;
        return;
    case 2:
        mHighAngle = 3.0f;
        mLowAngle = 1.0f;
        mRollSpeed = 2.0f;
        return;
    case 3:
        mHighAngle = 3.0f;
        mLowAngle = 0.0f;
        mRollSpeed = 3.0f;
        return;
    }
}

// fn_8010B4B0 (arg is a const char*, unused by callers in this unit)
void Yukkina::startWaitAction() {
    const char* pName = STR8A8(0x0);
    if (mHasPlayerLink == 1) {
        pName = STR8A8(0x8);
    }
    MR::startAction(this, pName);
    MR::startBva(this, STRAT(lbl_807CF678));
    MR::setBvaFrameAndStop(this, (f32)(3 - mHasPlayerLink));
}

// fn_8010B540
void Yukkina::updateSound() {
    f32 speed = MR::calcVelocityLength(this);
    MR::startActionSound(this, STRAT(lbl_8067B994), (s32)(3.0f * (speed * speed)), -1, -1);
    if (isBindedAndHigh()) {
        MR::startActionSound(this, STRAT(lbl_8067B99C), -1, -1, -1);
    }
}

// fn_8010B5D0
bool Yukkina::isNotIdle() {
    if (isNerve(NRV(lbl_807D1D1C)) || isNerve(NRV(lbl_807D1D44)) || isNerve(NRV(lbl_807D1D48))) {
        return false;
    }
    return true;
}

// fn_8010B640
bool Yukkina::isBindedAndHigh() {
    return MR::isBindedGround(this) && mCounter >= 0x14;
}
