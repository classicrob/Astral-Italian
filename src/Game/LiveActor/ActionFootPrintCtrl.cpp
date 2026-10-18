#include "Game/LiveActor/ActionFootPrintCtrl.hpp"

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FootPrint.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

static const char* sActorName = "ActionFootPrintCtrl";

ActionFootPrintCtrl* ActionFootPrintCtrl::tryCreate(LiveActor* pActor) {
    if (MR::isExistFileInArc(MR::getResourceHolder(pActor), "%s.bcsv", sActorName)) {
        return new ActionFootPrintCtrl(pActor);
    }
    return NULL;
}

// NONMATCHING: loop index / bck-ctrl pointer swap registers (r28/r29/r30 vs target r28/r29/r30 order); same instructions otherwise
void ActionFootPrintCtrl::update() {
    if (mPlayingBckName != MR::getPlayingBckName(mActor)) {
        mPlayingBckName = MR::getPlayingBckName(mActor);
        updateInfoEnabled();
    }

    J3DFrameCtrl* pBckCtrl = MR::getBckCtrl(mActor);
    for (s32 i = 0; i < mInfoNum; i++) {
        ActionFootPrintInfo* pInfo = &mInfos[i];
        if (pInfo->mIsEnabled && pBckCtrl->checkPass(pInfo->mStartFrame)) {
            if (pInfo->mJointName != NULL) {
                MR::addFootPrintToJoint(mHandle, mActor, pInfo->mJointName, pInfo->mIsRight, 200.0f);
            } else {
                MR::addFootPrintToPos(mHandle, mActor, &mActor->mPosition, pInfo->mIsRight, 200.0f);
            }
        }
    }
}

ActionFootPrintCtrl::ActionFootPrintCtrl(LiveActor* pActor) : mActor(pActor), mPlayingBckName(NULL), mHandle(0), mInfoNum(0), mInfos(NULL) {
    JMapInfo* pCsv = MR::tryCreateCsvParser(MR::getResourceHolder(pActor), "%s.bcsv", sActorName);
    if (pCsv == NULL) {
        return;
    }

    s32 infoNum = MR::getCsvDataElementNum(pCsv);
    mInfoNum = infoNum;
    if (infoNum > 0) {
        mHandle = MR::createFootPrintHandle(mActor);
    }

    mInfos = new ActionFootPrintInfo[infoNum];
    for (s32 i = 0; i < mInfoNum; i++) {
        ActionFootPrintInfo* pInfo = &mInfos[i];

        MR::getCsvDataStr(&pInfo->mBckName, pCsv, "BckName", i);
        MR::getCsvDataF32(&pInfo->mStartFrame, pCsv, "BckStartFrame", i);
        MR::getCsvDataStrOrNULL(&pInfo->mJointName, pCsv, "JointName", i);

        const char* pIsRight = NULL;
        MR::getCsvDataStr(&pIsRight, pCsv, "IsRight", i);
        if (pIsRight != NULL && pIsRight[0] == 'o') {
            pInfo->mIsRight = 1;
        }
    }
}

// NONMATCHING: loop index / info pointer swap registers (target r27 = index, r28 = info); same instructions otherwise
void ActionFootPrintCtrl::updateInfoEnabled() {
    if (mPlayingBckName != NULL) {
        for (s32 i = 0; i < mInfoNum; i++) {
            ActionFootPrintInfo* pInfo = &mInfos[i];
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

ActionFootPrintInfo::ActionFootPrintInfo() : mBckName(NULL), mStartFrame(0.0f), mJointName(NULL), mIsRight(0), mIsEnabled(0) {
}
