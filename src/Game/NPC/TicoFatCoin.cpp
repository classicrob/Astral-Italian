#pragma auto_inline off
#include "Game/NPC/TicoFatCoin.hpp"
#include "Game/NPC/TicoStateEatCoin.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/PartsModel.hpp"

char sTicoFatCoinFlyAction[4] = "Fly";

namespace NrvTicoFatCoin {
    NEW_NERVE(TicoFatCoinNrvWait, TicoFatCoin, Wait);
    class TicoFatCoinNrvEat : public Nerve { public: virtual void execute(Spine*) const; static TicoFatCoinNrvEat sInstance; };
    TicoFatCoinNrvEat TicoFatCoinNrvEat::sInstance;
    NEW_NERVE(TicoFatCoinNrvMetamorphosis, TicoFatCoin, Metamorphosis);
    NEW_NERVE(TicoFatCoinNrvMove, TicoFatCoin, Move);
    NEW_NERVE(TicoFatCoinNrvTransform, TicoFatCoin, Transform);
    NEW_NERVE(TicoFatCoinNrvTransformEnd, TicoFatCoin, TransformEnd);
    NEW_NERVE(TicoFatCoinNrvWipe, TicoFatCoin, Wipe);
    NEW_NERVE(TicoFatCoinNrvInformation, TicoFatCoin, Information);
    class TicoFatCoinNrvDelayEnd : public Nerve { public: virtual void execute(Spine*) const; static TicoFatCoinNrvDelayEnd sInstance; };
    TicoFatCoinNrvDelayEnd TicoFatCoinNrvDelayEnd::sInstance;
    NEW_NERVE(TicoFatCoinNrvEnd, TicoFatCoin, End);
    class TicoFatCoinNrvDone : public Nerve { public: virtual void execute(Spine*) const; static TicoFatCoinNrvDone sInstance; };
    TicoFatCoinNrvDone TicoFatCoinNrvDone::sInstance;
}

TicoFatCoin::TicoFatCoin(const char* name) : NPCActor(name), mRequiredCoins(5), mCameraInfo(0), mEatState(0) { mEffectMtx.identity(); mSmokeMtx.identity(); }
TicoFatCoin::~TicoFatCoin() {}
void TicoFatCoin::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("TicoFatCoin");
    caps.setDefault();
    caps.mMessage = false;
    caps.mNerveStep = 1;
    caps.mSwitchDead = false;
    initialize(iter, caps, "TicoFatCoin", 0, 0);
    const char* waitAction = "Wait";
    const char* talkAction = "Talk";
    mParam._14=waitAction;
    mParam._18=waitAction;
    mParam._1C=talkAction;
    mParam._20=talkAction;
    mSpinAction=cNPCSpinDefault;
    mTrampledAction=cNPCTrampledDefault;
    mPointingAction=cNPCPointingDefault;
    mReactionAction=cNPCReactionDefault;
    mSpinAttackRange=1000.0f;
    mTalkNerve=GET_NERVE(TicoFatCoin, TicoFatCoinNrvWait);
    MR::getJMapInfoArg1NoInit(iter,&mRequiredCoins);
    mEatState = new TicoStateEatCoin(this,mRequiredCoins);
    MR::initActorState(this,mEatState,GET_NERVE(TicoFatCoin, TicoFatCoinNrvEat),"EatDemo");
    NPCActorItem item("TicoFatCoin");
    MR::getNPCItemData(&item,0);
    equipment(item,false);
    MR::startAction(mGoods0,"TicoFatGoodsCoinL");
    MR::startAction(mGoods1,"TicoFatGoodsCoinR");
    initTalkCtrlDirect(iter,"TicoFatCoin_Request000",TVec3f(0.0f,150.0f,0.0f),0);
    MR::registerBranchFunc(getMsgCtrl(),TalkMessageFunc(this,&TicoFatCoin::branchFunc));
    MR::registerEventFunc(getMsgCtrl(),TalkMessageFunc(this,&TicoFatCoin::eventFunc));
    MR::setMessageArg(getMsgCtrl(),mRequiredCoins);
    MR::initMultiActorCamera(this,iter,&mCameraInfo,"飛行");
    MR::startBrk(this,"Normal");
    MR::startBva(this,"Small0");
    MR::joinToGroupArray(this,iter,"惑星出現",32);
    bool useTalkCamera=false;
    MR::getJMapInfoArg7NoInit(iter,&useTalkCamera);
    if(useTalkCamera) MR::initTalkCamera(getMsgCtrl(),iter);
    makeActorAppeared();
}
void TicoFatCoin::control() { NPCActor::control(); }
void TicoFatCoin::exeWait() {
    if(!MR::tryStartReactionAndPushNerve(this,mReactionNerve) && MR::tryTalkNearPlayerAtEndAndStartTalkAction(this)) setNerve(mWaitNerve);
}
void TicoFatCoin::exeMetamorphosis() {
    if(MR::isFirstStep(this)) MR::startAction(this,"Demo");
    if(MR::isStep(this,90)) { mGoods0->kill(); mGoods1->kill(); }
    if(MR::isActionEnd(this)) {
        if(MR::isExistRail(this)) { setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvMove)); return; }
        setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvTransform));
    }
}
void TicoFatCoin::exeMove() {
    if(MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this,0);
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateShadowAll(this);
        MR::startAction(this,sTicoFatCoinFlyAction);
        MR::startActionSound(this,sTicoFatCoinFlyAction,-1,-1,-1);
        MR::startMultiActorCameraTargetSelf(this,mCameraInfo,"飛行",-1);
    }
    MR::startActionSound(this,"VolDownBgm",-1,-1,-1);
    MR::setRailCoord(this,MR::calcNerveEaseOutValue(this,90,0.0f,MR::getRailTotalLength(this)));
    MR::moveTransToCurrentRailPos(this);
    if(MR::isStep(this,90)) setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvTransform));
}
// NONMATCHING: temporary vector stack layout and register allocation differ.
namespace {
    inline const TVec3f& getTicoCoinPosition(const LiveActor& actor) {
        return actor.mPosition;
    }
}

