#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemDataHolder.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/LMS.hpp"
#include "Game/System/MarioFaceShipAttentionConversationDirector.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <cstdio>
#include <cstring>


MarioFaceShipEventDataHolder::MarioFaceShipEventDataHolder(const char* pName) : NameObj(pName) {
    mEventInfoNum = 0;
    mValidEventInfoNum = 0;
    mAttentionTalkNum = 0;
    mNormalTalkNum = 0;
    mEventInfos = nullptr;
    mAttentionConversationNum = 0;
    mNormalConversationNum = 0;
}

void MarioFaceShipEventDataHolder::init(const JMapInfoIter&) {
    initCastTable();
    initEventTable();
}

void MarioFaceShipEventDataHolder::initCastTable() {
    const JMapInfo* pTable = GameSystemDataFunction::getMarioFaceShipEventCastTable();
    mCastNum = MR::getCsvDataElementNum(pTable);
    mCasts = new MarioFaceShipEventCast*[mCastNum];

    for (s32 i = 0; i < mCastNum; i++) {
        mCasts[i] = new MarioFaceShipEventCast();
        MR::getCsvDataS32(&mCasts[i]->mEventNo, pTable, "EventNo", i);
        MR::getCsvDataS32(&mCasts[i]->mCastNum, pTable, "CastNum", i);

        if (mCasts[i]->mCastNum > 0) {
            mCasts[i]->mCastIds = new s32[mCasts[i]->mCastNum];
            mCasts[i]->mCastNames = new const char*[mCasts[i]->mCastNum];
            s32* pIds = mCasts[i]->mCastIds;
            const char** pNames = mCasts[i]->mCastNames;

            for (s32 j = 0; j < mCasts[i]->mCastNum; j++) {
                char nameKey[0x40];
                snprintf(nameKey, 0x40, "CastName%d", j);
                MR::getCsvDataStrOrNULL(&pNames[j], pTable, nameKey, i);
                char idKey[0x40];
                snprintf(idKey, 0x40, "CastID%d", j);
                MR::getCsvDataS32(&pIds[j], pTable, idKey, i);
            }
        }
    }
}

void MarioFaceShipEventDataHolder::initEventTable() {
    const JMapInfo* pTable = GameSystemDataFunction::getMarioFaceShipEventDataTable();
    mEventInfoNum = MR::getCsvDataElementNum(pTable);
    mEventInfos = new MarioFaceShipEventInfo*[mEventInfoNum];

    for (s32 i = 0; i < mEventInfoNum; i++) {
        mEventInfos[i] = new MarioFaceShipEventInfo();
        MR::getCsvDataStrOrNULL(&mEventInfos[i]->mEventType, pTable, "EventType", i);
        MR::getCsvDataStrOrNULL(&mEventInfos[i]->mKind, pTable, "FlagType", i);
        MR::getCsvDataStrOrNULL(&mEventInfos[i]->mName1, pTable, "StageName", i);
        MR::getCsvDataStrOrNULL(&mEventInfos[i]->mName2, pTable, "OrStageName", i);
        MR::getCsvDataStrOrNULL(&mEventInfos[i]->mReturnType, pTable, "ReturnType", i);
        MR::getCsvDataS32(&mEventInfos[i]->mStarId1, pTable, "ScenarioNo", i);
        MR::getCsvDataS32(&mEventInfos[i]->mStarId2, pTable, "OrScenarioNo", i);
        MR::getCsvDataS32(&mEventInfos[i]->mThreshold, pTable, "PowerStarNum", i);
        MR::getCsvDataS32(&mEventInfos[i]->mWatchedEventNo, pTable, "WatchedEventNo", i);
        MR::getCsvDataS32(&mEventInfos[i]->mSwitchId, pTable, "SwitchNo", i);
        MR::getCsvDataS32(&mEventInfos[i]->mObjId, pTable, "EventNo", i);
        MR::getCsvDataU8(&mEventInfos[i]->mUseTimeKeepDemo, pTable, "UseTimeKeepDemo", i);
        findCast(mEventInfos[i]->mObjId, &mEventInfos[i]->mCast);

        if (mEventInfos[i]->mSwitchId != -1) {
            mValidEventInfoNum++;
        }
        bool isAttention = false;
        const char* pType = mEventInfos[i]->mEventType;
        if (MR::isEqualString(pType, "注目会話") || MR::isEqualString(pType, "注目会話[注目先行]")) {
            isAttention = true;
        }
        if (isAttention) {
            mAttentionTalkNum++;
        }
        if (MR::isEqualString(mEventInfos[i]->mEventType, "会話")) {
            mNormalTalkNum++;
        }
    }
}

