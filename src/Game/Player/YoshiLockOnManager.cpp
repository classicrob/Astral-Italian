#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Screen/YoshiTargetMark.hpp"
#include "Game/Util.hpp"
#include <cstdio>

class YoshiLockOnIterator {
public:
    YoshiLockOnIterator(MR::BothDirPtrLink* link):mLink(link){}
    MR::BothDirPtrLink* operator++(int) NO_INLINE;
    MR::BothDirPtrLink* mLink;
};
MR::BothDirPtrLink* YoshiLockOnIterator::operator++(int){MR::BothDirPtrLink* old=mLink;mLink=old->mNextLink;return old;}
void YoshiLockOnTargetControl::reset(){mTarget=nullptr;mTimer=0;mMark=nullptr;}
YoshiLockOnManager::YoshiLockOnManager(Yoshi* yoshi):NameObj("ターゲットロック管理"),_14(0),mContextActor(nullptr),mContextActive(false),_1E(false),mFree(),mFading(),mActive(),mSoundLatch(0){fn_80419EB0(yoshi);}
YoshiLockOnManager::~YoshiLockOnManager(){}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::init(const JMapInfoIter&){
    for(u32 i=0;i<16;++i)mFree.append(new YoshiLockOnTargetControl());
    MR::connectToScene(this,0x27,-1,-1,0x46);MR::registerDemoSimpleCastAll(this);
    mMarkCount=sTargetMax*2;mMarks=new YoshiTargetMark*[sTargetMax*2];
    for(s32 i=0;i<mMarkCount;++i){mMarks[i]=new YoshiTargetMark("ヨッシーターゲットマーク");mMarks[i]->initWithoutIter();}
}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::movement(){if(mContextActive){if(_1D)return;if(MR::isPlayerInRushOrDemoOrTalk())fn_8041A0B0();fn_8041A590();fn_8041A660();_1E=true;}}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::draw()const{
    if(_1E){for(YoshiLockOnIterator it(mFading.mHead);it.mLink;it++)static_cast<YoshiLockOnTargetControl*>(it.mLink->mValue)->calcScreenPosition();for(YoshiLockOnIterator it(mActive.mHead);it.mLink;it++)static_cast<YoshiLockOnTargetControl*>(it.mLink->mValue)->calcScreenPosition();}
}
void YoshiLockOnManager::fn_80419EB0(Yoshi* yoshi){
    char name[64];snprintf(name,sizeof(name),"%s.bcsv",sParamFile);JMapInfo* csv=MR::tryCreateCsvParser(MR::getResourceHolder(yoshi),name);
    if(csv){MR::getCsvDataS32(&sTimer,csv,"TimerCount",0);MR::getCsvDataF32(&sDistance0,csv,"EatTargetDistMax",0);MR::getCsvDataF32(&sDistance2,csv,"HookTargetDistMax",0);MR::getCsvDataF32(&sDistance4,csv,"PullTargetDistMax",0);MR::getCsvDataS32(&sTargetMax,csv,"TargetMax",0);}
}
bool YoshiLockOnManager::fn_80419F90(YoshiLockOnTarget* t){return fn_8041A130(t,0,1,3,sDistance0);}
bool YoshiLockOnManager::fn_80419FB0(YoshiLockOnTarget* t){return fn_8041A130(t,1,1,3,sDistance0);}
bool YoshiLockOnManager::fn_80419FD0(YoshiLockOnTarget* t,bool alternate){if(alternate)return fn_8041A290(t,3,sDistance2);return fn_8041A290(t,2,sDistance2);}
bool YoshiLockOnManager::fn_80419FF0(YoshiLockOnTarget* t){return fn_8041A290(t,4,sDistance4);}
bool YoshiLockOnManager::fn_8041A000(YoshiLockOnTarget* t){return fn_8041A290(t,5,sDistance5);}
u32 YoshiLockOnManager::fn_8041A010()const{return 16;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
u32 YoshiLockOnManager::fn_8041A020(YoshiLockOnTargetDesc* out,u32){u32 count=0;for(YoshiLockOnIterator it(mActive.mHead);it.mLink;it++)static_cast<YoshiLockOnTargetControl*>(it.mLink->mValue)->copyTargetInfo(out+count++);fn_8041A0B0();return count;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::fn_8041A0B0(){for(YoshiLockOnIterator it(mActive.mHead);it.mLink;){YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>((it++)->mValue);c->disappearMark();mActive.remove(c);mFading.append(c);}}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiLockOnManager::fn_8041A130(YoshiLockOnTarget* t,s32 type,s32 mode,u32 limit,f32 distance){
    if(!mContextActive||_1D)return false;if(PSVECDistance(&mContextActor->mPosition,&t->mActor->mPosition)>=distance)return false;
    if(_14!=mode)fn_8041A0B0();else if(fn_8041A720(t))return true;
    if(mActive.mCount>=limit)return false;if(fn_8041A770(t))return true;if(!fn_8041A3E0(t,sTimer,type))return false;
    _14=mode;if(!mSoundLatch&&!MR::isDemoActive()){MR::startActionSound(mContextActor,"CurMarking",-1,-1,-1);mSoundLatch=1;}return true;
}
bool YoshiLockOnManager::fn_8041A290(YoshiLockOnTarget* t,s32 type,f32 distance){
    if(!mContextActive||_1D)return false;if(PSVECDistance(&mContextActor->mPosition,&t->mActor->mPosition)>=distance)return false;
    YoshiLockOnTargetControl* c=fn_8041A4A0(t);if(c){c->mTimer=sTimer;mSoundLatch=0;return true;}
    fn_8041A0B0();if(!mSoundLatch&&!MR::isDemoActive()){MR::startActionSound(mContextActor,"CurMarking",-1,-1,-1);mSoundLatch=1;}
    if(fn_8041A770(t))return true;if(!fn_8041A3E0(t,sTimer,type))return false;_14=0;return true;
}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiLockOnManager::fn_8041A3E0(YoshiLockOnTarget* t,s32 timer,s32 type){
    if(!mFree.mCount)return false;MR::BothDirPtrLink* link=mFree.mHead;mFree.remove(static_cast<YoshiLockOnTargetControl*>(link->mValue));mActive.append(static_cast<YoshiLockOnTargetControl*>(link->mValue));
    YoshiTargetMark* mark=fn_8041A510();if(mark)mark->appear();YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>(link->mValue);c->mTarget=t;c->mTimer=timer;c->mMark=mark;c->mType=type;c->updateMarkPosition();return true;
}
YoshiLockOnTargetControl* YoshiLockOnManager::fn_8041A4A0(YoshiLockOnTarget* t){for(YoshiLockOnIterator it(mActive.mHead);it.mLink;it++){YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>(it.mLink->mValue);if(c->mTarget==t)return static_cast<YoshiLockOnTargetControl*>(c->mValue);}return nullptr;}
YoshiTargetMark* YoshiLockOnManager::fn_8041A510(){for(s32 i=0;i<mMarkCount;++i)if(MR::isDead(mMarks[i]))return mMarks[i];return nullptr;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::fn_8041A590(){for(YoshiLockOnIterator it(mFading.mHead);it.mLink;){YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>((it++)->mValue);c->update(mContextActor->mPosition,getYoshiLockOnDistance(c->mType));if(c->isMarkDead()){if(c->mMark)c->mMark->disappear();c->reset();mFading.remove(static_cast<YoshiLockOnTargetControl*>(c->mValue));mFree.append(static_cast<YoshiLockOnTargetControl*>(c->mValue));}}}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::fn_8041A660(){for(YoshiLockOnIterator it(mActive.mHead);it.mLink;){YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>((it++)->mValue);c->update(mContextActor->mPosition,getYoshiLockOnDistance(c->mType));if(c->isInvalid()){c->disappearMark();mActive.remove(static_cast<YoshiLockOnTargetControl*>(c->mValue));mFading.append(static_cast<YoshiLockOnTargetControl*>(c->mValue));}c->invalidate();}}
bool YoshiLockOnManager::fn_8041A720(YoshiLockOnTarget* t){YoshiLockOnTargetControl* c=fn_8041A4A0(t);if(c){c->mTimer=sTimer;mSoundLatch=0;return true;}return false;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiLockOnManager::fn_8041A770(YoshiLockOnTarget* t){for(YoshiLockOnIterator it(mFading.mHead);it.mLink;){YoshiLockOnTargetControl* c=static_cast<YoshiLockOnTargetControl*>((it++)->mValue);if(c->mTarget==t){c->appearMark();c->mTimer=sTimer;mFading.remove(static_cast<YoshiLockOnTargetControl*>(c->mValue));mActive.append(static_cast<YoshiLockOnTargetControl*>(c->mValue));return true;}}return false;}
