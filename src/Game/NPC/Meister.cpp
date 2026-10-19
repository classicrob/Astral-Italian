#pragma auto_inline off
#include "Game/NPC/Meister.hpp"
#include "Game/NPC/MeisterDemoMarioFaceShipChange.hpp"
#include "Game/NPC/MeisterDemoExitItemRoom.hpp"
#include "Game/NPC/MeisterDemoGrandStarReturn.hpp"
#include "Game/NPC/MeisterDemoSimple.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util.hpp"

extern char sMeisterTalkWaitAction[];

namespace NrvMeister {
class MeisterNrvDemo : public Nerve {
public:
    virtual void execute(Spine* spine) const { static_cast<Meister*>(spine->mExecutor)->mDemoState->updateNerve(); }
    static MeisterNrvDemo sInstance;
};
MeisterNrvDemo MeisterNrvDemo::sInstance;
NEW_NERVE(MeisterNrvWait, Meister, Wait);
NEW_NERVE(MeisterNrvTalk, Meister, Talk);
class MeisterNrvTalkWait : public Nerve {
public:
    virtual void execute(Spine* spine) const {
        Meister* host=static_cast<Meister*>(spine->mExecutor);
        if(MR::isFirstStep(host)) MR::startAction(host,sMeisterTalkWaitAction);
    }
    static MeisterNrvTalkWait sInstance;
};
MeisterNrvTalkWait MeisterNrvTalkWait::sInstance;
NEW_NERVE(MeisterNrvReaction, Meister, Reaction);
}
TVec3f gMeisterTalkOffset(0.0f,250.0f,0.0f);
Meister::Meister(const char* name) : NPCActor(name), mWasHitByStarPiece(0), mDemoState(0) {}


const TVec3f& Meister::getTalkOffset() const { return gMeisterTalkOffset; }


void Meister::refreshEventTalk() { mTalkControl->removeInvalidTalk(); }


// NONMATCHING: remaining register allocation and string-base instruction scheduling differ.
void Meister::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("Meister");
    caps.setDefault();
    if (MR::isExistMessageId(iter)) caps.mMessage = true;
    else caps.mMessage = false;
    caps.mWaitNerve = &NrvMeister::MeisterNrvWait::sInstance;
    caps.mTalkNerve = &NrvMeister::MeisterNrvTalk::sInstance;
    mParam._14 = "Wait";
    mParam._18 = "Wait";
    mParam._1C = "TalkA";
    mParam._20 = "TalkA";
    mParam._8 = 0.7f;
    _130 = cNPCSpinDefault;
    _134 = cNPCTrampledDefault;
    _138 = cNPCPointingDefault;
    _13C = "Spin";
    initialize(iter, caps, caps._0, 0, "MeisterAnim");
    MR::onCalcGravity(this);
    MR::tryRegisterDemoCast(this, "ハラペコスターピースチコ紹介", iter);
    MR::tryRegisterDemoCast(this, "マリオ顔惑星変化", iter);
    MR::tryRegisterDemoCast(this, "グランドスター帰還後（ワールド１）", iter);
    MR::tryRegisterDemoCast(this, "グランドスター帰還後（ワールド２）", iter);
    MR::tryRegisterDemoCast(this, "グランドスター帰還後（ワールド３）", iter);
    MR::tryRegisterDemoCast(this, "グランドスター帰還後（ワールド４）", iter);
    MR::tryRegisterDemoCast(this, "グランドスター帰還後（ワールド５）", iter);
    MR::tryRegisterDemoCast(this, "ノーマルエンディング後デモ", iter);
    MR::tryRegisterDemoCast(this, "スター120個エンディング後デモ", iter);
    MR::tryRegisterDemoCast(this, "アイテム部屋からの退室", iter);
    MR::tryRegisterDemoCast(this, "スター１個目直後のゲームスタート", iter);
    MR::tryRegisterDemoCast(this, "パワースター帰還[初回]", iter);
    MR::tryRegisterDemoCast(this, "ゲームオーバー後のデモ", iter);
    new MeisterDemoSimple(this, iter);
    if (MR::isDemoCast(this, "マリオ顔惑星変化")) initSound(4, "MeisterDemoMarioFaceMetamorphose", 0, TVec3f(0.0f));
    else initSound(4, "Meister", 0, TVec3f(0.0f));
    if (MR::isDemoCast(this, "マリオ顔惑星変化")) new MeisterDemoMarioFaceShipChange(this, iter);
    if (MR::isDemoCast(this, "アイテム部屋からの退室")) new MeisterDemoExitItemRoom(this, iter);
    if (MR::isDemoCast(this, "グランドスター帰還後（ワールド１）") || MR::isDemoCast(this, "グランドスター帰還後（ワールド２）") || MR::isDemoCast(this, "グランドスター帰還後（ワールド３）") || MR::isDemoCast(this, "グランドスター帰還後（ワールド４）") || MR::isDemoCast(this, "グランドスター帰還後（ワールド５）")) new MeisterDemoGrandStarReturn(this, iter);
    MR::registerDemoSimpleCastAll(this);
    if (MR::isStageMarioFaceShip()) {
        bool useSwitch = false;
        MR::getJMapInfoArg0NoInit(iter, &useSwitch);
        if (useSwitch) { MR::registerMeisterToAttentionConversation(this, iter); MR::registerMeisterToNormalConversation(this, iter); }
    }
    mTalkControl = new MeisterTalkHolder(this, iter, gMeisterTalkOffset);
    mMsgCtrl = mTalkControl->getTalkCtrl();
}


