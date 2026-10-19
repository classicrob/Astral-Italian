#include "Game/Camera/CameraParallel.hpp"
#include "Game/Camera/CamTranslatorParallel.hpp"
#include "Game/Camera/CameraCalc.hpp"
#include "Game/Camera/CameraHeightArrange.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include <cmath>

namespace {
    static const f32 sDistance = 3000.0f;
    static const f32 sRoundDiv = 8.0f;
    static const f32 sRoundAddition = 0.08f;
};  // namespace

CameraParallel::CameraParallel(const char* pName)
    : Camera(pName), mAngleX(), mAngleY(), mDist(::sDistance), mIsEnableValid(), mRoundIndex(0) {
    createVPanObj();
    for (s32 i = 0; i < 8; i++) {
        mIsRounding[i] = false;
        mRoundTarget[i] = 0.0f;
        mRoundAngle[i] = 0.0f;
        mRoundAddition[i] = 0.0f;
    }
}

void CameraParallel::reset() {
    mVPan->resetJump();
    mVPan->calcWatchPos(nullptr);
    calcIdealPose();
    mVPan->reset();
    mIsRounding[0] = false;
    mRoundTarget[0] = 0.0f;
    mRoundAngle[0] = 0.0f;
    mRoundAddition[0] = ::sRoundAddition;
}

CameraTargetObj* CameraParallel::calc() {
    mVPan->updateJump();
    mVPan->calcWatchPos(nullptr);
    calcIdealPose();
    mVPan->arrange();

    return CameraLocalUtil::getTarget(this);
}

CamTranslatorBase* CameraParallel::createTranslator() {
    return new CamTranslatorParallel(this);
}

bool CameraParallel::isEnableToRoundLeft() const {
    return mIsEnableValid;
}

bool CameraParallel::isEnableToRoundRight() const {
    return mIsEnableValid;
}

bool CameraParallel::isEnableToReset() const {
    return mIsEnableValid;
}

void CameraParallel::calcIdealPose() {
    if (mDist < 300.0f) {
        mDist = 300.0f;
    }

    TVec3f watchPos = CameraLocalUtil::getWatchPos(this);

    TVec3f pos;
    MR::polarToCrossDegree(TVec3f(0.0f, 0.0f, 0.0f), &pos, mDist, mAngleX, mAngleY - 90.0f);

    TVec3f up;
    up.set< f32 >(0.0f, 1.0f, 0.0f);
    calcRound();

    f32 length, angleX, angleY;
    MR::crossToPolar(TVec3f(0.0f, 0.0f, 0.0f), pos, &length, &angleX, &angleY);
    angleY += mRoundAngle[mRoundIndex];
    MR::polarToCross(TVec3f(0.0f, 0.0f, 0.0f), &pos, length, angleX, angleY);

    mZoneMatrix.mult33(pos);
    mZoneMatrix.mult33(up);

    pos.add(watchPos);
    CameraLocalUtil::setPos(this, pos);
    CameraLocalUtil::setUpVec(this, up);
    CameraLocalUtil::setWatchPos(this, watchPos);
    CameraLocalUtil::setWatchUpVec(this, up);
}

void CameraParallel::calcRound() {
    if (!mIsEnableValid) {
        return;
    }

    if (mIsRounding[mRoundIndex]) {
        if (mRoundTarget[mRoundIndex] > mRoundAngle[mRoundIndex]) {
            mRoundAngle[mRoundIndex] += ::sRoundAddition;
            if (mRoundAngle[mRoundIndex] >= mRoundTarget[mRoundIndex]) {
                mRoundAngle[mRoundIndex] = mRoundTarget[mRoundIndex];
                mIsRounding[mRoundIndex] = false;
            }
        } else {
            mRoundAngle[mRoundIndex] -= ::sRoundAddition;
            if (mRoundAngle[mRoundIndex] <= mRoundTarget[mRoundIndex]) {
                mRoundAngle[mRoundIndex] = mRoundTarget[mRoundIndex];
                mIsRounding[mRoundIndex] = false;
            }
        }
        return;
    }

    f32 roundInterval = MR::toRadian(360.0f / ::sRoundDiv);

    if (CameraLocalUtil::testCameraPadTriggerReset(true)) {
        mRoundTarget[mRoundIndex] = 0.0f;
        mIsRounding[mRoundIndex] = true;
        mRoundAddition[mRoundIndex] = ::sRoundAddition * 2.0f;
        return;
    }

    if (testRoundLeft()) {
        s32 divNum = (mRoundAngle[mRoundIndex] < 0.0f ? -1 : 1) * (s32)(((roundInterval / 2) + MR::abs(mRoundAngle[mRoundIndex])) / roundInterval) - 1;

        if (divNum < -::sRoundDiv / 2) {
            divNum += ::sRoundDiv;
            mRoundAngle[mRoundIndex] += MR::pi() * 2.0f;
        }

        mRoundTarget[mRoundIndex] = divNum * roundInterval;
        mIsRounding[mRoundIndex] = true;
        mRoundAddition[mRoundIndex] = ::sRoundAddition;
        return;
    }

    if (testRoundRight()) {
        s32 divNum = (mRoundAngle[mRoundIndex] < 0.0f ? -1 : 1) * (s32)(((roundInterval / 2) + MR::abs(mRoundAngle[mRoundIndex])) / roundInterval) + 1;

        if (divNum > ::sRoundDiv / 2) {
            divNum -= ::sRoundDiv;
            mRoundAngle[mRoundIndex] -= MR::pi() * 2.0f;
        }

        mRoundTarget[mRoundIndex] = divNum * roundInterval;
        mIsRounding[mRoundIndex] = true;
        mRoundAddition[mRoundIndex] = ::sRoundAddition;
        return;
    }
}

bool CameraParallel::isRoundControlReversed() const {
    f32 roll = CameraLocalUtil::getRoll(this);
    roll = -MR::pi() + static_cast<f32>(fmod(2.0f * MR::pi() + (roll - -MR::pi()), static_cast<f64>(2.0f * MR::pi())));
    return roll < -MR::pi() / 2.0f || roll > MR::pi() / 2.0f;
}

bool CameraParallel::testRoundLeft() const {
    if (isRoundControlReversed()) return CameraLocalUtil::testCameraPadTriggerRoundRight();
    return CameraLocalUtil::testCameraPadTriggerRoundLeft();
}

bool CameraParallel::testRoundRight() const {
    if (isRoundControlReversed()) return CameraLocalUtil::testCameraPadTriggerRoundLeft();
    return CameraLocalUtil::testCameraPadTriggerRoundRight();
}
