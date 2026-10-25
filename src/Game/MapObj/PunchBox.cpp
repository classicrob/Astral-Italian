#include "Game/Util/Functor.hpp"
#include "Game/MapObj/PunchBox.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace NrvPunchBox {
    NEW_NERVE(PunchBoxNrvWait, PunchBox, Wait);
    NEW_NERVE(PunchBoxNrvHit, PunchBox, Hit);
    class PunchBoxNrvHidden : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static PunchBoxNrvHidden sInstance;
    };
    PunchBoxNrvHidden PunchBoxNrvHidden::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvPunchBox

PunchBox::PunchBox(const char* pName) : LiveActor(pName) {
}

void PunchBox::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_NERVE(PunchBox, PunchBoxNrvWait));

    s32 arg0 = 1;
    s32 arg1 = 1;

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0NoInit(rIter, &arg0);
        MR::getJMapInfoArg1NoInit(rIter, &arg1);
    }

    mCoinCount = arg0;
    mRemainingHits = 1;
    mBreakTimer = 0;
    mAnimTimer = MR::getRandom(0l, 31l);
    mFloorTouchTimer = 0;

    MR::validateClipping(this);

    mHasShadow = false;
    if (arg1 != 0) {
        mHasShadow = true;
    }

    MR::declareCoin(this, mCoinCount);
    MR::declareStarPiece(this, 1);

    appear();

    if (mHasShadow) {
        MR::excludeCalcShadowToMyCollision(this, nullptr);
    } else {
        MR::invalidateShadowAll(this);
    }

    MR::addToAttributeGroupSearchTurtle(this);

    if (MR::isValidSwitchA(this)) {
        MR::listenStageSwitchOnA(this, MR::Functor_Inline(this, &PunchBox::kill));
    }

    MR::invalidateHitSensor(this, "Body");
}

void PunchBox::initAfterPlacement() {
    if (mHasShadow) {
        MR::onCalcGravity(this);
        MR::calcGravity(this);
        MR::onCalcShadowOneTime(this, nullptr);
    }
}

void PunchBox::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayer(pReceiver)) {
        return;
    }

    if (pSender != getSensor("eye")) {
        return;
    }

    if (!MR::isSensorPlayer(pReceiver)) {
        return;
    }

    TVec3f distance(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));

    if (distance.length() < 410.0f) {
        mIsPlayerNear = true;
    }
}

void PunchBox::control() {
    if (mBreakTimer == 0) {
        mAnimTimer++;
    }

    if (mFloorTouchTimer != 0) {
        mFloorTouchTimer--;
    }
}

void PunchBox::calcAndSetBaseMtx() {
    if (mHasShadow && (mAnimTimer & 0x7F) == 0x7F) {
        MR::onCalcShadowOneTime(this, nullptr);
    }

    LiveActor::calcAndSetBaseMtx();
}

void PunchBox::exeWait() {
    if (mBreakTimer != 0 && --mBreakTimer == 0) {
        mRemainingHits = 1;
    }

    if (mRemainingHits == 0) {
        setNerve(GET_NERVE(PunchBox, PunchBoxNrvHit));
    }

    mIsPlayerNear = false;
    MR::attachSupportTicoToTarget(this);
}

void PunchBox::exeHit() {
    if (getNerveStep() == 0) {
        MR::invalidateHitSensor(this, "Body");
    }

    if (getNerveStep() == 5) {
        MR::hideModel(this);
    }

    if (getNerveStep() == 15) {
        if (mCoinCount != 0) {
            MR::appearCoinPop(this, mPosition, mCoinCount);
        }

        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }

        setNerve(GET_NERVE(PunchBox, PunchBoxNrvHidden));
        makeActorDead();
    }
}

void PunchBox::makeActorAppeared() {
    if (!isNerve(GET_NERVE(PunchBox, PunchBoxNrvHidden))) {
        LiveActor::makeActorAppeared();
    }
}

void PunchBox::makeActorDead() {
    LiveActor::makeActorDead();
}

void PunchBox::kill() {
    if (MR::isDead(this)) {
        return;
    }

    MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
    LiveActor::kill();
    setNerve(GET_NERVE(PunchBox, PunchBoxNrvHidden));
}

void PunchBox::doHit(HitSensor* pSender, HitSensor* pReceiver) {
    if (mRemainingHits != 0) {
        mRemainingHits--;
    }

    if (mRemainingHits == 0) {
        MR::invalidateCollisionParts(this);
    }

    mBreakTimer = 30;

    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::emitEffect(this, "BreakWater");
        MR::startActionSound(this, "OjPunchBoxBreakW");
    } else {
        MR::emitEffect(this, "Break");
        MR::startActionSound(this, "OjPunchBoxBreak");
    }
}

bool PunchBox::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(PunchBox, PunchBoxNrvHit))) {
        return false;
    }

    if (MR::isMsgPlayerUpperPunch(msg)) {
        if (mRemainingHits != 0) {
            doHit(pSender, pReceiver);
            MR::sendMsgReflectV(pSender, pReceiver);
        }

        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerHipDropFloor(msg)) {
        if (mFloorTouchTimer != 0) {
            doHit(pSender, pReceiver);
            return true;
        }

        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        doHit(pSender, pReceiver);
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg) && !MR::isOnPlayer(this)) {
        if (mRemainingHits != 0) {
            doHit(pSender, pReceiver);
        }

        return true;
    }

    return false;
}

bool PunchBox::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mRemainingHits != 0) {
        doHit(pSender, pReceiver);
        return true;
    }

    return false;
}

bool PunchBox::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mFloorTouchTimer = 2;
    }

    return false;
}
