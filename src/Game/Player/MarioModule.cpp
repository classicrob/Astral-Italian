#include "Game/Player/MarioModule.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioMapCode.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "revolution/os.h"

bool MarioModule::checkSquat(bool a1) {
    if (MR::getKarikariClingNum() < 1 == false) {
        return false;
    }
    if (mActor->_468) {
        return false;
    }
    if (mActor->_B94 != 0) {
        return false;
    }
    if (mActor->mBeeWallWalk != 0) {
        return false;
    }
    if (mActor->mMario->mActor->mPlayerMode == 10) {
        if (mActor->getYoshi()->mDashTimer > 0) {
            return false;
        }
        if (mActor->mMarioAnim->isAnimationRun("空中ひねり")) {
            return false;
        }
    }
    if (((mActor->mMario->mMovementStates.mHighWord >> 28) & 1U)) {
        return false;
    }
    if (((mActor->mMario->mMovementStates.mHighWord >> 30) & 1U)) {
        if (!mActor->checkButtonType(9, false)) {
            mActor->mMario->mMovementStates.mHighWord &= 0xBFFFFFFFU;
        }
        return false;
    }
    if (!a1 && mActor->mMario->mVerticalSpeed > 100.0f) {
        return false;
    }
    return mActor->checkButtonType(9, false);
}

bool MarioModule::isInputDisable() const {
    if (((mActor->mMario->mMovementStates.mHighWord >> 29) & 1U)) {
        return true;
    }
    if (mActor->mMario->isStatusActive(MarioStatus_FpView)) {
        return true;
    }
    if (isAnimationRun("ハード着地")) {
        return true;
    }
    if (isAnimationRun("中ダメージ着地")) {
        return true;
    }
    if (isAnimationRun("中後ダメージ着地")) {
        return true;
    }
    if (isAnimationRun("ステージインB")) {
        return true;
    }
    return mActor->_3C0;
}

Mario* MarioModule::getPlayer() const {
    return mActor->mMario;
}

MarioAnimator* MarioModule::getAnimator() const {
    return mActor->mMarioAnim;
}

TVec3f& MarioModule::getShadowPos() const {
    return mActor->mMario->mShadowPos;
}

const TVec3f& MarioModule::getGravityVec() const {
    return *mActor->mMario->getGravityVec();
}

const TVec3f& MarioModule::getAirGravityVec() const {
    return mActor->mMario->getAirGravityVec();
}

TVec3f& MarioModule::getFrontVec() const {
    return mActor->mMario->mFrontVec;
}

bool MarioModule::isStatusActiveID(u32 id) const {
    return mActor->mMario->isStatusActive(id);
}

bool MarioModule::isStatusActiveS(MarioState* pState) const {
    return mActor->mMario->isStatusActive(pState->mStatusId);
}

u16 MarioModule::getPlayerMode() const {
    return mActor->mPlayerMode;
}

bool MarioModule::isPlayerModeBee() const {
    return mActor->mPlayerMode == PlayerMode_Bee;
}

bool MarioModule::isPlayerModeIce() const {
    return mActor->mPlayerMode == PlayerMode_Ice;
}

bool MarioModule::isPlayerModeTeresa() const {
    return mActor->mPlayerMode == PlayerMode_Teresa;
}

bool MarioModule::isPlayerModeHopper() const {
    return mActor->mPlayerMode == PlayerMode_Hopper;
}

bool MarioModule::isPlayerModeInvincible() const {
    return mActor->mPlayerMode == PlayerMode_Invincible;
}

bool MarioModule::isPlayerModeFoo() const {
    return mActor->mPlayerMode == PlayerMode_Foo;
}

void MarioModule::changeAnimation(const char* pAnim1, const char* pAnim2) {
    if (!mActor->_B90) {
        if (pAnim2) {
            mActor->mMarioAnim->changeDefault(pAnim2);
        }
        if (pAnim1) {
            mActor->mMarioAnim->change(pAnim1);
        }
    }
}

void MarioModule::changeAnimationNonStop(const char* pAnim) {
    if (!mActor->_B90) {
        if (pAnim) {
            changeAnimation(pAnim);
        }
        if (mActor->mMarioAnim->isFrameCtrlAttribute(0)) {
            mActor->mMarioAnim->setAnimationFrameCtrlAttribute(1);
        }
    }
}

