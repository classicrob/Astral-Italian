#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE
#define JGEOMETRY_VEC3_ZERO_NOINLINE
#include "Game/Enemy/Wanwan.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"

WanwanCreator::WanwanCreator(const char* pName) : LiveActor(pName), mWanwans(nullptr), mType(0), mSpeed(10.0f), mBreakAtGoal(false), mInterval(-1), mCount(0), mTimer(0), mValid(true) {
}

namespace NrvWanwanCreator {
    NEW_NERVE(WanwanCreatorNrvActive, WanwanCreator, Active);
    NEW_NERVE(WanwanCreatorNrvDeactive, WanwanCreator, Deactive);
};  // namespace NrvWanwanCreator

void WanwanCreator::init(const JMapInfoIter& rIter) {
    MR::connectToSceneEnemyMovement(this);
    mType = Wanwan::getType(rIter);
    initMapToolInfo(rIter);
    initRailRider(rIter);
    initNerve(&NrvWanwanCreator::WanwanCreatorNrvActive::sInstance, 0);
    MR::syncStageSwitchAppearAndSync(this, rIter);
    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< WanwanCreator*, void (WanwanCreator::*)() >(this, &WanwanCreator::invalidate));
    }
    bool once = false;
    if (mInterval < 0 || mType == 2) {
        once = true;
    }
    if (once) {
        mCount = 1;
        MR::declareStarPiece(this, Wanwan::getAppearStarPieceNum((Rock::Type)mType));
    } else {
        s32 stay = Wanwan::getGoalStayStep();
        s32 travel = (s32)(MR::getRailTotalLength(this) / mSpeed);
        travel += stay;
        mCount = travel / mInterval + 2;
        MR::declareStarPiece(this, Wanwan::getAppearStarPieceNum((Rock::Type)mType) * 3);
    }
    mWanwans = new Wanwan[mCount];
    for (s32 i = 0; i < mCount; i++) {
        mWanwans[i].mItemSource = this;
        mWanwans[i].mSpeed = mSpeed;
        mWanwans[i].init(rIter);
        if (mBreakAtGoal) {
            mWanwans[i].mBreakAtGoal = true;
        }
    }
    makeActorDead();
}

void WanwanCreator::appear() {
    if (mValid) {
        LiveActor::appear();
        mTimer = 0;
        MR::invalidateClipping(this);
        setNerve(&NrvWanwanCreator::WanwanCreatorNrvActive::sInstance);
    }
}

void WanwanCreator::kill() {
    if (mValid) {
        LiveActor::kill();
        for (s32 i = 0; i < mCount; i++) {
            if (!MR::isDead(&mWanwans[i])) {
                mWanwans[i].kill();
            }
        }
    }
}

void WanwanCreator::invalidate() {
    if (mValid) {
        kill();
        mValid = false;
    }
}

void WanwanCreator::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mSpeed);
    if (MR::getJMapInfoArg1NoInit(rIter, &mInterval)) {
        mInterval *= 60;
    }
    Wanwan::getType(rIter);
    MR::getJMapInfoArg5NoInit(rIter, &mBreakAtGoal);
}

void WanwanCreator::create() {
    for (s32 i = 0; i < mCount; i++) {
        if (MR::isDead(&mWanwans[i])) {
            mWanwans[i].appear();
            return;
        }
    }
}

namespace {
    inline bool isSingleWanwanCreation(const WanwanCreator& creator) {
        bool once = false;
        if (creator.mInterval < 0 || creator.mType == 2) {
            once = true;
        }
        return once;
    }
}

void WanwanCreator::exeActive() {
    bool once = isSingleWanwanCreation(*this);
    if (once || mTimer % mInterval == 0) {
        create();
        once = false;
        if (mInterval < 0 || mType == 2) {
            once = true;
        }
        if (once) {
            setNerve(&NrvWanwanCreator::WanwanCreatorNrvDeactive::sInstance);
            return;
        }
        mTimer = 0;
    }
    mTimer++;
}

void WanwanCreator::exeDeactive() {
    if (MR::isFirstStep(this)) {
        mTimer = 0;
    }
    if (mType == 2 && MR::isDead(&mWanwans[0])) {
        if (mWanwans[0].isGoalIn()) {
            invalidate();
        } else if (mInterval >= 0) {
            mTimer++;
            if (mInterval < mTimer) {
                setNerve(&NrvWanwanCreator::WanwanCreatorNrvActive::sInstance);
            }
        }
    }
}

WanwanCreator::~WanwanCreator() {
}

