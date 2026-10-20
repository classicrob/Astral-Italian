#include "Game/System/MarioFaceShipAttentionConversationDirector.hpp"
#include "Game/MapObj/MarioFaceShipNpcHolder.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <cstdio>

static const char* sAttentionKindName = "注目会話";
static const char* sAttentionEventName = "顔惑星注目会話イベント";

MarioFaceShipAttentionConversationDirector::MarioFaceShipAttentionConversationDirector(const char* pName) : NameObj(pName) {
    mParams = nullptr;
    mParamNum = 0;
    mExecutors = nullptr;
    mExecutorNum = -1;
}

void MarioFaceShipAttentionConversationDirector::init(const JMapInfoIter&) {
    s32 infoNum = MarioFaceShipEventDataHolder::getEventInfoNum();
    mParamNum = MarioFaceShipEventDataHolder::getAttentionTalkNum();
    mParams = new Param*[mParamNum];

    s32 n = 0;
    for (s32 i = 0; i < infoNum; i++) {
        MarioFaceShipEventInfo* pInfo = MarioFaceShipEventDataHolder::getEventInfo(i);
        if (MarioFaceShipEventDataHolder::isAttentionTalkEvent(pInfo)) {
            mParams[n] = new Param();
            mParams[n]->mTalkIndex = pInfo->mObjId;
            mParams[n]->mIsArg4 = pInfo->isAttentionPriorTalk();
            mParams[n]->mIsArg5 = pInfo->mUseTimeKeepDemo != 0;
            mParams[n]->mKind = pInfo->mKind;
            mParams[n]->mStageName = pInfo->mName1;
            mParams[n]->mOrStageName = pInfo->mName2;
            mParams[n]->mScenarioNo = pInfo->mStarId1;
            mParams[n]->mOrScenarioNo = pInfo->mStarId2;
            mParams[n]->mPowerStarNum = pInfo->mThreshold;
            mParams[n]->mWatchedEventNo = pInfo->mWatchedEventNo;
            mParams[n]->mActorGroup = new LiveActorGroup("キャストグループ", 0x10);
            mParams[n]->mCast = pInfo->mCast;
            mParams[n++]->mCameraInfo = nullptr;
        }
    }

    calcSatisfiedNum();

    if (mExecutorNum > 0) {
        mExecutors = new MarioFaceShipAttentionConversationExecutor*[mExecutorNum];
        for (s32 i = 0; i < mExecutorNum; i++) {
            mExecutors[i] = new MarioFaceShipAttentionConversationExecutor("マリオ顔惑星注目会話デモ実行者");
            mExecutors[i]->initWithoutIter();
        }
    }
}

void MarioFaceShipAttentionConversationDirector::initAfterPlacement() {
    if (MR::isExistSceneObj(SceneObj_MarioFaceShipNpcHolder)) {
        Param* pParam;
        for (s32 i = 0; i < mParamNum; i++) {
            pParam = mParams[i];
            MR::registerMarioFaceShipNpcGroup(pParam->mActorGroup, pParam->mCast);
            bool found = false;
            for (s32 j = 0; j < pParam->mActorGroup->mObjNum; j++) {
                if (pParam->mActorGroup->getActor(j) == pParam->mTargetActor) {
                    found = true;
                }
            }
            if (!found && pParam->mTargetActor != nullptr) {
                pParam->mActorGroup->registerActor(pParam->mTargetActor);
            }
        }

        if (mExecutors != nullptr) {
            for (s32 i = 0; i < mExecutorNum; i++) {
                Param* pParam = nullptr;
                if (getSatisfiedParam(&pParam, i)) {
                    mExecutors[i]->initConversation(pParam, mMeister);
                }
            }
        }
    }
}

