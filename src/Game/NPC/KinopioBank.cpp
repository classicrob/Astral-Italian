// NONMATCHING: Remaining nonmatches are register allocation, stack layout, literal addressing, and equivalent comparison code generation.
#include "Game/NPC/KinopioBank.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Screen/StarPieceBank.hpp"
#include "Game/System/MarioFacePlanetSwitch.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util.hpp"




namespace NrvKinopioBank {
    class WaitSelect : public Nerve {
    public:
        virtual void execute(Spine* spine) const;
        static WaitSelect sInstance;
    };
    WaitSelect WaitSelect::sInstance;
    NEW_NERVE(StarPieceWait, KinopioBank, StarPieceWait);
    class TradeEnd : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TradeEnd sInstance;
    };
    TradeEnd TradeEnd::sInstance;
    NEW_NERVE(CountUp, KinopioBank, CountUp);
    NEW_NERVE(End, KinopioBank, End);
    class Complete : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static Complete sInstance;
    };
    Complete Complete::sInstance;
    void WaitSelect::execute(Spine* spine) const {
        KinopioBank* actor = static_cast<KinopioBank*>(spine->mExecutor);
        if (MR::isDeadYesNoLayout()) actor->setNerve(&StarPieceWait::sInstance);
    }
}

namespace {
    const s32 sLifeThresholds[] = {200,500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
    const s32 sGoodsThresholds[] = {0,1000,2000,4000,6000,8000};
    struct BankActions { const char* wait; const char* turn; const char* talk; const char* spin; const char* reaction; };
    const BankActions sActions[] = {
        {"WaitStarPiece","TurnStarPiece","TalkStarPiece","SpinStarPiece","ReactionStarPiece"},
        {"Wait","Turn","Talk","Spin","Reaction"},
        {"WaitWeapon","TurnWeapon","TalkWeapon","SpinWeapon","ReactionWeapon"},
        {"WaitPickel","TurnPickel","TalkPickel","SpinPickel","ReactionPickel"},
        {"Wait","Turn","Talk","Spin","Reaction"},
        {"Wait","Turn","Talk","Spin","Reaction"}
    };
}

KinopioBank::KinopioBank(const char* name) : NPCActor(name), mStarPieceBank(nullptr), mBankIndex(0), mChildObj(nullptr), mGoodsA(nullptr), mGoodsB(nullptr), mIsFirstTime(false) {}

void KinopioBank::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("Kinopio");
    caps.setDefault();
    caps.mMessage = false;
    initialize(iter,caps,"Kinopio",nullptr,nullptr);
    MR::setScale(this,1.2f);
    MR::invalidateHitSensor(this,"Pickel");
    MR::offBind(this);
    mParam._14=cNPCWaitDefault; mParam._18=cNPCTurnDefault; mParam._1C="Talk"; mParam._20="Talk";
    setTalkAction("Walk");
    mSpinAction=cNPCSpinDefault; mTrampledAction=cNPCTrampledDefault; mPointingAction=cNPCPointingDefault; mReactionAction=cNPCReactionDefault;
    mSpinAttackRange=450.0f;
    initTalkCtrlDirect(iter,"KinopioBank",TVec3f(0.0f,150.0f,0.0f),nullptr);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&KinopioBank::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&KinopioBank::eventFunc));
    mStarPieceBank=new StarPieceBank("銀行レイアウト");
    s32 count=GameDataFunction::getMaxBankedStarPieceNum();
    s32 i;
    s32 bankIndex=0;
    for(i=0;i<11;i++) if(sLifeThresholds[i]<=count) bankIndex=i+1;
    mBankIndex=bankIndex;
    MR::startBrk(this,"ColorChange");
    MR::setBrkFrameAndStop(this,1.0f);
    createGoodsTables();
    for(s32 i=0;i<MR::getChildObjNum(iter);i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        if(MR::isEqualString(name,"Kinopio")) { mChildObj=static_cast<LiveActor*>(NameObjFactory::initChildObj(iter,i)); break; }
    }
    if(!MarioFacePlanetSwitch::checkMarkerInScene()) {
        mChildObj->makeActorAppeared();
        makeActorDead();
        return;
    }
    MR::tryRegisterDemoCast(this,"ファイルセレクトからの開始",iter);
    mIsFirstTime=!MR::isOnGameEventFlagKinopioBankAtFirst();
    mChildObj->makeActorDead();
    updateBank(1);
    makeActorAppeared();
}

