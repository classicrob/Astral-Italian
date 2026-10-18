#include "Game/MapObj/SandBird.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/SandBirdParts.hpp"
#include "Game/Util.hpp"

// PLACEHOLDER(objects): fn_8009A780 builds the base TR matrix from (quat, pos); resolve when core names fn_8009A780 (Sandy.cpp also declares it)
extern "C" void fn_8009A780(TPos3f*, const TQuat4f*, const TVec3f*);
// PLACEHOLDER(objects): fn_800578F0(iter, index) returns the child object name (wraps MR::getChildObjName); resolve when core names fn_800578F0
extern "C" const char* fn_800578F0(const JMapInfoIter&, s32);
// PLACEHOLDER(objects): fn_8004FEE0 is a bare blr returning r3 (player check used in exeCruise); resolve when core names fn_8004FEE0
extern "C" bool fn_8004FEE0();

// Nerve executor the target inlines into Nrv::execute (defined before the NEW_NERVE block).
inline void SandBird::exeWait() {}

namespace NrvSandBird {
    NEW_NERVE(SandBirdNrvWait, SandBird, Wait);
    NEW_NERVE(SandBirdNrvStart, SandBird, Start);
    NEW_NERVE(SandBirdNrvCruise, SandBird, Cruise);
    NEW_NERVE(SandBirdNrvStop, SandBird, Stop);
    NEW_NERVE(SandBirdNrvReturn, SandBird, Return);
};  // namespace NrvSandBird

SandBird::SandBird(const char* pName)
    : LiveActor(pName), mPartsGroup(nullptr), _94(0), mQuat2(0.0f, 0.0f, 0.0f, 1.0f), mQuat(0.0f, 0.0f, 0.0f, 1.0f), mHomePos(0.0f, 0.0f, 1.0f),
      mUp(0.0f, 1.0f, 0.0f), mPlayerPos(0.0f, 0.0f, 0.0f), mZDir(0.0f, 0.0f, 1.0f), mTimer(0), mRailCoordPrev(0.0f), mRailCoord(6.0f),
      mHitFlag(false), _125(false), mIsPlayerOn(false) {
    mBaseMtx.identity();
}

void SandBird::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::makeQuatFromRotate(&mQuat, this);
    mQuat.getZDir(mZDir);
    mQuat2.set(mQuat);
    fn_8009A780(&mBaseMtx, &mQuat, &mPosition);

    s32 childNum = MR::getChildObjNum(rIter);
    if (childNum > 0) {
        mPartsGroup = new LiveActorGroup("\x83\x70\x81\x5B\x83\x63\x8A\xC7\x97\x9D", childNum);
        for (s32 i = 0; i < childNum; i++) {
            SandBirdParts* pPart = new SandBirdParts(fn_800578F0(rIter, i), this, getSensor(nullptr));
            MR::initChildObj(pPart, rIter, i);
            mPartsGroup->registerActor(pPart);
        }
    }

    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
    }
    mHomePos.set(mPosition);
    MR::getJMapInfoArg0NoInit(rIter, &mRailCoord);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    initNerve(GET_NERVE(SandBird, SandBirdNrvWait), 0);
    MR::hideModelAndOnCalcAnim(this);
    MR::invalidateClipping(this);
    makeActorAppeared();
}

void SandBird::control() {
    mHitFlag = false;
    MR::turnQuatZDirRate(&mQuat, mQuat, mZDir, 0.02f);
    MR::turnQuatYDirRate(&mQuat, mQuat, mUp, 0.05f);

    f32 dot = PSQUATDotProduct(&mQuat, &mQuat);
    if (dot > 3.8146972656250e-06f) {
        mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        static_cast< TVec4f& >(mQuat).scale(JGeometry::TUtil< f32 >::inv_sqrt(dot));
    }

    mTimer--;
    if (MR::isOnGroundPlayer()) {
        mIsPlayerOn = MR::isActorOnPlayer(this);
    }
    fn_8009A780(&mBaseMtx, &mQuat, &mPosition);
}

MtxPtr SandBird::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

bool SandBird::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mPlayerPos.set(*MR::getPlayerPos());
        mHitFlag = true;
        mTimer = 30;
        if (isNerve(GET_NERVE(SandBird, SandBirdNrvWait)) && MR::isExistRail(this)) {
            setNerve(GET_NERVE(SandBird, SandBirdNrvStart));
        }
    }
    return false;
}