void MarioFaceShipAttentionConversationDirector::registerNpc(const MarioFaceShipConversationNpcInfo* pNpc, const JMapInfoIter& rIter) {
    for (s32 i = 0; i < mParamNum; i++) {
        Param* pParam = mParams[i];
        if (pParam->mIsArg5) {
            MarioFaceShipEventCast* pCast = pParam->mCast;
            for (s32 j = 0; j < pCast->mCastNum; j++) {
                if (MR::isEqualString(pNpc->mActor->mName, pCast->mCastNames[j]) &&
                    (pCast->mCastIds[j] == -1 || pCast->mCastIds[j] == MR::getJMapInfoDemoCastID(rIter))) {
                    char name[0x80];
                    if (!MR::isDemoCast(pNpc->mActor, name)) {
                        snprintf(name, 0x80, "%s%d", sAttentionEventName, pCast->mEventNo);
                        MR::tryRegisterDemoCast(pNpc->mActor, name, rIter);
                    }
                }
            }
        }

        if (pNpc->mEventNo >= 0 && pParam->mTalkIndex == pNpc->mEventNo) {
            if (pNpc->mFlag14) {
                pParam->mTargetActor = pNpc->mActor;
#ifdef GALAXY_NATIVE
                pParam->mDemoIndex = pNpc->mDemoName;
#else
                pParam->mDemoIndex = reinterpret_cast< s32 >(pNpc->mDemoName);
#endif
                pParam->mCameraInfo = pNpc->mCameraInfo;
            }
            return;
        }
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void MarioFaceShipAttentionConversationDirector::registerMeister(Meister* pMeister, const JMapInfoIter& rIter) {
    mMeister = pMeister;

    TVec3f offset;
    for (s32 i = 0; i < mParamNum; i++) {
        char talkName[0x100];
        snprintf(talkName, 0x100, "Meister_%dEventNotice", mParams[i]->mTalkIndex);
        // The target copies the talk offset with the paired-single copy inline.
#ifdef GALAXY_NATIVE
        offset = mMeister->getTalkOffset();
#else
        const register Vec* v_a = &mMeister->getTalkOffset();
        register Vec* v_b = &offset;
        register f32 a_x;
        register f32 b_x;
        asm {
            psq_l a_x, 0(v_a), 0, 0
            lfs b_x, 8(v_a)
            psq_st a_x, 0(v_b), 0, 0
            stfs b_x, 8(v_b)
        }
#endif
        mParams[i]->mTalkCtrl = MR::createTalkCtrlDirectOnRootNodeAutomatic(mMeister, rIter, talkName, offset, nullptr);

        if (mParams[i]->mIsArg5) {
            char castName[0x80];
            snprintf(castName, 0x80, "%s%d", sAttentionEventName, mParams[i]->mTalkIndex);
            MR::tryRegisterDemoCast(pMeister, castName, rIter);
            DemoFunction::registerDemoTalkMessageCtrlDirect(mMeister, mParams[i]->mTalkCtrl, castName);
        }

        MR::disableTalkCamera(mParams[i]->mTalkCtrl);
    }
}

bool MarioFaceShipAttentionConversationDirector::getSatisfiedParam(Param** ppParam, s32 index) {
    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }
    if (!GameSequenceFunction::hasResultPowerStar()) {
        return false;
    }
    const char* pStageName = GameSequenceFunction::getClearedStageName();
    s32 starId = GameSequenceFunction::getClearedPowerStarId();
    s32 starNum = MR::getPowerStarNum();

    s32 n = 0;
    for (s32 i = 0; i < mParamNum; i++) {
        Param* pParam = mParams[i];
        if (isParamSatisfied(pParam, pStageName, starId, starNum)) {
            if (index == n) {
                *ppParam = pParam;
                return true;
            }
            n++;
        }
    }
    return false;
}

void MarioFaceShipAttentionConversationDirector::calcSatisfiedNum() {
    mExecutorNum = 0;
    if (GameSequenceFunction::hasStageResultSequence()) {
        if (GameSequenceFunction::hasResultPowerStar()) {
            s32 n = 0;
            const char* pStageName = GameSequenceFunction::getClearedStageName();
            s32 starId = GameSequenceFunction::getClearedPowerStarId();
            s32 starNum = MR::getPowerStarNum();
            for (s32 i = 0; i < mParamNum; i++) {
                if (isParamSatisfied(mParams[i], pStageName, starId, starNum)) {
                    n++;
                }
            }
            mExecutorNum = n;
        }
    }
}

s32 MarioFaceShipAttentionConversationDirector::getTalkIndex(s32 index) const {
    return mExecutors[index]->mTalkIndex;
}

bool MarioFaceShipAttentionConversationDirector::makeTalkMessageId(s32 index, char* pDst, u32 size) {
    Param* pParam = nullptr;
    if (!getSatisfiedParam(&pParam, index)) {
        return false;
    }
    snprintf(pDst, size, "Meister_%dEventNoticeSub", pParam->mTalkIndex);
    return true;
}

bool MarioFaceShipAttentionConversationDirector::isParamSatisfied(const Param* pParam, const char* pStageName, s32 starId, s32 starNum) const {
    if (MarioFaceShipEventDataHolder::isKindStarOwned(pParam->mKind)) {
        bool res = false;
        if (pParam->mScenarioNo == starId && MR::isEqualString(pStageName, pParam->mStageName)) {
            res = true;
        }
        return res;
    }
    if (MarioFaceShipEventDataHolder::isKindThresholdReached(pParam->mKind)) {
        return pParam->mPowerStarNum == starNum;
    }
    if (MarioFaceShipEventDataHolder::isKindDualStarOwned(pParam->mKind)) {
        return MarioFaceShipEventDataHolder::isPairedStarNotOwnedStatic(pStageName, starId, pParam->mStageName, pParam->mScenarioNo, pParam->mOrStageName,
                                                                        pParam->mOrScenarioNo);
    }
    if (MarioFaceShipEventDataHolder::isKindStarAmountReached(pParam->mKind)) {
        return GameSequenceFunction::isClearedAll120PowerStarAtResult();
    }
    if (MarioFaceShipEventDataHolder::isKindObjectSet(pParam->mKind)) {
        return MarioFaceShipEventDataHolder::isSpecialEventSatisfiedStatic(pParam->mTalkIndex, pStageName, starId);
    }
    return false;
}

void MarioFaceShipAttentionConversationDirector::registerNpcToConversation(const MarioFaceShipConversationNpcInfo* pNpc, const JMapInfoIter& rIter) {
    MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->registerNpc(pNpc, rIter);
}

void MR::registerMeisterToAttentionConversation(Meister* pMeister, const JMapInfoIter& rIter) {
    MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->registerMeister(pMeister, rIter);
}

s32 MarioFaceShipAttentionConversationDirector::getConversationNum() {
    return MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->mExecutorNum;
}

void MarioFaceShipAttentionConversationDirector::tryStartDemo(s32 index) {
    MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->mExecutors[index]->tryStartDemo();
}

s32 MarioFaceShipAttentionConversationDirector::getTalkIndexByIndex(s32 index) {
    return MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->getTalkIndex(index);
}

bool MarioFaceShipAttentionConversationDirector::makeTalkMessageIdByIndex(s32 index, char* pDst, u32 size) {
    return MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->makeTalkMessageId(index, pDst, size);
}

bool MarioFaceShipAttentionConversationDirector::isWaiting(s32 index) {
    return MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->mExecutors[index]->isWaitingNerve();
}

bool MarioFaceShipAttentionConversationDirector::isDemoEnd(s32 index) {
    return MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector)->mExecutors[index]->isNerveDemoEnd();
}

