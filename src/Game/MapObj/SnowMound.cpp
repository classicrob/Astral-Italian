#include "Game/MapObj/SnowMound.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/SnowBall.hpp"
#include "Game/MapObj/WhiteSnowTree.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvSnowMound {
    class SnowMoundNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SnowMoundNrvWait sInstance;
    };
    SnowMoundNrvWait SnowMoundNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(SnowMoundNrvMelt, SnowMound, Melt);
    NEW_NERVE(SnowMoundNrvHide, SnowMound, Hide);
    NEW_NERVE(SnowMoundNrvHidden, SnowMound, Hidden);
};  // namespace NrvSnowMound

SnowMound::SnowMound(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mBreakDustDemo = nullptr;
    mSnowBalls = nullptr;
    mSnowBallNum = 0;
    mCsvValue = 1;
    mIsCsvFlag = true;
    mSnowTree = nullptr;
    mPowerStarId = -1;
    mIsRiddle = false;
}

void SnowMound::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initCsvParam();
    MR::initMapPartsClipping(this, rIter, nullptr, false);
    mBreakModel = MR::createBreakModel(this, nullptr);
    MR::registerDemoSimpleCastAll(mBreakModel);
    mBreakDustDemo = MR::tryCreateBreakDustDemoObj(this, getBaseMtx());
    mSnowBallNum = 2;
    MR::getJMapInfoArg0NoInit(rIter, &mSnowBallNum);
    if (mSnowBallNum > 0) {
        mSnowBalls = new SnowBall*[mSnowBallNum];
        for (s32 i = 0; i < mSnowBallNum; i++) {
            mSnowBalls[i] = new SnowBall("\x90\xe1\x82\xbe\x82\xdc\x5b\x90\xe1\x92\xcb\x97\x70\x5d");
            MR::setPosition(mSnowBalls[i], mPosition);
            mSnowBalls[i]->initWithoutIter();
            mSnowBalls[i]->makeActorDead();
        }
    }

    const char* pObjName = nullptr;
    MR::getObjectName(&pObjName, rIter);
    if (MR::isEqualString(pObjName, "SnowDollKoopa")) {
        s32 riddleArg = -1;
        MR::getJMapInfoArg2NoInit(rIter, &riddleArg);
        if (riddleArg != -1) {
            mIsRiddle = true;
        }
        MR::getJMapInfoArg3NoInit(rIter, &mPowerStarId);
        if (mPowerStarId != -1) {
            MR::declarePowerStar(this, mPowerStarId);
        }
    }
    if (MR::isEqualString(pObjName, "SnowMoundL")) {
        bool hasSnowTree = false;
        MR::getJMapInfoArg2NoInit(rIter, &hasSnowTree);
        if (hasSnowTree) {
            mSnowTree = new WhiteSnowTree("\x90\xe1\x82\xcc\x92\xcb\x5b\x96\xd8\x5d");
            mSnowTree->initWithoutIter();
            MR::invalidateClipping(mSnowTree);
            mSnowTree->makeActorDead();
            MR::setPosition(mSnowTree, mPosition);
            MR::setRotation(mSnowTree, mRotation);
        }
    }
    initNerve(&NrvSnowMound::SnowMoundNrvWait::sInstance, 0);
    makeActorAppeared();
}

void SnowMound::initCsvParam() {
    mCsvValue = 1;
    mIsCsvFlag = true;
    JMapInfo* pCsv = MR::tryCreateCsvParser(this, "Reaction");
    if (pCsv != nullptr) {
        if (MR::hasCsvItem(pCsv, "Life")) {
            MR::getCsvDataS32(&mCsvValue, pCsv, "Life", 0);
        }
        if (MR::hasCsvItem(pCsv, "NormalEnemyAttack")) {
            MR::getCsvDataBool(&mIsCsvFlag, pCsv, "NormalEnemyAttack", 0);
        }
    }
}

void SnowMound::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    afterKill();
}

bool SnowMound::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHitProjectiles(msg)) {
        tryStartMelt();
        return true;
    }
    return false;
}

bool SnowMound::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackRoll(msg)) {
        tryStartMelt();
        return true;
    }
    if (mIsCsvFlag && MR::isMsgEnemyAttack(msg)) {
        tryStartMelt();
        return true;
    }
    return false;
}

void SnowMound::exeMelt() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
        MR::startActionSound(this, "Damage", -1, -1, -1);
        if (MR::startBvaIfExist(this, "HitPoint")) {
            MR::setBvaFrameAndStop(this, (f32)mCsvValue);
        }
        mBreakModel->appear();
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSnowMound::SnowMoundNrvWait::sInstance);
    }
}

void SnowMound::exeHide() {
    if (MR::isFirstStep(this)) {
        afterKill();
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        if (mBreakDustDemo != nullptr) {
            mBreakDustDemo->appear();
        }
        mBreakModel->appear();
    }
    if (MR::isStep(this, 2)) {
        MR::hideModel(this);
    }
    if (MR::isDead(mBreakModel)) {
        if (mPowerStarId != -1) {
            MR::requestAppearPowerStar(this, mPowerStarId, mPosition);
        }
        setNerve(&NrvSnowMound::SnowMoundNrvHidden::sInstance);
    }
}

void SnowMound::exeHidden() {
    if (MR::isFirstStep(this) && mBreakDustDemo == nullptr) {
        kill();
    }
}

bool SnowMound::tryStartMelt() {
    if (!isNerve(&NrvSnowMound::SnowMoundNrvWait::sInstance)) {
        return false;
    }
    mCsvValue--;
    if (mCsvValue == 0) {
        setNerve(&NrvSnowMound::SnowMoundNrvHide::sInstance);
    } else {
        setNerve(&NrvSnowMound::SnowMoundNrvMelt::sInstance);
    }
    return true;
}

// NONMATCHING: only the velocity copy differs (target loads the source address into r4 before r3 for the copy-ctor, and the
// TVec3f temporaries sit in swapped stack slots); the snowball angles, positions, and launch call are identical.
void SnowMound::launchSnowBalls() {
    TVec3f pos(mPosition);
    TVec3f up(0.0f);
    TVec3f front(0.0f);
    MR::calcUpVec(&up, this);
    MR::calcFrontVec(&front, this);
    pos += up * 200.0f;
    for (s32 i = 0; i < mSnowBallNum; i++) {
        TVec3f dir(0.0f);
        MR::rotateVecDegree(&dir, front, up, (360.0f / (f32)mSnowBallNum) * (f32)i);
        TVec3f launchPos = pos + dir * 100.0f;
        TVec3f launchVel = up * 30.0f + dir * 8.0f;
        mSnowBalls[i]->launch(launchPos, launchVel, false);
    }
}

void SnowMound::afterKill() {
    MR::startActionSound(this, "Break", -1, -1, -1);
    if (mIsRiddle) {
        MR::startActionSound(this, "RiddleSE", -1, -1, -1);
    }
    launchSnowBalls();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    if (mSnowTree != nullptr) {
        mSnowTree->appear();
    }
}

SnowMound::~SnowMound() {
}
