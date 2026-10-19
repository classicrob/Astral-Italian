#include "Game/MapObj/ClipAreaDrop.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ClipArea.hpp"
#include "Game/MapObj/ClipAreaShape.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvClipAreaDrop {
    NEW_NERVE(ClipAreaDropNrvHold, ClipAreaDrop, Hold);
    NEW_NERVE(ClipAreaDropNrvWait, ClipAreaDrop, Wait);
};  // namespace NrvClipAreaDrop

ClipAreaDrop::ClipAreaDrop(const char* pName, const char* pShapeName)
    : ClipArea(pName), mShapeName(pShapeName), mShapeSphere(nullptr), mBaseSize(500.0f), mExtendsEndTime(15), mShrinkStartTime(60), mShrinkEndTime(240), mStep(0), mCountUp(true) {
    mShapeSphere = new ClipAreaShapeSphere(mShapeName);

    setShape(mShapeSphere);
}

void ClipAreaDrop::init(const JMapInfoIter& rIter) {
    initBaseMatrix(rIter);
    MR::connectToScene(this, 0x1B /* SMG2 MovementType_ClippedMapParts (our enum: 0x1C) */, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_ClipArea);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(ClipAreaDrop, ClipAreaDropNrvWait));
    makeActorDead();
}

void ClipAreaDrop::setBaseSize(f32 baseSize) {
    mBaseSize = baseSize;
}

void ClipAreaDrop::setExtendsEndTime(s32 time) {
    if (time >= 0) {
        mExtendsEndTime = time;
    } else {
        mExtendsEndTime = 15;
    }
}

void ClipAreaDrop::setShrinkTime(s32 startTime, s32 endTime) {
    if (startTime >= 0) {
        mShrinkStartTime = startTime;
    } else {
        mShrinkStartTime = 60;
    }

    if (endTime >= 0) {
        mShrinkEndTime = endTime;
    } else {
        mShrinkEndTime = 240;
    }
}

void ClipAreaDrop::startHold() {
    mShrinkStartTime = -1;
    mShrinkEndTime = -1;
    mExtendsEndTime = -1;
    mShapeSphere->mRadius = mBaseSize;
    setNerve(GET_NERVE(ClipAreaDrop, ClipAreaDropNrvHold));
}

void ClipAreaDrop::appear() {
    LiveActor::appear();

    mShapeSphere->mRadius = 0.0f;

    setNerve(GET_NERVE(ClipAreaDrop, ClipAreaDropNrvWait));
}

void ClipAreaDrop::control() {
    TPos3f mtx;
    mtx.set(getBaseMtx());
    mtx.setTrans(mPosition);

    mBaseMtx.set(mtx.mMtx);
}

void ClipAreaDrop::exeHold() {
    mShapeSphere->mRadius = mBaseSize;
}

void ClipAreaDrop::exeWait() {
    if (MR::isFirstStep(this)) {
        mStep = 0;
    }

    f32 radius;

    if (mStep < mExtendsEndTime) {
        radius = MR::getEaseOutValue(MR::normalizeAndClamp01(mStep, 0.0f, mExtendsEndTime), 0.0f, mBaseSize, 1.0f);
    } else {
        radius = MR::getEaseOutValue(MR::normalizeAndClamp01(mStep, mShrinkStartTime, mShrinkEndTime), mBaseSize, 0.0f, 1.0f);
    }

    mShapeSphere->mRadius = radius;

    if (mShrinkEndTime <= mStep) {
        kill();
    }

    if (mCountUp) {
        mStep++;
    }
}
