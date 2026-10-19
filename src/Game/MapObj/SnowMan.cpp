#include "Game/MapObj/SnowMan.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sStepForMelt = 20;
};  // namespace

namespace NrvSnowMan {
    NEW_NERVE(SnowManNrvWait, SnowMan, Wait);
    NEW_NERVE(SnowManNrvWaitBody, SnowMan, WaitBody);
    NEW_NERVE(SnowManNrvWaitHead, SnowMan, WaitHead);
    NEW_NERVE(SnowManNrvMeltHead, SnowMan, MeltHead);
    NEW_NERVE(SnowManNrvMeltBody, SnowMan, MeltBody);
    NEW_NERVE(SnowManNrvDownHead, SnowMan, DownHead);
    NEW_NERVE(SnowManNrvDownBody, SnowMan, DownBody);
};  // namespace NrvSnowMan

SnowMan::SnowMan(const char* pName) : LiveActor(pName), mHeadCollisionParts(), mBodyCollisionParts(), mBreakDustDemo(), mAnimScaleCtrl(), mAnimScaleParam() {
}

void SnowMan::init(const JMapInfoIter& rrIter) {
    MR::initActor(this, rrIter, false);
    mHeadCollisionParts = MR::createCollisionPartsFromLiveActor(this, "HeadCol", getSensor("Head"), MR::getJointMtx(this, "Head"),
                                                                MR::CollisionScaleType_NotUsingScale);
    mBodyCollisionParts = MR::createCollisionPartsFromLiveActor(this, "BodyCol", getSensor("Body"), MR::getJointMtx(this, "Body"),
                                                                MR::CollisionScaleType_NotUsingScale);
    MR::validateCollisionParts(mHeadCollisionParts);
    MR::validateCollisionParts(mBodyCollisionParts);
    mAnimScaleParam = new AnimScaleParam();
    mAnimScaleParam->mMinScaleY = 0.95f;
    mAnimScaleParam->mMaxScaleY = 1.05f;
    mAnimScaleParam->mHitReactionSpringStrength = 0.60f;
    mAnimScaleParam->mHitReactionDamping = 0.8f;
    mAnimScaleCtrl = new AnimScaleController(mAnimScaleParam);
    mBreakDustDemo = MR::tryCreateBreakDustDemoObj(this, getBaseMtx());
    initNerve(GET_NERVE(SnowMan, SnowManNrvWait));
    makeActorAppeared();
}

void SnowMan::control() {
    if (mHeadCollisionParts->mIsRegistered) {
        mHeadCollisionParts->setMtx();
    }

    if (mBodyCollisionParts->mIsRegistered) {
        mBodyCollisionParts->setMtx();
    }
    mAnimScaleCtrl->updateNerve();
}

void SnowMan::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TVec3f scale = mAnimScaleCtrl->mScale * mScale;
    MR::setBaseScale(this, scale);
}

void SnowMan::exeWait() {
}

void SnowMan::exeWaitBody() {
}

void SnowMan::exeWaitHead() {
}

void SnowMan::exeMeltHead() {
    if (MR::isFirstStep(this)) {
        MR::invalidateCollisionParts(mHeadCollisionParts);
        MR::startAction(this, "MeltHead");
        MR::shakeCameraWeak();
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }

    if (MR::isActionEnd(this)) {
        MR::hideMaterial(this, "SnowManBucketMat_v");
        setNerve(GET_NERVE(SnowMan, SnowManNrvWaitBody));
    }
}

void SnowMan::exeMeltBody() {
    if (MR::isFirstStep(this)) {
        MR::invalidateCollisionParts(mBodyCollisionParts);
        MR::startAction(this, "MeltBody");
        MR::shakeCameraWeak();
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SnowMan, SnowManNrvWaitHead));
    }
}

void SnowMan::exeDownHead() {
    if (MR::isFirstStep(this)) {
        MR::invalidateCollisionParts(mHeadCollisionParts);
        MR::invalidateCollisionParts(mBodyCollisionParts);
        MR::startAction(this, "DownHead");
        MR::shakeCameraWeak();
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void SnowMan::exeDownBody() {
    if (MR::isFirstStep(this)) {
        MR::invalidateCollisionParts(mHeadCollisionParts);
        MR::invalidateCollisionParts(mBodyCollisionParts);
        MR::startAction(this, "DownBody");
        MR::shakeCameraWeak();
        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void SnowMan::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(GET_NERVE(SnowMan, SnowManNrvMeltBody)) && MR::isEqualSensor(sender, this, "Push")) {
        MR::sendMsgPush(receiver, sender);
    }
}

bool SnowMan::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Push")) return false;
    if (MR::isMsgFireBallAttack(msg) || MR::isMsgRockAttack(msg) || MR::isMsgSnowBallAttack(msg)) {
        if (pReceiver == getSensor("Head")) {
            damageHead();
        } else {
            damageBody();
        }

        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (mAnimScaleCtrl->isHitReaction(-1)) {
            return false;
        } else {
            mAnimScaleCtrl->startHitReaction();
            MR::startActionSound(this, "Shake");
            return true;
        }
    }

    return false;
}

void SnowMan::damageHead() {
    if (isNerve(GET_NERVE(SnowMan, SnowManNrvWait))) {
        setNerve(GET_NERVE(SnowMan, SnowManNrvMeltHead));
    } else if (isNerve(GET_NERVE(SnowMan, SnowManNrvWaitHead))) {
        setNerve(GET_NERVE(SnowMan, SnowManNrvDownHead));
    }
}

void SnowMan::damageBody() {
    if (isNerve(GET_NERVE(SnowMan, SnowManNrvWait))) {
        setNerve(GET_NERVE(SnowMan, SnowManNrvMeltBody));
    } else if (isNerve(GET_NERVE(SnowMan, SnowManNrvWaitBody))) {
        setNerve(GET_NERVE(SnowMan, SnowManNrvDownBody));
    }
    if (mBreakDustDemo && MR::isDead(mBreakDustDemo)) {
        mBreakDustDemo->appear();
    }
}