void MarioFaceShipEventDataHolder::findCast(s32 eventNo, MarioFaceShipEventCast** ppCast) const {
    for (s32 i = 0; i < mCastNum; i++) {
        if (eventNo == mCasts[i]->mEventNo) {
            *ppCast = mCasts[i];
            return;
        }
    }
}

bool MarioFaceShipEventDataHolder::makeNoticeMessageId(s32 index, char* pDst, u32 size) const {
    bool isAttention;
    const char* pType;
    s32 eventNo;

    eventNo = GameDataFunction::getFaceShipEventNo(index);

    for (s32 i = 0; i < mEventInfoNum; i++) {
        if (eventNo == mEventInfos[i]->mObjId) {
            pType = mEventInfos[i]->mEventType;
            isAttention = false;
            if (MR::isEqualString(pType, "注目会話") || MR::isEqualString(pType, "注目会話[注目先行]")) {
                isAttention = true;
            }
            if (isAttention) {
                snprintf(pDst, size, "Meister_%dEventNoticeSub", eventNo);
                return true;
            }
            if (MR::isEqualString(mEventInfos[i]->mEventType, "会話")) {
                snprintf(pDst, size, "Meister_%dEventTalkSub", eventNo);
                return true;
            }
        }
    }

    return false;
}

bool MarioFaceShipEventDataHolder::isPairedStarNotOwned(const char* pStage, s32 starId, const char* pStage1, s32 starId1, const char* pStage2, s32 starId2) const {
    if (MR::isEqualString(pStage, pStage1) && starId == starId1) {
        return !MR::isOnGameEventFlagPowerStarSuccess(pStage2, starId2);
    }
    if (MR::isEqualString(pStage, pStage2) && starId == starId2) {
        return !MR::isOnGameEventFlagPowerStarSuccess(pStage1, starId1);
    }
    return false;
}

bool MarioFaceShipEventDataHolder::isSpecialEventSatisfied(s32 eventId, const char* pStage, s32 starId) const {
    if (eventId == 0x190) {
        return MR::isBigTreeCloudGardenPairedStarUncollected(pStage, starId);
    }
    if (eventId == 0x12D) {
        return MR::isPowerStarNumWorld3Enough();
    }
    return false;
}

s32 MarioFaceShipEventDataHolder::getSpecialEventValue(s32 eventId) const {
    if (eventId == 0x190) {
        return MR::isBigTreeOrCloudGardenPowerStarSuccess();
    }
    return 0;
}

s32 MarioFaceShipEventDataHolder::calcTalkNum() {
    mAttentionConversationNum = 0;
    mNormalConversationNum = 0;
    mAttentionConversationNum = MarioFaceShipAttentionConversationDirector::getConversationNum();
    mNormalConversationNum = MarioFaceShipNormalConversationDirector::getConversationNum();
    s32 special = MarioFaceShipNormalConversationDirector::hasSpecialConversation();
    s32 sum = special + mAttentionConversationNum;
    return mNormalConversationNum + sum;
}

void MarioFaceShipEventDataHolder::tryStartTalk(s32 index) {
    if (MarioFaceShipNormalConversationDirector::hasSpecialConversation()) {
        if (index == 0) {
            MarioFaceShipNormalConversationDirector::tryStartSpecialDemo(index);
            return;
        }
        index--;
    }
    if (index < mNormalConversationNum) {
        MarioFaceShipNormalConversationDirector::tryStartDemo(index);
        return;
    }
    MarioFaceShipAttentionConversationDirector::tryStartDemo(index - mNormalConversationNum);
}

