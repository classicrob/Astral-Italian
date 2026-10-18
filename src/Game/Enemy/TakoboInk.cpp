#include "Game/Util.hpp"
#include "Game/Enemy/TakoboInk.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

namespace NrvTakoboInk {
    NEW_NERVE(TakoboInkNrvWait, TakoboInk, Wait);
};  // namespace NrvTakoboInk

TakoboInk::TakoboInk(const char* pName) : LiveActor(pName) {
}

void TakoboInk::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "TakoHeiInk", false);
    initNerve(&NrvTakoboInk::TakoboInkNrvWait::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void TakoboInk::kill() {
    LiveActor::kill();
    MR::tryHitReactionBreak(this);
    MR::emitEffect(this, "Break");
}

void TakoboInk::control() {}

void TakoboInk::start(const TVec3f& rPos, const TVec3f& rVel, bool isBindOff) {
    MR::setPosition(this, rPos);
    setNerve(&NrvTakoboInk::TakoboInkNrvWait::sInstance);
    appear();
    mVelocity.x = rVel.x;
    mVelocity.y = rVel.y;
    mVelocity.z = rVel.z;
    if (isBindOff) {
        MR::offBind(this);
    } else {
        MR::onBind(this);
    }
}

void TakoboInk::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            kill();
            return;
        }
    }
    if (MR::isSensorYoshi(pReceiver) && MR::sendMsgPush(pReceiver, pSender)) {
        kill();
    }
}

bool TakoboInk::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgInvincibleAttack(msg)) {
        kill();
        MR::tryHitReaction(this, "無敵マリオ破壊");
        return false;
    }
    return false;
}

void TakoboInk::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Wait");
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    MR::addVelocityToGravity(this, 0.35f);
    MR::attenuateVelocity(this, 0.99f);
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        kill();
    }
}

// NONMATCHING: target symbol __ct__15TakoboInkHolderFv has no parameter list yet the body reads r4 (TakoSho passes 2)
TakoboInkHolder::TakoboInkHolder(s32 numMax) : DeriveActorGroup< TakoboInk >("TakoboInkHolder", numMax) {
    for (s32 i = 0; i < numMax; i++) {
        TakoboInk* pInk = new TakoboInk("TakoHeiInk");
        pInk->initWithoutIter();
        registerActor(pInk);
    }
}
