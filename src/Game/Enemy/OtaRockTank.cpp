#include "Game/Enemy/OtaRockTank.hpp"
void fn_801DDD70(void* pThis);
#include "Game/Util.hpp"
#include "Game/Enemy/CocoNutBall.hpp"
#include "Game/Enemy/FireBall.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/Nerve.hpp"

// PLACEHOLDER(enemies): unnamed data at this address (owner unit not named); resolve when the owner unit names it.
extern TVec3f lbl_8072942C;
// PLACEHOLDER(enemies): unnamed data at this address (owner unit not named); resolve when the owner unit names it.
extern u8 lbl_8068E95C[];
// PLACEHOLDER(enemies): unnamed nerve/data in the .sbss range 0x807D2xxx (instance of an owner nerve class not yet named); resolve when the owner unit names it.
extern u32 lbl_807D2D8C;

// PLACEHOLDER(enemies): fn_8017E260 is a placement ctor (r3 = raw object, r4 = name data, sets a vtable); the class is not yet named; resolve when the class that owns vtable lbl_80686668 is ported.
void fn_8017E260(void* pThis, const void* pData);

// PLACEHOLDER(enemies): unnamed nerve/data in the .sbss range 0x807D2xxx (instance of an owner nerve class not yet named); resolve when the owner unit names it.
extern u32 lbl_807D2D78;
// PLACEHOLDER(enemies): unnamed nerve/data in the .sbss range 0x807D2xxx (instance of an owner nerve class not yet named); resolve when the owner unit names it.
extern u32 lbl_807D2D7C;
// PLACEHOLDER(enemies): unnamed nerve/data in the .sbss range 0x807D2xxx (instance of an owner nerve class not yet named); resolve when the owner unit names it.
extern u32 lbl_807D2D80;
// PLACEHOLDER(enemies): unnamed nerve/data in the .sbss range 0x807D2xxx (instance of an owner nerve class not yet named); resolve when the owner unit names it.
extern u32 lbl_807D2D84;

OtaRockTank::OtaRockTank(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mActorCameraInfo = nullptr;
    mAnimCameraInfo = nullptr;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mHasPowerStar = false;
    mUnkB0 = 3;
    mChief = nullptr;
    mJointCtrl0 = nullptr;
    mJointCtrl1 = nullptr;
    mUnkC4 = 0.0f;
    mUnkC8 = 1;
    mUnkCC = 0;
    mUnkD0 = 0;
    mCocoNutBalls = nullptr;
    mFireBalls = nullptr;
    mFixedPositions = nullptr;
    mRailClipPos.x = 0.0f;
    mRailClipPos.y = 0.0f;
    mRailClipPos.z = 0.0f;
    mRailDirection.x = 0.0f;
    mRailDirection.y = 0.0f;
    mRailDirection.z = 0.0f;
    mPrevRailDirection.x = 0.0f;
    mPrevRailDirection.y = 0.0f;
    mPrevRailDirection.z = 0.0f;
    mUnk10C = 10.0f;
    mUnk110 = 0xFF;
    mCurrentRailPointNo = 0;
    mUnk118 = 0;
    mUnk11C = 16.0f;
    mUnk120 = 0;
    mUnk121 = 0;
    mUnk122 = 0;
    mUnk124 = nullptr;
    mUnk128 = 2;
    mUnk12C = 0;
    mUnk130 = 0.0f;
}

void fn_801DDD70(void* pThis) {
    fn_8017E260(pThis, lbl_8068E95C);
}

void OtaRockTank::appear() {
    LiveActor::appear();
}

void OtaRockTank::control() {
    TVec3f trans[6];
    TVec3f dir[6];
    for (s32 i = 0; i < 6; i++) {
        FixedPosition* pFixedPos = mFixedPositions[i];
        pFixedPos->calc();
        pFixedPos->mMtx.getTrans(trans[i]);
        pFixedPos->mMtx.getZDir(dir[i]);
    }
    if (MR::isEffectValid(mChief, "AngrySmoke")) {
        MR::startActionSound(mChief, "Angry", -1, -1, -1);
    }
}

void OtaRockTank::kill() {
    LiveActor::kill();
    MR::endActorCamera(this, mActorCameraInfo, false, -1);
    if (mHasPowerStar) {
        MR::requestAppearPowerStar(this, mPosition);
    }
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void OtaRockTank::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mRailDirection);
    MR::extractMtxTrans(getBaseMtx(), &mPosition);
    TVec3f sp14(0.0f);
    PSMTXMultVecSR(getBaseMtx(), &lbl_8072942C, &sp14);
    TVec3f sp8(mPosition);
    sp8 += sp14;
    mPos = sp8;
    mJointCtrl0->registerCallBack();
    mJointCtrl1->registerCallBack();
    mJointCtrl2->registerCallBack();
}

f32 OtaRockTank::fn_801DFBB0() {
    bool flag = false;
    if (this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D78)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D7C)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D84)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D80))) {
        flag = true;
    }
    if (flag) {
        return 25.0f;
    }
    if (this->mUnkB0 == 1) {
        return 21.0f;
    }
    return 16.0f;
}

f32 OtaRockTank::fn_801DFC60() {
    bool flag = false;
    if (this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D78)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D7C)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D84)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D80))) {
        flag = true;
    }
    if (flag) {
        return 35.0f;
    }
    if (this->mUnkB0 == 1) {
        return 25.0f;
    }
    return 25.0f;
}

f32 OtaRockTank::fn_801DFD10() {
    bool flag = false;
    if (this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D78)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D7C)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D84)) ||
        this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D80))) {
        flag = true;
    }
    if (flag) {
        return 1000.0f;
    }
    if (this->mUnkB0 == 1) {
        return 900.0f;
    }
    return 800.0f;
}

bool OtaRockTank::fn_801E1010(TPos3f* pMtx) {
    return this->fn_801E1030(pMtx);
}

bool OtaRockTank::fn_801E1020(TPos3f* pMtx) {
    return this->fn_801E1030(pMtx);
}

bool OtaRockTank::fn_801E1030(TPos3f* pMtx) {
    if (this->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D2D8C))) {
        return false;
    }
    MR::rotateMtxLocalXDegree(reinterpret_cast<MtxPtr>(pMtx), this->mUnkC4);
    return true;
}
