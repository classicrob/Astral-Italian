#include "Game/Util.hpp"
#include "Game/Enemy/MameMuimui.hpp"
#include "Game/Enemy/MameMuimuiListener.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

#define GET_MAMEMUIMUI_NERVE(name) (&NrvMameMuimui::name::sInstance)

// The target has no standalone exeEaten symbol: the Eaten nerve's execute carries the body.
inline void MameMuimui::exeEaten() {
    if (MR::isFirstStep(this)) {
        notifyScoreEvent();
    }
}

namespace NrvMameMuimui {
    NEW_NERVE(MameMuimuiNrvAppearSign, MameMuimui, AppearSign);
    NEW_NERVE(MameMuimuiNrvAppear, MameMuimui, Appear);
    NEW_NERVE(MameMuimuiNrvWait, MameMuimui, Wait);
    NEW_NERVE(MameMuimuiNrvHide, MameMuimui, Hide);
    NEW_NERVE(MameMuimuiNrvPress, MameMuimui, Press);
    NEW_NERVE(MameMuimuiNrvBlow, MameMuimui, Blow);
    NEW_NERVE(MameMuimuiNrvEaten, MameMuimui, Eaten);

    // Star-pointer bind state (SMG2 keeps it as an ActorState nerve; the execute just hands control back to Wait).
    class MameMuimuiNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait));
        };
        static MameMuimuiNrvBindStarPointer sInstance;
    };
    MameMuimuiNrvBindStarPointer MameMuimuiNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMameMuimui

MameMuimui::MameMuimui(const char* pName) : LiveActor(pName) {
    mListener = NULL;
    mAnimScaleCtrl = NULL;
    mStarPointBind = NULL;
    mFront.set(0.0f, 0.0f, 1.0f);
    mType = 0;
    mWaitTime = 60;
    mPoints = 1;
    mYoshiLockOn = NULL;
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void MameMuimui::setTypeGreen() {
    mType = 0;
    mPoints = 10;
}

void MameMuimui::setTypeYellow() {
    mType = 2;
    mPoints = 50;
}

void MameMuimui::setTypeSpike() {
    mType = 3;
    mPoints = 0;
}

void MameMuimui::setMuimuiListener(MameMuimuiListener* pListener) {
    mListener = pListener;
}

void MameMuimui::notifyScoreEvent() {
    if (mListener != NULL) {
        mListener->onActorEventEmpty(this);
        mListener->scoreHit(mPoints, this);
    }
}

void MameMuimui::respawnAt(const TVec3f& rPos, s32 waitTime) {
    if (MR::isDead(this)) {
        mWaitTime = waitTime;
        MR::resetPosition(this, rPos);
        appear();
        setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppearSign));
        mVelocity.set(0.0f, 0.0f, 0.0f);
    }
}

void MameMuimui::tryHideMuimui() {
    if (MR::isDead(this) == false) {
        if (isHideableNerve()) {
            MR::invalidateHitSensors(this);
            setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvHide));
        }
    }
}

bool MameMuimui::tryBlowAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (canBeAttacked() == false) {
        return false;
    }
    MR::fn_80010180(&mQuat, this, pReceiver, pSender);
    MR::setVelocityBlowAttack(this, pSender, pReceiver, 50.0f, 45.0f, 4);
    setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvBlow));
    return true;
}

bool MameMuimui::tryPressAttack() {
    if (canBeAttacked() == false) {
        return false;
    }
    MR::invalidateHitSensors(this);
    setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvPress));
    return true;
}

void MameMuimuiListener::onActorEventEmpty(MameMuimui* pActor) {}

void MameMuimuiListener::onListenerEmpty() {}

void MameMuimuiListener::onUpdateEmpty() {}

void MameMuimui::init(const JMapInfoIter& rIter) {
    const char* pName = "MameMuimuiGreen";
    switch (mType) {
    case 0:
        pName = "MameMuimuiGreen";
        break;
    case 1:
        pName = "MameMuimuiRed";
        break;
    case 2:
        pName = "MameMuimuiYellow";
        break;
    case 3:
        pName = "MameMuimuiSpike";
        break;
    }
    MR::initActor(this, rIter, pName, false);
    initNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppearSign), 1);

    mAnimScaleCtrl = new AnimScaleController(NULL);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);

    MR::initActorState(this, mStarPointBind, &NrvMameMuimui::MameMuimuiNrvBindStarPointer::sInstance, "SupportFreeze");
    MR::invalidateClipping(this);

    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(NULL), TVec3f(0.0f));
    makeActorDead();
}

