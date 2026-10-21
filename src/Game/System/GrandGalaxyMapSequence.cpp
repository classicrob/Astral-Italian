#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/GrandGalaxyMapStateMove.hpp"
#include "Game/System/GrandGalaxyMapStates.hpp"
#include "Game/System/GrandGalaxyMapStateWorldWarp.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapSequence {
    NEW_NERVE(GrandGalaxyMapSequenceEnterDemo, GrandGalaxyMapSequence, Enter);
    NEW_NERVE(GrandGalaxyMapSequenceEnterDirect, GrandGalaxyMapSequence, Enter);
    NEW_NERVE(GrandGalaxyMapSequenceWait, GrandGalaxyMapSequence, Wait);
    NEW_NERVE(GrandGalaxyMapSequenceNews, GrandGalaxyMapSequence, News);
    NEW_NERVE(GrandGalaxyMapSequencePointMove, GrandGalaxyMapSequence, PointMove);
    NEW_NERVE(GrandGalaxyMapSequenceWorldWarp, GrandGalaxyMapSequence, WorldWarp);
};  // namespace NrvGrandGalaxyMapSequence

using namespace NrvGrandGalaxyMapSequence;

GrandGalaxyMapSequence::GrandGalaxyMapSequence(GrandGalaxyMap* pHost) : NerveExecutor("シーケンス管理"), mHost(pHost), mWait(nullptr), mIsStickTilted(false) {}

void GrandGalaxyMapSequence::init(bool isDirect) {
    mHost->declareAreaCameras();
    initNerve(&GrandGalaxyMapSequenceWait::sInstance, 16);
    mEnterDemo = new GrandGalaxyMapStateEnterDemo(mHost);
    mStart = new GrandGalaxyMapStateStart(mHost);
    mWait = new GrandGalaxyMapStateWait(mHost);
    mNews = new GrandGalaxyMapStateNews(mHost);
    mWorldWarp = new GrandGalaxyMapStateWorldWarp(mHost);
    MR::initActorState(this, mEnterDemo, &GrandGalaxyMapSequenceEnterDemo::sInstance, "EnterDemo");
    MR::initActorState(this, mStart, &GrandGalaxyMapSequenceEnterDirect::sInstance, "EnterDirect");
    MR::initActorState(this, mWait, &GrandGalaxyMapSequenceWait::sInstance, "Wait");
    MR::initActorState(this, mNews, &GrandGalaxyMapSequenceNews::sInstance, "News");
    MR::initActorState(this, new GrandGalaxyMapStateMove(mHost), &GrandGalaxyMapSequencePointMove::sInstance, "PointMove");
    MR::initActorState(this, mWorldWarp, &GrandGalaxyMapSequenceWorldWarp::sInstance, "WorldWarp");
    if (isDirect) {
        setNerve(&GrandGalaxyMapSequenceEnterDirect::sInstance);
    } else {
        setNerve(&GrandGalaxyMapSequenceEnterDemo::sInstance);
    }
}

void GrandGalaxyMapSequence::start() {
    if (!isNerve(&GrandGalaxyMapSequenceEnterDirect::sInstance)) {
        setNerve(&GrandGalaxyMapSequenceEnterDemo::sInstance);
    }
}

void GrandGalaxyMapSequence::end() {}

void GrandGalaxyMapSequence::exeEnter() {
    if (MR::updateActorState(this)) {
        if (mNews->checkNews()) {
            setNerve(&GrandGalaxyMapSequenceNews::sInstance);
        } else {
            mHost->startAppear();
            setNerve(&GrandGalaxyMapSequenceWait::sInstance);
        }
    }
}

void GrandGalaxyMapSequence::exeNews() {
    if (MR::updateActorState(this)) {
        mHost->startAppear();
        setNerve(&GrandGalaxyMapSequenceWait::sInstance);
    }
}

void GrandGalaxyMapSequence::exeWait() {
    if (MR::isFirstStep(this)) {
        mIsStickTilted = WorldMapOwner::isStickTilted();
        MR::disableStarPointerShootStarPiece();
    }

    MR::updateActorState(this);
#ifdef GALAXY_NATIVE
    // PartsWarpInfo retains Wii offsets; it is not the native state object's layout.
    PartsWarpInfo warpInfo = {};
    if (!tryPointMove() && mHost->tryWorldWarp(&warpInfo)) {
        mWorldWarp->mWarpParam0 = warpInfo.mParam0;
        mWorldWarp->mWarpParam1 = warpInfo.mParam1;
#else
    if (!tryPointMove() && mHost->tryWorldWarp(reinterpret_cast< PartsWarpInfo* >(mWorldWarp))) {
#endif
        MR::startActionSound(mHost->getRocket(), "PushAButton", -1, -1, -1);
        mHost->requestEnd();
        setNerve(&GrandGalaxyMapSequenceWorldWarp::sInstance);
    }
}

bool GrandGalaxyMapSequence::isWorldWarp() const {
    return isNerve(&GrandGalaxyMapSequenceWorldWarp::sInstance);
}

bool GrandGalaxyMapSequence::isWorldWarpEnd() const {
    return mWorldWarp->mIsDead;
}

bool GrandGalaxyMapSequence::tryPointMove() {
    s32 point = mHost->tryUsePointMark();
    if (point > -1) {
        mWait->startPointing(point);
    } else {
        mWait->endPointing();
    }

    bool isDecided = mHost->tryDecide(point);
    bool isStickMove = mHost->tryStickMove();
    if (isDecided) {
        MR::startCSSound("CS_CLICK_WMAP", nullptr, 0, 0, 1.0f);
    }

    if (isDecided || isStickMove) {
        setNerve(&GrandGalaxyMapSequencePointMove::sInstance);
        return true;
    }

    LiveActor* pRocket = mHost->getRocket();
    if (!isDecided && point > 0 && WorldMapOwner::isPressedA()) {
        MR::startActionSound(pRocket, "NoMove", -1, -1, -1);
    }

    bool hasStickTarget = mHost->findStickTarget(0.1f) >= 0;
    if (!isStickMove && !mIsStickTilted && WorldMapOwner::isStickTilted() && !hasStickTarget) {
        MR::startActionSound(pRocket, "NoMove", -1, -1, -1);
    }

    mIsStickTilted = WorldMapOwner::isStickTilted();
    return false;
}

GrandGalaxyMapSequence::~GrandGalaxyMapSequence() {}

inline void GrandGalaxyMapSequence::exePointMove() {
    if (MR::updateActorState(this)) {
        setNerve(&GrandGalaxyMapSequenceWait::sInstance);
    }
}

inline void GrandGalaxyMapSequence::exeWorldWarp() {
    MR::updateActorState(this);
}
