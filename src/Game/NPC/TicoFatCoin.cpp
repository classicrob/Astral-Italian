#pragma auto_inline off
#include "Game/NPC/TicoFatCoin.hpp"
#include "Game/NPC/TicoStateEatCoin.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
extern "C" {
extern char lbl_806BE390[], lbl_806BE410[], lbl_806BE434[], lbl_806BE43C[], lbl_806BE498[], lbl_806BE4B0[];
extern char lbl_806B7AEC[], lbl_806B7AE0[], lbl_806B7AD4[], lbl_806B7AC8[];
extern const char lbl_807CFF48[4];
extern const s32 lbl_807CFE90, lbl_807CFE94, lbl_807CFE98, lbl_807CFE9C;
extern s32 lbl_807D4C20,lbl_807D4C24,lbl_807D4C28,lbl_807D4C2C,lbl_807D4C30,lbl_807D4C34,lbl_807D4C38,lbl_807D4C3C,lbl_807D4C40,lbl_807D4C48;
}
#define RAW(T, o) (*(T*)((u8*)this+(o)))
#define COIN_NERVE(label) reinterpret_cast<Nerve*>(&label)
TicoFatCoin::TicoFatCoin(const char* name) : NPCActor(name), mRequiredCoins(5), mCameraInfo(0), mEatState(0) { mEffectMtx.identity(); mSmokeMtx.identity(); }
TicoFatCoin::~TicoFatCoin() {}
// NONMATCHING: register allocation and member-pointer table addressing differ.
void TicoFatCoin::init(const JMapInfoIter& iter) {
    const char* strings = lbl_806BE390;
    NPCActorCaps caps(strings + 0x18);
    caps.setDefault();
    caps.mMessage = false;
    caps.mNerveStep = 1;
    caps.mSwitchDead = false;
    initialize(iter, caps, strings+0x18, 0, 0);
    RAW(const char*,0x100)=strings+0x24;
    RAW(const char*,0x104)=strings+0x24;
    RAW(const char*,0x108)=strings+0x2C;
    RAW(const char*,0x10C)=strings+0x2C;
    RAW(const char*,0x134)=lbl_806B7AEC;
    RAW(const char*,0x138)=lbl_806B7AE0;
    RAW(const char*,0x13C)=lbl_806B7AD4;
    RAW(const char*,0x140)=lbl_806B7AC8;
    RAW(f32,0x130)=1000.0f;
    RAW(Nerve*,0x158)=COIN_NERVE(lbl_807D4C20);
    MR::getJMapInfoArg1NoInit(iter,&mRequiredCoins);
    mEatState = new TicoStateEatCoin(this,mRequiredCoins);
    MR::initActorState(this,mEatState,COIN_NERVE(lbl_807D4C24),strings+0x38);
    NPCActorItem item;
    item.mActor=strings+0x18;
    item.mGoods0=reinterpret_cast<const char*>(&lbl_807CFE9C);
    item.mGoodsJoint0=reinterpret_cast<const char*>(&lbl_807CFE98);
    item.mGoods1=reinterpret_cast<const char*>(&lbl_807CFE94);
    item.mGoodsJoint1=reinterpret_cast<const char*>(&lbl_807CFE90);
    MR::getNPCItemData(&item,0);
    equipment(item,false);
    MR::startAction(RAW(LiveActor*,0x98),strings+0x40);
    MR::startAction(RAW(LiveActor*,0x9C),strings+0x54);
    initTalkCtrlDirect(iter,strings+0x68,TVec3f(0.0f,150.0f,0.0f),0);
    MR::registerBranchFunc(getMsgCtrl(),TalkMessageFunc(this,&TicoFatCoin::branchFunc));
    MR::registerEventFunc(getMsgCtrl(),TalkMessageFunc(this,&TicoFatCoin::eventFunc));
    MR::setMessageArg(getMsgCtrl(),mRequiredCoins);
    MR::initMultiActorCamera(this,iter,&mCameraInfo,strings+0x80);
    MR::startBrk(this,strings+0x88);
    MR::startBva(this,strings+0x90);
    MR::joinToGroupArray(this,iter,strings+0x98,32);
    bool useTalkCamera=false;
    MR::getJMapInfoArg7NoInit(iter,&useTalkCamera);
    if(useTalkCamera) MR::initTalkCamera(getMsgCtrl(),iter);
    makeActorAppeared();
}
void TicoFatCoin::control() { NPCActor::control(); }
void TicoFatCoin::exeWait() {
    if(!MR::tryStartReactionAndPushNerve(this,RAW(Nerve*,0x15C)) && MR::tryTalkNearPlayerAtEndAndStartTalkAction(this)) setNerve(RAW(Nerve*,0x154));
}
void TicoFatCoin::exeMetamorphosis() {
    if(MR::isFirstStep(this)) MR::startAction(this,lbl_806BE434);
    if(MR::isStep(this,90)) { RAW(LiveActor*,0x98)->kill(); RAW(LiveActor*,0x9C)->kill(); }
    if(MR::isActionEnd(this)) {
        if(MR::isExistRail(this)) { setNerve(COIN_NERVE(lbl_807D4C2C)); return; }
        setNerve(COIN_NERVE(lbl_807D4C30));
    }
}
void TicoFatCoin::exeMove() {
    if(MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this,0);
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateShadowAll(this);
        MR::startAction(this,lbl_807CFF48);
        MR::startActionSound(this,lbl_807CFF48,-1,-1,-1);
        MR::startMultiActorCameraTargetSelf(this,mCameraInfo,lbl_806BE410,-1);
    }
    MR::startActionSound(this,lbl_806BE43C,-1,-1,-1);
    MR::setRailCoord(this,MR::calcNerveEaseOutValue(this,90,0.0f,MR::getRailTotalLength(this)));
    MR::moveTransToCurrentRailPos(this);
    if(MR::isStep(this,90)) setNerve(COIN_NERVE(lbl_807D4C30));
}
// NONMATCHING: temporary vector stack layout and register allocation differ.
void TicoFatCoin::exeTransform() {
    const char* strings=lbl_806BE390;
    if(MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(60);
        MR::tryRumblePadVeryStrong(this,0);
        MR::shakeCameraNormal();
        MR::startActionSound(this,strings+0xB8,-1,-1,-1);
        MR::emitEffect(this,strings+0xD0)->setHostMtx(mEffectMtx.toMtxPtr());
        MR::emitEffect(this,strings+0xE0)->setHostMtx(mSmokeMtx.toMtxPtr());
    }
    MR::startActionSound(this,strings+0xAC,-1,-1,-1);
    TVec3f dir(mPosition-MR::getCamPos());
    MR::normalizeOrZero(&dir);
    TVec3f position(MR::getCamPos()+dir*500.0f);
    MR::makeMtxFrontUpPos(&mEffectMtx,-MR::getCamZdir(),MR::getCamYdir(),position);
    MR::makeMtxFrontUpPos(&mSmokeMtx,-MR::getCamZdir(),MR::getCamYdir(),mPosition);
    if(!MR::isWipeActive()) {
        MR::stopBck(this);
        MR::emitEffect(this,strings+0xF8)->setHostMtx(mEffectMtx.toMtxPtr());
        MR::deleteEffect(this,strings+0xD0);
        setNerve(COIN_NERVE(lbl_807D4C34));
    }
}
void TicoFatCoin::exeTransformEnd() {
    if(MR::isFirstStep(this)) MR::startActionSound(this,lbl_806BE498,-1,-1,-1);
    MR::startActionSound(this,lbl_806BE43C,-1,-1,-1);
    if(MR::isStep(this,90)) setNerve(COIN_NERVE(lbl_807D4C38));
}
void TicoFatCoin::exeWipe() {
    if(MR::isFirstStep(this)) { MR::onSwitchA(this); MR::openWipeWhiteFade(60); }
    if(MR::isLessStep(this,46)) MR::startActionSound(this,lbl_806BE43C,-1,-1,-1);
    if(!MR::isWipeActive()) setNerve(COIN_NERVE(lbl_807D4C3C));
}
void TicoFatCoin::exeInformation() {
    if(MR::isFirstStep(this)) { MR::startAppearPlanetJingle(); MR::appearInformationMessage(MR::getGameMessageDirect(lbl_806BE4B0),true); }
    if(MR::isDeadInformationMessage()) setNerve(COIN_NERVE(lbl_807D4C40));
}
void TicoFatCoin::exeEnd() {
    if(MR::isFirstStep(this)) MR::endMultiActorCamera(this,mCameraInfo,lbl_806BE410,false,-1);
    if(MR::isStep(this,60)) { appearPlanet(true); setNerve(COIN_NERVE(lbl_807D4C48)); }
}
void TicoFatCoin::initAfterPlacement() {
    if(GameDataFunction::isTicoCoinFeed(MR::getCurrentStageName())) {
        MR::callMakeActorAppearedAllGroupMember(this);
        MR::onSwitchA(this);
        makeActorDead();
    }
}
bool TicoFatCoin::branchFunc(u32) { return mRequiredCoins<=MR::getCoinNum(); }
bool TicoFatCoin::eventFunc(u32 event) {
    if(event==0) { MR::forceAppearCoinCounterForTicoFat(); return true; }
    if(event==1) { MR::disappearCoinCounterForTicoFat(); return true; }
    if(event==2) {
        if(isNerve(COIN_NERVE(lbl_807D4C20))) { setNerve(COIN_NERVE(lbl_807D4C24)); return false; }
        if(isNerve(COIN_NERVE(lbl_807D4C24)) && mEatState->isComplete()) { MR::forceSyncCoinCounter(); MR::disappearCoinCounterForTicoFat(); return true; }
        return false;
    }
    if(event==3) {
        if(isNerve(COIN_NERVE(lbl_807D4C24))) { setNerve(COIN_NERVE(lbl_807D4C28)); return false; }
        if(isNerve(COIN_NERVE(lbl_807D4C48))) { kill(); return true; }
        return false;
    }
    return true;
}
void TicoFatCoin::appearPlanet(bool animate) {
    if(animate) {
        if(MR::isProductGameScene()) GameDataFunction::onTicoCoinFeed(MR::getCurrentStageName());
        MR::callAppearAllGroupMember(this);
    } else { MR::callMakeActorAppearedAllGroupMember(this); MR::onSwitchA(this); }
}
