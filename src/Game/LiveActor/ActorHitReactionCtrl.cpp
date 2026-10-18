#include "Game/LiveActor/ActorHitReactionCtrl.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    const char* sFileName = "ActorHitReactionCtrl";

    // Camera shake names are Shift-JIS: 強震 / 震 / 中震 / 中 / 中弱 / 弱 / 強弱.
    const char* sShakeVeryStrong = "\x8d\xc5\x8b\xad";
    const char* sShakeStrong = "\x8b\xad";
    const char* sShakeNormalStrong = "\x92\x86\x8b\xad";
    const char* sShakeNormal = "\x92\x86";
    const char* sShakeNormalWeak = "\x92\x86\x8e\xe3";
    const char* sShakeWeak = "\x8e\xe3";
    const char* sShakeVeryWeak = "\x8d\xc5\x8e\xe3";
}  // namespace

ActorHitReactionCtrl* ActorHitReactionCtrl::tryCreate(LiveActor* pActor) {
    if (MR::getResourceHolder(pActor) == NULL) {
        return NULL;
    }

    if (MR::isExistFileInArc(MR::getResourceHolder(pActor), "%s.bcsv", sFileName)) {
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
        if (pFlag != NULL && pFlag[0] == 'o') {
            pEntry->mIsStopSceneForHitEffect = true;
        }
    }
}

// Tries to rumble the pad with the named pattern. Returns false if there is no name.
static bool tryRumblePadPattern(const ActorHitReactionCtrl* pCtrl, const char* pName) {
    if (pName == NULL) {
        return false;
    }

    MR::tryRumblePad(pCtrl, pName, 0);
    return true;
}

// Shakes the camera with the strength named by pName. Returns false if there is no name.
static bool tryCameraShakeByName(const char* pName) {
    if (pName == NULL) {
        return false;
    }

    if (MR::isEqualStringCase(pName, sShakeVeryStrong)) {
        MR::shakeCameraVeryStrong();
    } else if (MR::isEqualStringCase(pName, sShakeStrong)) {
        MR::shakeCameraStrong();
    } else if (MR::isEqualStringCase(pName, sShakeNormalStrong)) {
        MR::shakeCameraNormalStrong();
    } else if (MR::isEqualStringCase(pName, sShakeNormal)) {
        MR::shakeCameraNormal();
    } else if (MR::isEqualStringCase(pName, sShakeNormalWeak)) {
        MR::shakeCameraNormalWeak();
    } else if (MR::isEqualStringCase(pName, sShakeWeak)) {
        MR::shakeCameraWeak();
    } else if (MR::isEqualStringCase(pName, sShakeVeryWeak)) {
        MR::shakeCameraVeryWeak();
    }

    return true;
}

void ActorHitReactionCtrl::reactToHit(const char* pReactionName, const TVec3f* pPos, const HitSensor* pSensorA,
                                      const HitSensor* pSensorB) {
    for (s32 i = 0; i < mEntryNum; i++) {
        ActorHitReactionCtrlEntry* pEntry = &mEntries[i];
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

        tryRumblePadPattern(this, pEntry->mPadRumbleName);
        tryCameraShakeByName(pEntry->mCameraShakeName);

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