void MarioModule::changeAnimationWithAttr(const char* pAnim, u32 attribute) {
    if (!mActor->_B90) {
        if (pAnim) {
            changeAnimation(pAnim);
        }
        mActor->mMarioAnim->setAnimationFrameCtrlAttribute(attribute);
    }
}

void MarioModule::stopAnimation(const char* pAnim1, const char* pAnim2) {
    // SMG2: the body moved into MarioAnimator::stopAnimation(const char*, const char*)
    if (!mActor->_B90) {
        mActor->mMarioAnim->stopAnimation(pAnim1, pAnim2);
    }
}

bool MarioModule::isDefaultAnimationRun(const char* pAnim) const {
    return mActor->mMarioAnim->isDefaultAnimationRun(pAnim);
}

void MarioModule::changeAnimationInterpoleFrame(u32 a1) {
    mActor->mMarioAnim->setAnimationInterpoleFrame(a1);
}

bool MarioModule::isAnimationRun(const char* pAnimName) const {
    return mActor->mMarioAnim->isAnimationRun(pAnimName);
}

bool MarioModule::isAnimationTerminate(const char* pAnim) const {
    return mActor->mMarioAnim->isAnimationTerminate(pAnim);
}

bool MarioModule::isAnimationTerminateUpper(const char* pAnim) const {
    if (pAnim) {
        return mActor->mMarioAnim->mXanimePlayerUpper->isTerminate(pAnim);
    }
    return mActor->mMarioAnim->mXanimePlayerUpper->isTerminate();
}

const char* MarioModule::getAnimationStringPointer(const char* pAnim) const {
    return mActor->mMarioAnim->getAnimationStringPointer(pAnim);
}

const char* MarioModule::getCurrentBckName() const {
    if (mActor->mMarioAnim->isAnimationRunSimple()) {
        return mActor->mMarioAnim->fn_803DF0D0();
    } else {
        return mActor->mMarioAnim->mCurrBck;
    }
}

void MarioModule::changeAnimationUpper(const char* pAnim1, const char* pAnim2) {
    if (!mActor->_B90 && mActor->mPlayerMode != 10) {
        mActor->mMarioAnim->changeDefaultUpper(pAnim2);
        if (pAnim1) {
            mActor->mMarioAnim->changeUpper(pAnim1);
        }
    }
}

void MarioModule::changeAnimationUpperWeak(const char* pAnim1, const char* pAnim2) {
    if (!mActor->mMarioAnim->_6C && !mActor->_B90) {
        mActor->mMarioAnim->changeDefaultUpper(pAnim2);
        if (pAnim1) {
            mActor->mMarioAnim->changeUpper(pAnim1);
        }
    }
}

void MarioModule::stopAnimationUpper(const char* pAnim1, const char* pAnim2) {
    if (!mActor->_B90) {
        mActor->mMarioAnim->changeDefaultUpper(pAnim2);
        mActor->mMarioAnim->stopUpper(pAnim1);
    }
}

void MarioModule::stopAnimationUpperForce() {
    mActor->mMarioAnim->stopUpper(nullptr);
}

f32 MarioModule::getAnimationFrame() const {
    return mActor->mMarioAnim->getFrame();
}

void MarioModule::changeAnimation(const char* pAnim, u32 index) {
    if (!mActor->_B90) {
        changeAnimation(pAnim);
        mActor->mMarioAnim->setAnimationFlag(index);
    }
}

bool MarioModule::isAnimationRun(const char* pAnim, u32 index) {
    return mActor->mMarioAnim->isAnimationRun(pAnim, index);
}

void MarioModule::stopAnimation(const char* pAnim, u32 index) {
    if (!mActor->_B90 && mActor->mMarioAnim->isAnimationFlag(index)) {
        stopAnimation(pAnim);
        mActor->mMarioAnim->resetAnimationFlag(index);
    }
}

void MarioModule::setJointGlobalMtx(u16 a1, MtxPtr mtx) {
    mActor->mMarioAnim->setMainJointMatrix(a1, mtx);
}

void MarioModule::setYangleOffset(f32 offset) {
    mActor->mMario->mYAngleOffset = offset;
}

