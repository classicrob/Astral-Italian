#include "Game/MapObj/ElectricRailMoving.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/Gravity/GravityInfo.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/SchedulerUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <revolution/gd/GDBase.h>
#include <revolution/gx/GXEnum.h>


namespace {
    const char* cSensorNameTable[] = {"body0", "body1", "body2", "body3", "body4", "body5", "body6", "body7"};
};  // namespace

namespace NrvElectricRailMoving {
    NEW_NERVE(ElectricRailMovingNrvWait, ElectricRailMoving, Wait);
    NEW_NERVE(ElectricRailMovingNrvFreeze, ElectricRailMoving, Freeze);
    NEW_NERVE(ElectricRailMovingNrvDisappear, ElectricRailMoving, Disappear);
    NEW_NERVE(ElectricRailMovingNrvDisappeared, ElectricRailMoving, Disappeared);
};  // namespace NrvElectricRailMoving

ElectricRailMovingPoint::ElectricRailMovingPoint(const char* pName) : LiveActor(pName) {
    mHasShadow = 1;
    mIsActive = 1;
}

void ElectricRailMovingPoint::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "ElectricRailPoint", false);
    MR::connectToSceneMapObjDecoration(this);

    if (mHasShadow) {
        MR::initShadowVolumeSphere(this, 35.0f);
        MR::onCalcGravity(this);
    }

    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Body", 8, 35.0f, TVec3f(0.0f, 0.0f, 0.0f));
    TVec3f pointerOffset;
    pointerOffset.x = 0.0f;
    pointerOffset.y = 0.0f;
    pointerOffset.z = 0.0f;
    MR::initStarPointerTarget(this, 50.0f, pointerOffset);
    initEffectKeeper(1, "ElectricRailMovingPoint", false);
    MR::addEffect(this, "PointerTouch");
    makeActorAppeared();
}

void ElectricRailMovingPoint::control() {
    f32 radius;
    if (MR::isPlayerHipDropFalling()) {
        radius = ElectricRailFunction::getHipDropSensorRadius();
    } else {
        radius = 35.0f;
    }
    MR::setSensorRadius(this, "Body", radius);
}

void ElectricRailMovingPoint::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (mIsActive) {
            MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

ElectricRailMoving::ElectricRailMoving(const char* pName)
    : LiveActor(pName), mSegmentNum(10), mMovementSpeed(10.0f), mSegmentLength(500.0f), mStackHeight(1), mAlpha(0x80), mSensorOffsets(), mDisplayListBuffer(), mDisplayListSize(),
      mRailPosCount(), mRailPositions(), mScrollCoord(), mEaseIn(30.0f) {
    mTexMtx.identity();
}

void ElectricRailMoving::init(const JMapInfoIter& rIter) {
    s32 arg = 0;
    MR::getJMapInfoArg4NoInit(rIter, &arg);
    ElectricRailFunction::registerRail(this, (ElectricRailType)(arg + 2));
    MR::connectToSceneMapObjMovement(this);
    initMapToolInfo(rIter);
    initRail(rIter);
    initSensor();
    MR::setClippingFar200m(this);
    MR::setGroupClipping(this, rIter, 16);
    initPoints();
    initDisplayList();
    initSound(4, "ElectricRailMoving", mSensorOffsets, TVec3f(0.0f));
    initNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvWait));
    makeActorAppeared();
}

void ElectricRailMoving::draw() const {
    if (MR::isValidDraw(this)) {
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, 0x21, 0, 0x7D);
        GXLoadTexMtxImm(mTexMtx, 0x21, GX_MTX2x4);
        GXSetTevColor(GX_TEVREG1, Color8(0xFF, 0xFF, 0xFF, mAlpha));

        if (isNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvWait)) ||
            isNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvFreeze))) {
            GXCallDisplayList(mDisplayListBuffer, mDisplayListSize);
        } else {
            drawRailGX(mEaseIn);
        }
    }
}

void ElectricRailMoving::disappear() {
    setNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvDisappear));
}

