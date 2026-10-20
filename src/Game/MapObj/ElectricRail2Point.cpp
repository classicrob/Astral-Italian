#pragma auto_inline off

#include "Game/MapObj/ElectricRail2Point.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/LiveActor/RailObj.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "JSystem/JMath/JMath.hpp"
#include "JSystem/JGeometry/TVec.hpp"

namespace NrvElectricRail2Point {
    NEW_NERVE(ElectricRail2PointNrvMove, ElectricRail2Point, Move);
}  // namespace NrvElectricRail2Point

ElectricRail2Point::ElectricRail2Point(const char* name) : LiveActor(name), mFootPos(0.0f,0.0f,0.0f), mCurIndex(0), mDivCount(60), mSideA(0.0f,1.0f,0.0f), mSideB(1.0f,0.0f,0.0f), mElemCount(0) {}
// NONMATCHING: vector copy/assignment inlining and array loop scheduling.
void ElectricRail2Point::init(const JMapInfoIter& iter) {
    ElectricRailFunction::registerRail(this,ElectricRail_Red);
    MR::initDefaultPos(this,iter);
    initHitSensor(1);
    MR::addHitSensorPosEnemyAttack(this,"Attack",8,35.0f,&mFootPos,TVec3f(0.0f,0.0f,0.0f));
    initSound(4,"ElectricRail",&mFootPos,TVec3f(0.0f));
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg0NoInit(iter,&mDivCount);
    s32 count=MR::getChildObjNum(iter);
    mRailGroup=new DeriveActorGroup<LiveActor>("２点移動電撃レールガイドグループ",2);
    mRailGroup->initWithoutIter();
    for (s32 i=0;i<count;i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        if (MR::isEqualString(name,"RailObj")) {
            LiveActor* actor=MR::createRailObj("２点移動電撃レールガイド",this,iter,i);
            actor->makeActorAppeared();
            mRailGroup->registerActor(actor);
        }
    }
    mElems=new ElectricRailMovingPoint[count];
    mElemCount=count;
    for (s32 i=0;i<count;i++) {
        LiveActor* rail = mRailGroup->getActor(i);
        ElectricRailMovingPoint* point = &mElems[i];
        TVec3f pos(MR::getRailPos(rail));
        MR::setPosition(point,pos);
        mElems[i].initWithoutIter();
    }
    initShadow(iter);
    MR::setClippingTypeSphere(this,1000.0f);
    MR::setGroupClipping(this,iter,16);
    MR::invalidateClipping(this);
    initNerve(&NrvElectricRail2Point::ElectricRail2PointNrvMove::sInstance);
    MR::calcGravityOrZero(this);
    TVec3f up(-mGravity);
    if (!MR::isNearZero(up,0.001f)) mSideA=up;
    MR::useStageSwitchSyncAppear(this,iter);
}

void writeGxVec3(f32, f32, f32) NO_INLINE;

#ifndef GALAXY_NATIVE
namespace {
    inline volatile f32* gxFifoF32() {
        return reinterpret_cast<volatile f32*>(0xCC008000);
    }
}  // namespace
#endif

ElectricRail2Point::~ElectricRail2Point() {
}

void ElectricRail2Point::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::isEqualSensor(pSender, this, "Attack")) {
        MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
    }
}

