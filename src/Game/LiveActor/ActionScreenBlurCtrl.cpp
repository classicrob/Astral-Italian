#include "Game/LiveActor/ActionScreenBlurCtrl.hpp"

#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

static const char* sActorName = "ActionScreenBlurCtrl";

ActionScreenBlurCtrl* ActionScreenBlurCtrl::tryCreate(const ModelManager* pModelManager) {
    if (MR::isExistFileInArc(pModelManager->getResourceHolder(), "%s.bcsv", sActorName)) {
        return new ActionScreenBlurCtrl(pModelManager);
    }
    return NULL;
}

void ActionScreenBlurCtrl::update() {
    ActionScreenBlurInfo* pInfo;
    if (mPlayingBckName != mModelManager->getPlayingBckName()) {
        mPlayingBckName = mModelManager->getPlayingBckName();
        updateInfoEnabled();
    }

    J3DFrameCtrl* pBckCtrl = mModelManager->getBckCtrl();
    for (s32 i = 0; i < mInfoNum; i++) {
        pInfo = &mInfos[i];
        if (pInfo->mIsEnabled && pBckCtrl->checkPass(pInfo->mStartFrame)) {
            MR::startCenterScreenBlur(pInfo->mTotalStep, pInfo->mOffset, pInfo->mAlpha, pInfo->mFadeInStep, pInfo->mFadeOutStep);
        }
    }
}

ActionScreenBlurCtrl::ActionScreenBlurCtrl(const ModelManager* pModelManager) : mModelManager(pModelManager), mPlayingBckName(NULL), mInfoNum(0), mInfos(NULL) {
    JMapInfo* pCsv = MR::tryCreateCsvParser(pModelManager->getResourceHolder(), "%s.bcsv", sActorName);
    if (pCsv == NULL) {
        return;
    }

    s32 infoNum = MR::getCsvDataElementNum(pCsv);
    mInfoNum = infoNum;
    mInfos = new ActionScreenBlurInfo[infoNum];
    for (s32 i = 0; i < mInfoNum; i++) {
        ActionScreenBlurInfo* pInfo = &mInfos[i];

        MR::getCsvDataStr(&pInfo->mBckName, pCsv, "BckName", i);
        MR::getCsvDataF32(&pInfo->mStartFrame, pCsv, "BckStartFrame", i);
        MR::getCsvDataS32(&pInfo->mTotalStep, pCsv, "BlurTotalStep", i);
        MR::getCsvDataF32(&pInfo->mOffset, pCsv, "BlurOffset", i);
        MR::getCsvDataS32(&pInfo->mAlpha, pCsv, "BlurAlpha", i);
        MR::getCsvDataS32(&pInfo->mFadeInStep, pCsv, "BlurFadeInStep", i);
        MR::getCsvDataS32(&pInfo->mFadeOutStep, pCsv, "BlurFadeOutStep", i);
    }

    MR::createCenterScreenBlur();
}

void ActionScreenBlurCtrl::updateInfoEnabled() {
    ActionScreenBlurInfo* pInfo;
    if (mPlayingBckName != NULL) {
        for (s32 i = 0; i < mInfoNum; i++) {
            pInfo = &mInfos[i];
            if (pInfo->mBckName != NULL) {
                if (MR::isEqualStringCase(mPlayingBckName, pInfo->mBckName)) {
                    pInfo->mIsEnabled = 1;
                } else {
                    pInfo->mIsEnabled = 0;
                }
            }
        }
    }
}