bool MarioModule::isStickOn() const {
    return !MR::isNearZero(mActor->mMario->mStickPos.z);
}

bool MarioModule::isStickFull() const {
    return mActor->mMario->mStickPos.z > 0.9f;
}

TVec3f& MarioModule::getWorldPadDir() const {
    return mActor->mMario->mWorldPadDir;
}

bool MarioModule::calcWorldPadDir(TVec3f* pDest, f32 a2, f32 a3, bool a4) {
    pDest->zero();
    if (MR::isNearZero(a2) && MR::isNearZero(a3)) {
        return false;
    }
    if (!((mActor->mMario->mFlags14.mLowWord >> 14) & 1U)) {
        if (MR::abs(a3) > mActor->mConst->getTable()->mStickMarginYstart) {
            if (MR::abs(a2) < mActor->mConst->getTable()->mStickMarginX) {
                a2 = 0.0f;
            } else if (a2 > 0.0f) {
                a2 = (a2 - mActor->mConst->getTable()->mStickMarginX) / (1.0f - mActor->mConst->getTable()->mStickMarginX);
            } else {
                a2 = (a2 + mActor->mConst->getTable()->mStickMarginX) / (1.0f - mActor->mConst->getTable()->mStickMarginX);
            }
        } else if (MR::abs(a2) > mActor->mConst->getTable()->mStickMarginXstart) {
            if (MR::abs(a3) < mActor->mConst->getTable()->mStickMarginY) {
                a3 = 0.0f;
            } else if (a3 > 0.0f) {
                a3 = (a3 - mActor->mConst->getTable()->mStickMarginY) / (1.0f - mActor->mConst->getTable()->mStickMarginY);
            } else {
                a3 = (a3 + mActor->mConst->getTable()->mStickMarginY) / (1.0f - mActor->mConst->getTable()->mStickMarginY);
            }
        }
    }
    mActor->mMario->calcMoveDir(a2, a3, pDest, a4);
    return true;
}

void MarioModule::addVelocity(const TVec3f& rAdd) {
    mActor->mMario->mVelocity += rAdd;
}

void MarioModule::addVelocity(const TVec3f& rAdd, f32 scale) {
    TVec3f& velocity = mActor->mMario->mVelocity;
    velocity.scaleAdd(scale, rAdd);
}

void MarioModule::addVelocityAfter(const TVec3f& rAdd) {
    mActor->mMario->mVelocityAfter += rAdd;
}

void MarioModule::clearVelocity() {
    mActor->mMario->mVelocity.zero();
}

TVec3f& MarioModule::getVelocity() const {
    return mActor->mMario->mVelocity;
}

f32 MarioModule::cutGravityElementFromJumpVec(bool useActorGravity) {
    if (useActorGravity) {
        return MR::vecKillElement(mActor->getMario()->mJumpVec, mActor->mAppliedGravityVec, &mActor->getMario()->mJumpVec);
    } else {
        TVec3f* pJumpVec = &mActor->mMario->mJumpVec;
        return MR::vecKillElement(*pJumpVec, mActor->getGravityVec(), pJumpVec);
    }
}

f32 MarioModule::cutVecElementFromJumpVec(const TVec3f& rCut) {
    return MR::vecKillElement(mActor->mMario->mJumpVec, rCut, &mActor->mMario->mJumpVec);
}

TVec3f& MarioModule::getJumpVec() const {
    return mActor->mMario->mJumpVec;
}

void MarioModule::setJumpVec(const TVec3f& rVec) {
    mActor->mMario->mJumpVec = rVec;
}

void MarioModule::playEffect(const char* pEffectName) {
    mActor->playEffect(pEffectName);
}

void MarioModule::playEffectTrans(const char* pEffectName, const TVec3f& rTrans) {
    mActor->playEffectTrans(pEffectName, rTrans);
}

void MarioModule::playEffectRT(const char* pEffectName, const TVec3f& a2, const TVec3f& a3) {
    mActor->playEffectRT(pEffectName, a2, a3);
}

void MarioModule::playEffectRTZ(const char* pEffectName, const TVec3f& a2, const TVec3f& a3) {
    mActor->playEffectRTZ(pEffectName, a2, a3);
}