const char* MarioFaceShipAttentionConversationDirector::getKindName() {
    return sAttentionKindName;
}

bool MarioFaceShipAttentionConversationDirector::makeEventName(char* pDst, u32 size, s32 index) {
    MR::getSceneObj< MarioFaceShipAttentionConversationDirector >(SceneObj_MarioFaceShipAttentionConversationDirector);
    snprintf(pDst, size, "%s%d", sAttentionEventName, index);
    return true;
}

MarioFaceShipAttentionConversationDirector::~MarioFaceShipAttentionConversationDirector() {
}

MarioFaceShipNormalConversationDirector::MarioFaceShipNormalConversationDirector(const char* pName) : NameObj(pName) {
    mParams = nullptr;
    mParamNum = 0;
    mSpecialExecutor = nullptr;
    mExecutors = nullptr;
    mExecutorNum = -1;
    mHasSpecial = false;
}

void MarioFaceShipNormalConversationDirector::init(const JMapInfoIter&) {
    s32 infoNum = MarioFaceShipEventDataHolder::getEventInfoNum();
    mParamNum = MarioFaceShipEventDataHolder::getNormalTalkNum();
    mParams = new Param*[mParamNum];

    s32 n = 0;
    for (s32 i = 0; i < infoNum; i++) {
        MarioFaceShipEventInfo* pInfo = MarioFaceShipEventDataHolder::getEventInfo(i);
        if (MarioFaceShipEventDataHolder::isNormalTalkEvent(pInfo)) {
            mParams[n] = new Param();
            mParams[n]->mTalkIndex = pInfo->mObjId;
            mParams[n]->mFlagType = pInfo->mKind;
            mParams[n]->mStageName = pInfo->mName1;
            mParams[n]->mOrStageName = pInfo->mName2;
            mParams[n]->mScenarioNo = pInfo->mStarId1;
            mParams[n]->mOrScenarioNo = pInfo->mStarId2;
            mParams[n]->mPowerStarNum = pInfo->mThreshold;
            mParams[n]->mWatchedEventNo = pInfo->mWatchedEventNo;
            mParams[n++]->mActorGroup = new LiveActorGroup("キャストグループ", 0x10);
        }
    }

    calcSatisfiedNum();

    mExecutors = new MarioFaceShipNormalConversationExecutor*[mExecutorNum];
    for (s32 i = 0; i < mExecutorNum; i++) {
        mExecutors[i] = new MarioFaceShipNormalConversationExecutor("マリオ顔惑星会話デモ実行者");
        mExecutors[i]->initWithoutIter();
    }

    mHasSpecial = true;
    Param* pSpecial = nullptr;
    if (getSpecialParam(&pSpecial)) {
        mSpecialExecutor = new MarioFaceShipNormalConversationExecutor("マリオ顔惑星会話デモ実行者");
        mSpecialExecutor->initWithoutIter();
    }
}

