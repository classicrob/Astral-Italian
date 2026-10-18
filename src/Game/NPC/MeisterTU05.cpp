// NONMATCHING: callback temporaries, register allocation, equivalent boolean branches, and literal aliases.
#pragma auto_inline off
#include "Game/NPC/Meister.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
TalkMessageCtrl* MeisterTalkHolder::getTalkCtrl() {
    TalkMessageCtrl* ctrl = mMainTalk;
    if (!ctrl) ctrl = getEventTalk();
    if (!ctrl) ctrl = getGeneralTalk();
    if (!ctrl) ctrl = mDefaultTalk;
    return ctrl;
}
// NONMATCHING: commutative add operands are reversed.
void MeisterTalkHolder::nextTalk() { mTalkIndex = (mEventTalkCount + mTalkIndex + 1) % mEventTalkCount; }
bool MeisterTalkHolder::branchFunc(u32 arg) {
    switch (arg) { case 0: return mStarCount >= 5; case 1: return _38; case 2: return MR::isConnectedWPad(1); }
    return false;
}
TalkMessageCtrl* MeisterTalkHolder::getEventTalk() {
    if (mEventTalkCount > 0) return mEventTalks[mTalkIndex];
    return 0;
}
// NONMATCHING: register allocation and argument evaluation scheduling differ.
TalkMessageCtrl* MeisterTalkHolder::getGeneralTalk() {
    s32 index = -1;
    for (s32 i = 0; i < 15; i++) {
        if (mGeneralTalkFlags[i] && isGeneralTalkEnabled(i)) { index = i; break; }
    }
    if (index != -1) {
        if (index == 6) { TalkMessageCtrl** ctrls = mGeneralTalks; s32 count = MR::getStarPieceNum(); MR::setMessageArg(ctrls[index], count); }
        return mGeneralTalks[index];
    }
    return 0;
}
TalkMessageCtrl* MeisterTalkHolder::getReactionTalk() { return mReactionTalk; }

#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/WPad.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/Util/WorldMapUtil.hpp"
bool MeisterTalkHolder::isGeneralTalkInitiallyEnabled(s32 index) {
    switch(index) {
    case 0: case 1: return GameSequenceFunction::isTicoFatStarPieceActive();
    case 3: return mStarCount==77;
    case 8: return mStarCount>=100 && !GameDataFunction::isOnGameEventFlag("ルイージプレイ済");
    case 9: return GameDataFunction::getPlayerMissNum()%19==0 && GameDataFunction::getPlayerMissNum()!=0;
    case 10: return GameDataFunction::getPlayerMissNum()%13==0 && GameDataFunction::getPlayerMissNum()!=0;
    case 11: return GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=8;
    case 12: return GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=4;
    case 14: return GameDataFunction::isOnGameEventFlagEnding120Stars();
    default: return true;
    }
}
bool MeisterTalkHolder::isGeneralTalkEnabled(s32 index) {
    switch(index) {
    case 0: return GameDataFunction::getTicoFatStarPieceAlms(_2C)<_30;
    case 1: return GameDataFunction::getTicoFatStarPieceAlms(_2C)>=_30;
    case 2: return MR::getWPad(0)->getBattery()<=1;
    case 4: return MR::getStarPieceNum()==777;
    case 5: return MR::getStarPieceNum()==7777;
    case 6: return MR::getStarPieceNum()%1000==0 && MR::getStarPieceNum()!=0;
    case 7: return MR::getStarPieceNum()==9999;
    case 13: {
        OSCalendarTime time;
        OSTicksToCalendarTime(OSGetTime(),&time);
        return time.hour>=3 && time.hour<5;
    }
    default: return true;
    }
}
void MeisterTalkHolder::removeInvalidTalk() {
    s32 removed=0;
    s32 count=mEventTalkCount;
    for(u32 i=0;i<static_cast<u32>(mEventTalkCount);i++) {
        if(!MR::isMarioFaceShipEventTalkValid(i)) { ++removed; mEventTalks[i]=nullptr; }
    }
    if(removed) {
        for(u32 i=0;i<static_cast<u32>(count);i++) {
            if(!mEventTalks[i]) {
                for(u32 j=i+1;j<static_cast<u32>(count);j++) {
                    TalkMessageCtrl* next=mEventTalks[j];
                    if(next) { TalkMessageCtrl* previous=mEventTalks[i]; mEventTalks[i]=next; mEventTalks[j]=previous; break; }
                }
            }
        }
        mEventTalkCount-=removed;
        nextTalk();
    }
}

