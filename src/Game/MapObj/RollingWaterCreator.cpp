#pragma auto_inline off
#include "Game/MapObj/RollingWaterCreator.hpp"
#include "Game/MapObj/RollingWater.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

static s32 sDefaultInterval = 5;
static s32 sDefaultDelay = 10;

namespace NrvRollingWaterCreator {
    NEW_NERVE(Wait, RollingWaterCreator, Wait);
}

RollingWaterCreator::RollingWaterCreator(const char* pName) : LiveActor(pName) {
    mRollingWaterGroup = nullptr;
    mInterval = sDefaultInterval;
    mDelay = sDefaultDelay;
    mCounter = 0;
}

void RollingWaterCreator::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg1NoInit(rIter, &mInterval);
    if (mInterval <= 0) {
        mInterval = sDefaultInterval;
    }
    mInterval *= 60;
    MR::getJMapInfoArg2NoInit(rIter, &mDelay);
    if (mDelay <= 0) {
        mDelay = sDefaultDelay;
    }
    mDelay *= 60;

    RollingWater* pFirst = new RollingWater("\x83\x53\x83\x8D\x90\x85\x8B\xCA");
    pFirst->init(rIter);
    pFirst->makeActorDead();
    s32 count = pFirst->calcRailStepCount() / mInterval + 2;

    mRollingWaterGroup = new DeriveActorGroup<RollingWater>("\x83\x53\x83\x8D\x90\x85\x8B\xCA\x83\x8A\x83\x58\x83\x67", count);
    mRollingWaterGroup->registerActor(pFirst);

    for (s32 i = 0; i < count - 1; i++) {
        RollingWater* pRW = new RollingWater("\x83\x53\x83\x8D\x90\x85\x8B\xCA");
        pRW->init(rIter);
        pRW->makeActorDead();
        mRollingWaterGroup->registerActor(pRW);
    }

    initNerve(GET_NERVE(RollingWaterCreator, Wait), 0);
    MR::invalidateClipping(this);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

bool RollingWaterCreator::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgRollingWaterPlayerEnter(msg)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        return true;
    }

    if (MR::isMsgRollingWaterPlayerExit(msg)) {
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        return true;
    }

    return false;
}

void RollingWaterCreator::exeWait() {
    if (mCounter % mInterval == 0) {
        LiveActorGroup* pGroup = mRollingWaterGroup;
        RollingWater* pDead = pGroup->getDeadActor() != nullptr
            ? static_cast< RollingWater* >(pGroup->getDeadActor()) : nullptr;
        if (pDead != nullptr) {
            pDead->appearFromCreator(this, mDelay);
        }
        mCounter = 0;
    }
    mCounter++;
}

RollingWaterCreator::~RollingWaterCreator() {
}
