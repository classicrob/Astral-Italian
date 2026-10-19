#include "Game/MapObj/StarPieceMother.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Util.hpp"

namespace {
    // const f32 hMoveSpeed = _;
    const f32 hAddRandomVectorValue = 0.5f;
    const f32 hAddRotateY = 10.0f;
};  // namespace

namespace NrvStarPieceMother {
    NEW_NERVE(HostTypeNrvMoveOnRail, StarPieceMother, MoveOnRail);
    NEW_NERVE(HostTypeNrvWait, StarPieceMother, Wait);
};  // namespace NrvStarPieceMother

StarPieceMother::StarPieceMother(const char* pName) : LiveActor(pName), mStarPieceArray(), mPieceRailCoords(), mPieceNum(6), mNextPieceIdx(), _9C(), mMoveSpeed(10.0f) {
}

void StarPieceMother::init(const JMapInfoIter& rIter) {
    s32 isRailNotConnected = false;

    if (!MR::isValidInfo(rIter) || !MR::isConnectedWithRail(rIter)) {
        isRailNotConnected = true;

        MR::initDefaultPos(this, rIter);

        MR::useStageSwitchSyncAppear(this, rIter);
    } else {
        initRailRider(rIter);
        MR::moveCoordAndTransToRailStartPoint(this);
        MR::syncStageSwitchAppearAndSync(this, rIter);
    }

    MR::initUseStageSwitchWriteDead(this, rIter);

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnOffA(this, MR::Functor(this, &StarPieceMother::onSwitchA), MR::Functor(this, &StarPieceMother::offSwitchA));
    }

    MR::initActor(this, rIter, "StarPieceMother", false);
    MR::connectToSceneNoSilhouettedMapObj(this);
    MR::onCalcGravity(this);
    MR::initShadowVolumeSphere(this, 60.0f);
    initSound(6, "StarPieceMother", nullptr, TVec3f(0.0f));
    MR::invalidateClipping(this);
    MR::tryRegisterDemoCast(this, rIter);

    s32 arg = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg);
    if (arg != -1) {
        mPieceNum = arg;
    }

    mStarPieceArray = new StarPiece*[mPieceNum];
    mPieceRailCoords = new f32[mPieceNum];

    for (u32 idx = 0; idx < mPieceNum; idx++) {
        mStarPieceArray[idx] = new StarPiece("スターピースマザーピース");
        mStarPieceArray[idx]->initAndSetFloatingFromGroup(rIter);
        mStarPieceArray[idx]->makeActorDead();

        if (MR::isValidSwitchA(this)) {
            mStarPieceArray[idx]->mFlags._1 = false;
        }
    }

    if (isRailNotConnected) {
        initNerve(GET_NERVE(StarPieceMother, HostTypeNrvWait));
        initHitSensor(1);
        MR::addHitSensorMapObj(this, "Body", 8, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));
        makeActorAppeared();
    } else {
        initNerve(GET_NERVE(StarPieceMother, HostTypeNrvMoveOnRail));

        arg = -1;
        MR::getJMapInfoArg1NoInit(rIter, &arg);
        f32 railPos = (arg == -1) ? 0.0f : static_cast< f32 >(arg);
        placementPieceOnRail(railPos);

        arg = -1;
        MR::getJMapInfoArg2NoInit(rIter, &arg);
        if (arg != -1 && arg != 0) {
            mMoveSpeed = static_cast< f32 >(arg);
        }

        makeActorDead();
    }

    MR::startBpk(this, "StarPieceMother");
}

void StarPieceMother::kill() {
    MR::stopSound(this, "SE_OJ_STAR_PIECE_MO_ST");
    MR::startActionSound(this, "OjStarPieceMoEd");
    LiveActor::kill();

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void StarPieceMother::control() {
    mRotation.y += ::hAddRotateY;
}

void StarPieceMother::exeMoveOnRail() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjStarPieceMoSt");
    }

    MR::startActionSound(this, "OjLvStarPieceMoMv");
    MR::moveCoordAndFollowTrans(this, mMoveSpeed);

    f32 railCoord = MR::getRailCoord(this);  // Necessary to match
    if (mPieceRailCoords[mNextPieceIdx] <= railCoord) {
        mStarPieceArray[mNextPieceIdx]->appear();
        MR::startActionSound(this, "OjStarPieceBurst");
        mNextPieceIdx++;
    }

    if (MR::isRailReachedGoal(this)) {
        kill();
    }
}

void StarPieceMother::placementPieceOnRail(f32 distance) {
    TVec3f railPos;
    MR::calcRailPosAtCoord(&railPos, this, distance);
    MR::setPosition(mStarPieceArray[0], railPos);
    mPieceRailCoords[0] = distance;

    f32 step;
    f32 difference = MR::getRailTotalLength(this) - distance;
    if (1 < mPieceNum) {
        step = 1.0f / (mPieceNum - 1.0f);
        step *= difference;
    }

    for (int idx = 1; idx < mPieceNum; idx++) {
        distance += step;
        f32 value = distance >= difference ? difference : distance;
        MR::calcRailPosAtCoord(&railPos, this, value);
        MR::setPosition(mStarPieceArray[idx], railPos);
        mPieceRailCoords[idx] = value;
    }
}

void StarPieceMother::onSwitchA() {
    for (u32 idx = 0; idx < mPieceNum; idx++) {
        mStarPieceArray[idx]->mFlags._1 = true;
    }
}

void StarPieceMother::offSwitchA() {
    for (u32 idx = 0; idx < mPieceNum; idx++) {
        mStarPieceArray[idx]->mFlags._1 = false;
    }
}

void StarPieceMother::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this))) {
        emitStarPieces();
        kill();
    }
}

// NONMATCHING: extra saved register for the by-value direction copy and loop stack slots.
#pragma push
#pragma opt_loop_invariants off
void StarPieceMother::emitStarPieces() {
    for (u32 idx = 0; idx < mPieceNum; idx++) {
        TVec3f position;
        const TVec3f* playerPos = MR::getPlayerCenterPos();
        position.x = playerPos->x;
        position.y = playerPos->y;
        position.z = playerPos->z;
        position.sub(mPosition);
        MR::normalizeOrZero(&position);
        MR::addRandomVector(&position, position, ::hAddRandomVectorValue);
        MR::normalizeOrZero(&position);

        if (MR::isNearZero(position)) {
            position.set(0.0f, 0.0f, 1.0f);
        }

        mStarPieceArray[idx]->makeActorAppeared();
        mStarPieceArray[idx]->goToPlayer(position);
    }
}

#pragma pop

void StarPieceMother::exeWait() {
}