extern "C" {
extern const char lbl_806BA840[];
extern const char* lbl_806BAA68[];
}
TVec3f gMeisterReactionTalkOffset(0.0f,80.0f,0.0f);
MeisterTalkHolder::MeisterTalkHolder(Meister* host,const JMapInfoIter& iter,const TVec3f& offset)
    : mHost(host),mMainTalk(nullptr),mReactionTalk(nullptr),mDefaultTalk(nullptr),mGeneralTalks(nullptr),mGeneralTalkCount(0),mEventTalks(nullptr),mEventTalkCount(0),mGeneralTalkFlags(nullptr),mGeneralFlagCount(0),mTalkIndex(0),_2C(-1),_30(0),mStarCount(0),_38(true) {
    const char* strings=lbl_806BA840;
    mStarCount=MR::getPowerStarNum();
    mGeneralTalks=new TalkMessageCtrl*[15];
    mGeneralTalkCount=15;
    mGeneralTalkFlags=new u8[15];
    mGeneralFlagCount=15;
    for(s32 i=0;i<15;i++) {
        const char* id=lbl_806BAA68[i];
        if(i==12) {
            bool repeatedMiss=false;
            if(GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=4) repeatedMiss=true;
            if(repeatedMiss) {
                if(GameDataFunction::getStageClearsAfterRepeatedMissCount()>=4) GameDataFunction::setStageClearsAfterRepeatedMissCount(0);
                if(GameDataFunction::getStageClearsAfterRepeatedMissCount()==0) id=strings+0x27C;
                GameDataFunction::incStageClearsAfterRepeatedMissCount();
            }
        }
        if(i==14) {
            bool greenOwned=false;
            ScenarioDataIter galaxy=MR::makeBeginScenarioDataIter();
            while(!galaxy.isEnd()) {
                GalaxyStatusAccessor status=galaxy.makeAccessor();
                for(s32 star=1;star<=status.getPowerStarNum();star++) {
                    if(status.isGreenStar(star) && status.hasPowerStar(star)) { greenOwned=true; break; }
                }
                if(greenOwned) break;
                galaxy.goNext();
            }
            if(greenOwned || MR::isPending120StarEndingDemo()) id=strings+0x2A0;
        }
        mGeneralTalks[i]=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,id,offset,nullptr);
        MR::registerBranchFunc(mGeneralTalks[i],TalkMessageFuncM<MeisterTalkHolder*,bool(MeisterTalkHolder::*)(u32)>(this,&MeisterTalkHolder::branchFunc));
        mGeneralTalkFlags[i]=isGeneralTalkInitiallyEnabled(i);
    }
    if(MR::isExistSceneObj(0x97)) {
        if(!GameSequenceFunction::isTicoFatStarPieceActive()) {
            s32 count=MR::getMarioFaceShipEventTalkCount();
            mEventTalks=new TalkMessageCtrl*[count];
            mEventTalkCount=count;
            for(s32 i=0;i<count;i++) {
                char id[0x100];
                MR::getMarioFaceShipEventTalkMessageId(i,id,sizeof(id));
                mEventTalks[i]=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,id,offset,nullptr);
            }
        }
        if(MR::isEqualStageName(strings+0x2C0) && MR::getCurrentScenarioNo()==1) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x2D4,offset,nullptr);
        else if(MR::isPendingNormalEndingDemo()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x2F4,offset,nullptr);
        else if(MR::isPending120StarEndingDemo()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x314,offset,nullptr);
        else if(GameSequenceFunction::isFaceShipRespondGameOver()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x334,offset,nullptr);
    }
    switch(MR::getProgressWorldNo()) {
    case 1: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x368,offset,nullptr); break;
    case 2: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x384,offset,nullptr); break;
    case 3: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x3A0,offset,nullptr); break;
    case 4: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x3BC,offset,nullptr); break;
    case 5: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x3D8,offset,nullptr); break;
    case 6: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x3F4,offset,nullptr); break;
    case 7: mDefaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x410,offset,nullptr); break;
    default: mDefaultTalk=nullptr; break;
    }
    MR::registerBranchFunc(mDefaultTalk,TalkMessageFuncM<MeisterTalkHolder*,bool(MeisterTalkHolder::*)(u32)>(this,&MeisterTalkHolder::branchFunc));
    mReactionTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,strings+0x42C,offset+gMeisterReactionTalkOffset,nullptr);
    if(GameSequenceFunction::isTicoFatStarPieceActive()) {
        _2C=GameSequenceFunction::getTicoFatStarPieceId();
        _30=GameSequenceFunction::getTicoFatStarPieceRequirement();
    }
    if(GameDataFunction::getFaceShipEventGrandStarNo()>0) GameSequenceFunction::clearFaceShipClearedScenariosCount();
    _38=(GameSequenceFunction::getFaceShipClearedScenariosCount()&1)==0;
    if(GameSequenceFunction::hasStageResultSequence() && mEventTalkCount==0) GameSequenceFunction::incFaceShipClearedScenariosCount();
}

#include "Game/Util/Array.hpp"
template MR::AssignableArray<TalkMessageCtrl*>::~AssignableArray();