void ElectricRail2Point::initShadow(const JMapInfoIter&) {
    MR::initShadowControllerList(this, 3);
    MR::addShadowVolumeCylinder(this, "start", 20.0f);
    MR::addShadowVolumeCylinder(this, "end", 20.0f);
    f32 dropLength = 50.0f;
    MR::setShadowDropPositionPtr(this, "start", &mElems->mPosition);
    MR::setShadowDropPositionPtr(this, "end", &mElems[1].mPosition);
    MR::setShadowDropLength(this, "start", dropLength);
    MR::setShadowDropLength(this, "end", dropLength);
    MR::onCalcShadowDropPrivateGravity(this, "start");
    MR::onCalcShadowDropPrivateGravity(this, "end");
    MR::addShadowVolumeLine(this, "line", this, "start", 20.0f, this, "end", 20.0f);
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

// NONMATCHING: register allocation and vector temporary scheduling differ.
void ElectricRail2Point::exeMove() {
    bool anyAttached = false;
    for (s32 i = 0; i < mElemCount; i++) {
        if (MR::attachSupportTicoToTarget(&mElems[i])) {
            MR::tryEmitEffect(&mElems[i], "PointerTouch");
            anyAttached = true;
        } else {
            MR::tryDeleteEffect(&mElems[i], "PointerTouch");
        }
    }

    MR::startActionSound(this, "OjLvElecRailHam", -1, -1, -1);

    s32 railCount = mRailGroup->getObjNum();
    TVec3f playerPos = *MR::getPlayerPos();

    if (!anyAttached) {
        mCurIndex++;
        if (mCurIndex >= mDivCount) {
            mCurIndex = 0;
        }
        for (s32 i = 0; i < railCount; i++) {
            LiveActor* pRail = mRailGroup->getActor(i);
            if (static_cast<RailObj*>(pRail)->mArg0 == -1) {
                s32 index = mCurIndex;
                f32 total = MR::getRailTotalLength(pRail);
                f32 coord = total / (f32)mDivCount;
                coord *= (f32)index;
                if (!MR::isLoopRail(pRail)) {
                    if (!MR::isRailGoingToEnd(pRail)) {
                        coord = total - coord;
                    }
                }
                MR::setRailCoord(pRail, coord);
                if (!MR::isLoopRail(pRail) && index == mDivCount - 1) {
                    MR::reverseRailDirection(pRail);
                }
            }
        }
    }

    for (s32 i = 0; i < railCount - 1; i++) {
        LiveActor* pA = mRailGroup->getActor(i);
        LiveActor* pB = mRailGroup->getActor(i + 1);
        TVec3f b = MR::getRailPos(pA);
        TVec3f a = MR::getRailPos(pB);
        TVec3f ab = b - a;
        MR::calcPerpendicFootToLineInside(&mFootPos, playerPos, b, a);
        (void)ab;
    }

    LiveActor* firstActor = mRailGroup->getActor(0);
    LiveActor* lastActor = mRailGroup->getActor(railCount - 1);
    TVec3f first = MR::getRailPos(firstActor);
    TVec3f last = MR::getRailPos(lastActor);
    TVec3f sideDir = last - first;
    (void)PSVECMag(&sideDir);
    MR::normalizeOrZero(&sideDir);
    updateSideVectors(sideDir);
    MR::setPosition(&mElems[0], first);
    MR::setPosition(&mElems[railCount - 1], last);
}

// unnamed fn_8029B490: keeps the two side vectors perpendicular to the rail direction
void ElectricRail2Point::updateSideVectors(const TVec3f& rDir) {
    TVec3f tmp(0.0f);
    PSVECCrossProduct(&mSideA, &rDir, &tmp);
    if (MR::isNearZero(tmp, 0.001f)) {
        PSVECCrossProduct(&rDir, &mSideB, &mSideA);
        PSVECCrossProduct(&rDir, &mSideA, &mSideB);
        return;
    }
    PSVECCrossProduct(&rDir, &tmp, &mSideA);
    mSideB.x = tmp.x;
    mSideB.y = tmp.y;
    mSideB.z = tmp.z;
}

void ElectricRail2Point::draw() const {
    drawRailSide(30.0f);
}

// unnamed fn_8029B550: draws both sides of the rail
void ElectricRail2Point::drawRailSide(f32 width) const {
    drawRailQuad(width, width, -width, -width);
    drawRailQuad(-width, width, width, -width);
}

// NONMATCHING: the endpoint actor registers r30/r31 are interchanged.
void ElectricRail2Point::drawRailQuad(f32 w0, f32 w1, f32 h0, f32 h1) const {
    s32 railCount = mRailGroup->getObjNum();
    if (railCount > 0) {
        GXBegin((GXPrimitive)0x98, (GXVtxFmt)0, 4);
        LiveActor* first = mRailGroup->getActor(0);
        LiveActor* last = mRailGroup->getActor(railCount - 1);
        TVec3f start(MR::getRailPos(first));
        TVec3f end(MR::getRailPos(last));
        f32 texLength = (end - start).length() / 400.0f;
        TVec3f sideA = mSideB * w0;
        TVec3f sideB = mSideA * w1;
        TVec3f vertex = (start + sideA) + sideB;
        writeGxVec3(vertex.x, vertex.y, vertex.z);
#ifdef GALAXY_NATIVE
        GXTexCoord2f32(0.0f, 0.0f);
#else
        *gxFifoF32() = 0.0f;
        *gxFifoF32() = 0.0f;
#endif
        sideA = mSideB * h0;
        sideB = mSideA * h1;
        vertex = (start + sideA) + sideB;
        writeGxVec3(vertex.x, vertex.y, vertex.z);
#ifdef GALAXY_NATIVE
        GXTexCoord2f32(0.0f, 1.0f);
#else
        *gxFifoF32() = 0.0f;
        *gxFifoF32() = 1.0f;
#endif
        sideA = mSideB * w0;
        sideB = mSideA * w1;
        vertex = (end + sideA) + sideB;
        writeGxVec3(vertex.x, vertex.y, vertex.z);
#ifdef GALAXY_NATIVE
        GXTexCoord2f32(texLength, 0.0f);
#else
        *gxFifoF32() = texLength;
        *gxFifoF32() = 0.0f;
#endif
        sideA = mSideB * h0;
        sideB = mSideA * h1;
        vertex = (end + sideA) + sideB;
        writeGxVec3(vertex.x, vertex.y, vertex.z);
#ifdef GALAXY_NATIVE
        GXTexCoord2f32(texLength, 1.0f);
#else
        *gxFifoF32() = texLength;
        *gxFifoF32() = 1.0f;
#endif
    }
}

// unnamed fn_8029B8B0: three f32 writes to the GX FIFO
void writeGxVec3(f32 x, f32 y, f32 z) NO_INLINE;
void writeGxVec3(f32 x, f32 y, f32 z) {
#ifdef GALAXY_NATIVE
    GXPosition3f32(x, y, z);
#else
    *gxFifoF32() = x;
    *gxFifoF32() = y;
    *gxFifoF32() = z;
#endif
}
