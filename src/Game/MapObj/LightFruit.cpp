#pragma auto_inline off
#include "Game/MapObj/LightFruit.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
namespace NrvLightFruit {
    NEW_NERVE(LightFruitNrvAppear, LightFruit, Appear);
    NEW_NERVE(LightFruitNrvWait, LightFruit, Wait);
    NEW_NERVE(LightFruitNrvTrampled, LightFruit, Reaction);
    NEW_NERVE(LightFruitNrvStarPiece, LightFruit, Reaction);
    NEW_NERVE(LightFruitNrvSpin, LightFruit, Reaction);
    NEW_NERVE(LightFruitNrvShake, LightFruit, Reaction);
    class LightFruitNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static LightFruitNrvEaten sInstance;
    };
    LightFruitNrvEaten LightFruitNrvEaten::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(LightFruitNrvHide, LightFruit, Hide);
    NEW_NERVE(LightFruitNrvHipDropCrash, LightFruit, HipDropCrash);
}
LightFruit::LightFruit(const char* name)
    : LiveActor(name), mOriginalRotation(0.0f, 0.0f, 0.0f, 1.0f), mOriginalPosition(0.0f, 0.0f, 0.0f), mReaction(nullptr), mYoshiTarget(nullptr) {
    MR::createLightFruitClipArea();
}
LightFruit::~LightFruit() {}
void LightFruit::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "LightFruit", false);
    MR::makeQuatFromRotate(&mOriginalRotation, this);
    mOriginalPosition.x = mPosition.x;
    mOriginalPosition.y = mPosition.y;
    mOriginalPosition.z = mPosition.z;
    mYoshiTarget = MR::createYoshiLockOnTarget(this, "Center", TVec3f(0.0f));
    initNerve(&NrvLightFruit::LightFruitNrvWait::sInstance);
    makeActorAppeared();
}
void LightFruit::control() {
    if (isEatable()) YoshiFunction::tryYoshiLick(mYoshiTarget);
}
void LightFruit::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (isPushable()) MR::sendMsgPush(receiver, sender);
}
bool LightFruit::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
    if (MR::isMsgStarPieceAttack(msg) && isEatable()) {
        mReaction = "Shake";
        setNerve(&NrvLightFruit::LightFruitNrvStarPiece::sInstance);
        return true;
    }
    if (MR::isMsgPlayerTrample(msg) && isPushable()) {
        mReaction = "Trampled";
        setNerve(&NrvLightFruit::LightFruitNrvTrampled::sInstance);
        return true;
    }
    if (MR::isMsgPlayerHipDrop(msg) && isPushable()) {
        setNerve(&NrvLightFruit::LightFruitNrvHipDropCrash::sInstance);
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isEatable() && !isNerve(&NrvLightFruit::LightFruitNrvSpin::sInstance)) {
        mReaction = "Spin";
        setNerve(&NrvLightFruit::LightFruitNrvSpin::sInstance);
        return true;
    }
    return false;
}
bool LightFruit::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (isEatable()) {
        if (MR::fn_80422890(msg)) return true;
        if (MR::isMsgYoshiStartTongue(msg) || MR::isMsgYoshiAutoEatStart(msg)) {
            MR::invalidateClipping(this);
            setNerve(&NrvLightFruit::LightFruitNrvEaten::sInstance);
            return true;
        }
    }
    if (isNerve(&NrvLightFruit::LightFruitNrvEaten::sInstance)) {
        if (MR::isMsgYoshiLoopTongue(msg)) return true;
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            MR::tryHitReaction(this, "取得");
            MR::startYoshiLightFruit();
            setNerve(&NrvLightFruit::LightFruitNrvHide::sInstance);
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            MR::setRotationFromQuat(this, mOriginalRotation);
            MR::setScale(this, 1.0f);
            MR::resetPosition(this, mOriginalPosition);
            MR::showModel(this);
            setNerve(&NrvLightFruit::LightFruitNrvAppear::sInstance);
            return true;
        }
        if (MR::isMsgYoshiAutoEatEnd(msg)) {
            MR::tryHitReaction(this, "取得");
            MR::startYoshiLightFruit();
            setNerve(&NrvLightFruit::LightFruitNrvHide::sInstance);
            return true;
        }
    }
    return false;
}
void LightFruit::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Appear");
    if (MR::isActionEnd(this)) setNerve(&NrvLightFruit::LightFruitNrvWait::sInstance);
}
void LightFruit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::showModel(this);
        MR::validateClipping(this);
    }
    MR::attachSupportTicoToTarget(this);
}
void LightFruit::exeReaction() {
    if (!mReaction) {
        setNerve(&NrvLightFruit::LightFruitNrvWait::sInstance);
        return;
    }
    if (MR::isFirstStep(this)) MR::startAction(this, mReaction);
    if (MR::isActionEnd(this)) {
        mReaction = nullptr;
        setNerve(&NrvLightFruit::LightFruitNrvWait::sInstance);
    }
}
void LightFruit::exeHipDropCrash() {
    if (MR::isFirstStep(this)) MR::startAction(this, "HipDropCrash");
    if (MR::isActionEnd(this)) {
        MR::tryHitReactionBreak(this);
        setNerve(&NrvLightFruit::LightFruitNrvHide::sInstance);
    }
}
void LightFruit::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::hideModel(this);
    }
    if (MR::isGreaterStep(this, 200)) {
        MR::setRotationFromQuat(this, mOriginalRotation);
        MR::setScale(this, 1.0f);
        MR::resetPosition(this, mOriginalPosition);
        MR::showModel(this);
        setNerve(&NrvLightFruit::LightFruitNrvAppear::sInstance);
    }
}
bool LightFruit::isEatable() const {
    if (isNerve(&NrvLightFruit::LightFruitNrvEaten::sInstance)
        || isNerve(&NrvLightFruit::LightFruitNrvHide::sInstance)
        || isNerve(&NrvLightFruit::LightFruitNrvHipDropCrash::sInstance)) return false;
    return true;
}
bool LightFruit::isPushable() const {
    if (!isEatable()) return false;
    if (MR::isPlayerElementModeYoshi() && YoshiFunction::tryYoshiPushEat()) return false;
    return true;
}
