#pragma auto_inline off

#include "Game/MapObj/ElectricRail2Point2.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "JSystem/JMath/JMath.hpp"
#include "JSystem/JGeometry/TVec.hpp"

// Labels (.data strings), GX write-gather pipe and float constants.
extern "C" char lbl_8069FEF8[];  // sensor name (attackSensor)
extern "C" char lbl_8069FF70[];  // shadow volume name A
extern "C" char lbl_8069FF78[];  // shadow volume line name
extern "C" char lbl_8069FF80[];  // effect name
extern "C" char lbl_8069FF90[];  // sound name
extern "C" char lbl_807CFC20[];  // shadow volume name B
extern "C" f32 lbl_807D3A98;     // NrvMove instance (sbss)
extern "C" f32 lbl_807DD370;
extern "C" f32 lbl_807DD374;
extern "C" f32 lbl_807DD37C;
extern "C" f32 lbl_807DD380;
extern "C" f32 lbl_807DD388;
extern "C" f32 lbl_807DD38C;
extern "C" f32 lbl_807DD390;

namespace {
    inline volatile f32* gxFifoF32() {
        return reinterpret_cast<volatile f32*>(0xCC008000);
    }
}  // namespace

ElectricRail2Point::~ElectricRail2Point() {
}

void ElectricRail2Point::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::isEqualSensor(pSender, this, lbl_8069FEF8)) {
        MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
    }
}

// NONMATCHING: the shadow-volume calls match; the helper-set order for the second actor differs slightly.
bool ElectricRail2Point::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    MR::initShadowControllerList(this, 3);
    MR::addShadowVolumeCylinder(this, lbl_8069FF70, lbl_807DD38C);
    MR::addShadowVolumeCylinder(this, lbl_807CFC20, lbl_807DD38C);
    f32 dropLength = lbl_807DD390;
    MR::setShadowDropPositionPtr(this, lbl_8069FF70, &mElems->mPosition);
    MR::setShadowDropPositionPtr(this, lbl_807CFC20, reinterpret_cast<const TVec3f*>(reinterpret_cast<u8*>(mElems) + 0xA8));
    MR::setShadowDropLength(this, lbl_8069FF70, dropLength);
    MR::setShadowDropLength(this, lbl_807CFC20, dropLength);
    MR::onCalcShadowDropPrivateGravity(this, lbl_8069FF70);
    MR::onCalcShadowDropPrivateGravity(this, lbl_807CFC20);
    // NONMATCHING: the 8-argument addShadowVolumeLine call (rail shadow line between this and the drop volumes) is not
    // fully resolved from the target's register setup.
    MR::addShadowVolumeLine(this, lbl_8069FF78, this, lbl_8069FF70, lbl_807DD38C, this, lbl_807CFC20, lbl_807DD38C);
    (void)msg;
    (void)pSender;
    (void)pReceiver;
    return false;
}

void ElectricRail2Point::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mCurIndex = 0;
    for (s32 i = 0; i < mElemCount; i++) {
        mElems[i].makeActorAppeared();
    }
}

void ElectricRail2Point::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < mElemCount; i++) {
        mElems[i].makeActorDead();
    }
}

