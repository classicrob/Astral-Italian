#pragma auto_inline off
#include "Game/NPC/Meister.hpp"
#include "Game/NPC/MeisterDemoMarioFaceShipChange.hpp"
#include "Game/NPC/MeisterDemoExitItemRoom.hpp"
#include "Game/NPC/MeisterDemoGrandStarReturn.hpp"
#include "Game/NPC/MeisterDemoSimple.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
extern "C" {
extern char lbl_806BA2A0[], lbl_806BA0D0[], lbl_806BA2AC[], lbl_806BA2BC[], lbl_806BA2B4[];
extern s32 lbl_807D49B8, lbl_807D49C4, lbl_807D49C8;
}
#define RAW(T, o) (*(T*)((u8*)this + (o)))
Meister::Meister(const char* name) : NPCActor(name), _164(0), mDemoState(0) {}
Meister::~Meister() {}
void Meister::makeActorAppeared() { NPCActor::makeActorAppeared(); MR::emitEffect(this, lbl_806BA2A0); }
void Meister::makeActorDead() { MR::forceDeleteEffect(this, lbl_806BA2A0); NPCActor::makeActorDead(); }
void Meister::startDemoState(NerveExecutor* state) { mDemoState = state; pushNerve(reinterpret_cast<Nerve*>(&lbl_807D49B8)); }
void Meister::startWait() { setNerve(reinterpret_cast<Nerve*>(&lbl_807D49C4)); }
void Meister::resetPose() { setInitPose(); setNerve(RAW(Nerve*, 0x154)); }
void Meister::endDemoState() { mDemoState = 0; popNerve(); }
void Meister::exeWait() {
    if (!tryReaction()) {
        updateTalkDistance();
        RAW(TalkMessageCtrl*, 0x94) = mTalkControl->getTalkCtrl();
        if (!MR::isDemoActive() && MR::tryTalkNearPlayerAndStartTalkAction(this)) setNerve(RAW(Nerve*, 0x158));
    }
}
void Meister::exeTalk() {
    if (!tryReaction()) {
        if (MR::tryChangeTalkActionRandom(this, lbl_806BA0D0, lbl_806BA2AC, 0)) MR::startAction(this, RAW(const char*, 0x108));
        RAW(TalkMessageCtrl*, 0x94) = mTalkControl->getTalkCtrl();
        if (MR::tryTalkNearPlayerAtEndAndStartMoveTalkAction(this)) { mTalkControl->nextTalk(); setNerve(RAW(Nerve*, 0x154)); }
    }
}
void Meister::exeReaction() {
    if (MR::isFirstStep(this)) MR::startAction(this, lbl_806BA2BC);
    MR::tryTalkForce(mTalkControl->getReactionTalk());
    if (MR::isActionEnd(this)) popNerve();
}
void Meister::control() { NPCActor::control(); _164 = 0; }
bool Meister::tryReaction() {
    if (_164) { pushNerve(reinterpret_cast<Nerve*>(&lbl_807D49C8)); return true; }
    return MR::tryStartReactionAndPushNerve(this, RAW(Nerve*, 0x15C));
}
bool Meister::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgStarPieceReflect(msg)) return false;
    if (MR::isMsgStarPieceAttack(msg)) { MR::limitedStarPieceHitSound(); _164 = 1; return true; }
    return NPCActor::receiveMsgPlayerAttack(msg, sender, receiver);
}

extern "C" { extern char lbl_806BA0C0[], lbl_806B7AEC[], lbl_806B7AE0[], lbl_806B7AD4[]; extern s32 lbl_807D49BC, lbl_807D49C0;  }
// NONMATCHING: remaining register allocation and string-base instruction scheduling differ.
void Meister::init(const JMapInfoIter& iter) {
    const char* strings = lbl_806BA0C0;
    NPCActorCaps caps(strings);
    caps.setDefault();
    if (MR::isExistMessageId(iter)) caps.mMessage = true;
    else caps.mMessage = false;
    caps.mWaitNerve = reinterpret_cast<Nerve*>(&lbl_807D49BC);
    caps.mTalkNerve = reinterpret_cast<Nerve*>(&lbl_807D49C0);
    RAW(const char*, 0x100) = strings + 8;
    RAW(const char*, 0x104) = strings + 8;
    RAW(const char*, 0x108) = strings + 0x10;
    RAW(const char*, 0x10C) = strings + 0x10;
    RAW(f32, 0xF4) = 0.7f;
    RAW(const char*, 0x134) = lbl_806B7AEC;
    RAW(const char*, 0x138) = lbl_806B7AE0;
    RAW(const char*, 0x13C) = lbl_806B7AD4;
    RAW(const char*, 0x140) = strings + 0x18;
    initialize(iter, caps, caps._0, 0, strings + 0x20);
    MR::onCalcGravity(this);
    MR::tryRegisterDemoCast(this, strings + 0x2C, iter);
    MR::tryRegisterDemoCast(this, strings + 0x4C, iter);
    MR::tryRegisterDemoCast(this, strings + 0x60, iter);
    MR::tryRegisterDemoCast(this, strings + 0x84, iter);
    MR::tryRegisterDemoCast(this, strings + 0xA8, iter);
    MR::tryRegisterDemoCast(this, strings + 0xCC, iter);
    MR::tryRegisterDemoCast(this, strings + 0xF0, iter);
    MR::tryRegisterDemoCast(this, strings + 0x114, iter);
    MR::tryRegisterDemoCast(this, strings + 0x130, iter);
    MR::tryRegisterDemoCast(this, strings + 0x150, iter);
    MR::tryRegisterDemoCast(this, strings + 0x168, iter);
    MR::tryRegisterDemoCast(this, strings + 0x18C, iter);
    MR::tryRegisterDemoCast(this, strings + 0x1A4, iter);
    new MeisterDemoSimple(this, iter);
    if (MR::isDemoCast(this, strings + 0x4C)) initSound(4, strings + 0x1BC, 0, TVec3f(0.0f));
    else initSound(4, strings, 0, TVec3f(0.0f));
    if (MR::isDemoCast(this, strings + 0x4C)) new MeisterDemoMarioFaceShipChange(this, iter);
    if (MR::isDemoCast(this, strings + 0x150)) new MeisterDemoExitItemRoom(this, iter);
    if (MR::isDemoCast(this, strings + 0x60) || MR::isDemoCast(this, strings + 0x84) || MR::isDemoCast(this, strings + 0xA8) || MR::isDemoCast(this, strings + 0xCC) || MR::isDemoCast(this, strings + 0xF0)) new MeisterDemoGrandStarReturn(this, iter);
    MR::registerDemoSimpleCastAll(this);
    if (MR::isStageMarioFaceShip()) {
        bool useSwitch = false;
        MR::getJMapInfoArg0NoInit(iter, &useSwitch);
        if (useSwitch) { MR::registerMeisterToAttentionConversation(this, iter); MR::registerMeisterToNormalConversation(this, iter); }
    }
    mTalkControl = new MeisterTalkHolder(this, iter, gMeisterTalkOffset);
    RAW(TalkMessageCtrl*, 0x94) = mTalkControl->getTalkCtrl();
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
    RAW(f32, 0xF4) = MR::getInterpolateValue(MR::abs(cross.dot(up)), 0.7f, 2.0f);
}

const TVec3f& Meister::getTalkOffset() const { return gMeisterTalkOffset; }
void Meister::refreshEventTalk() { mTalkControl->removeInvalidTalk(); }
