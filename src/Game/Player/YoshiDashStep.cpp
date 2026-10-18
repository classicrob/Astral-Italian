#include "Game/Player/YoshiDashStep.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util.hpp"
YoshiDashStep::YoshiDashStep(const char* name):LiveActor(name),mCollision(nullptr){mMatrix.identity();}
YoshiDashStep::~YoshiDashStep(){}
void YoshiDashStep::init(const JMapInfoIter&){
    MR::makeMtxTR(mMatrix.toMtxPtr(),this);MR::connectToSceneMapObjMovement(this);
    initHitSensor(1);HitSensor* sensor=MR::addBodyMessageSensorMapObj(this);
    ResourceHolder* resource=MR::createAndAddResourceHolder("DashYoshiStep.arc");
    mCollision=MR::createDashYoshiStepCollisionPartsFromResourceHolder(resource,"DashYoshiStep",sensor,mMatrix.toMtxPtr(),MR::CollisionScaleType_Default);
    MR::invalidateCollisionParts(mCollision);MR::invalidateClipping(this);makeActorDead();
}
void YoshiDashStep::makeActorAppeared(){MR::validateCollisionParts(mCollision);mCollision->setMtx();LiveActor::makeActorAppeared();}
void YoshiDashStep::makeActorDead(){MR::invalidateCollisionParts(mCollision);LiveActor::makeActorDead();}
void YoshiDashStep::fn_80415B90(MtxPtr matrix){mMatrix.set(matrix);}
void YoshiDashStep::control(){mCollision->setMtx();}
MtxPtr YoshiDashStep::getBaseMtx()const{return const_cast<YoshiDashStep*>(this)->mMatrix.toMtxPtr();}
YoshiHistory15C50::YoshiHistory15C50():mEntries(nullptr),mIndex(0){mEntries=new YoshiDashStep[4];for(s32 i=0;i<4;++i)mEntries[i].initWithoutIter();}
void YoshiHistory15C50::fn_80415CF0(MtxPtr matrix){mEntries[mIndex].fn_80415B90(matrix);mEntries[mIndex].makeActorAppeared();++mIndex;if(mIndex>=4)mIndex=0;if(!MR::isDead(&mEntries[mIndex]))mEntries[mIndex].makeActorDead();}
void YoshiHistory15C50::fn_80415DA0(){for(s32 i=0;i<4;++i)mEntries[i].makeActorDead();mIndex=0;}