bool ElectricRailMoving::isTouchRail(const HitSensor* pSensor, TVec3f* pPos, TVec3f* pDirection) const {
    f32 radius;
    f32 coord;

    TVec3f nearest;
    coord = MR::calcNearestRailPos(&nearest, this, *MR::getSensorPos(pSensor));
    f32 railRadius = ElectricRailFunction::getHitSensorRadius();
    radius = MR::getSensorRadius(const_cast< HitSensor* >(pSensor));
    radius += railRadius;

    if (isValidCoord(coord) && nearest.squared(*MR::getSensorPos(pSensor)) < radius * radius) {
        if (pPos != nullptr) {
            pPos->set(nearest);
        }

        if (pDirection != nullptr) {
            MR::calcRailDirectionAtCoord(pDirection, this, coord);
        }

        return true;
    }

    return false;
}

void ElectricRailMoving::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (!MR::isPlayerSquat()) {
            MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
        }
    }
}

void ElectricRailMoving::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mSegmentNum);
    MR::getJMapInfoArg1NoInit(rIter, &mMovementSpeed);
    MR::getJMapInfoArg3NoInit(rIter, &mStackHeight);

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::Functor_Inline(this, &ElectricRailMoving::disappear));
    }
}

void ElectricRailMoving::initSensor() {
    initHitSensor(mStackHeight);
    mSensorOffsets = new TVec3f[mStackHeight];

    for (s32 i = 0; i < mStackHeight; i++) {
        MR::addHitSensorPosMapObj(this, ::cSensorNameTable[i], 8, ElectricRailFunction::getHitSensorRadius(), &mSensorOffsets[i],
                                  TVec3f(0.0f, 0.0f, 0.0f));
    }

    updateHitSensorPos();
}

void ElectricRailMoving::initRail(const JMapInfoIter& rIter) {
    initRailRider(rIter);
    f32 v11;
    MR::calcRailClippingInfo(&mPosition, &v11, this, 100.0f, 500.0f);
    MR::setClippingTypeSphere(this, v11, &mPosition);

    if (!MR::getJMapInfoArg2NoInit(rIter, &mSegmentLength)) {
        mSegmentLength = MR::getRailTotalLength(this) / (2 * mSegmentNum);
    }

    mRailPosCount = static_cast< s32 >(MR::getRailTotalLength(this) / 100.0f) + 2;
    mRailPositions = new TVec3f[mRailPosCount];

    for (s32 i = 0; i < mRailPosCount; i++) {
        f32 totalLength = MR::getRailTotalLength(this);
        f32 len = 100.0f * i >= totalLength ? totalLength : 100.0f * i;

        MR::calcRailPosAtCoord(&mRailPositions[i], this, len);
    }
}

void ElectricRailMoving::initPoints() {
    mMovingPointCount = 2 * mSegmentNum;

    if (!MR::isLoopRail(this)) {
        mMovingPointCount += 2;
    }

    mMovingPointCount *= mStackHeight;
    mMovingPoints = new ElectricRailMovingPoint[mMovingPointCount];

    if (!MR::isLoopRail(this)) {
        u32 v4 = mMovingPointCount - 2 * mStackHeight;
        TVec3f startPos, endPos;
        MR::calcRailStartPointPos(&startPos, this);
        MR::calcRailEndPointPos(&endPos, this);

        MR::setPosition(&mMovingPoints[v4++], startPos);
        MR::setPosition(&mMovingPoints[v4++], endPos);
        u32 v6 = v4;

        if (mStackHeight > 1) {
            TVec3f g;
            MR::calcGravityVector(this, startPos, &g, nullptr, 0);
            g.scale(-100.0f);

            TVec3f v16;
            v16.set(startPos);
            u32 v8 = v6;

            for (s32 i = 1; i < mStackHeight; i++) {
                v16.add(g);
                MR::setPosition(&mMovingPoints[v6++], v16);
            }

            MR::calcGravityVector(this, endPos, &g, nullptr, 0);
            g.scale(-100.0f);
            v16.set(endPos);

            for (s32 i = 1; i < mStackHeight; i++) {
                v16.add(g);
                MR::setPosition(&mMovingPoints[v6++], v16);
            }
        }
    }

    updatePointPos();
    s32 v14 = 0;
    s32 v15 = 0;

    while (v14 < mMovingPointCount) {
        if (v14 % (2 * mStackHeight) >= 2) {
            mMovingPoints[v15].mHasShadow = 0;
        }

        mMovingPoints[v15].initWithoutIter();
        v14++;
        v15++;
    }
}

