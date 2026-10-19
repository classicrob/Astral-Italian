#include "Game/MapObj/DashRing.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "JSystem/JGeometry/TVec.hpp"

DashRing::DashRing(const char* pName) : LiveActor(pName) {
}

void DashRing::init(const JMapInfoIter& rIter) {
    mActiveTime = 0;
    mRevivalTime = 0;

    MR::initActor(this, rIter, false);
    MR::calcGravity(this);
    calcAxis();

    mBoostTime = 300;
    mMaxDuration = 120;
    mSpeedScale = 2.5f;
    mDoTurn = false;

    if (MR::checkJMapDataEntries(rIter)) {
        s32 arg0 = -1;
        s32 arg1 = -1;
        s32 arg2 = -1;

        MR::getJMapInfoArg0NoInit(rIter, &arg0);
        MR::getJMapInfoArg1NoInit(rIter, &arg1);

        if (arg0 != -1) {
            mBoostTime = arg0;
        }

        if (arg1 != -1) {
            mMaxDuration = arg1;
        }

        if (MR::getJMapInfoArg2NoInit(rIter, &arg2) && arg2 != -1) {
            mSpeedScale = arg2 / 100.0f;
        }

        s32 arg3 = -1;
        MR::getJMapInfoArg3NoInit(rIter, &arg3);

        if (arg3 != -1) {
            mDoTurn = true;
        }

        s32 arg7 = -1;
        MR::getJMapInfoArg7WithInit(rIter, &arg7);

        if (arg7 > -1) {
            MR::invalidateShadowAll(this);
        }
    }

    MR::useStageSwitchSyncAppear(this, rIter);
    MR::startAction(this, "Loop");

    mInitialPosition = mPosition;
}

void DashRing::initAfterPlacement() {
    MR::setClippingRangeIncludeShadow(this, &mClippingPos, 300.0f);
}

void DashRing::calcAndSetBaseMtx() {
    TPos3f pos;
    MR::makeMtxFrontSidePos(&pos, mAxis, mSubAxis, mPosition);
    PSMTXCopy(pos, getBaseMtx());
}

void DashRing::calcAxis() {
    TVec3f axis(0.0f, 1.0f, 0.0f);

    Mtx mtxTR;
    MR::makeMtxTR(mtxTR, 0.0f, 0.0f, 0.0f, mRotation.x, mRotation.y, mRotation.z);

    PSMTXMultVec(mtxTR, &axis, &axis);
    mAxis = axis;

    calcSubAxis();
}

void DashRing::calcSubAxis() {
    TVec3f axis(mAxis);
    TVec3f subAxis;

    if (!MR::isNearZero(axis.y)) {
        subAxis.x = axis.y;
        subAxis.y = -axis.x;
        subAxis.z = 0.0f;
    } else {
        subAxis.x = axis.z;
        subAxis.y = 0.0f;
        subAxis.z = -axis.x;
    }

    MR::normalizeOrZero(&subAxis);
    mSubAxis = subAxis;
}

void DashRing::updateAxisTowardsPlayer() {
    if (mActiveTime != 0) {
        return;
    }

    if (!mDoTurn) {
        return;
    }

    TVec3f dir(0.0f);
    MR::calcVecToPlayer(&dir, this);
    dir = -dir;

    TVec3f vec40;
    f32 val = MR::vecKillElement(dir, mAxis, &vec40);

    if (!(MR::abs(val) > 300.0f || vec40.length() > 300.0f)) {
        return;
    }

    if (MR::normalizeOrZero(&dir)) {
        return;
    }

    f32 f1 = MR::diffAngleAbs(mAxis, dir);

    if (f1 > MR::diffAngleAbs(mAxis, -dir)) {
        MR::vecBlendSphere(mAxis, -dir, &mAxis, 0.03f);
    } else {
        MR::vecBlendSphere(mAxis, dir, &mAxis, 0.03f);
    }

    calcSubAxis();
}

void DashRing::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mRevivalTime != 0) {
        return;
    }

    if (!MR::isSensorPlayer(pReceiver)) {
        return;
    }

    TVec3f dir(*MR::getSensorPos(pReceiver) - mPosition);
    TVec3f vec40;
    f32 val = MR::vecKillElement(dir, mAxis, &vec40);

    if (MR::abs(val) < 30.0f) {
        MR::sendArbitraryMsg(0x4B /* ACTMES_PASS_RING */, pReceiver, pSender);

        if (mActiveTime != 0) {
            return;
        }

        if (mRevivalTime != 0) {
            return;
        }

        mActiveTime = 15;

        MR::invalidateClipping(this);

        MR::startActionSound(this, "SyGetDashRing", -1, -1, -1);

        const char* pName = "Start";
        MR::startAction(this, pName);
    }
}

void DashRing::control() {
    updateAxisTowardsPlayer();

    if (mActiveTime != 0) {
        mActiveTime--;

        if (mActiveTime == 0) {
            mRevivalTime = 180;
            MR::invalidateHitSensors(this);

            const char* pName = "End";
            MR::startAction(this, pName);

            return;
        }

        mPosition = *MR::getPlayerPos() + *MR::getPlayerVelocity();

        if ((mPosition - mInitialPosition).length() > 2000.0f) {
            mPosition = mInitialPosition;
            mActiveTime = 0;
        }

        return;
    }

    if (mRevivalTime != 0) {
        mRevivalTime--;

        if (mRevivalTime != 0) {
            return;
        }

        mPosition = mInitialPosition;

        const char* pName = "Revival";
        MR::startAction(this, pName);

        MR::validateHitSensors(this);

        return;
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        const char* pName = "Loop";
        MR::startAction(this, pName);

        MR::validateClipping(this);
    }
}

void DashRing::draw() const {
    TVec3f axis, subAxis;
    axis = mAxis;
    subAxis = mSubAxis;

    f32 f1 = 1.0f;

    if (mActiveTime > 105) {
        f1 = static_cast< f32 >(mActiveTime - 105) / 15.0f;

        if (f1 > 1.0f) {
            f1 = 1.0f;
        }

        if (f1 < 0.1f) {
            f1 = 0.1f;
        }
    } else if (mActiveTime != 0) {
        f1 = 0.1f;
    }

    f32 f2 = mScale.x * 200.0f * f1;
    f32 segmentLength = (f2 * JMath::TAngleConstant_< f32 >::RADIAN_DEG360()) / 64.0f;

    TPos3f rotMtx;
    PSMTXRotAxisRad(rotMtx, axis, TWO_PI / 64.0f);

    TDDraw::setup(0, 1, 0);

    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);

    for (u32 idx = 0; idx <= 64; idx++) {
        TVec3f vecB0 = mPosition + subAxis * f2;

        TVec3f crossVec;
        crossVec.cross(subAxis, axis);

        TDDraw::drawCylinder(vecB0 - crossVec * segmentLength * 0.5f, crossVec * segmentLength, mScale.y * f1 * 30.0f, 0x00808060, 0x00808080, 0x10);

        PSMTXMultVec(rotMtx, subAxis, subAxis);
    }
}

DashRing::~DashRing() {
}