bool MarioFaceShipEventDataHolder::isTalkValid(s32 index) const {
    s32 eventNo = getTalkIndex(index);
    if (eventNo == 0) {
        return false;
    }
    for (s32 i = 0; i < mEventInfoNum; i++) {
        if (eventNo == mEventInfos[i]->mObjId) {
            return !MR::isEqualString(mEventInfos[i]->mReturnType, "アイテム惑星");
        }
    }
    return false;
}

s32 MarioFaceShipEventDataHolder::getTalkIndex(s32 index) const {
    if (MarioFaceShipNormalConversationDirector::hasSpecialConversation()) {
        if (index == 0) {
            return MarioFaceShipNormalConversationDirector::getSpecialTalkIndex(index);
        }
        index--;
    }
    if (index < mNormalConversationNum) {
        return MarioFaceShipNormalConversationDirector::getTalkIndexByIndex(index);
    }
    return MarioFaceShipAttentionConversationDirector::getTalkIndexByIndex(index - mNormalConversationNum);
}

bool MarioFaceShipEventDataHolder::makeTalkMessageId(s32 index, char* pDst, u32 size) const {
    if (MarioFaceShipNormalConversationDirector::hasSpecialConversation()) {
        if (index == 0) {
            return MarioFaceShipNormalConversationDirector::makeSpecialTalkMessageIdByIndex(index, pDst, size);
        }
        index--;
    }
    if (index < mNormalConversationNum) {
        return MarioFaceShipNormalConversationDirector::makeTalkMessageIdByIndex(index, pDst, size);
    }
    if (mAttentionConversationNum > 0) {
        return MarioFaceShipAttentionConversationDirector::makeTalkMessageIdByIndex(index - mNormalConversationNum, pDst, size);
    }
    return makeNoticeMessageId(index, pDst, size);
}

bool MarioFaceShipEventDataHolder::isTalkEnd(s32 index) const {
    if (MarioFaceShipNormalConversationDirector::hasSpecialConversation()) {
        if (index == 0) {
            return MarioFaceShipNormalConversationDirector::isSpecialDemoEnd(index);
        }
        index--;
    }
    if (index < mNormalConversationNum) {
        return MarioFaceShipNormalConversationDirector::isDemoEnd(index);
    }
    return MarioFaceShipAttentionConversationDirector::isDemoEnd(index - mNormalConversationNum);
}

bool MarioFaceShipEventDataHolder::isTalkWaiting(s32 index) const {
    if (MarioFaceShipNormalConversationDirector::hasSpecialConversation()) {
        if (index == 0) {
            return MarioFaceShipNormalConversationDirector::isSpecialWaiting(index);
        }
        index--;
    }
    if (index < mNormalConversationNum) {
        return MarioFaceShipNormalConversationDirector::isWaiting(index);
    }
    return MarioFaceShipAttentionConversationDirector::isWaiting(index - mNormalConversationNum);
}

bool MarioFaceShipEventDataHolder::isNpcTalkAllowed() const {
    if (!MR::isDemoActive()) {
        return true;
    }
    s32 num = const_cast< MarioFaceShipEventDataHolder* >(this)->calcTalkNum();
    for (s32 i = 0; i < num; i++) {
        if (getTalkIndex(i) == 0xC8 && isTalkWaiting(i)) {
            return true;
        }
    }
    return false;
}