void TicoFatCoin::exeTransform() {
    if(MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(60);
        MR::tryRumblePadVeryStrong(this,0);
        MR::shakeCameraNormal();
        MR::startActionSound(this,"DmTicofatMorphWipeOut",-1,-1,-1);
        MR::emitEffect(this,"ScreenEffect")->setHostMtx(mEffectMtx.toMtxPtr());
        MR::emitEffect(this,"ScreenEffectLight")->setHostMtx(mSmokeMtx.toMtxPtr());
    }
    MR::startActionSound(this,"VolDownBgm",-1,-1,-1);
    TVec3f dir(getTicoCoinPosition(*this)-MR::getCamPos());
    MR::normalizeOrZero(&dir);
    MR::makeMtxFrontUpPos(&mEffectMtx,-MR::getCamZdir(),MR::getCamYdir(),MR::getCamPos()+dir*500.0f);
    MR::makeMtxFrontUpPos(&mSmokeMtx,-MR::getCamZdir(),MR::getCamYdir(),mPosition);
    if(!MR::isWipeActive()) {
        MR::stopBck(this);
        MR::emitEffect(this,"ScreenEffectFog")->setHostMtx(mEffectMtx.toMtxPtr());
        MR::deleteEffect(this,"ScreenEffect");
        setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvTransformEnd));
    }
}
void TicoFatCoin::exeTransformEnd() {
    if(MR::isFirstStep(this)) MR::startActionSound(this,"DmTicofatMorphWipeIn",-1,-1,-1);
    MR::startActionSound(this,"VolDownBgm",-1,-1,-1);
    if(MR::isStep(this,90)) setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvWipe));
}
void TicoFatCoin::exeWipe() {
    if(MR::isFirstStep(this)) { MR::onSwitchA(this); MR::openWipeWhiteFade(60); }
    if(MR::isLessStep(this,46)) MR::startActionSound(this,"VolDownBgm",-1,-1,-1);
    if(!MR::isWipeActive()) setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvInformation));
}
void TicoFatCoin::exeInformation() {
    if(MR::isFirstStep(this)) { MR::startAppearPlanetJingle(); MR::appearInformationMessage(MR::getGameMessageDirect("InformationPlanet"),true); }
    if(MR::isDeadInformationMessage()) setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvDelayEnd));
}
void TicoFatCoin::exeEnd() {
    if(MR::isFirstStep(this)) MR::endMultiActorCamera(this,mCameraInfo,"飛行",false,-1);
    if(MR::isStep(this,60)) { appearPlanet(true); setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvDone)); }
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
        if(isNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvWait))) { setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvEat)); return false; }
        if(isNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvEat)) && mEatState->isComplete()) { MR::forceSyncCoinCounter(); MR::disappearCoinCounterForTicoFat(); return true; }
        return false;
    }
    if(event==3) {
        if(isNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvEat))) { setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvMetamorphosis)); return false; }
        if(isNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvDone))) { kill(); return true; }
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

void NrvTicoFatCoin::TicoFatCoinNrvEat::execute(Spine* spine) const {
    MR::updateActorState(static_cast<TicoFatCoin*>(spine->mExecutor));
}
void NrvTicoFatCoin::TicoFatCoinNrvDone::execute(Spine*) const {}
void NrvTicoFatCoin::TicoFatCoinNrvDelayEnd::execute(Spine* spine) const {
    TicoFatCoin* actor = static_cast<TicoFatCoin*>(spine->mExecutor);
    if (MR::isStep(actor, 20)) actor->setNerve(GET_NERVE(TicoFatCoin, TicoFatCoinNrvEnd));
}
