#include "Game/LiveActor/ActionBgmCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

namespace {
    const char* sFileName = "ActionBgmCtrl";

    bool isCurrentStageScenario(const char* pStageName, s32 scenarioNo) {
        if (pStageName != NULL) {
            if (!MR::isEqualStageName(pStageName)) {
                return false;
            }
        }

        if (scenarioNo > 0) {
            if (MR::getCurrentScenarioNo() != scenarioNo) {
                return false;
            }
        }

        return true;
    }
}  // namespace

ActionBgmInfo::ActionBgmInfo()
    : mBckName(nullptr), mBckStartFrame(0.0f), mBgmName(nullptr), mBgmStateNo(-1), mIsBgmStop(false), mBgmFadeStep(60),
      mStageName(nullptr), mStageScenarioNo(-1), mIsBckMatch(false), mIsUpdated(false) {}

ActionBgmCtrl* ActionBgmCtrl::tryCreate(const ModelManager* pModelManager) {
    if (MR::isExistFileInArc(pModelManager->getResourceHolder(), "%s.bcsv", sFileName)) {
        return new ActionBgmCtrl(pModelManager);
    }

    return NULL;
}

void ActionBgmCtrl::update() {
    if (mPlayingBckName != mModelManager->getPlayingBckName()) {
        mPlayingBckName = mModelManager->getPlayingBckName();
        findAndUpdateBgm();
    }

    ActionBgmInfo* pInfo;
    J3DFrameCtrl* pBckCtrl = mModelManager->getBckCtrl();
    for (s32 i = 0; i < mInfoNum; i++) {
        pInfo = &mInfos[i];
        if (pInfo->mIsBckMatch && !pInfo->mIsUpdated && pBckCtrl->checkPass(pInfo->mBckStartFrame)) {
            if (isCurrentStageScenario(pInfo->mStageName, pInfo->mStageScenarioNo)) {
                updateBgm(*pInfo);
            }
        }
    }
}

ActionBgmCtrl::ActionBgmCtrl(const ModelManager* pModelManager)
    : mModelManager(pModelManager), mPlayingBckName(NULL), mInfoNum(0), mInfos(NULL) {
    JMapInfo* pCsv = MR::tryCreateCsvParser(pModelManager->getResourceHolder(), "%s.bcsv", sFileName);
    if (pCsv == NULL) {
        return;
    }

    mInfoNum = MR::getCsvDataElementNum(pCsv);
    mInfos = new ActionBgmInfo[mInfoNum];

    for (s32 i = 0; i < mInfoNum; i++) {
        ActionBgmInfo* pInfo = &mInfos[i];

        MR::getCsvDataStrOrNULL(&pInfo->mBckName, pCsv, "BckName", i);
        MR::getCsvDataF32(&pInfo->mBckStartFrame, pCsv, "BckStartFrame", i);
        MR::getCsvDataStrOrNULL(&pInfo->mBgmName, pCsv, "BgmName", i);
        MR::getCsvDataS32(&pInfo->mBgmStateNo, pCsv, "BgmStateNo", i);

        const char* pStopStr = NULL;
        MR::getCsvDataStr(&pStopStr, pCsv, "IsBgmStop", i);
        if (pStopStr != NULL && pStopStr[0] == 'o') {
            pInfo->mIsBgmStop = true;
        }

        MR::getCsvDataS32(&pInfo->mBgmFadeStep, pCsv, "BgmFadeStep", i);
        MR::getCsvDataStrOrNULL(&pInfo->mStageName, pCsv, "StageName", i);
        MR::getCsvDataS32(&pInfo->mStageScenarioNo, pCsv, "StageScenarioNo", i);
    }
}

void ActionBgmCtrl::findAndUpdateBgm() {
    if (mPlayingBckName == NULL) {
        return;
    }

    ActionBgmInfo* pInfo;
    for (s32 i = 0; i < mInfoNum; i++) {
        pInfo = &mInfos[i];
        if (pInfo->mBckName == NULL) {
            continue;
        }

        if (MR::isEqualStringCase(mPlayingBckName, pInfo->mBckName)) {
            pInfo->mIsBckMatch = true;
        } else {
            if (pInfo->mIsBckMatch && !pInfo->mIsUpdated) {
                if (isCurrentStageScenario(pInfo->mStageName, pInfo->mStageScenarioNo)) {
                    updateBgm(*pInfo);
                }
            }

            pInfo->mIsBckMatch = false;
            pInfo->mIsUpdated = false;
        }
    }
}

void ActionBgmCtrl::updateBgm(const ActionBgmInfo& rInfo) {
    if (rInfo.mBgmName != NULL) {
        MR::startStageBGM(rInfo.mBgmName, false);
    }

    if (rInfo.mBgmStateNo >= 0) {
        MR::setStageBGMState(rInfo.mBgmStateNo, rInfo.mBgmFadeStep);
    }

    if (rInfo.mIsBgmStop) {
        MR::stopStageBGM(rInfo.mBgmFadeStep);
    }

    rInfo.mIsUpdated = true;
}
