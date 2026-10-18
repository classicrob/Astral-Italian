#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudMeNameConverter.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/AudioLib/AudRemixMgr.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSpeakerWrap.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/AudioLib/CSSoundNameConverter.hpp"
#include "Game/GameAudio/AudBgmConductor.hpp"
#include "Game/GameAudio/AudEffectDirector.hpp"
#include "Game/GameAudio/AudSeKeeper.hpp"
#include "Game/GameAudio/AudStageBgmWrap.hpp"
#include "Game/GameAudio/AudTalkSoundData.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/RhythmLib/AudChordInfo.hpp"
#include "Game/RhythmLib/AudMeObject.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

bool SoundUtil_FORCE_MATCH_DATA(const char* pName) {
    return MR::isEqualString(pName, "SE_SY_READ_RIDDLE_S");
}

namespace {
};  // namespace

namespace MR {
    JAISoundHandle* startSystemSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startSoundParam(id, param2, param3);
    }

    JAISoundHandle* startSystemLevelSE(const char* pName, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startLevelSoundParam(pName, param2, param3);
    }

    JAISoundHandle* startSystemLevelSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startLevelSoundParam(id, param2, param3);
    }

    void stopSystemSE(const char* pName, u32 param2) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        stopSystemSE(id, param2);
    }

    void stopSystemSE(JAISoundID id, u32 param2) {
        AudWrap::getSystemSeObject()->stopSound(id, param2);
    }

    JAISoundHandle* startAtmosphereSE(const char* pName, s32 param2, s32 param3) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return AudWrap::getAtmosphereSeObject()->startSoundParam(id, param2, param3);
    }

    JAISoundHandle* startAtmosphereLevelSE(const char* pName, s32 param2, s32 param3) {
        return AudWrap::getAtmosphereSeObject()->startLevelSoundParam(pName, param2, param3);
    }

    JAISoundHandle* startSoundObject(AudSoundObject* pSoundObject, const char* pName) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return pSoundObject->startSound(id);
    }

    JAISoundHandle* startSoundObjectLevel(AudSoundObject* pSoundObject, const char* pName, s32 param3) {
        JAISoundHandle* pSoundHandle = pSoundObject->startLevelSound(pName);

        if (param3 > 0 && pSoundHandle != nullptr) {
            (*pSoundHandle)->updateLifeTime(param3);
        }

        return pSoundHandle;
    }

    JAISoundHandle* startSoundObjectLevel(AudSoundObject* pSoundObject, JAISoundID id, s32 param3) {
        JAISoundHandle* pSoundHandle = pSoundObject->startLevelSound(id);

        if (param3 > 0 && pSoundHandle != nullptr) {
            (*pSoundHandle)->updateLifeTime(param3);
        }

        return pSoundHandle;
    }

















    bool hasME() {
        AudChordInfo* pChordInfo = AudWrap::getSystem()->getChordInfo();

        return pChordInfo->mTable.mLoaded && pChordInfo->mCurChord != nullptr && pChordInfo->mCurScale != nullptr;
    }

    void startSystemME(const char* pName) {
        if (!hasME()) {
            return;
        }

        u32 id = AudSingletonHolder< AudMeNameConverter >::get()->getMeID(pName);

        AudWrap::getSystemMeObject()->startMe(id);
    }

    s32 getMapSoundCodeFoot(const LiveActor* pActor) {
        const Binder* pBinder = pActor->mBinder;
        if (pBinder == nullptr) {
            return -1;
        }

        s32 groundCode = -1;
        s32 wallCode = -1;
        s32 roofCode = -1;
        const Triangle* pGround = &pBinder->mGroundInfo.mParentTriangle;
        if (pGround != nullptr) {
            groundCode = getSoundCodeIndex(pGround->getAttributes());
        } else if (&pBinder->mWallInfo.mParentTriangle != nullptr) {
            wallCode = getSoundCodeIndex(pBinder->mWallInfo.mParentTriangle.getAttributes());
        } else if (&pBinder->mRoofInfo.mParentTriangle != nullptr) {
            roofCode = getSoundCodeIndex(pBinder->mRoofInfo.mParentTriangle.getAttributes());
        }

        if (groundCode >= 0) {
            return groundCode;
        }

        if (roofCode >= 0) {
            return roofCode;
        }

        if (wallCode >= 0) {
            return wallCode;
        }

        return -1;
    }


    void startTalkSound(u8 soundNo, const LiveActor* pActor) {
        JAISoundID id = AudTalkSoundData::getSoundIDFromTalkSoundNo(soundNo);
        if (!id.isAnonymous()) {
            if (pActor == nullptr) {
                startSystemSE(id);
            } else {
                startSound(pActor, id);
            }
        }
    }

    void startRemixSound(s32 melodyNo, s32 param2, f32 param3) {
        AudWrap::getRemixMgr()->getRemixNoteGroupDataFromMelodyNo(melodyNo);

        AudRemixSequencer* pRemixSequencer = AudWrap::getRemixSequencer();
        f32 playerSpeed = getPlayerVelocity()->length();

        if (playerSpeed < 3.0f) {
            playerSpeed = 3.0f;
        }

        pRemixSequencer->setTempo((playerSpeed * 3600.0f) / param3);

        RemixNoteGroupData* pRemixNoteGroupData = AudWrap::getRemixMgr()->getRemixNoteGroupDataFromMelodyNo(melodyNo);

        for (s32 i = 0; i < pRemixNoteGroupData->mTrackCount; i++) {
            RemixNoteTrackData* pRemixNoteTrackData = &pRemixNoteGroupData->mRemixTracks[i];

            pRemixSequencer->addNoteData(pRemixNoteTrackData, &pRemixNoteTrackData->_4[param2]);
        }
    }

    s32 getRemixMelodyNoteNum(s32 melodyNo) {
        return AudWrap::getRemixMgr()->getRemixNoteGroupDataFromMelodyNo(melodyNo)->mNoteCount;
    }

    void limitedSound(const char* pName, s32 param2) NO_INLINE {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        AudWrap::getSystem()->registerLimitedSound(id, param2);
    }






    void submitTrigSE() {
        AudSystem* pSystem = AudWrap::getSystem();

        pSystem->_82B = 1;
    }






    bool isPermitSE() {
        return !AudWrap::getSystem()->_82B && !AudWrap::getSystem()->_82C;
    }

    void setMicMtx(MtxPtr pMtx, s32 param2) {
        AudMicWrap::setMicMtx(pMtx, param2);
    }

    const TVec3f& getMicPos() {
        return AudMicWrap::getMicPos();
    }

    f32 getMicDistance(const LiveActor* pActor) {
        return PSVECDistance(&pActor->mPosition, &getMicPos());
    }

    void setSoundVolumeSetting(s32 param1, u32 param2) {
        AudWrap::getSystem()->setSeVolumeSet(param1, param2);
    }

    void recoverSoundVolumeSetting(u32 param1) {
        AudWrap::getSystem()->recoverSeVolumeSet(param1);
    }


    void startCSSound(const char* pCSName, const char* pSEName, s32 param3) {
        if (AudSpeakerWrap::isPlayable(-1)) {
            s32 id = AudSingletonHolder< CSSoundNameConverter >::get()->getSoundID(pCSName);

            if (id != -1) {
                AudSpeakerWrap::start(id, param3);
            }
        } else if (pSEName != nullptr) {
            startSystemSE(pSEName);
        }
    }

    void startCSSound2P(const char* pCSName, const char* pSEName) {
        if (isConnectedWPad(1)) {
            startCSSound(pCSName, pSEName, 1);
        }
    }

    JAISoundHandle* startSystemSE(const char* pName, s32 param2, s32 param3) {
        if (MR::isEqualString(pName, "SE_SY_READ_RIDDLE_S") && MR::isPlayerDead()) {
            return nullptr;
        }

        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return startSystemSE(id, param2, param3);
    }

    void startSpinHitSound(const LiveActor* pActor) {
        startCSSound("CS_SPIN_HIT", nullptr, 0, 0, 1.0f);
    }

    void startBlowHitSound(const LiveActor* pActor) {
        startSound(pActor, "SE_PM_SPIN_HIT");
    }

    void startDPDHitSound() {
    }

    void startDPDFreezeLevelSound(const LiveActor* pActor) {
        startLevelSound(pActor, "SE_EM_LV_DPD_POINT");
    }

    void start2PJumpAssistSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_JUMP");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_JUMP");
        }

        startCSSound2P("CS_DPD_JUMP", nullptr);
    }

    void start2PJumpAssistJustSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_JUMP_L");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_JUMP_L");
        }

        startCSSound2P("CS_DPD_JUMP_HIGH", nullptr);
    }

    void start2PAttackAssistSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_ATTACK");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_ATTACK");
        }

        startCSSound2P("CS_DPD_PUNCH", nullptr);
    }

    void limitedStarPieceHitSound() {
        limitedSound("SE_OJ_STAR_PIECE_HIT_ENEMY", 1);
    }


};  // namespace MR