bool MarioFaceShipEventDataHolder::hasEventAtResult() const {
    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }
    if (!GameSequenceFunction::hasResultPowerStar()) {
        return false;
    }
    const char* pStageName = GameSequenceFunction::getClearedStageName();
    s32 starId = GameSequenceFunction::getClearedPowerStarId();
    s32 starNum = MR::getPowerStarNum();

    for (s32 i = 0; i < mEventInfoNum; i++) {
        MarioFaceShipEventInfo* pInfo = mEventInfos[i];
        if (MR::isEqualString(pInfo->mKind, "シナリオ")) {
            if (pInfo->mStarId1 == starId && MR::isEqualString(pStageName, pInfo->mName1) && MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "スター数")) {
            if (pInfo->mThreshold == starNum && MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "コメットオープン")) {
            if (MR::isAnyGalaxyAvailableWithStars() && MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "シナリオA or シナリオB")) {
            if (isPairedStarNotOwned(pStageName, starId, pInfo->mName1, pInfo->mStarId1, pInfo->mName2, pInfo->mStarId2) &&
                MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "特殊")) {
            if (isSpecialEventSatisfied(pInfo->mObjId, pStageName, starId) && MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "イエロースター120個")) {
            if (GameSequenceFunction::isClearedAll120PowerStarAtResult() && MR::isEqualString(pInfo->mReturnType, "下")) {
                return true;
            }
        }
    }

    return false;
}

bool MarioFaceShipEventDataHolder::hasItemEventAtResult() const {
    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }
    if (!GameSequenceFunction::hasResultPowerStar()) {
        return false;
    }
    const char* pStageName = GameSequenceFunction::getClearedStageName();
    s32 starId = GameSequenceFunction::getClearedPowerStarId();
    s32 starNum = MR::getPowerStarNum();

    for (s32 i = 0; i < mEventInfoNum; i++) {
        MarioFaceShipEventInfo* pInfo = mEventInfos[i];
        if (MR::isEqualString(pInfo->mKind, "シナリオ")) {
            if (pInfo->mStarId1 == starId && MR::isEqualString(pStageName, pInfo->mName1) && MR::isEqualString(pInfo->mReturnType, "アイテム惑星")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "スター数")) {
            if (pInfo->mThreshold == starNum && MR::isEqualString(pInfo->mReturnType, "アイテム惑星")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "コメットオープン")) {
            if (MR::isAnyGalaxyAvailableWithStars() && MR::isEqualString(pInfo->mReturnType, "アイテム惑星")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "シナリオA or シナリオB")) {
            if (isPairedStarNotOwned(pStageName, starId, pInfo->mName1, pInfo->mStarId1, pInfo->mName2, pInfo->mStarId2) &&
                MR::isEqualString(pInfo->mReturnType, "アイテム惑星")) {
                return true;
            }
        } else if (MR::isEqualString(pInfo->mKind, "特殊")) {
            if (isSpecialEventSatisfied(pInfo->mObjId, pStageName, starId) && MR::isEqualString(pInfo->mReturnType, "アイテム惑星")) {
                return true;
            }
        }
    }

    return false;
}

// NONMATCHING: initial flag scheduling (FP order/grouping checked: no FP instructions).
bool MarioFaceShipEventDataHolder::isAttentionTalkEvent(const MarioFaceShipEventInfo* pInfo) {
    bool isAttention;
    const char* pType = pInfo->mEventType;
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    isAttention = false;
    if (MR::isEqualString(pType, "注目会話") || MR::isEqualString(pType, "注目会話[注目先行]")) {
        isAttention = true;
    }
    return isAttention;
}

bool MarioFaceShipEventDataHolder::isNormalTalkEvent(const MarioFaceShipEventInfo* pInfo) {
    const char* pType = pInfo->mEventType;
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pType, "会話");
}

bool MarioFaceShipEventInfo::isAttentionPriorTalk() const {
    return MR::isEqualString(mEventType, "注目会話[注目先行]");
}

bool MarioFaceShipEventDataHolder::isKindStarOwned(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "シナリオ");
}

bool MarioFaceShipEventDataHolder::isKindThresholdReached(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "スター数");
}

bool MarioFaceShipEventDataHolder::isKindCometOpen(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "コメットオープン");
}

bool MarioFaceShipEventDataHolder::isKindDualStarOwned(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "シナリオA or シナリオB");
}

