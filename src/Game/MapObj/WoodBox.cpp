#include "Game/MapObj/WoodBox.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "revolution/types.h"

namespace NrvWoodBox {
    class WoodBoxNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            WoodBox* actor = reinterpret_cast< WoodBox* >(pSpine->mExecutor);
            MR::attachSupportTicoToTarget(actor);
        }
        static WoodBoxNrvWait sInstance;
    };
    WoodBoxNrvWait WoodBoxNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(WoodBoxNrvHit, WoodBox, Hit);

    class WoodBoxNrvKilled : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static WoodBoxNrvKilled sInstance;
    };
    WoodBoxNrvKilled WoodBoxNrvKilled::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWoodBox

WoodBox::WoodBox(const char* pName) : LiveActor(pName) {
}

WoodBox::~WoodBox() {
}

void WoodBox::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_NERVE(WoodBox, WoodBoxNrvWait));

    s32 arg0_Coins = 1;
    s32 arg1_Respawn = -1;
    s32 arg2_StarBits = 0;
    s32 arg3 = 0;
    s32 arg4_1Up = 0;
    s32 arg7_PowerStar = 0;

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0NoInit(rIter, &arg0_Coins);
        MR::getJMapInfoArg1NoInit(rIter, &arg1_Respawn);
        MR::getJMapInfoArg2NoInit(rIter, &arg2_StarBits);
        MR::getJMapInfoArg3NoInit(rIter, &arg3);
        MR::getJMapInfoArg4NoInit(rIter, &arg4_1Up);
        MR::getJMapInfoArg7NoInit(rIter, &arg7_PowerStar);
    }

    PSMTXCopy(getBaseMtx(), mBaseMtx);
    MR::addTransMtx(mBaseMtx, TVec3f(0.0f, 100.0f, 0.0f));

    mHasPowerStar = false;

    if (arg7_PowerStar != 0) {
        mHasPowerStar = true;
        MR::declarePowerStar(this);
        mStarDemoModel = MR::createPowerStarDemoModel(this, "パワースターデモモデル", mBaseMtx);
        mStarDemoModel->makeActorDead();
    }

    mCoinCount = arg0_Coins;
    mStarBitCount = arg2_StarBits;
    s32 arg4 = arg4_1Up;
    mHitPoint = 1;
    mIsNoRespawn = true;

    if (arg1_Respawn == 0) {
        mIsNoRespawn = false;
    }

    mPlaySolveSE = false;

    if (arg3 == 1) {
        mPlaySolveSE = true;
    }

    mBreakModel = MR::createBreakModel(this, nullptr);
    mFloorTouchTimer = 0;
    MR::setClippingFar200m(this);
    MR::validateClipping(this);

    if (mCoinCount != 0) {
        MR::declareCoin(this, mCoinCount);
    }

    if (mStarBitCount != 0) {
        MR::declareStarPiece(this, mStarBitCount);
    }

    if (arg4 != 0) {
        mOneUp = MR::createKinokoOneUp();
    } else {
        mOneUp = nullptr;
    }

    makeActorAppeared();
}

void WoodBox::control() {
    if (mFloorTouchTimer > 0) {
        mFloorTouchTimer--;
    }
}

// NONMATCHING: target passes r4/r5 (msg, sender) straight into doHit without moving them; source form does not reproduce that.
bool WoodBox::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mHitPoint == 0) {
        return false;
    }

    if (MR::isMsgPlayerUpperPunch(msg)) {
        doHit();
        MR::sendMsgReflectV(pSender, pReceiver);
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        doHit();
        return false;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        doHit();
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerHipDropFloor(msg)) {
        if (mFloorTouchTimer != 0) {
            doHit();
            return true;
        }

        return true;
    }

    return false;
}

bool WoodBox::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mFloorTouchTimer = 2;
    }

    return false;
}

// NONMATCHING: same doHit argument-register pattern as receiveMsgPlayerAttack.
bool WoodBox::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mHitPoint != 0) {
        doHit();
        return true;
    }

    return false;
}

void WoodBox::kill() {
    if (!mIsNoRespawn) {
        LiveActor::kill();
    } else {
        setNerve(GET_NERVE(WoodBox, WoodBoxNrvKilled));
        MR::invalidateHitSensors(this);
        MR::invalidateCollisionParts(this);
    }

    if (mHasPowerStar) {
        mStarDemoModel->makeActorDead();
        MR::requestAppearPowerStar(this, mPosition);
    }
}

void WoodBox::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensor(this, "Body");

        if (mHasPowerStar) {
            mStarDemoModel->makeActorAppeared();
        }
    }

    if (MR::isStep(this, 15)) {
        TVec3f upVec(0.0f);
        MR::calcUpVec(&upVec, this);

        if (mCoinCount != 0) {
            MR::appearCoinPopToDirection(this, mPosition, upVec, mCoinCount);
        }

        if (mStarBitCount != 0) {
            MR::appearStarPieceToDirection(this, mPosition, upVec, mStarBitCount, 10.0f, 40.0f, false);
        }

        if (mOneUp != nullptr) {
            TVec3f oneUpDirection;
            MR::calcUpVec(&oneUpDirection, this);
            Mtx spawnMtx;
            PSMTXCopy(getBaseMtx(), spawnMtx);

            MR::addTransMtx(spawnMtx, oneUpDirection * 50.0f);

            MR::appearKinokoOneUpHop(mOneUp, spawnMtx);
        }

        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }

        if (mPlaySolveSE) {
            MR::startActionSound(this, "SyReadRiddleS", -1, -1, -1);
        }
    }

    if (MR::isDead(mBreakModel)) {
        kill();
    }
}

void WoodBox::appear() {
    mFloorTouchTimer = 0;
    mHitPoint = 1;
    mBreakModel->kill();
    MR::validateCollisionParts(this);
    MR::validateHitSensors(this);
    MR::startBvaAndSetFrameAndStop(this, "Dust", 0.0f);
    setNerve(GET_NERVE(WoodBox, WoodBoxNrvWait));
    LiveActor::appear();
}

void WoodBox::doHit() {
    if (mHitPoint != 0) {
        if (mHitPoint != 0) {
            mHitPoint--;
        }

        if (mHitPoint == 0) {
            MR::invalidateCollisionParts(this);
        }

        if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(this, "OjWoodBoxBreakW");
        } else {
            MR::startActionSound(this, "OjWoodBoxBreak");
        }

        if (!MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(this, "EmExplodeS");
        }

        mBreakModel->appear();

        if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::emitEffect(mBreakModel, "BreakWater");
        } else {
            MR::emitEffect(mBreakModel, "Break");
        }

        if (mIsNoRespawn) {
            MR::startBva(this, "Dust");
        } else {
            MR::hideModel(this);
        }

        setNerve(GET_NERVE(WoodBox, WoodBoxNrvHit));
    }
}

void WoodBox::forceReappear() {
    if (!isNerve(GET_NERVE(WoodBox, WoodBoxNrvWait))) {
        if (!MR::isDead(mBreakModel)) {
            mBreakModel->kill();
        }

        MR::emitEffect(this, "Appear");
        MR::startActionSound(this, "Appear", -1, -1, -1);
    }

    appear();
}

void WoodBox::registerBreakModelDemoCast() {
    MR::registerDemoSimpleCastAll(mBreakModel);
}