void MarioFaceShipNormalConversationDirector::initAfterPlacement() {
    if (mExecutors != nullptr) {
        for (s32 i = 0; i < mExecutorNum; i++) {
            Param* pParam = nullptr;
            if (getSatisfiedParam(&pParam, i)) {
                mExecutors[i]->initConversation(pParam, mMeister);
            }
        }
        if (mSpecialExecutor != nullptr) {
            Param* pParam = nullptr;
            if (getSpecialParam(&pParam)) {
                mSpecialExecutor->initConversation(pParam, mMeister);
            }
        }
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void MarioFaceShipNormalConversationDirector::registerMeister(Meister* pMeister, const JMapInfoIter& rIter) {
    mMeister = pMeister;

    TVec3f offset;
    for (s32 i = 0; i < mParamNum; i++) {
        char talkName[0x100];
        snprintf(talkName, 0x100, "Meister_%dEventTalk", mParams[i]->mTalkIndex);
        // The target copies the talk offset with the paired-single copy inline.
#ifdef GALAXY_NATIVE
        offset = mMeister->getTalkOffset();
#else
        const register Vec* v_a = &mMeister->getTalkOffset();
        register Vec* v_b = &offset;
        register f32 a_x;
        register f32 b_x;
        asm {
            psq_l a_x, 0(v_a), 0, 0
            lfs b_x, 8(v_a)
            psq_st a_x, 0(v_b), 0, 0
            stfs b_x, 8(v_b)
        }
#endif
        mParams[i]->mTalkCtrl = MR::createTalkCtrlDirectOnRootNodeAutomatic(mMeister, rIter, talkName, offset, nullptr);
        MR::disableTalkCamera(mParams[i]->mTalkCtrl);
    }
}

bool MarioFaceShipNormalConversationDirector::getSatisfiedParam(Param** ppParam, s32 index) {
    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }
    if (!GameSequenceFunction::hasResultPowerStar()) {
        return false;
    }
    const char* pStageName = GameSequenceFunction::getClearedStageName();
    s32 starId = GameSequenceFunction::getClearedPowerStarId();
    s32 starNum = MR::getPowerStarNum();

    s32 n = 0;
    for (s32 i = 0; i < mParamNum; i++) {
        Param* pParam = mParams[i];
        if (isParamSatisfied(pParam, pStageName, starId, starNum)) {
            if (index == n) {
                *ppParam = pParam;
                return true;
            }
            n++;
        }
    }
    return false;
}

bool MarioFaceShipNormalConversationDirector::getSpecialParam(Param** ppParam) {
    for (s32 i = 0; i < mParamNum; i++) {
        Param* pParam = mParams[i];
        if (MarioFaceShipEventDataHolder::isKindCometOpen(pParam->mFlagType) && MR::isAnyGalaxyAvailableWithStars()) {
            *ppParam = pParam;
            return true;
        }
    }
    return false;
}

void MarioFaceShipNormalConversationDirector::calcSatisfiedNum() {
    mExecutorNum = 0;
    if (GameSequenceFunction::hasStageResultSequence()) {
        if (GameSequenceFunction::hasResultPowerStar()) {
            s32 n = 0;
            const char* pStageName = GameSequenceFunction::getClearedStageName();
            s32 starId = GameSequenceFunction::getClearedPowerStarId();
            s32 starNum = MR::getPowerStarNum();
            for (s32 i = 0; i < mParamNum; i++) {
                if (isParamSatisfied(mParams[i], pStageName, starId, starNum)) {
                    n++;
                }
            }
            mExecutorNum = n;
        }
    }
}

s32 MarioFaceShipNormalConversationDirector::getTalkIndex(s32 index) const {
    return mExecutors[index]->mTalkIndex;
}

bool MarioFaceShipNormalConversationDirector::makeTalkMessageId(s32 index, char* pDst, u32 size) {
    Param* pParam = nullptr;
    if (!getSatisfiedParam(&pParam, index)) {
        return false;
    }
    snprintf(pDst, size, "Meister_%dEventTalkSub", pParam->mTalkIndex);
    return true;
}

bool MarioFaceShipNormalConversationDirector::makeSpecialTalkMessageId(s32, char* pDst, u32 size) {
    Param* pParam = nullptr;
    if (!getSpecialParam(&pParam)) {
        return false;
    }
    snprintf(pDst, size, "Meister_%dEventTalkSub", pParam->mTalkIndex);
    return true;
}

bool MarioFaceShipNormalConversationDirector::isParamSatisfied(const Param* pParam, const char* pStageName, s32 starId, s32 starNum) const {
    if (MarioFaceShipEventDataHolder::isKindStarOwned(pParam->mFlagType)) {
        bool res = false;
        if (pParam->mScenarioNo == starId && MR::isEqualString(pStageName, pParam->mStageName)) {
            res = true;
        }
        return res;
    }
    if (MarioFaceShipEventDataHolder::isKindThresholdReached(pParam->mFlagType)) {
        return pParam->mPowerStarNum == starNum;
    }
    if (MarioFaceShipEventDataHolder::isKindDualStarOwned(pParam->mFlagType)) {
        return MarioFaceShipEventDataHolder::isPairedStarNotOwnedStatic(pStageName, starId, pParam->mStageName, pParam->mScenarioNo, pParam->mOrStageName,
                                                                        pParam->mOrScenarioNo);
    }
    if (MarioFaceShipEventDataHolder::isKindStarAmountReached(pParam->mFlagType)) {
        return GameSequenceFunction::isClearedAll120PowerStarAtResult();
    }
    return false;
}

void MR::registerMeisterToNormalConversation(Meister* pMeister, const JMapInfoIter& rIter) {
    MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->registerMeister(pMeister, rIter);
}

s32 MarioFaceShipNormalConversationDirector::hasSpecialConversation() {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mSpecialExecutor != nullptr;
}

s32 MarioFaceShipNormalConversationDirector::getConversationNum() {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mExecutorNum;
}

s32 MarioFaceShipNormalConversationDirector::getTalkIndexByIndex(s32 index) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->getTalkIndex(index);
}

