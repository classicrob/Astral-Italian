#pragma auto_inline off
#include "Game/NPC/KoopaNpc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaNpc {
    NEW_NERVE(SwoonWait, KoopaNpc, SwoonWait);
    NEW_NERVE(SwoonWaitEnd, KoopaNpc, SwoonWaitEnd);
    NEW_NERVE(AngryStart, KoopaNpc, AngryStart);
    NEW_NERVE(Angry, KoopaNpc, Angry);
    NEW_NERVE(AngryEnd, KoopaNpc, AngryEnd);
    class Reaction : public Nerve {
    public: virtual void execute(Spine* pSpine) const { MR::tryStartReactionAndPopNerve(static_cast<KoopaNpc*>(pSpine->mExecutor)); }
    static Reaction sInstance;
    };
    Reaction Reaction::sInstance;
}

KoopaNpc::KoopaNpc(const char* name) : NPCActor(name) {}
// NONMATCHING: equivalent string-pointer register allocation differs.
void KoopaNpc::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("KoopaNpc");
    caps.setDefault();
    caps.mMessage = false;
    caps.mReactionNerve = &NrvKoopaNpc::Reaction::sInstance;
    initialize(iter, caps, "DemoKoopa", "KoopaNpc", nullptr);
    const char* pressAction = "Press";
    mTrampledAction = pressAction;
    mPointingAction = pressAction;
    mReactionAction = pressAction;
    mSpinAction = "Reaction";
    mRailMoveAction = "Wait";
    mRailTalkAction = nullptr;
    const char* swoonAction = "SwoonWait";
    mParam._14 = swoonAction;
    mParam._18 = swoonAction;
    mFollowRailOnGround = true;
    mParam._8 = 0.0f;
    appear();
}
void KoopaNpc::appear() {
    setNerve(&NrvKoopaNpc::SwoonWait::sInstance);
    LiveActor::appear();
}
void KoopaNpc::control() {
    if (mTrampledStart) MR::startActionSound(this, "Trampled", -1, -1, -1);
    NPCActor::control();
}
bool KoopaNpc::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvKoopaNpc::SwoonWait::sInstance)) return tryWakeUp(msg, sender, receiver);
    return NPCActor::receiveMsgPlayerAttack(msg, sender, receiver);
}
bool KoopaNpc::receiveMsgPush(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvKoopaNpc::SwoonWait::sInstance)) return tryWakeUp(0, sender, receiver);
    return false;
}
bool KoopaNpc::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvKoopaNpc::SwoonWait::sInstance)) {
        if (MR::isMsgYoshiStartTongue(msg)) tryWakeUp(msg, sender, receiver);
        return false;
    }
    return NPCActor::receiveOtherMsg(msg, sender, receiver);
}
bool KoopaNpc::tryWakeUp(u32 msg, HitSensor* sender, HitSensor*) {
    if (!isNerve(&NrvKoopaNpc::SwoonWait::sInstance)) return false;
    if (MR::isMsgPlayerJump(msg)) return false;
    if (MR::isSensorPlayerOrRide(sender) || MR::isMsgStarPieceAttack(msg) || MR::isMsgSupportTicoSpin(msg)) {
        setNerve(&NrvKoopaNpc::SwoonWaitEnd::sInstance);
        return true;
    }
    return false;
}
void KoopaNpc::exeSwoonWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "SwoonWait");
    bool wake = false;
    if (MR::isPlayerHipDropLand() && MR::isNearPlayer(this, 500.0f)) wake = true;
    if (wake) setNerve(&NrvKoopaNpc::SwoonWaitEnd::sInstance);
}
void KoopaNpc::exeSwoonWaitEnd() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this, 0);
        MR::startAction(this, "SwoonWaitEnd");
    }
    if (MR::isActionEnd(this)) setNerve(&NrvKoopaNpc::AngryStart::sInstance);
}
void KoopaNpc::exeAngryStart() {
    if (MR::tryStartReaction(this)) {
        pushNerve(&NrvKoopaNpc::AngryStart::sInstance);
        setNerve(mReactionNerve);
        return;
    }
    if (MR::isFirstStep(this)) MR::startAction(this, "AngryStart");
    if (MR::isActionEnd(this)) setNerve(&NrvKoopaNpc::Angry::sInstance);
}
void KoopaNpc::exeAngry() {
    if (MR::tryStartReaction(this)) {
        pushNerve(&NrvKoopaNpc::AngryStart::sInstance);
        setNerve(mReactionNerve);
        return;
    }
    turnToPlayer();
    if (MR::isFirstStep(this)) MR::startAction(this, "Angry");
    if (MR::isActionEnd(this)) setNerve(&NrvKoopaNpc::AngryEnd::sInstance);
}
void KoopaNpc::exeAngryEnd() {
    if (MR::tryStartReaction(this)) {
        pushNerve(&NrvKoopaNpc::AngryStart::sInstance);
        setNerve(mReactionNerve);
        return;
    }
    if (MR::isFirstStep(this)) MR::startAction(this, "AngryEnd");
    if (MR::isActionEnd(this)) setNerve(&NrvKoopaNpc::AngryStart::sInstance);
}
KoopaNpc::~KoopaNpc() {}