bool MarioFaceShipEventDataHolder::isKindStarAmountReached(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "イエロースター120個");
}

bool MarioFaceShipEventDataHolder::isKindObjectSet(const char* pKind) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
    return MR::isEqualString(pKind, "特殊");
}

s32 MarioFaceShipEventDataHolder::getEventInfoNum() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->mEventInfoNum;
}

s32 MarioFaceShipEventDataHolder::getValidEventInfoNum() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->mValidEventInfoNum;
}

s32 MarioFaceShipEventDataHolder::getAttentionTalkNum() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->mAttentionTalkNum;
}

s32 MarioFaceShipEventDataHolder::getNormalTalkNum() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->mNormalTalkNum;
}

MarioFaceShipEventInfo* MarioFaceShipEventDataHolder::getEventInfo(s32 index) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->mEventInfos[index];
}

s32 MarioFaceShipEventDataHolder::calcAllTalkNum() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->calcTalkNum();
}

s32 MR::getMarioFaceShipEventTalkCount() {
    s32 num = MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->calcTalkNum();
    if (num <= 0) {
        MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder);
        num = GameDataFunction::calcFaceShipEventNum();
    }
    return num;
}

void MarioFaceShipEventDataHolder::tryStartTalkByIndex(s32 index) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->tryStartTalk(index);
}

bool MR::isMarioFaceShipEventTalkValid(u32 index) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->isTalkValid(index);
}

s32 MarioFaceShipEventDataHolder::getTalkIndexByIndex(s32 index) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->getTalkIndex(index);
}

void MR::getMarioFaceShipEventTalkMessageId(u32 index, char* pDst, u32 size) {
    MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->makeTalkMessageId(index, pDst, size);
}

bool MarioFaceShipEventDataHolder::isTalkEndByIndex(s32 index) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->isTalkEnd(index);
}

bool MR::isMarioFaceShipNpcTalkAllowed() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->isNpcTalkAllowed();
}

bool MarioFaceShipEventDataHolder::hasEventAtResultStatic() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->hasEventAtResult();
}

bool MR::hasMarioFaceShipItemEventAtResult() {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->hasItemEventAtResult();
}

bool MarioFaceShipEventDataHolder::isPairedStarNotOwnedStatic(const char* pStage, s32 starId, const char* pStage1, s32 starId1, const char* pStage2, s32 starId2) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->isPairedStarNotOwned(pStage, starId, pStage1, starId1, pStage2, starId2);
}

bool MarioFaceShipEventDataHolder::isSpecialEventSatisfiedStatic(s32 eventId, const char* pStage, s32 starId) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->isSpecialEventSatisfied(eventId, pStage, starId);
}

s32 MarioFaceShipEventDataHolder::getObjectValue(s32 eventId) {
    return MR::getSceneObj< MarioFaceShipEventDataHolder >(SceneObj_MarioFaceShipEventDataHolder)->getSpecialEventValue(eventId);
}

MarioFaceShipEventDataHolder::~MarioFaceShipEventDataHolder() {
}

static const char* sSystemMessageArc = "/MessageData/SystemMessage.arc";
static const char* sLayoutMessageArc = "/MessageData/LayoutMessage.arc";
static const char* sSceneCommonMessageArc = "/MessageData/SceneCommonMessage.arc";

static void* allocLMSMemory(u32 size) {
    return new u8[size];
}

static void freeLMSMemory(void*) {
}

// Loads every .msbt of an archive as a MessageData.
static // NONMATCHING: register allocation (FP order/grouping checked: no FP instructions).
void loadMessageDatasFromArchive(MessageData*** pOutDatas, s32* pOutNum, const char* pArchivePath) NO_INLINE;

