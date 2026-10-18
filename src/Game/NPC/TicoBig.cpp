// NONMATCHING: literal addresses.
#include "Game/NPC/TicoBig.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/Util.hpp"
TicoBig::TicoBig(const char* name) : NPCActor(name) {}
TicoBig::~TicoBig() {}
void TicoBig::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("TicoBig");
    caps.setDefault();
    caps._70="Center";
    caps._7C=true;
    caps._95=false;
    _134="Trampled";
    _130="Spin";
    _12C=1500.0f;
    mParam._14="Wait";
    mParam._1C="Talk";
    mParam._4=5000.0f;
    mParam._8=2.0f;
    initialize(iter,caps,"TicoBig",nullptr,nullptr);
    MR::setDistanceToTalk(mMsgCtrl,500.0f);
    mLodCtrl->setDistanceToMiddleAndLow(4000.0f,6000.0f);
    mLodCtrl->setClippingTypeSphereContainsModelBoundingBox(100.0f);
    makeActorAppeared();
}
void TicoBig::control() {
    NPCActor::control();
    if(MR::changeShowModelFlagSyncNearClipping(this,1000.0f)) MR::onDrawEffect(this);
    else MR::offDrawEffect(this);
    if(_E4) MR::startActionSound(this,"Pointing",-1,-1,-1);
    if(_DB) MR::startActionSound(this,"StarPieceHit",-1,-1,-1);
}
bool TicoBig::receiveMsgPlayerAttack(u32 msg,HitSensor* sender,HitSensor* receiver) {
    if(MR::isSensorMapObj(receiver)) return false;
    return NPCActor::receiveMsgPlayerAttack(msg,sender,receiver);
}