void SandBird::startAllPartsMoveSign() {
    s32 count = mPartsGroup->getObjectCount();
    for (s32 i = 0; i < count; i++) {
        setPartsMoveSign(mPartsGroup->getActor(i));
    }
}

bool SandBird::isAllPartsWait() {
    s32 count = mPartsGroup->getObjectCount();
    for (s32 i = 0; i < count; i++) {
        if (!isPartsWait(mPartsGroup->getActor(i))) {
            return false;
        }
    }
    return true;
}

void SandBird::setAllPartsMove() {
    s32 count = mPartsGroup->getObjectCount();
    for (s32 i = 0; i < count; i++) {
        setPartsMove(mPartsGroup->getActor(i));
    }
}

void SandBird::setAllPartsShrink() {
    s32 count = mPartsGroup->getObjectCount();
    for (s32 i = 0; i < count; i++) {
        setPartsShrink(mPartsGroup->getActor(i));
    }
}

void SandBird::setAllPartsAppear() {
    s32 count = mPartsGroup->getObjectCount();
    for (s32 i = 0; i < count; i++) {
        setPartsAppear(mPartsGroup->getActor(i));
    }
}

SandBird::~SandBird() {}

void SandBird::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "StartCruise", -1, -1, -1);
        startAllPartsMoveSign();
    }

    if (MR::isGreaterStep(this, 0x1E)) {
        setNerve(GET_NERVE(SandBird, SandBirdNrvCruise));
    }
}

void SandBird::exeCruise() {
    if (MR::isFirstStep(this)) {
        setAllPartsMove();
        mRailCoordPrev = mRailCoord;
    }

    TVec3f prevPos(mPosition);
    MR::startActionSound(this, "Move", -1, -1, -1);

    MR::getCurrentRailPointArg0NoInit(this, &mRailCoordPrev);
    f32 railCoord = mRailCoord;
    if (railCoord < mRailCoordPrev) {
        railCoord += 0.1f;
        if (railCoord > mRailCoordPrev) {
            railCoord = mRailCoordPrev;
        }
    } else {
        railCoord -= 0.1f;
        if (railCoord < mRailCoordPrev) {
            railCoord = mRailCoordPrev;
        }
    }
    mRailCoord = railCoord;

    f32 ease = MR::calcNerveEaseInRate(this, 0x1E);
    mRailCoord = mRailCoord * ease;
    MR::moveCoordAndFollowTrans(this, mRailCoord);

    TVec3f frontPos;
    MR::calcRailPosFrontCoord(&frontPos, this, 80.0f * mRailCoord);

    TVec3f toTarget;
    MR::calcVecToTargetPosH(&toTarget, this, frontPos, nullptr);
    if (!MR::isNearZero(toTarget, 0.001f)) {
        mZDir.set(toTarget);
    }

    mUp.set(-mGravity);

    if (!MR::isOnGroundPlayer() && mIsPlayerOn) {
        TVec3f delta(mPosition);
        delta.sub(prevPos);
        if (MR::isPlayerSwingAction() || MR::isPlayerHipDropFalling()) {
            MR::pushPlayer(delta);
        }
        if (fn_8004FEE0()) {
            TVec3f front(0.0f);
            MR::getPlayerFrontVec(&front);
            if (MR::diffAngleAbs(front, delta) > 1.9856194257736206) {
                MR::pushPlayer(delta);
            }
        }
    }

    if (MR::isNearPlayerHorizontal(this, 5000.0f)) {
        setNerve(GET_NERVE(SandBird, SandBirdNrvStop));
    }
}

void SandBird::exeStop() {
    if (MR::isFirstStep(this)) {
        setAllPartsShrink();
    }

    if (MR::isGreaterStep(this, 0x78)) {
        setNerve(GET_NERVE(SandBird, SandBirdNrvReturn));
    }
}

void SandBird::exeReturn() {
    if (MR::isFirstStep(this)) {
        MR::resetPosition(this, mHomePos);
        MR::moveCoordAndTransToNearestRailPos(this);
        mQuat.set(mQuat2);

        f32 x = mQuat.x;
        f32 y = mQuat.y;
        f32 z = mQuat.z;
        f32 w = mQuat.w;
        mUp.set(2.0f * (x * y) - 2.0f * (w * z), 1.0f - 2.0f * (x * x) - 2.0f * (z * z), 2.0f * (y * z) + 2.0f * (w * x));
        mQuat.getZDir(mZDir);

        setAllPartsAppear();
    }

    if (isAllPartsWait()) {
        setNerve(GET_NERVE(SandBird, SandBirdNrvWait));
    }
}