// NONMATCHING: draft of the rail sampling. The rail-state word at +0x90 of a rail actor is read raw, the
// TVec3f operators are inline in our headers (target calls __mi out of line), and the direction sign is unverified.
void ElectricRail2Point::exeMove() {
    bool anyAttached = false;
    for (s32 i = 0; i < mElemCount; i++) {
        if (MR::attachSupportTicoToTarget(&mElems[i])) {
            MR::emitEffect(&mElems[i], lbl_8069FF80);
            anyAttached = true;
        } else {
            MR::tryDeleteEffect(&mElems[i], lbl_8069FF80);
        }
    }

    MR::startActionSound(this, lbl_8069FF90, -1, -1, -1);

    s32 railCount = mRailGroup->getObjNum();
    TVec3f playerPos = *MR::getPlayerPos();

    if (!anyAttached) {
        mCurIndex++;
        if (mCurIndex >= mDivCount) {
            mCurIndex = 0;
        }
        for (s32 i = 0; i < railCount; i++) {
            LiveActor* pRail = mRailGroup->getActor(i);
            if (*reinterpret_cast<s32*>(reinterpret_cast<u8*>(pRail) + 0x90) == -1) {
                f32 total = MR::getRailTotalLength(pRail);
                f32 coord = (total / (f32)mDivCount) * (f32)mCurIndex;
                if (!MR::isLoopRail(pRail)) {
                    if (!MR::isRailGoingToEnd(pRail)) {
                        coord = total - coord;
                    }
                }
                MR::setRailCoord(pRail, coord);
                if (!MR::isLoopRail(pRail) && mCurIndex == mDivCount - 1) {
                    MR::reverseRailDirection(pRail);
                }
            }
        }
    }

    for (s32 i = 0; i < railCount - 1; i++) {
        LiveActor* pA = mRailGroup->getActor(i);
        LiveActor* pB = mRailGroup->getActor(i + 1);
        TVec3f a = MR::getRailPos(pB);
        TVec3f b = MR::getRailPos(pA);
        TVec3f ab = a - b;
        MR::calcPerpendicFootToLineInside(&mFootPos, playerPos, b, a);
        (void)ab;
    }

    TVec3f first = MR::getRailPos(mRailGroup->getActor(0));
    TVec3f last = MR::getRailPos(mRailGroup->getActor(railCount - 1));
    TVec3f sideDir = first - last;
    MR::normalizeOrZero(&sideDir);
    updateSideVectors(sideDir);
    MR::setPosition(&mElems[0], first);
    MR::setPosition(&mElems[railCount - 1], last);
}

// unnamed fn_8029B490: keeps the two side vectors perpendicular to the rail direction
void ElectricRail2Point::updateSideVectors(const TVec3f& rDir) {
    TVec3f tmp(lbl_807DD374);
    PSVECCrossProduct(&mSideA, &rDir, &tmp);
    if (MR::isNearZero(tmp, lbl_807DD388)) {
        PSVECCrossProduct(&rDir, &mSideB, &mSideA);
        PSVECCrossProduct(&rDir, &mSideA, &mSideB);
        return;
    }
    PSVECCrossProduct(&rDir, &tmp, &mSideA);
    mSideB = tmp;
}

// NONMATCHING: draw/drawRailSide only partially transcribed (the target passes an extra f32 per GX quad).
void ElectricRail2Point::draw() const {
    drawRailSide(lbl_807DD37C);
}

// unnamed fn_8029B550: draws both sides of the rail
void ElectricRail2Point::drawRailSide(f32 width) const {
    drawRailQuad(width, -width, -width, -width);
    drawRailQuad(width, -width, width, width);
}

// unnamed fn_8029B5B0 (0x300 bytes). NONMATCHING: only the GX begin and the rail end points are emitted here;
// the per-vertex offset math (TVec3f ops, 0xCC008000 writes) is not transcribed yet.
void ElectricRail2Point::drawRailQuad(f32 w0, f32 w1, f32 h0, f32 h1) const {
    (void)w0;
    (void)w1;
    (void)h0;
    (void)h1;
    s32 railCount = mRailGroup->getObjNum();
    if (railCount > 0) {
        GXBegin((GXPrimitive)0x98, (GXVtxFmt)0, 4);
    }
}

// unnamed fn_8029B8B0: three f32 writes to the GX FIFO
void writeGxVec3(f32 x, f32 y, f32 z) NO_INLINE;
void writeGxVec3(f32 x, f32 y, f32 z) {
    *gxFifoF32() = x;
    *gxFifoF32() = y;
    *gxFifoF32() = z;
}

ElectricRail2Point::ElectricRail2Point(const char* pName) : LiveActor(pName) {
}

namespace NrvElectricRail2Point {
    NEW_NERVE(ElectricRail2PointNrvMove, ElectricRail2Point, Move);
}  // namespace NrvElectricRail2Point
