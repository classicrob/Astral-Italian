#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif
#include "Game/Boss/Sandy.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

static const bool sCactusTable[20] = {false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false};
static u32 sBallCount = 0;

// 800FD1F0
SandyBallHolder::SandyBallHolder(Sandy* pSandy)
    : NameObj("\203T\203\223\203f\203B\201[\202\314\203T\203{\203e\203\223\203{\201[\203\213\225\333\216\235"), mSandy(pSandy) {
    mBalls0.init(48);
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        *p = new SandyBall(0, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls1.init(32);
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        *p = new SandyBall(2, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls2.init(16);
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        *p = new SandyBall(1, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls3.init(16);
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        *p = new SandyBall(3, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
    mBalls4.init(2);
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        *p = new SandyBall(4, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
}

// 800FD480
SandyBall* SandyBallHolder::getBall(LiveActor* pOwner, s32 type) {
    if (type == 2) {
        return getFreeBallType2(pOwner);
    }
    if (type == 3) {
        return getFreeBallType3(pOwner);
    }
    if (type == 4) {
        return getFreeBallType4(pOwner);
    }
    u32 idx = sBallCount;
    if (idx >= 20) {
        idx = 0;
    }
    sBallCount = idx + 1;
    if (sCactusTable[idx] && !MR::isGalaxyDarkCometAppearInCurrentStage()) {
        return getFreeBallType1(pOwner);
    }
    return getFreeBall(pOwner);
}

// 800FD540
SandyBall* SandyBallHolder::getFreeBall(LiveActor* pOwner) {
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD5C0
SandyBall* SandyBallHolder::getFreeBallType2(LiveActor* pOwner) {
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD640
SandyBall* SandyBallHolder::getFreeBallType1(LiveActor* pOwner) {
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        if ((*p)->isFree() && !(*p)->mCoinPopped) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD6E0
SandyBall* SandyBallHolder::getFreeBallType3(LiveActor* pOwner) {
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD770
SandyBall* SandyBallHolder::getFreeBallType4(LiveActor* pOwner) {
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD800
void SandyBallHolder::killAll() {
    std::for_each(mBalls0.begin(), mBalls0.end(), std::mem_fun(&SandyBall::tryKill));
    std::for_each(mBalls1.begin(), mBalls1.end(), std::mem_fun(&SandyBall::tryKill));
    std::for_each(mBalls2.begin(), mBalls2.end(), std::mem_fun(&SandyBall::tryKill));
    std::for_each(mBalls3.begin(), mBalls3.end(), std::mem_fun(&SandyBall::tryKill));
    std::for_each(mBalls4.begin(), mBalls4.end(), std::mem_fun(&SandyBall::tryKill));
}

// 800FDA70
SandyBallHolder::~SandyBallHolder() {
}