void Meister::makeActorAppeared() { NPCActor::makeActorAppeared(); MR::emitEffect(this, "HeadLight"); }


void Meister::makeActorDead() { MR::forceDeleteEffect(this, "HeadLight"); NPCActor::makeActorDead(); }


void Meister::startDemoState(NerveExecutor* state) { mDemoState = state; pushNerve(&NrvMeister::MeisterNrvDemo::sInstance); }


void Meister::startWait() { setNerve(&NrvMeister::MeisterNrvTalkWait::sInstance); }


void Meister::resetPose() { setInitPose(); setNerve(mWaitNerve); }


void Meister::endDemoState() { mDemoState = 0; popNerve(); }


void Meister::exeWait() {
    if (!tryReaction()) {
        updateTalkDistance();
        mMsgCtrl = mTalkControl->getTalkCtrl();
        if (!MR::isDemoActive() && MR::tryTalkNearPlayerAndStartTalkAction(this)) setNerve(mTalkNerve);
    }
}


void Meister::exeTalk() {
    if (!tryReaction()) {
        if (MR::tryChangeTalkActionRandom(this, "TalkA", "TalkB", 0)) MR::startAction(this, mParam._1C);
        mMsgCtrl = mTalkControl->getTalkCtrl();
        if (MR::tryTalkNearPlayerAtEndAndStartMoveTalkAction(this)) { mTalkControl->nextTalk(); setNerve(mWaitNerve); }
    }
}


char sMeisterTalkWaitAction[] = "Talk";

void Meister::exeReaction() {
    if (MR::isFirstStep(this)) MR::startAction(this, "StarPieceReaction");
    MR::tryTalkForce(mTalkControl->getReactionTalk());
    if (MR::isActionEnd(this)) popNerve();
}


void Meister::control() { NPCActor::control(); mWasHitByStarPiece = 0; }


bool Meister::tryReaction() {
    if (mWasHitByStarPiece) { pushNerve(&NrvMeister::MeisterNrvReaction::sInstance); return true; }
    return MR::tryStartReactionAndPushNerve(this, mReactionNerve);
}


bool Meister::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgStarPieceReflect(msg)) return false;
    if (MR::isMsgStarPieceAttack(msg)) { MR::limitedStarPieceHitSound(); mWasHitByStarPiece = 1; return true; }
    return NPCActor::receiveMsgPlayerAttack(msg, sender, receiver);
}


void Meister::updateTalkDistance() {
    TVec3f cross;
    TVec3f dir;
    TVec3f up;
    TVec3f front;
    _A0.getZDir(front);
    _A0.getYDir(up);
    MR::normalizeOrZero(&dir, *MR::getPlayerPos() - mPosition);
    PSVECCrossProduct(&front, &dir, &cross);
    mParam._8 = MR::getInterpolateValue(MR::abs(cross.dot(up)), 0.7f, 2.0f);
}


Meister::~Meister() {}
