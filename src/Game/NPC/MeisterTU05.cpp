#pragma auto_inline off
#include "Game/NPC/Meister.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/WPad.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/Util/WorldMapUtil.hpp"
#include "Game/Util/Array.hpp"

const char* gMeisterGeneralTalkMessages[15] = {
    "Meister_PriorityTalkTicoFatExplain",
    "Meister_PriorityTalkTicoFatDisappear",
    "Meister_PriorityTalkBatteryEmpty",
    "Meister_PriorityTalkStar77",
    "Meister_PriorityTalkStarPiece777",
    "Meister_PriorityTalkStarPiece7777",
    "Meister_PriorityTalkStarPieceMultiple1000",
    "Meister_PriorityTalkStarPiece9999",
    "Meister_PriorityTalkLuigiPetition",
    "Meister_PriorityTalkMissMultiple19",
    "Meister_PriorityTalkMissMultiple13",
    "Meister_PriorityTalkPreviousScenarioMiss20",
    "Meister_PriorityTalkPreviousScenarioMiss10",
    "Meister_PriorityTalkMidnight",
    "Meister_AfterStar120EndingSub",
};

TVec3f gMeisterReactionTalkOffset(0.0f,80.0f,0.0f);
static inline bool hasMeisterGreenStar() {
    ScenarioDataIter galaxy=MR::makeBeginScenarioDataIter();
    while(!galaxy.isEnd()) {
        GalaxyStatusAccessor status=galaxy.makeAccessor();
        for(s32 star=1;star<=status.getPowerStarNum();star++) {
            if(status.isGreenStar(star) && status.hasPowerStar(star)) return true;
        }
        galaxy.goNext();
    }
    return false;
}