void MarioModule::playEffectRTW(const char* pEffectName, const TVec3f& a2, const TVec3f& a3) {
    mActor->playEffectRTW(pEffectName, a2, a3);
}

void MarioModule::playEffectSRT(const char* pEffectName, f32 a2, const TVec3f& a3, const TVec3f& a4) {
    mActor->playEffectSRT(pEffectName, a2, a3, a4);
}

void MarioModule::stopEffect(const char* pEffectName) {
    mActor->stopEffect(pEffectName);
}

void MarioModule::stopEffectForce(const char* pEffectName) {
    mActor->stopEffectForce(pEffectName);
}

void MarioModule::playSound(const char* pSoundName, s32 a2) {
    mActor->mMario->playSoundJ(pSoundName, a2);
}

void MarioModule::stopSound(const char* pSoundName, u32 a2) {
    mActor->mMario->stopSoundJ(pSoundName, a2);
}

void MarioModule::startCamVib(u32 strength) {
    switch (strength) {
    case 1:
        MR::shakeCameraWeak();
        return;
    case 2:
        MR::shakeCameraWeak();
        return;
    case 0:
        MR::shakeCameraNormal();
        return;
    case 3:
        MR::shakeCameraNormalStrong();
        return;
    case 4:
        MR::shakeCameraStrong();
        return;
    default:
        return;
    }
}

TVec3f& MarioModule::getCamDirX() const {
    return mActor->mCameraBasisX;
}

TVec3f& MarioModule::getCamDirY() const {
    return mActor->mCameraBasisY;
}

TVec3f& MarioModule::getCamDirZ() const {
    return mActor->mCameraBasisZ;
}

void MarioModule::startPadVib(u32 strength) {
    switch (strength) {
    case 1:
        MR::tryRumblePadVeryWeak(mActor, WPAD_CHAN0);
        return;
    case 0:
        MR::tryRumblePadWeak(mActor, WPAD_CHAN0);
        return;
    case 2:
        MR::tryRumblePadMiddle(mActor, WPAD_CHAN0);
        return;
    case 3:
        MR::tryRumblePadStrong(mActor, WPAD_CHAN0);
        return;
    default:
        return;
    }
}

void MarioModule::startPadVib(const char* pStrength) {
    MR::tryRumblePad(mActor, pStrength, WPAD_CHAN0);
}

f32 MarioModule::getStickX() const {
    return mActor->mMario->mStickPos.x;
}

f32 MarioModule::getStickY() const {
    return mActor->mMario->mStickPos.y;
}

f32 MarioModule::getStickP() const {
    return mActor->mMario->mStickPos.z;
}

bool MarioModule::checkTrgA() const {
    return mActor->checkButtonType(3, false);
}

bool MarioModule::checkLvlA() const {
    return mActor->checkButtonType(5, false);
}

bool MarioModule::checkTrgZ() const {
    return mActor->checkButtonType(7, false);
}

bool MarioModule::checkLvlZ() const {
    return mActor->checkButtonType(9, false);
}

bool MarioModule::checkPreLvlZ() const {
    return mActor->checkButtonType(9, true);
}

TVec3f& MarioModule::getTrans() const {
    return mActor->mMario->mPosHolder->mPos;
}

const Triangle* MarioModule::getGroundPolygon() const {
    return mActor->mMario->mGroundPolygon;
}

bool MarioModule::isOnSlipGround() const {
    Mario* pMario = mActor->mMario;
    if (((pMario->mMovementStates.mLowWord >> 30) & 1U) && pMario->isSlipPolygon(pMario->mGroundPolygon)) {
        return true;
    }
    return false;
}

bool MarioModule::isSlipPolygon(const Triangle* triangle) const {
    if (!triangle->isValid()) { return false; }
    switch (mActor->mMario->_95C->getCode(triangle)) {
    case 2:
        return !(calcAngleD(*triangle->getNormal(0)) <= 3.0f);
    case 3: case 13: case 20: case 21: case 22: case 30: case 39: case 43:
        return false;
    default:
        f32 angle = calcAngleD(*triangle->getNormal(0));
        return angle >= mActor->getConst().getTable()->mFlatAngle - 0.5f;
    }
}