void ElectricRailMoving::initDisplayList() {
    MR::ProhibitSchedulerAndInterrupts interrupt(false);
    mDisplayListSize = ElectricRailFunction::calcDisplayListSize(20, mRailPosCount * 4 * mStackHeight);
    mDisplayListBuffer = new (32) u8[mDisplayListSize];

    GDLObj dl;
    GDInitGDLObj(&dl, mDisplayListBuffer, mDisplayListSize);
    __GDCurrentDL = &dl;

    drawPlane(30.0f, 30.0f, -30.0f, -30.0f);
    drawPlane(-30.0f, 30.0f, 30.0f, -30.0f);
    GDPadCurr32();
    mDisplayListSize = dl.ptr - dl.start;
    DCStoreRange(mDisplayListBuffer, mDisplayListSize);
}

void ElectricRailMoving::drawRailGX(f32 a1) const {
    drawPlaneGX(a1, a1, -a1, -a1);
    drawPlaneGX(-a1, a1, a1, -a1);
}

void ElectricRailMoving::drawPlane(f32 a2, f32 a3, f32 a4, f32 a5) const {
    for (s32 i = 0; i < mStackHeight; i++) {
        u32 vertexFormat = (2 * mRailPosCount) & 0xFFFE;

        GDOverflowCheck(1);
        *__GDCurrentDL->ptr++ = 0x98;

        GDOverflowCheck(2);
        *__GDCurrentDL->ptr++ = vertexFormat >> 8;
        *__GDCurrentDL->ptr++ = vertexFormat;

        for (s32 j = 0; j < mRailPosCount; j++) {
            setVertexAttribute(j, i, a2, a3, a4, a5, GDPosition3f32, GDTexCoord2f32);
        }
    }
}

void ElectricRailMoving::drawPlaneGX(f32 a1, f32 a2, f32 a3, f32 a4) const {
    for (s32 i = 0; i < mStackHeight; i++) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, mRailPosCount * 2);

        for (s32 j = 0; j < mRailPosCount; j++) {
            setVertexAttribute(j, i, a1, a2, a3, a4, GXPosition3f32, GXTexCoord2f32);
        }
    }
}

void ElectricRailMoving::setVertexAttribute(int a2, int a3, f32 a4, f32 a5, f32 a6, f32 a7, PosAttrFunc pPosAttr, TexAttrFunc pTexAttr) const {
    f32 railLength = MR::getRailTotalLength(this);

    if (100.0f * a2 >= railLength) {
        railLength = railLength;
    } else {
        railLength = 100.0f * a2;
    }

    f32 texCoord = 0.25f * railLength / 100.0f;
    TVec3f position;
    position.set(mRailPositions[a2]);
    TVec3f railDirection;
    TVec3f gravity;

    MR::calcRailDirectionAtCoord(&railDirection, this, railLength);
    MR::calcGravityVector(this, position, &gravity, 0, 0);

    gravity.negate();

    if (a3 > 0) {
        f32 height = 100.0f * a3;
        position += gravity * height;
    }

    TVec3f axis1;
    TVec3f axis2;
    TVec3f negRailDirection = -railDirection;

    if (MR::isSameDirection(gravity, negRailDirection)) {
        MR::makeAxisCrossPlane(&axis1, &axis2, negRailDirection);
    } else {
        MR::makeAxisFrontUp(&axis1, &axis2, negRailDirection, gravity);
    }

    pPosAttr(position.x + axis1.x * a4 + axis2.x * a5, position.y + axis1.y * a4 + axis2.y * a5, position.z + axis1.z * a4 + axis2.z * a5);
    pTexAttr(texCoord, 0.0f);
    pPosAttr(position.x + axis1.x * a6 + axis2.x * a7, position.y + axis1.y * a6 + axis2.y * a7, position.z + axis1.z * a6 + axis2.z * a7);
    pTexAttr(texCoord, 1.0f);
}

