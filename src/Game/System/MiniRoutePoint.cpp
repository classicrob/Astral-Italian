#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/MapObj/StarPieceScatter.hpp"
#include "Game/System/MiniRoutePoint.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Effect/PlayerRocketSmokeMoveParticleCallBack.hpp"
#include "Game/Effect/PlayerRocketSmokeWaitParticleCallBack.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"


// ---- MiniRoutePoint ----

MiniRoutePoint::MiniRoutePoint() : LiveActor("中継点") {
    mPointIndex = -1;
    mNotReady = false;
    mIsTicoBuild = false;
}

MiniRoutePoint::~MiniRoutePoint() {}

// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
void MiniRoutePoint::init(const JMapInfoIter&) {
    MR::initActor(this, "MiniRoutePoint", false);
    MR::setClippingFarMax(this);
    MR::registerDemoSimpleCastAll(this);

    mShadow = new LiveActor("中継点影");
    MR::initActor(mShadow, "MiniRoutePointShadow", false);
    MR::setClippingFarMax(mShadow);
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShadow);
    makeActorDead();
}

void MiniRoutePoint::makeActorAppeared() {
    MR::setScale(this, mNotReady ? 0.4f : 1.0f);
    mScale.y = 1.0f;
    MR::setScale(mShadow, mScale);
    MR::setRotation(mShadow, mRotation);
    LiveActor::makeActorAppeared();
    mShadow->makeActorAppeared();
}

void MiniRoutePoint::makeActorDead() {
    LiveActor::makeActorDead();
    mShadow->makeActorDead();
}

bool MiniRoutePoint::isOpening() const {
    if (MR::isDead(this)) {
        return false;
    }
    return WorldMapFunction::isPointing(const_cast<MiniRoutePoint*>(this));
}

bool MiniRoutePoint::tryOpen() {
    if (MR::isDead(this)) {
        return false;
    }

    WorldMapFunction::approachScale(this, 0.1f, mNotReady ? 0.7f : 1.2f);
    mScale.y = 1.0f;
    return true;
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void MiniRoutePoint::setPosition(const TVec3f& rPos) {
    MR::setPosition(this, rPos);

    TVec3f shadowPos(rPos.x, -700.0f + rPos.y, rPos.z);
    MR::setPosition(mShadow, shadowPos);
}

// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
void MiniRoutePoint::setReachable(bool isReachable) {
    if (isReachable) {
        MR::startAction(this, "Open");
        if (mIsTicoBuild) {
            MR::startBrk(this, "TicoBuild");
        } else {
            MR::startBrk(this, "Normal");
        }
    } else {
        MR::startAction(this, "Close");
    }
}

bool MiniRoutePoint::isStarting() const {
    return MR::isAnyAnimPlaying(this, "Open");
}

bool MiniRoutePoint::start() {
    MR::startActionSoundOneShot(this, "RoadExtend", -1, -1, -1);
    return true;
}

bool MiniRoutePoint::startOpenSound() {
    MR::startActionSoundOneShot(this, "PointOn", -1, -1, -1);
    return true;
}

bool MiniRoutePoint::startCloseSound() {
    MR::startActionSoundOneShot(this, "PointOff", -1, -1, -1);
    return true;
}

void MiniRoutePoint::control() {
    WorldMapFunction::returnScale(this, 0.05f, mNotReady ? 0.4f : 1.0f);
    mScale.y = 1.0f;
    MR::setScale(mShadow, mScale);
    MR::setRotation(mShadow, mRotation);
}