bool MarioModule::isSlipFloorCode(s32 a1) const {
    switch (a1) {
    case 2:
        return true;
    case 3:
    case 0xd:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x1e:
    case 0x27:
    case 0x2b:
        return false;
    }
    return true;
}

u32 MarioModule::fn_803EF7C0() const {
    u32 code = mActor->mMario->_95C->getCode(mActor->mMario->mGroundPolygon);
    if (mActor->mPlayerMode == 10 && code == 0x20) code = 0;
    return code;
}

u32 MarioModule::getFloorCode() const {
    return mActor->mMario->_95C->getCode(mActor->mMario->mGroundPolygon);
}

f32 MarioModule::calcPolygonAngleD(const Triangle* pTri) const {
    if (pTri->isValid() == false) {
        return 0.0f;
    }
    if (MR::isNearZero(*pTri->getNormal(0))) {
        return 0.0f;
    }
    return calcAngleD(*pTri->getNormal(0));
}

f32 MarioModule::calcAngleD(const TVec3f& rVec) const {
    f32 f31 = (MR::acosEx(-mActor->mMario->getAirGravityVec().dot(rVec)) / 3.14159f) * 180.0f;
    if (MR::isNearZero(f31, 1.0f)) {
        f31 = 0.0f;
    }
    if (MR::isNearZero(f31 - 90.0f, 1.0f)) {
        f31 = 90.0f;
    }
    return f31;
}

Triangle* MarioModule::getTmpPolygon() const {
    return mActor->mMario->mTmpPolygon;
}

bool MarioModule::sendPunch(HitSensor* pSensor, const Triangle* pTriangle, bool a2) {
    if (!pSensor) {
        return false;
    }
    bool ret = mActor->sendPunch(pSensor, pTriangle, a2);
    if (ret) {
        mActor->reactionPunch(pSensor);
    }
    return ret;
}

HitSensor* MarioModule::getSensor(const Triangle* pTri) const {
    if (pTri->isValid() == false) {
        return nullptr;
    } else {
        return pTri->mSensor;
    }
}

f32 MarioModule::marioAcos(f32 a1) const {
    return MR::acosEx(a1);
}


bool MarioModule::isPlayerModeFire() const { return mActor->mPlayerMode == 2; }
bool MarioModule::isPlayerModeYoshi() const { return mActor->mPlayerMode == 10; }
bool MarioModule::isPlayerModeCloud() const { return mActor->mPlayerMode == 12; }
void MarioModule::fn_803EE8F0(const char* name) {
    if (mActor->mPlayerMode == 10) {
        mActor->getYoshi()->fn_8040EDA0(name);
    }
}
bool MarioModule::isAnimationStop() const { return mActor->mMarioAnim->isAnimationStop(); }
bool MarioModule::isMainAnimationRun(const char* name) const { return mActor->mMarioAnim->isMainAnimationRun(name); }
bool MarioModule::fn_803EEA90(const char* name) const { return mActor->mMarioAnim->isUpperAnimationTerminated(name); }
f32 MarioModule::getFrame() const { return mActor->mMarioAnim->getFrame(); }
f32 MarioModule::getAnimationTrackWeight(u32 track) { return mActor->mMarioAnim->getAnimationTrackWeight(track); }
void MarioModule::fn_803EEC20(const char* name, const char* defaultName) {
    if (!mActor->mMarioAnim->isUpperAnimationActive()) {
        if (!mActor->_B90) {
            mActor->mMarioAnim->changeDefaultUpper(defaultName);
            if (name) { mActor->mMarioAnim->changeUpper(name); }
        }
    }
}
s32 MarioModule::fn_803EF000() const { return mActor->checkButtonType(4, false); }
const TVec3f& MarioModule::fn_803EF300() const { return mActor->mMario->_16C; }
const TVec3f& MarioModule::fn_803EF5B0() const { return mActor->mMario->mShadowPos; }
TVec3f& MarioModule::getCamPos() const { return mActor->mCameraPosition; }

Triangle* MarioModule::fn_803EFA20() const {
    return mActor->mMario->mTmpPolygon;
}

bool MarioModule::fn_803EFA30(const HitSensor* sensor, const char* name) const {
    return mActor->mActionTable1080->testEffect(MR::getSensorHost(sensor)->mName, name);
}
