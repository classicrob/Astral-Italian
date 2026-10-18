#include "Game/Player/YoshiAnimationModelCtrl.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Util.hpp"
#include <cstdio>
namespace {
const char* sAnimationJoint="PartsControl";
const char* sLeftJoint="HandL0";
const char* sRightJoint="HandR0";
u8 quantizeYoshiHand(f32 value){s32 index=0.49f+value/10.0f;return MR::clamp(index,0,7);}
}
YoshiAnimationModelCtrlBase::YoshiAnimationModelCtrlBase():mActor(nullptr),mLeftModel(nullptr),mRightModel(nullptr),mLeftJoint(nullptr),mRightJoint(nullptr),mPlayer(nullptr),mAnimationTranslation(0.0f,0.0f,0.0f),mAppeared(false){}
YoshiAnimationModelCtrlBase::~YoshiAnimationModelCtrlBase(){}
void YoshiAnimationModelCtrlBase::fn_8040B2A0(Yoshi* actor,XanimePlayer* player,PartsModel* left,PartsModel* right,u8 joints){
    mActor=actor;mLeftModel=left;mRightModel=right;
    mLeftJoint=new YoshiJointModelCtrl(actor,sLeftJoint,mLeftModel,joints);
    mRightJoint=new YoshiJointModelCtrl(actor,sRightJoint,mRightModel,joints);mPlayer=player;
}
void YoshiAnimationModelCtrlBase::calcAnim(){
    mPlayer->getMainAnimationTrans(static_cast<u16>(MR::getJointIndex(mActor,sAnimationJoint)),&mAnimationTranslation);
    mLeftJoint->fn_80386400(quantizeYoshiHand(mAnimationTranslation.y));
    mRightJoint->fn_80386400(quantizeYoshiHand(mAnimationTranslation.z));
}
void YoshiAnimationModelCtrlBase::appear(){mLeftJoint->fn_80386480();mRightJoint->fn_80386480();mAppeared=true;}
void YoshiAnimationModelCtrlBase::kill(){mLeftJoint->fn_80386490();mRightJoint->fn_80386490();mAppeared=false;}
YoshiAnimationModelCtrl::YoshiAnimationModelCtrl(Yoshi* actor,XanimePlayer* player,const char* name):YoshiAnimationModelCtrlBase(){
    char model[128];snprintf(model,sizeof(model),"%sL",name);
    PartsModel* left=new PartsModel(actor,"左手差し替えモデル",model,nullptr,0x16,false);
    left->initWithoutIter();left->initFixedPosition(sLeftJoint);MR::initLightCtrl(left);
    snprintf(model,sizeof(model),"%sR",name);
    PartsModel* right=new PartsModel(actor,"右手差し替えモデル",model,nullptr,0x16,false);
    right->initWithoutIter();right->initFixedPosition(sRightJoint);MR::initLightCtrl(right);
    fn_8040B2A0(actor,player,left,right,MR::getJointNum(left)-1);
}
YoshiAnimationModelCtrl::~YoshiAnimationModelCtrl(){}
void YoshiAnimationModelCtrl::appear(){mLeftModel->makeActorAppeared();mRightModel->makeActorAppeared();YoshiAnimationModelCtrlBase::appear();}
void YoshiAnimationModelCtrl::kill(){YoshiAnimationModelCtrlBase::kill();mLeftModel->makeActorDead();mRightModel->makeActorDead();}
void YoshiAnimationModelCtrl::fn_8040B6D0(const char* animation){MR::startBrkIfExist(mLeftModel,animation);MR::startBrkIfExist(mRightModel,animation);}