void MameMuimui::appear() {
    MR::setRotation(this, 0.0f);
    MR::setScale(this, 1.0f);
    MR::fn_80010270(&mQuat, &mFront, this);
    LiveActor::appear();
    MR::invalidateHitSensors(this);
}

void MameMuimui::kill() {
    LiveActor::kill();
}

void MameMuimui::control() {
    mAnimScaleCtrl->updateNerve();
    if (isWaitOrAppearNerve() && mStarPointBind->tryStartPointBind()) {
        setNerve(&NrvMameMuimui::MameMuimuiNrvBindStarPointer::sInstance);
    }
}

void MameMuimui::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::setBaseTRMtx(this, mQuat);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void MameMuimui::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (mType == 3) {
            if (MR::sendMsgEnemyAttackFlipJump(pReceiver, pSender)) {
                MR::startActionSound(this, "SpikeAttack", -1, -1, -1);
                if (mListener != NULL) {
                    mListener->breakCombo(this);
                }
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool MameMuimui::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return tryBlowAttack(pSender, pReceiver);
    }
    return false;
}

bool MameMuimui::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        if (isWaitOrAppearNerve()) {
            mAnimScaleCtrl->startHitReaction();
            setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait));
            return true;
        }
    }
    if (mType == 3) {
        return false;
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        return tryPressAttack();
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        return tryBlowAttack(pSender, pReceiver);
    }
    return false;
}

bool MameMuimui::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    return MR::isSensorPlayer(pSender);
}

bool MameMuimui::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg)) {
        return tryBlowAttack(pSender, pReceiver);
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isTongueTargetNerve()) {
            if (mType == 3) {
                MR::fn_80422290();
                MR::startActionSound(this, "SpikeAttack", -1, -1, -1);
                if (mListener != NULL) {
                    mListener->breakCombo(this);
                }
                return false;
            }
            MR::invalidateHitSensors(this);
            setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvEaten));
            MR::zeroVelocity(this);
            return true;
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvEaten))) {
            return true;
        }
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvEaten))) {
            kill();
            return true;
        }
    }
    if (MR::isMsgYoshiCancelEaten(msg) && isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvEaten))) {
        kill();
        return true;
    }
    return false;
}

void MameMuimui::exeAppearSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AppearSign");
    }
    if (MR::isGreaterStep(this, 8)) {
        MR::validateHitSensors(this);
        setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppear));
    }
}

void MameMuimui::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 3.0f);
    MR::fn_800104F0(&mQuat, this, &mFront, 0.2f, 0.2f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait));
    }
}

void MameMuimui::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 3.0f);
    MR::fn_800104F0(&mQuat, this, &mFront, 0.2f, 0.2f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isGreaterStep(this, mWaitTime)) {
        setNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvHide));
    }
}

void MameMuimui::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void MameMuimui::exePress() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        notifyScoreEvent();
    }
    if (MR::isActionEnd(this)) {
        MR::tryHitReactionDead(this);
        kill();
    }
}

void MameMuimui::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::tryHitReactionBlow(this, &mPosition);
        notifyScoreEvent();
    }
    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.96f);
    if (MR::isGreaterStep(this, 20)) {
        MR::tryHitReactionDead(this);
        kill();
        MR::startActionSound(this, "Explode", -1, -1, -1);
    }
}

bool MameMuimui::isWaitOrAppearNerve() const {
    if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait)) || isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppear))) {
        return true;
    }
    return false;
}

bool MameMuimui::isTongueTargetNerve() const {
    if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppear)) || isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait)) ||
        isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvHide))) {
        return true;
    }
    return false;
}

bool MameMuimui::isHideableNerve() const {
    if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppearSign)) || isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppear)) ||
        isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait)) || isNerve(&NrvMameMuimui::MameMuimuiNrvBindStarPointer::sInstance)) {
        return true;
    }
    return false;
}

bool MameMuimui::canBeAttacked() const {
    if (mType == 3) {
        return false;
    }
    if (isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvAppear)) || isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvWait)) ||
        isNerve(GET_MAMEMUIMUI_NERVE(MameMuimuiNrvHide)) || isNerve(&NrvMameMuimui::MameMuimuiNrvBindStarPointer::sInstance)) {
        return true;
    }
    return false;
}

bool MameMuimui::isYellowType() const {
    return mType == 2;
}

MameMuimui::~MameMuimui() {}