s32 MarioFaceShipNormalConversationDirector::getSpecialTalkIndex(s32) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mSpecialExecutor->mTalkIndex;
}

void MarioFaceShipNormalConversationDirector::tryStartDemo(s32 index) {
    MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mExecutors[index]->tryStartDemo();
}

void MarioFaceShipNormalConversationDirector::tryStartSpecialDemo(s32) {
    MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mSpecialExecutor->tryStartDemo();
}

bool MarioFaceShipNormalConversationDirector::makeTalkMessageIdByIndex(s32 index, char* pDst, u32 size) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->makeTalkMessageId(index, pDst, size);
}

bool MarioFaceShipNormalConversationDirector::makeSpecialTalkMessageIdByIndex(s32 index, char* pDst, u32 size) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->makeSpecialTalkMessageId(index, pDst, size);
}

bool MarioFaceShipNormalConversationDirector::isWaiting(s32 index) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mExecutors[index]->isDemoActive();
}

bool MarioFaceShipNormalConversationDirector::isSpecialWaiting(s32) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mSpecialExecutor->isDemoActive();
}

bool MarioFaceShipNormalConversationDirector::isDemoEnd(s32 index) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mExecutors[index]->isDemoEnd();
}

bool MarioFaceShipNormalConversationDirector::isSpecialDemoEnd(s32) {
    return MR::getSceneObj< MarioFaceShipNormalConversationDirector >(SceneObj_MarioFaceShipNormalConversationDirector)->mSpecialExecutor->isDemoEnd();
}

MarioFaceShipNormalConversationDirector::~MarioFaceShipNormalConversationDirector() {
}

MarioFaceShipEventCast::MarioFaceShipEventCast() {
    mEventNo = -1;
    mCastNum = 0;
    mCastNames = nullptr;
    mCastIds = nullptr;
}
