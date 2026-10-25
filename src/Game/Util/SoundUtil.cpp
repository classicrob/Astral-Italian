#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudMeNameConverter.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/AudioLib/AudRemixMgr.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSpeakerWrap.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudSeLifeTable.hpp"
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



namespace {
    inline void stopNamedSound(AudSoundObject* pObject, const char* pName, u32 frames) {
        JAISoundID id = AudSingletonHolder<AudSoundNameConverter>::get()->getSoundID(pName);
        pObject->stopSound(id, frames);
    }
};  // namespace

namespace MR {
    bool isDestroySceneKeepAllSound() {
        return AudWrap::getSystem()->mIsDestroySceneKeepAllSound;
    }

    void onDestroySceneKeepAllSound() {
        AudSystem* pSystem = AudWrap::getSystem();

        pSystem->mIsDestroySceneKeepAllSound = true;
    }

    void offDestroySceneKeepAllSound() {
        AudSystem* pSystem = AudWrap::getSystem();

        pSystem->mIsDestroySceneKeepAllSound = false;
    }

    void actorSoundMovement(LiveActor* pActor) {
        ActorSoundHolder* pSoundHolder = pActor->mSoundHolder;
        if (pSoundHolder == nullptr) {
            return;
        }

        AudAnmSoundObject* pSoundObject = getActorSoundObject(pActor);
        updateMapSoundCodeFoot(pActor);
        pSoundHolder->update();

        if (pActor->mModelManager != nullptr) {
            ResourceHolder* pResourceHolder = getResourceHolder(pActor);

            if (pResourceHolder->mMotionResTable->mCount != 0) {
                f32 frame = getBckCtrl(pActor)->mFrame;

                pSoundObject->update(frame);
            }
        }

        pSoundObject->process();
    }

    void startBasWithUpdateFoot(LiveActor* pActor, const char* pName, bool param3, bool param4, f32 param5, f32 param6) {
        startBas(pActor, pName, param3, param5, param6, param4);
    }

    bool startBas(const LiveActor* pActor, const char* pName, bool param3, f32 param4, f32 param5, bool updateFoot) {
        if (pActor->mSoundHolder == nullptr) {
            return false;
        }

        ResourceHolder* pResourceHolder = getResourceHolder(pActor);
        JAUSoundAnimation* pSoundAnimation = nullptr;

        if (pName != nullptr && pResourceHolder->mBasResTable->isExistRes(pName)) {
            pSoundAnimation = static_cast< JAUSoundAnimation* >(pResourceHolder->mBasResTable->getRes(pName));
        }

        if (updateFoot) {
            updateMapSoundCodeFoot(pActor);
        }

        if (pSoundAnimation != nullptr) {
            getActorSoundObject(pActor)->startAnimation(pSoundAnimation, param3, param4, param5);
            return true;
        }

        getActorSoundObject(pActor)->removeAnimation();
        return false;
    }

    void skipBas(LiveActor* pActor, f32 param2) {
        getActorSoundObject(pActor)->skip(param2);
    }

