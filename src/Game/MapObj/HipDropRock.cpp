#include "Game/MapObj/HipDropRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sStepToSwitchOn = 30;
    static const s32 sDefaultStarPieceNum = 6;
};  // namespace

namespace NrvHipDropRock {
    NEW_NERVE(HipDropRockNrvWait, HipDropRock, Wait);
    NEW_NERVE(HipDropRockNrvBreak, HipDropRock, Break);
    NEW_NERVE(HipDropRockNrvWreck, HipDropRock, Wreck);
};  // namespace NrvHipDropRock

HipDropRock::HipDropRock(const char* pName) : LiveActor(pName), mBreakModel(), mItemType(-1), mStarPieceNum(-1), mKillOnBreak() {
    mBreakModelMtx.identity();
}

void HipDropRock::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "HipDropRock", false);
    initBreakModel();
    MR::connectToSceneMapObjStrongLight(this);
    initHitSensor(2);
    MR::addBodyMessageSensorMapObj(this);
    MR::addHitSensor(this, "Launcher", 0x7F, 3, 150.0f, TVec3f(0.0f, 100.0f, 0.0f));
    MR::initCollisionParts(this, "HipDropRock", getSensor("Body"), nullptr);
    initEffectKeeper(0, nullptr, false);
    initSound(2, "HipDropRock", nullptr, TVec3f(0.0f));
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    HipDropRock::initItem(rIter);
    MR::getJMapInfoArg7NoInit(rIter, &mKillOnBreak);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::tryRegisterDemoCast(mBreakModel, rIter);
    }

    initNerve(GET_NERVE(HipDropRock, HipDropRockNrvWait));
    makeActorAppeared();
}

void HipDropRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
}

void HipDropRock::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjRockBreak");
        MR::invalidateCollisionParts(this);
        MR::invalidateClipping(this);

        if (mKillOnBreak) {
            MR::hideModel(this);
        } else {
            MR::startBva(this, "BreakLevel");
        }

        mBreakModel->makeActorAppeared();
        MR::invalidateClipping(mBreakModel);
        MR::startAction(mBreakModel, "HipDropRockBreak");

        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }

        HipDropRock::appearItem();
    }

    if (MR::isStep(this, ::sStepToSwitchOn) && MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    if (MR::isActionEnd(mBreakModel) && MR::isGreaterStep(this, ::sStepToSwitchOn)) {
        mBreakModel->kill();

        if (mKillOnBreak) {
            kill();
        } else {
            setNerve(GET_NERVE(HipDropRock, HipDropRockNrvWreck));
        }
    }
}

void HipDropRock::exeWreck() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
}

bool HipDropRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(HipDropRock, HipDropRockNrvWait)) || MR::isDead(this)) {
        return false;
    }

    if (MR::isMsgPlayerHipDropFloor(msg)) {
        setNerve(GET_NERVE(HipDropRock, HipDropRockNrvBreak));
        return true;
    }

    return false;
}

bool HipDropRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(HipDropRock, HipDropRockNrvWait)) || MR::isDead(this)) {
        return false;
    }

    if (msg == 0xF4) {
        setNerve(GET_NERVE(HipDropRock, HipDropRockNrvBreak));
        return true;
    }

    return false;
}

void HipDropRock::initBreakModel() {
    mBreakModelMtx.set(getBaseMtx());
    TVec3f upVec(0.0f, 1.0f, 0.0f);
    TVec3f randomRot;
    randomRot.scale(MR::getRandom(0.0f, 180.0f), upVec);
    TPos3f mtx;
    MR::makeMtxMoment(&mtx, randomRot);
    mBreakModelMtx.concat(mtx);

    mBreakModel = MR::createModelObjMapObj("ヒビ石壊れモデル", "HipDropRockBreak", mBreakModelMtx);
    mBreakModel->initWithoutIter();
    MR::invalidateClipping(mBreakModel);
    mBreakModel->makeActorDead();
}

void HipDropRock::initItem(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mItemType);
    MR::getJMapInfoArg1NoInit(rIter, &mStarPieceNum);

    if (mItemType == 1 && mStarPieceNum == -1) {
        mStarPieceNum = ::sDefaultStarPieceNum;
    }

    if (mItemType == 0) {
        MR::declareCoin(this, 1);
    } else if (mItemType == 1) {
        MR::declareStarPiece(this, mStarPieceNum);
    }
}

void HipDropRock::appearItem() {
    if (mItemType == -1) {
        return;
    }

    if (mItemType == 0) {
        MR::appearCoinPop(this, mPosition, 1);
    } else if (mItemType == 1) {
        MR::appearStarPiece(this, mPosition, mStarPieceNum, 10.0f, 40.0f, false);
    }
}