// NONMATCHING: callback temporaries and register allocation differ.
MeisterTalkHolder::MeisterTalkHolder(Meister* host,const JMapInfoIter& iter,const TVec3f& offset)
    : mHost(host),mMainTalk(nullptr),mReactionTalk(nullptr),mDefaultTalk(nullptr),mGeneralTalks(nullptr),mGeneralTalkCount(0),mEventTalks(nullptr),mEventTalkCount(0),mGeneralTalkFlags(nullptr),mGeneralFlagCount(0),mTalkIndex(0),mHungryLumaId(-1),mRequiredStarPieces(0),mStarCount(0),mIsEvenVisit(true) {
    mStarCount=MR::getPowerStarNum();
    mGeneralTalks=new TalkMessageCtrl*[15];
    mGeneralTalkCount=15;
    mGeneralTalkFlags=new u8[15];
    mGeneralFlagCount=15;
    for(s32 i=0;i<15;i++) {
        const char* id=gMeisterGeneralTalkMessages[i];
        if(i==12) {
            bool repeatedMiss=false;
            if(GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=4) repeatedMiss=true;
            if(repeatedMiss) {
                if(static_cast<u16>(GameDataFunction::getStageClearsAfterRepeatedMissCount())>=4) GameDataFunction::setStageClearsAfterRepeatedMissCount(0);
                if(static_cast<u16>(GameDataFunction::getStageClearsAfterRepeatedMissCount())==0) id="Meister_PriorityTalkAdvice2PSupport";
                GameDataFunction::incStageClearsAfterRepeatedMissCount();
            }
        }
        if(i==14) {
            bool greenOwned=hasMeisterGreenStar();
            if(greenOwned || MR::isPending120StarEndingDemo()) id="Meister_AfterStar120EndingEnd";
        }
        mGeneralTalks[i]=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,id,offset,nullptr);
        TalkMessageCtrl** talks = mGeneralTalks;
        TalkMessageFuncM<MeisterTalkHolder*,bool(MeisterTalkHolder::*)(u32)> callback = TalkMessageFunc_Inline(this, &MeisterTalkHolder::branchFunc);
        MR::registerBranchFunc(talks[i],callback);
        u8* flags = mGeneralTalkFlags;
        flags[i]=isGeneralTalkInitiallyEnabled(i);
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
        if(MR::isEqualStageName("MarioFaceShipGalaxy") && MR::getCurrentScenarioNo()==1) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_AfterIslandFleetGalaxy",offset,nullptr);
        else if(MR::isPendingNormalEndingDemo()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_AfterNormalEndingSub",offset,nullptr);
        else if(MR::isPending120StarEndingDemo()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_AfterStar120EndingSub",offset,nullptr);
        else if(GameSequenceFunction::isFaceShipRespondGameOver()) mMainTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_PriorityTalkPreviousScenarioMiss10_Flow000",offset,nullptr);
    }
    TalkMessageCtrl* defaultTalk;
    switch(MR::getProgressWorldNo()) {
    case 1: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld1",offset,nullptr); break;
    case 2: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld2",offset,nullptr); break;
    case 3: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld3",offset,nullptr); break;
    case 4: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld4",offset,nullptr); break;
    case 5: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld5",offset,nullptr); break;
    case 6: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld6",offset,nullptr); break;
    case 7: defaultTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_NormalTalkWorld7",offset,nullptr); break;
    default: defaultTalk=nullptr; break;
    }
    mDefaultTalk = defaultTalk;
    TalkMessageFuncM<MeisterTalkHolder*,bool(MeisterTalkHolder::*)(u32)> callback = TalkMessageFunc_Inline(this, &MeisterTalkHolder::branchFunc);
    MR::registerBranchFunc(mDefaultTalk,callback);
    mReactionTalk=MR::createTalkCtrlDirectOnRootNodeAutomatic(host,iter,"Meister_StarPieceReaction",offset+gMeisterReactionTalkOffset,nullptr);
    if(GameSequenceFunction::isTicoFatStarPieceActive()) {
        mHungryLumaId=GameSequenceFunction::getTicoFatStarPieceId();
        mRequiredStarPieces=GameSequenceFunction::getTicoFatStarPieceRequirement();
    }
    bool hasGrandStar = GameDataFunction::getFaceShipEventGrandStarNo()>0;
    if(hasGrandStar) GameSequenceFunction::clearFaceShipClearedScenariosCount();
    if ((GameSequenceFunction::getFaceShipClearedScenariosCount()&1)==0) mIsEvenVisit=true;
    else mIsEvenVisit=false;
    if(GameSequenceFunction::hasStageResultSequence() && mEventTalkCount==0) GameSequenceFunction::incFaceShipClearedScenariosCount();
}

bool MeisterTalkHolder::isGeneralTalkInitiallyEnabled(s32 index) {
    switch(index) {
    case 0: case 1: return GameSequenceFunction::isTicoFatStarPieceActive();
    case 3: return mStarCount==77;
    case 8: return mStarCount>=100 && !GameDataFunction::isOnGameEventFlag("ルイージプレイ済");
    case 9:
        if (GameDataFunction::getPlayerMissNum()%19==0 && GameDataFunction::getPlayerMissNum()!=0) return true;
        return false;
    case 10:
        if (GameDataFunction::getPlayerMissNum()%13==0 && GameDataFunction::getPlayerMissNum()!=0) return true;
        return false;
    case 11:
        if (GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=8) return true;
        return false;
    case 12: return GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::getResultMissCount()>=4;
    case 14: return GameDataFunction::isOnGameEventFlagEnding120Stars();
    default: return true;
    }
}

// NONMATCHING: equivalent signed-hour range uses a subtract-and-unsigned-range check.
bool MeisterTalkHolder::isGeneralTalkEnabled(s32 index) {
    switch(index) {
    case 0: return GameDataFunction::getTicoFatStarPieceAlms(mHungryLumaId)<mRequiredStarPieces;
    case 1: return GameDataFunction::getTicoFatStarPieceAlms(mHungryLumaId)>=mRequiredStarPieces;
    case 2: return MR::getWPad(0)->getBattery()<=1;
    case 4: return MR::getStarPieceNum()==777;
    case 5: return MR::getStarPieceNum()==7777;
    case 6: return MR::getStarPieceNum()%1000==0 && MR::getStarPieceNum()!=0;
    case 7: return MR::getStarPieceNum()==9999;
    case 13: {
        OSCalendarTime time;
        OSTicksToCalendarTime(OSGetTime(),&time);
        bool enabled = false;
        if (time.hour >= 3) {
            if (time.hour < 5) enabled = true;
        }
        return enabled;
    }
    default: return true;
    }
}

TalkMessageCtrl* MeisterTalkHolder::getTalkCtrl() {
    TalkMessageCtrl* ctrl = mMainTalk;
    if (!ctrl) ctrl = getEventTalk();
    if (!ctrl) ctrl = getGeneralTalk();
    if (!ctrl) ctrl = mDefaultTalk;
    return ctrl;
}

// NONMATCHING: commutative add operands are reversed.
void MeisterTalkHolder::nextTalk() { s32 count = mEventTalkCount; s32 index = mTalkIndex; index += count; mTalkIndex = (index + 1) % count; }

// NONMATCHING: loop register allocation differs.
void MeisterTalkHolder::removeInvalidTalk() {
    s32 removed=0;
    s32 count=mEventTalkCount;
    for(u32 i=0;i<static_cast<u32>(mEventTalkCount);i++) {
        if(!MR::isMarioFaceShipEventTalkValid(i)) { ++removed; mEventTalks[i]=nullptr; }
    }
    if(removed) {
        for(u32 i=0;i<static_cast<u32>(count);i++) {
            if(!mEventTalks[i]) {
                TalkMessageCtrl** talks = mEventTalks;
                for(u32 j=i+1;j<static_cast<u32>(count);j++) {
                    TalkMessageCtrl* next=talks[j];
                    if(next) { TalkMessageCtrl* previous=talks[i]; talks[i]=next; talks[j]=previous; break; }
                }
            }
        }
        mEventTalkCount-=removed;
        nextTalk();
    }
}

bool MeisterTalkHolder::branchFunc(u32 arg) {
    switch (arg) { case 0: return mStarCount >= 5; case 1: return mIsEvenVisit; case 2: return MR::isConnectedWPad(1); }
    return false;
}

TalkMessageCtrl* MeisterTalkHolder::getEventTalk() {
    if (mEventTalkCount > 0) return mEventTalks[mTalkIndex];
    return 0;
}

// NONMATCHING: array address evaluation and saved-register allocation differ.
TalkMessageCtrl* MeisterTalkHolder::getGeneralTalk() {
    s32 index = -1;
    for (s32 i = 0; i < 15; i++) {
        if (mGeneralTalkFlags[i] && isGeneralTalkEnabled(i)) { index = i; break; }
    }
    if (index != -1) {
        if (index == 6) { MR::setMessageArg(mGeneralTalks[index], MR::getStarPieceNum()); }
        return mGeneralTalks[index];
    }
    return 0;
}

TalkMessageCtrl* MeisterTalkHolder::getReactionTalk() { return mReactionTalk; }

template MR::AssignableArray<TalkMessageCtrl*>::~AssignableArray();