void ElectricRailMoving::updateHitSensorPos() {
    f32 nearestRailPos = MR::calcNearestRailPos(mSensorOffsets, this, *MR::getPlayerCenterPos());

    if (mStackHeight > 1) {
        TVec3f v9;
        MR::calcGravityVector(this, mSensorOffsets[0], &v9, nullptr, 0);
        v9 *= -100.0f;

        s32 v5 = 1;

        for (s32 i = 1; i < mStackHeight; i++) {
            mSensorOffsets[i].add(mSensorOffsets[i - 1], v9);
        }
    }

    bool isValid = isValidCoord(nearestRailPos);

    f32 radius;
    if (MR::isPlayerHipDropFalling()) {
        radius = ElectricRailFunction::getHipDropSensorRadius();
    } else {
        radius = ElectricRailFunction::getHitSensorRadius();
    }
    for (s32 i = 0; i < mStackHeight; i++) {
        if (isValid) {
            MR::validateHitSensor(MR::getSensorWithIndex(this, i));
            MR::setSensorRadius(MR::getSensorWithIndex(this, i), radius);
        } else {
            MR::invalidateHitSensor(MR::getSensorWithIndex(this, i));
        }
    }
    if (isValid && MR::isInTowerModeCylinder(mSensorOffsets[0], nullptr, nullptr, nullptr)) {
        for (s32 i = 0; i < mStackHeight; i++) {
            MR::fn_800143D0(&mSensorOffsets[i]);
        }
    }
}

bool ElectricRailMoving::updatePointPos() {
    bool reachedEnd;
    s32 pointIndex;
    s32 i;
    f32 railTotalLength = MR::getRailTotalLength(this);
    s32 segmentNum = mSegmentNum;

    f32 segmentLength;
    f32 railCoord;
    segmentLength = railTotalLength / segmentNum;
    reachedEnd = false;
    pointIndex = 0;
    railCoord = mScrollCoord;

    TVec3f position;
    TVec3f previousPosition;
    TVec3f gravity;
    TVec3f stackPosition;

    for (i = 0; i < mSegmentNum; i++) {
        f32 coord = getRepeatedCoord(railCoord);

        calcPointPos(&position, coord);
        f32 previousCoord = getRepeatedCoord(railCoord - mSegmentLength);

        calcPointPos(&previousPosition, previousCoord);

        MR::setPosition(&mMovingPoints[pointIndex++], position);
        MR::setPosition(&mMovingPoints[pointIndex++], previousPosition);

        if (mStackHeight > 1) {
            MR::calcGravityVector(this, position, &gravity, nullptr, 0);
            gravity *= -100.0f;

            stackPosition.set(position);

            for (s32 j = 1; j < mStackHeight; j++) {
                stackPosition += gravity;
                MR::setPosition(&mMovingPoints[pointIndex++], stackPosition);
            }

            MR::calcGravityVector(this, previousPosition, &gravity, nullptr, 0);
            gravity *= -100.0f;

            stackPosition.set(previousPosition);

            for (s32 j = 1; j < mStackHeight; j++) {
                stackPosition += gravity;
                MR::setPosition(&mMovingPoints[pointIndex++], stackPosition);
            }
        }

        if (!MR::isLoopRail(this) &&
            (getRepeatedCoord(railCoord + mMovementSpeed) - (mSegmentLength + (3.0f * mMovementSpeed))) < 0.0f) {
            reachedEnd = true;
        }

        railCoord = getRepeatedCoord(railCoord + segmentLength);
    }

    return reachedEnd;
}

void ElectricRailMoving::updatePointPosAndModel() {
    bool updated = updatePointPos();

    if (!MR::isLoopRail(this)) {
        if (updated) {
            for (s32 i = 0; i < 2 * mStackHeight; i++) {
                ElectricRailMovingPoint* point = &mMovingPoints[mMovingPointCount - 1 - i];
                MR::showModelIfHidden(point);
                MR::validateHitSensors(point);
            }
        } else {
            for (s32 i = 0; i < 2 * mStackHeight; i++) {
                ElectricRailMovingPoint* point = &mMovingPoints[mMovingPointCount - 1 - i];
                MR::hideModelIfShown(point);
                MR::invalidateHitSensors(point);
            }
        }
    }
}