    JAISoundHandle* startSystemSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startSoundParam(id, param2, param3);
    }

    JAISoundHandle* startSystemLevelSE(const char* pName, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startLevelSoundParam(pName, param2, param3);
    }

    JAISoundHandle* startSystemLevelSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getSystemSeObject()->startLevelSoundParam(id, param2, param3);
    }

    JAISoundHandle* startSystemLevelSE(JAISoundID id, s32 param2, s32 param3, s32 param4) {
        return startSoundObjectLevelParam(AudWrap::getSystemSeObject(), id, param2, param3, param4);
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

    JAISoundHandle* startAtmosphereSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getAtmosphereSeObject()->startSoundParam(id, param2, param3);
    }

    JAISoundHandle* startAtmosphereLevelSE(JAISoundID id, s32 param2, s32 param3) {
        return AudWrap::getAtmosphereSeObject()->startLevelSoundParam(id, param2, param3);
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

    JAISoundHandle* startSoundObjectParam(AudSoundObject* pSoundObject, JAISoundID id, s32 param3, s32 param4) {
        return pSoundObject->startSoundParam(id, param3, param4);
    }

    JAISoundHandle* startSoundObjectLevelParam(AudSoundObject* pSoundObject, const char* pName, s32 param3, s32 param4, s32 param5) {
        JAISoundHandle* pSoundHandle = pSoundObject->startLevelSoundParam(pName, param3, param4);

        if (param5 > 0 && pSoundHandle != nullptr) {
            (*pSoundHandle)->updateLifeTime(param5);
        }

        return pSoundHandle;
    }

    JAISoundHandle* startSoundObjectLevelParam(AudSoundObject* pSoundObject, JAISoundID id, s32 param3, s32 param4, s32 param5) {
        JAISoundHandle* pSoundHandle = pSoundObject->startLevelSoundParam(id, param3, param4);

        if (param5 > 0 && pSoundHandle != nullptr) {
            (*pSoundHandle)->updateLifeTime(param5);
        }

        return pSoundHandle;
    }

    inline JAISoundHandle* startSoundObjectParam(AudSoundObject* pSoundObject, const char* pName, s32 param3, s32 param4) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);
        return pSoundObject->startSoundParam(id, param3, param4);
    }

    JAISoundHandle* startSound(const LiveActor* pActor, const char* pName, s32 param3, s32 param4) {
        updateMapSoundCodeFoot(pActor);

        if (param3 != -1) {
            AudAnmSoundObject* pSoundObject = getActorSoundObject(pActor);

            return startSoundObjectParam(pSoundObject, pName, param3, param4);
        } else {
            AudAnmSoundObject* pSoundObject = getActorSoundObject(pActor);

            return startSoundObject(pSoundObject, pName);
        }
    }

    JAISoundHandle* startSound(const LiveActor* pActor, JAISoundID id, s32 param3, s32 param4) {
        updateMapSoundCodeFoot(pActor);

        if (param3 != -1) {
            u32 soundId = id;
            AudAnmSoundObject* object = getActorSoundObject(pActor);
            return object->startSoundParam(JAISoundID(soundId), param3, param4);
        } else {
            u32 soundId = id;
            AudAnmSoundObject* object = getActorSoundObject(pActor);
            return object->startSound(JAISoundID(soundId));
        }
    }

    JAISoundHandle* startLevelSound(const LiveActor* pActor, const char* pName, s32 param3, s32 param4, s32 param5) {
        updateMapSoundCodeFoot(pActor);

        if (param3 != -1) {
            return startSoundObjectLevelParam(getActorSoundObject(pActor), pName, param3, param4, param5);
        } else {
            return startSoundObjectLevel(getActorSoundObject(pActor), pName, param5);
        }
    }

    JAISoundHandle* startLevelSound(const LiveActor* pActor, JAISoundID id, s32 param3, s32 param4, s32 param5) {
        updateMapSoundCodeFoot(pActor);

        if (param3 != -1) {
            return startSoundObjectLevelParam(getActorSoundObject(pActor), id, param3, param4, param5);
        } else {
            return startSoundObjectLevel(getActorSoundObject(pActor), id, param5);
        }
    }

    // NONMATCHING: same non-const getSoundID symbol as startSound(char)
    void stopSound(const LiveActor* pActor, const char* pName, u32 param3) {
        AudAnmSoundObject* pSoundObject = getActorSoundObject(pActor);
        stopNamedSound(pSoundObject, pName, param3);

        AudSoundObject* pChild;
        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            pChild = getActorSoundObjectAt(pActor, i);
            stopNamedSound(pChild, pName, param3);
        }
    }

    void stopSound(const LiveActor* pActor, JAISoundID id, u32 param3) {
        u32 soundId = id;
        AudAnmSoundObject* object = getActorSoundObject(pActor);
        object->stopSound(JAISoundID(soundId), param3);

        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            u32 childId = id;
            AudSoundObject* child = getActorSoundObjectAt(pActor, i);
            child->stopSound(JAISoundID(childId), param3);
        }
    }

    void setSeVersion(const LiveActor* pActor, u32 version) {
        getActorSoundObject(pActor)->mSeVersion = version;

        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            getActorSoundObjectAt(pActor, i)->mSeVersion = version;
        }
    }

    // NONMATCHING: same non-const getSoundID symbol as startSound(char)
    void releaseSoundHandle(const LiveActor* pActor, const char* pName) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        getActorSoundObject(pActor)->releaseHandle(id);

        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            getActorSoundObjectAt(pActor, i)->releaseHandle(id);
        }
    }

    bool setSoundPositionPtr(const LiveActor* pActor, TVec3f* pPosition) {
        return pActor->mSoundHolder->setSoundPositionPtr(pPosition);
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

#ifdef GALAXY_NATIVE
    // DOL 80042230 / 800422F0 (unsplit, beside hasME and startSystemME).
    u32 getMEID(const char* pName) {
        return AudSingletonHolder< AudMeNameConverter >::get()->getMeID(pName);
    }

    void fn_800422F0(u32 id) {
        if (!hasME()) {
            return;
        }

        AudWrap::getSystemMeObject()->startMe(id);
    }
#endif

    inline s32 calcFootSoundCode(const LiveActor* pActor) {
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

    void updateMapSoundCodeFoot(const LiveActor* pActor) {
        s32 code = calcFootSoundCode(pActor);
        getActorSoundObject(pActor)->setMapCode(code);
        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            getActorSoundObjectAt(pActor, i)->setMapCode(code);
        }
    }

    void setMapSondCodeGravity(const LiveActor* pActor, s32 code) {
        if (pActor->mSoundHolder == nullptr) {
            return;
        }

        getActorSoundObject(pActor)->setMapCodeExtra(code);

        s32 count = getActorSoundObjectCount(pActor);
        for (s32 i = 0; i < count; i++) {
            getActorSoundObjectAt(pActor, i)->setMapCodeExtra(code);
        }
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

            pRemixSequencer->addNoteData(pRemixNoteTrackData, &pRemixNoteTrackData->mNotes[param2]);
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

        pSystem->mIsStartSoundBlocked = 0;
    }






    bool isPermitSE() {
        return !AudWrap::getSystem()->mIsStartSoundBlocked && !AudWrap::getSystem()->mIsStartLevelSoundBlocked;
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

    void setSeVolumeSetLevel(s32 param1, s32 param2) {
        AudWrap::getSystem()->setSeVolumeSetLevel(param1, param2, 2);
    }

    void setSoundField9A4(bool param1) {
        AudWrap::getSystem()->setField9A4(param1);
    }

    void setAudioSystemField9A0(s32 param1) {
        AudWrap::getSystem()->setField9A0(param1);
    }

    void setAudioEffectType(s32 param1, s32 param2, s32 param3) {
        MR::getSceneObj< AudEffectDirector >(0x12)->setEffectType(param1, param2, param3);
    }

    void resetAudioEffect() {
        MR::getSceneObj< AudEffectDirector >(0x12)->restoreEffectType();
    }

    JAISoundID getSoundID(const char* pName) {
        return AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);
    }

    void setSeVolumeSetLevelForMenu() {
        AudWrap::getSystem()->setSeVolumeSetLevel(3, 30, 2);
    }

    void addActorToSoundObjHolder(LiveActor* pActor) {
        getActorSoundObject(pActor)->addToSoundObjHolder();
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
        startLevelSound(pActor, "SE_SV_LV_SUPPORTTICO_PREVT_ENEMY");
        startLevelSound(pActor, "SE_EM_LV_DPD_POINT");
    }

    void start2PJumpAssistSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_JUMP");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_JUMP");
        }

        startCSSound2P("CS_DPD_JUMP", nullptr, 0, 1.0f);
    }

    void start2PJumpAssistJustSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_JUMP_L");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_JUMP_L");
        }

        startCSSound2P("CS_DPD_JUMP_HIGH", nullptr, 0, 1.0f);
    }

    void start2PAttackAssistSound() {
        if (hasME()) {
            startSystemME("ME_2P_ASSIST_ATTACK");
        } else {
            startSystemSE("SE_SY_2P_ASSIST_ATTACK");
        }

        startCSSound2P("CS_DPD_PUNCH", nullptr, 0, 1.0f);
    }

    void limitedStarPieceHitSound() {
        limitedSound("SE_OJ_STAR_PIECE_HIT_ENEMY", 1);
    }

    void startStarGetJingle() {
        startSystemSE("SE_SY_STAR_GET_JINGLE", -1, -1);
    }

    void startGrandStarGetJingle() {
        startSystemSE("SE_SY_GRAND_STAR_GET_JINGLE", -1, -1);
    }

    void fn_8005BAD0() {
        AudWrap::getSystem()->registerLimitedSound(JAISoundID(0xF5), 0x258);
    }


    // NONMATCHING: target preserves the unused boolean argument across getSystem().
    void stopAllSe(bool stop) {
        AudWrap::getSystem()->stopAllSe(stop);
    }

    void stopSeCategories(u32 categories) {
        AudWrap::getSystem()->stopSeCategories(categories);
    }

    AudSeLifeEntry* fn_8005BC60(s32 limit) {
        return AudWrap::getSystem()->mSeLifeTable->allocEntry(limit);
    }
};  // namespace MR