static void loadMessageDatasFromArchive(MessageData*** pOutDatas, s32* pOutNum, const char* pArchivePath) {
    JKRArchive* pArchive = nullptr;
    JKRHeap* pHeap = nullptr;
    MR::getMountedArchiveAndHeap(pArchivePath, &pArchive, &pHeap);

    *pOutNum = 0;
    {
        JKRArcFinder* pFinder = pArchive->getFirstFile("/");
        while (pFinder->mHasMoreFiles) {
            if (MR::isEqualSubString(pFinder->mName, ".msbt")) {
                (*pOutNum)++;
            }
            pFinder->findNextFile();
        }
        delete pFinder;
    }

    JKRArcFinder* pFinder = pArchive->getFirstFile("/");
    *pOutDatas = new MessageData*[*pOutNum];
    s32 n = 0;
    while (pFinder->mHasMoreFiles) {
        if (MR::isEqualSubString(pFinder->mName, ".msbt")) {
            (*pOutDatas)[n] = new MessageData(pArchivePath, pFinder->mName);
            n++;
        }
        pFinder->findNextFile();
    }
    delete pFinder;
}

MessageData::MessageData(const char* pArchivePath, const char* pFileName) {
    mMessageHandle = nullptr;
    mFlowHandle = nullptr;
    mMessageHandle = LMS_InitMessage(MR::getResourceFromMountedArchive(pArchivePath, pFileName));
    char flowName[0x100];
    strncpy(flowName, pFileName, 0x100);
    *strchr(flowName, '.') = 0;
    snprintf(flowName, 0x100, "%s.msbf", flowName);
    if (MR::isExistResourceInMountedArchive(pArchivePath, flowName)) {
        mFlowHandle = LMS_InitFlowchart(MR::getResourceFromMountedArchive(pArchivePath, flowName));
    }
}

bool MessageData::getMessageDirect(TalkMessageInfo* pInfo, const char* pLabel) const {
    if (mMessageHandle == nullptr) {
        return false;
    }
    const wchar_t* pText = LMS_GetTextByLabel(mMessageHandle, pLabel);
    if (pText == nullptr) {
        return false;
    }
    pInfo->mMessageText = reinterpret_cast<u8*>(const_cast<wchar_t*>(pText));
    MsbtAttribute* pAttr = LMS_GetAttribute(mMessageHandle, LMS_GetTextIndexByLabel(mMessageHandle, pLabel));
    pInfo->mTalkSoundId = pAttr->_0;
    pInfo->mCameraType = pAttr->_1;
    pInfo->mTalkType = pAttr->_2;
    pInfo->mBalloonType = pAttr->_3;
    pInfo->mCameraSetID = pAttr->_4;
    pInfo->mMessageAreaArg = pAttr->_6;
    pInfo->mAlreadyReadFlagId = pAttr->_7;
    return true;
}

bool MessageData::getMessageByIndex(TalkMessageInfo* pInfo, s32 index) const {
    const wchar_t* pText = LMS_GetText(mMessageHandle, index);
    if (pText == nullptr) {
        return false;
    }
    pInfo->mMessageText = reinterpret_cast<u8*>(const_cast<wchar_t*>(pText));
    MsbtAttribute* pAttr = LMS_GetAttribute(mMessageHandle, index);
    pInfo->mTalkSoundId = pAttr->_0;
    pInfo->mCameraType = pAttr->_1;
    pInfo->mTalkType = pAttr->_2;
    pInfo->mBalloonType = pAttr->_3;
    pInfo->mCameraSetID = pAttr->_4;
    pInfo->mMessageAreaArg = pAttr->_6;
    pInfo->mAlreadyReadFlagId = pAttr->_7;
    return true;
}

s32 MessageData::findEntryNodeIndex(const char* pLabel) const {
    if (mFlowHandle != nullptr) {
        return LMS_GetEntryNodeIndex(mFlowHandle, pLabel);
    }
    return -1;
}

void* MessageData::getNodeBranchPtr(u32 index) const {
    if (mFlowHandle != nullptr) {
        return LMS_GetNodeBranchPtr(mFlowHandle, index);
    }
    return nullptr;
}

void* MessageData::getNodeDataPtr(u32 index) const {
    if (mFlowHandle != nullptr) {
        return LMS_GetNodeDataPtr(mFlowHandle, index);
    }
    return nullptr;
}