bool ElectricRailMoving::isValidCoord(f32 coord) const {
    f32 repeated = getRepeatedCoord(coord - mScrollCoord);
    f32 interval = MR::getRailTotalLength(this) / mSegmentNum;
    f32 offset = MR::mod(repeated, interval);
    return interval - mSegmentLength < offset;
}

f32 ElectricRailMoving::getRepeatedCoord(f32 coord) const {
    f32 len = MR::getRailTotalLength(this);
    return MR::repeat(coord, 0.0f, len);
}

void ElectricRailMoving::calcPointPos(TVec3f* pOutPos, f32 c) const {
    s32 v5 = (c / 100.0f);
    f32 v6 = MR::mod(c, 100.0f);
    f32 v7 = 100.0f;

    if ((mRailPosCount - 3) < v5) {
        v7 = (MR::getRailTotalLength(this) - (100.0f * v5));
    }

    pOutPos->lerp(mRailPositions[v5], mRailPositions[v5 + 1], (v6 / v7));
}

#pragma push
#pragma global_optimizer off
void ElectricRailMoving::move(f32 speed) {
    mScrollCoord = getRepeatedCoord(mScrollCoord + speed);

    f32 scale = (100.0f * mSegmentNum) / (0.25f * MR::getRailTotalLength(this));

    mTexMtx.identity();

    mTexMtx.mMtx[0][0] = scale;
    mTexMtx.mMtx[0][3] = -((0.25f * (scale * mScrollCoord)) / 100.0f);
    f32 railTotalLength2 = MR::getRailTotalLength(this);
    f32 segmentLength = railTotalLength2 / mSegmentNum;
    f32 angle = (-TWO_PI * mSegmentLength) / segmentLength;
    f32 sinValue;
    sinValue = JMASinRadian(angle);
    f32 value = (15.0f * sinValue) + ((255.0f * mSegmentLength) / segmentLength);

    s32 c;
    if (static_cast< s32 >(value) < 0) {
        c = 0;
    } else if (static_cast< s32 >(value) > 255) {
        c = 255;
    } else {
        c = static_cast< s32 >(value);
    }

    mAlpha = c;

    updatePointPosAndModel();
}
#pragma pop

void ElectricRailMoving::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjElecRailVanish", -1, -1, -1);
        MR::invalidateHitSensors(this);

        for (s32 i = 0; i < mMovingPointCount; i++) {
            mMovingPoints[i].mIsActive = 0;
        }
    }

    move(mMovementSpeed);
    mEaseIn = MR::calcNerveEaseInValue(this, 20, 30.0f, 0.0f);

    if (MR::isStep(this, 20)) {
        setNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvDisappeared));
    }
}

void ElectricRailMoving::exeDisappeared() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }
}

bool ElectricRailMoving::tryFreeze() {
    bool frozen = false;
    for (s32 i = 0; i < mMovingPointCount; i++) {
        if (MR::isStarPointerPointing2PAttachTarget(&mMovingPoints[i])) {
            MR::tryEmitEffect(&mMovingPoints[i], "PointerTouch");
            setNerve(GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvFreeze));
            frozen = true;
        } else {
            MR::tryDeleteEffect(&mMovingPoints[i], "PointerTouch");
        }
    }
    return frozen;
}

void ElectricRailMoving::exeWait() {
    move(mMovementSpeed);
    updateHitSensorPos();
    MR::startActionSound(this, "OjLvElecRailHam", -1, -1, -1);
    tryFreeze();
}

void ElectricRailMoving::exeFreeze() {
    move(0.0f);
    updateHitSensorPos();
    MR::startActionSound(this, "OjLvElecRailHam", -1, -1, -1);
    if (!tryFreeze()) {
        MR::setNerveAtStep(this, GET_NERVE(ElectricRailMoving, ElectricRailMovingNrvWait), 20);
    }
}

ElectricRailMoving::~ElectricRailMoving() {
}

ElectricRailMovingPoint::~ElectricRailMovingPoint() {
}

