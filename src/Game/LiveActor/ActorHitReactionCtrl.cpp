#include "Game/LiveActor/ActorHitReactionCtrl.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/LiveActor/ActorReactionHelper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    const char* sFileName = "ActorHitReactionCtrl";

}  // namespace

ActorHitReactionCtrlEntry::ActorHitReactionCtrlEntry()
    : mReactionName(nullptr), mEffectName(nullptr), mEffectBetweenOffset(0.0f), mSoundName(nullptr),
      mPadRumbleName(nullptr), mCameraShakeName(nullptr), mStopSceneFrame(-1), mIsStopSceneForHitEffect(true) {
}

ActorHitReactionCtrl* ActorHitReactionCtrl::tryCreate(LiveActor* pActor) {
    ResourceHolder* pHolder = MR::getResourceHolder(pActor);
    if (pHolder == NULL) {
        return NULL;
    }

    if (MR::isExistFileInArc(pHolder, "%s.bcsv", sFileName)) {
        return new ActorHitReactionCtrl(pActor);
    }

    return NULL;
}

ActorHitReactionCtrl::ActorHitReactionCtrl(LiveActor* pActor) : mActor(pActor), mEntryNum(0), mEntries(NULL) {
    JMapInfo* pCsv = MR::tryCreateCsvParser(MR::getResourceHolder(pActor), "%s.bcsv", sFileName);

    s32 num = 0;
    if (pCsv != NULL) {
        num = MR::getCsvDataElementNum(pCsv);
    }

    mEntryNum = num;
    mEntries = new ActorHitReactionCtrlEntry[mEntryNum];

    for (s32 i = 0; i < mEntryNum; i++) {
        ActorHitReactionCtrlEntry* pEntry = &mEntries[i];

        MR::getCsvDataStr(&pEntry->mReactionName, pCsv, "ReactionName", i);
        MR::getCsvDataStrOrNULL(&pEntry->mEffectName, pCsv, "EffectName", i);
        MR::getCsvDataF32(&pEntry->mEffectBetweenOffset, pCsv, "EffectBetweenOffset", i);
        MR::getCsvDataStrOrNULL(&pEntry->mSoundName, pCsv, "SoundName", i);
        MR::getCsvDataStrOrNULL(&pEntry->mPadRumbleName, pCsv, "PadRumbleName", i);
        MR::getCsvDataStrOrNULL(&pEntry->mCameraShakeName, pCsv, "CameraShakeName", i);
        MR::getCsvDataS32(&pEntry->mStopSceneFrame, pCsv, "StopSceneFrame", i);

        const char* pFlag = NULL;
        MR::getCsvDataStr(&pFlag, pCsv, "IsStopSceneForHitEffect", i);
        if (pFlag[0] == 'o') {
            pEntry->mIsStopSceneForHitEffect = true;
        }
    }
}

void ActorHitReactionCtrl::reactToHit(const char* pReactionName, const TVec3f* pPos, const HitSensor* pSensorA,
                                      const HitSensor* pSensorB) {
    ActorHitReactionCtrlEntry* pEntry;
    for (s32 i = 0; i < mEntryNum; i++) {
        pEntry = &mEntries[i];
        if (!MR::isEqualString(pEntry->mReactionName, pReactionName)) {
            continue;
        }

        if (pEntry->mEffectName != NULL) {
            if (pPos != NULL) {
                MR::emitEffectHit(mActor, *pPos, pEntry->mEffectName);
            } else if (pSensorA != NULL && pSensorB != NULL) {
                MR::emitEffectHitBetweenSensors(mActor, pSensorA, pSensorB, pEntry->mEffectBetweenOffset, pEntry->mEffectName);
            } else {
                MR::emitEffect(mActor, pEntry->mEffectName);
            }
        }

        if (pEntry->mSoundName != NULL) {
            MR::startActionSound(mActor, pEntry->mSoundName, -1, -1, -1);
        }

        tryRumblePadByName(this, pEntry->mPadRumbleName);
        tryShakeCameraByName(pEntry->mCameraShakeName);

        if (pEntry->mStopSceneFrame > 0) {
            if (pEntry->mIsStopSceneForHitEffect) {
                MR::stopSceneForDefaultHit(pEntry->mStopSceneFrame);
            } else {
                MR::stopScene(pEntry->mStopSceneFrame);
            }
        }
        return;
    }
}