u16 MessageData::getFlowNodeIndex(const TalkNode* pNode) const {
    if (mFlowHandle != nullptr) {
        return LMS_GetFlowNodeIndex(mFlowHandle, reinterpret_cast<const MsbfNode*>(pNode));
    }
    return 0;
}

MessageHolder::MessageHolder() {
    mSystemMessageData = nullptr;
    mGameMessageDatas = nullptr;
    mSceneCommonMessageDatas = nullptr;
    mZoneMessageDatas = nullptr;
    LMS_SetMemFuncs(allocLMSMemory, freeLMSMemory);
}

void MessageHolder::initSystemData() {
    mSystemMessageData = new MessageData("EmbeddedMessage.arc", "EmbeddedMessage.msbt");
}

void MessageHolder::initGameData() {
    MessageData** pSystemDatas = nullptr;
    s32 systemNum = 0;
    loadMessageDatasFromArchive(&pSystemDatas, &systemNum, sSystemMessageArc);

    MessageData** pLayoutDatas = nullptr;
    s32 layoutNum = 0;
    loadMessageDatasFromArchive(&pLayoutDatas, &layoutNum, sLayoutMessageArc);

    mGameMessageNum = systemNum + layoutNum;
    mGameMessageDatas = new MessageData*[mGameMessageNum];
    for (s32 i = 0; i < systemNum; i++) {
        mGameMessageDatas[i] = pSystemDatas[i];
    }
    for (s32 i = 0; i < layoutNum; i++) {
        mGameMessageDatas[systemNum + i] = pLayoutDatas[i];
    }
}

void MessageHolder::initSceneData() {
    char commonArc[0x100];
    char zonePath[0x100];
    char zoneArc[0x100];
    char fileName[0x100];

    MR::makeFileNameConsideringLanguage(commonArc, 0x100, sSceneCommonMessageArc);
    MR::mountArchive(commonArc, MR::getSceneHeapGDDR3(), false);
    loadMessageDatasFromArchive(&mSceneCommonMessageDatas, &mSceneCommonMessageNum, sSceneCommonMessageArc);

    mZoneMessageDatas = new MessageData*[MR::getZoneNum()];
    for (s32 i = 0; i < MR::getZoneNum(); i++) {
        const char* pZoneName = MR::getZoneNameFromZoneId(i);
        snprintf(zonePath, 0x100, "/MessageData/%s.arc", pZoneName);
        MR::makeFileNameConsideringLanguage(zoneArc, 0x100, zonePath);
        if (!MR::isFileExistOnDisc(zoneArc, true)) {
            mZoneMessageDatas[i] = nullptr;
        } else {
            snprintf(fileName, 0x100, "%s.msbt", pZoneName);
            MR::mountArchive(zoneArc, MR::getSceneHeapGDDR3(), true);
            mZoneMessageDatas[i] = new MessageData(zoneArc, fileName);
        }
    }
}

MessageData* MessageSystem::getSceneMessageData(const char* pName, s32 zoneId) {
    if (zoneId != -1) {
        return MR::getGameSystemObjHolder()->mMessageHolder->mZoneMessageDatas[zoneId];
    }

    for (s32 i = 0; i < MR::getGameSystemObjHolder()->mMessageHolder->mSceneCommonMessageNum; i++) {
        MessageData* pData = MR::getGameSystemObjHolder()->mMessageHolder->mSceneCommonMessageDatas[i];
        if (MessageSystem::isExistMessage(pName, pData)) {
            return pData;
        }
    }
    for (s32 i = 0; i < MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageNum; i++) {
        MessageData* pData = MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageDatas[i];
        if (MessageSystem::isExistMessage(pName, pData)) {
            return pData;
        }
    }
    return nullptr;
}

bool MessageSystem::getMessageDirect(TalkMessageInfo* pInfo, const char* pLabel, const MessageData* pData) {
    if (pData != nullptr) {
        return pData->getMessageDirect(pInfo, pLabel);
    }
    return false;
}