void KinopioBank::control() {
    NPCActor::control();
    if(mTrampledStart) { MR::startActionSound(this,"SmNpcTrampled",-1,-1,-1); MR::startActionSound(this,"SvKinopioTrampled",-1,-1,-1); }
    if(isPointingSe()) { MR::startDPDHitSound(); MR::startActionSound(this,"SvKinopioPointing",-1,-1,-1); }
    if(mReactionStart) MR::startActionSound(this,"SvKinopioStarPieceHit",-1,-1,-1);
}

void KinopioBank::exeStarPieceWait() {
    if(MR::isFirstStep(this)) mStarPieceBank->appear();
    if(MR::isDead(mStarPieceBank)) setNerve(&NrvKinopioBank::TradeEnd::sInstance);
}
void KinopioBank::exeCountUp() {
    if(MR::isFirstStep(this)) awardBankLives();
    if(MR::isStep(this,90)) {
        if(needsGoodsUpdate()) setNerve(&NrvKinopioBank::End::sInstance);
        else setNerve(&NrvKinopioBank::Complete::sInstance);
    }
}
void KinopioBank::exeEnd() {
    if(MR::isFirstStep(this)) updateBank(0);
    if(MR::isStep(this,90)) setNerve(&NrvKinopioBank::Complete::sInstance);
}
void KinopioBank::createGoodsTables() {
    mGoodsA=new PartsModel*[6]; mGoodsB=new PartsModel*[6];
    for(s32 i=0;i<6;i++) {
        NPCActorItem item("KinopioBank");
        MR::getNPCItemData(&item,i);
        mGoodsA[i]=MR::createNPCGoods(this,item.mGoods0,item.mGoodsJoint0);
        mGoodsB[i]=MR::createNPCGoods(this,item.mGoods1,item.mGoodsJoint1);
        PartsModel* goods=mGoodsA[i];
        if(goods) { MR::registerDemoSimpleCastAll(goods); goods->makeActorDead(); }
        PartsModel* goodsB=mGoodsB[i];
        if(goodsB) { MR::registerDemoSimpleCastAll(goodsB); goodsB->makeActorDead(); }
    }
}
u32 KinopioBank::getBankData(PartsModel** a,PartsModel** b) {
    if(mIsFirstTime) { PartsModel** goodsA=mGoodsA; PartsModel** goodsB=mGoodsB; *a=goodsA[0]; *b=goodsB[0]; return 0; }
    u32 index=0;
    for(u32 i=0;i<6;i++) if(sGoodsThresholds[i]<=GameDataFunction::getBankedStarPieceNum()) { index=i; PartsModel** goodsA=mGoodsA; PartsModel** goodsB=mGoodsB; *a=goodsA[i]; *b=goodsB[i]; }
    return index;
}
void KinopioBank::updateBank(s32 immediate) {
    PartsModel* a=nullptr; PartsModel* b=nullptr;
    u32 index=getBankData(&a,&b);
    if(mGoods0==a && mGoods1==b) return;
    if(!immediate) { MR::emitEffect(this,"Smoke"); MR::startActionSound(this,"AppearGoodsSmoke",-1,-1,-1); }
    if(mGoods0) mGoods0->makeActorDead();
    if(mGoods1) mGoods1->makeActorDead();
    mGoods0=a; mGoods1=b;
    if(mGoods0) mGoods0->makeActorAppeared();
    if(mGoods1) mGoods1->makeActorAppeared();
    const BankActions& actions=sActions[index];
    mParam.setMoveAction(actions.wait, actions.turn); mParam.setTalkAction(actions.talk, actions.turn);
    mSpinAction=actions.spin; mReactionAction=actions.reaction; mTrampledAction=actions.reaction; mPointingAction=actions.turn;
    if(index==0) MR::emitEffect(mGoods0,"KinopioGoodsStarPieceLight");
    MR::startAction(this,mParam._1C);
}
bool KinopioBank::branchFunc(u32 id) {
    if(id==0) return !MR::isOnGameEventFlagKinopioBankAtFirst();
    if(id==1) return GameDataFunction::getBankInterest()>0;
    if(id==2) return GameSequenceFunction::isFaceShipSkipBankInfo();
    if(id==3) return sLifeThresholds[mBankIndex]<=GameDataFunction::getMaxBankedStarPieceNum();
    if(id>=10000) {
        if(mIsFirstTime) return true;
        u32 index=id-9999;
        s32 threshold;
        if(index>=6) threshold=9999;
        else threshold=sGoodsThresholds[index];
        if(threshold>=9999) return GameDataFunction::getBankedStarPieceNum()<9999;
        return GameDataFunction::getBankedStarPieceNum()<threshold;
    }
    return true;
}
bool KinopioBank::eventFunc(u32 id) {
    if(id==0) {
        s32 interest=MR::isOnGameEventFlagKinopioBankAtFirst()?GameDataFunction::getBankInterest():100;
        GameDataFunction::setBankedStarPieceNum(GameDataFunction::getBankedStarPieceNum()+interest);
        GameDataFunction::clearKinopioBankInterest();
        TalkMessageCtrl* ctrl = mMsgCtrl;
        MR::setMessageArg(ctrl,GameDataFunction::getBankedStarPieceNum());
        if(!MR::isOnGameEventFlagKinopioBankAtFirst()) MR::onGameEventFlagKinopioBankAtFirst();
        return true;
    }
    if(id==1) {
        if(isNerve(&NrvKinopioBank::TradeEnd::sInstance)) { setNerve(mTalkNerve); return true; }
        if(!isNerve(&NrvKinopioBank::StarPieceWait::sInstance) && !isNerve(&NrvKinopioBank::WaitSelect::sInstance)) { setNerve(&NrvKinopioBank::WaitSelect::sInstance); return false; }
        return false;
    }
    if(id==2) { TalkMessageCtrl* ctrl = mMsgCtrl; MR::setMessageArg(ctrl,GameDataFunction::getBankInterest()); return true; }
    if(id==3) { GameSequenceFunction::setFaceShipSkipBankInfo(); return true; }
    if(id==4) {
        if(!isNerve(&NrvKinopioBank::CountUp::sInstance) && !isNerve(&NrvKinopioBank::Complete::sInstance) && !isNerve(&NrvKinopioBank::End::sInstance)) {
            bool canCountUp = GameDataFunction::getMaxBankedStarPieceNum()>=sLifeThresholds[mBankIndex];
            if(canCountUp) setNerve(&NrvKinopioBank::CountUp::sInstance);
            else if(needsGoodsUpdate()) setNerve(&NrvKinopioBank::End::sInstance);
            else setNerve(&NrvKinopioBank::Complete::sInstance);
            return false;
        }
        if(isNerve(&NrvKinopioBank::Complete::sInstance)) { setNerve(mTalkNerve); return true; }
        return false;
    }
    return true;
}
void KinopioBank::awardBankLives() {
    s32 lives=0;
    for(u32 i=mBankIndex;i<11;i++) {
        if(GameDataFunction::getMaxBankedStarPieceNum()<sLifeThresholds[i]) break;
        mBankIndex=i+1;
        lives++;
    }
    MR::addPlayerLeft(lives);
}
bool KinopioBank::needsGoodsUpdate() {
    PartsModel* a=nullptr; PartsModel* b=nullptr;
    getBankData(&a,&b);
    return mGoods0!=a || mGoods1!=b;
}
