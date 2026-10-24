#include "Game/Map/RaceRail.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"

// TODO: this file has some weird compiler flags going on

namespace {
    static const f32 sRaceJudgeLength = 10000.0f;
    // static const f32 sWaterSurface = ;
};  // namespace

RaceRail::RaceRail(const char* pName) : NameObj(pName) {
}

PlayerRacer::PlayerRacer(const char* pName) : LiveActor(pName), mReserved0(0), mReserved1(0), mCameraInfo(), mGoalRadius(650.0f), mHasReachedGoal(false) {
}

void PlayerRacer::appear() {
    RaceManagerFunction::startRaceImmediately();
}

void PlayerRacer::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    initRailRider(rIter);
    s32 radius = -1;
    MR::getRailArg7WithInit(this, &radius);
    if (radius >= 0) mGoalRadius = radius;
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initActorCamera(this, rIter, &mCameraInfo);

    MR::useStageSwitchSyncAppear(this, rIter);

    MR::invalidateClipping(this);
}

void PlayerRacer::startRacer() {
    MR::endActorCamera(this, mCameraInfo, true);
}

// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited.
bool PlayerRacer::updateRacer(const RaceManager* pRaceManager) {
    if (mHasReachedGoal) return true;
    PlayerRacer* racer = pRaceManager->mPlayerRacer;
    f32 railCoord = MR::calcNearestRailCoord(racer, *MR::getPlayerCenterPos());
    f32 railLength = MR::getRailTotalLength(pRaceManager->mPlayerRacer);
    mPrevPosition.set<f32>(mCurrPosition);
    mCurrPosition.set<f32>(*MR::getPlayerCenterPos());
    TVec3f railPos;
    racer = pRaceManager->mPlayerRacer;
    MR::calcNearestRailPos(&railPos, racer, *MR::getPlayerCenterPos());
    MR::setRailCoord(this, mRailCoord);
    mRailCoord = railCoord;
    TVec3f endPos(0.0f, 0.0f, 0.0f);
    MR::calcRailEndPointPos(&endPos, this);
    if (MR::fabs(mRailCoord - railLength) < 1000.0f) {
        TVec3f end(0.0f, 0.0f, 0.0f);
        MR::calcRailEndPointPos(&end, this);
        if (MR::calcDistanceToPlayer(end) < mGoalRadius && pRaceManager->isGoal(this)) {
            MR::tryRumblePadVeryStrong(this, WPAD_CHAN0);
            mHasReachedGoal = true;
            return true;
        }
    }
    if (!mHasReachedGoal && mCurrPosition.z < endPos.z - 1000.0f) {
        MR::forceKillPlayerByAbyss();
        return false;
    }
    return false;
}

bool PlayerRacer::goalRacer() {
    return true;
}

void PlayerRacer::loseRacer() {
}

void PlayerRacer::exitRacer() {
}

PlayerRacer::~PlayerRacer() {
}

RaceRail::~RaceRail() {
}

void RaceRail::init(const JMapInfoIter& rIter) {
    mPlayerRacer = new PlayerRacer("レース判定用レール");
    mPlayerRacer->init(rIter);
    RaceManagerFunction::entryRacerPlayer(mPlayerRacer);
}

void PlayerRacer::initRacer() {
    AbstractRacer::initRacer();
    mHasReachedGoal = false;
    MR::startBckPlayer("Watch");
}

void PlayerRacer::prepRacer(const RaceManager* pRaceManager) {
    MR::moveCoordToStartPos(this);
    MR::moveTransToCurrentRailPos(this);

    TVec3f grav;
    MR::calcGravityVector(this, mPosition, &grav, nullptr, 0);

    TPos3f mtx;
    TVec3f pos;

    if (static_cast<u32>(pRaceManager->mCurrentRace) <= 1) {
        MR::findNamePos("レース開始時マリオ位置", &pos, nullptr);
        MR::makeMtxFrontUpPos(&mtx, MR::getRailDirection(this), -grav, pos);
    }

    MR::tryPlayerKillTakingActor();
    MR::setPlayerBaseMtx(mtx);
    MR::startActorCameraTargetPlayer(this, mCameraInfo);
}

void PlayerRacer::resetRacer(const RaceManager* pRaceManager) {
    MR::tryPlayerKillTakingActor();
    MR::setPlayerPosOnGround("レース終了後位置");

    if (static_cast<u32>(pRaceManager->mCurrentRace) <= 1) {
        MR::startBckPlayer("Watch", nullptr);
    }

    MR::progressPlayerBckOnPause();
    MR::resetPlayerStatus();
    MR::resetPlayerEffect();
}

void PlayerRacer::requestMovementOnRace() {
}
