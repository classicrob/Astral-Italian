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
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

namespace {
    AudBgmConductor* getAudBgmConductor() {
        return MR::getSceneObj< AudBgmConductor >(SceneObj_AudBgmConductor);
    }
};  // namespace

namespace MR {
    JAISoundHandle* startStageBGM(const char* pName, bool param2) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return AudWrap::startStageBgm(id, param2, -1);
    }

    JAISoundHandle* startStageBGM(u32 id, bool param2) {
        return AudWrap::startStageBgm(id, param2, -1);
    }

    JAISoundHandle* startStageBGMIfNotPlaying(const char* pName, bool param2) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        if (AudWrap::getBgmMgr()->mCurrentBGM[0] == id && isPlayingStageBgm()) {
            return nullptr;
        }

        return AudWrap::startStageBgm(id, param2, -1);
    }

    JAISoundHandle* startStageBGMFromStageName(const char* pSceneName, const char* pStageName, s32 scenarioNo, bool checkPlaying, bool lock) {
        JAISoundID id = AudStageBgmWrap::changeStageNameToSoundID(pSceneName, pStageName, scenarioNo);

        if (id.isAnonymous()) {
            return nullptr;
        }

        if (checkPlaying && AudWrap::getBgmMgr()->mCurrentBGM[0] == id && isPlayingStageBgm()) {
            return nullptr;
        }

        JAISoundHandle* pSoundHandle = AudWrap::startStageBgm(id, lock, -1);

        if (pSoundHandle != nullptr) {
            AudSceneMgr* pSceneMgr = AudWrap::getSceneMgr();

            pSceneMgr->_4 = 1;
        }

        return pSoundHandle;
    }

    void stopStageBGM(u32 param1) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->stop(param1);
        }
    }

    s32 unlockStageBGM() {
        s32 result = 0;
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            result = pStageBgm->playAfterPrepared();
        }

        return result;
    }

    void moveVolumeStageBGM(f32 param1, u32 param2) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->moveVolume(param1, param2);
        }
    }

    void moveVolumeStageBGMForNoteFairy(f32 param1, u32 param2) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->moveVolumeForNoteFairy(param1, param2);
        }
    }

    void requestStageBGMVolumeDown(s32 param1, bool param2) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->volDownWithLevel(param1, param2);
        }
    }

    void setBgmMgr954(s32 param1) {
        AudWrap::getBgmMgr()->_954 = param1;
    }

    // fn_8001AFE0
    void resetStageBGMInterrupted() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->resetInterrupted();
        }
    }

    void startCurrentStageBGM(bool param1) {
        s32 scenarioNo = getCurrentScenarioNo();

        startStageBGMFromStageName("Game", getCurrentStageName(), scenarioNo, param1, false);
    }

    JAISoundHandle* startLastStageBGM() {
        return AudWrap::startLastStageBgm();
    }

    bool isPlayingStageBgm() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            return !pStageBgm->isStopping();
        }

        return false;
    }

    bool isPlayingStageBgmID(u32 id) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr && !pStageBgm->isStopping() && pStageBgm->getSoundID() == id) {
            return true;
        }

        return false;
    }

    bool isPlayingStageBgmName(const char* pName) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return isPlayingStageBgmID(id);
    }

    bool isPreparedStageBgm() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            return pStageBgm->isPreparedPlay();
        }

        return false;
    }

    void setStageBGMState(s32 state, u32 time) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->changeTrackMuteState(state, time);
        }
    }

    void setStageBGMStateBit(u32 param1) {
        AudBgmConductor* pBgmConductor = ::getAudBgmConductor();

        if (pBgmConductor != nullptr) {
            pBgmConductor->setStateBit(param1);
        }
    }

    void moveStageBGMTrackVolume(s32 param1, f32 param2, u32 param3) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->moveTrackVolume(param2, param1, param3);
        }
    }

    // fn_8001B2A0
    void setStageBGMVolumeState(s32 param1, u32 param2, f32 param3) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->moveTrackVolume(param3, param1, param2);
        }
    }

    u32 getCurrentStageBgmId() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            return pStageBgm->getBgmId();
        }

        return -1;
    }

    const char* getPlayingStageBgmId() {
        u32 id = getCurrentStageBgmId();

        if (id == -1) {
            return nullptr;
        }

        JAISoundID soundID(id);

        return AudSingletonHolder< AudSoundNameConverter >::get()->getSoundNameID(soundID);
    }

    s32 getStageBgmState() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            return pStageBgm->getMuteStateIndex();
        }

        return -1;
    }

    void getStageBgmJMapArgs(s32* pArg4, s32* pArg5, const JMapInfoIter& rIter) {
        getJMapInfoArg4NoInit(rIter, pArg4);
        getJMapInfoArg5NoInit(rIter, pArg5);
    }

    // was fn_8001B430
    void fadeOutStageBGMForIndex(s32 index, s32 volumeIndex) {
        if (index < 0) {
            return;
        }

        s32 scenarioNo = getCurrentScenarioNo();
        JAISoundID id = AudStageBgmWrap::getStageBgmIDAtIndex(getCurrentStageName(), scenarioNo, index);

        if (id.isAnonymous()) {
            return;
        }

        if (AudWrap::getBgmMgr()->mCurrentBGM[0] != id) {
            AudBgm* pStageBgm = AudWrap::getStageBgm();
            if (pStageBgm != nullptr) pStageBgm->stop(60);
            return;
        }
        if (volumeIndex < 0) return;
        s32 volumeID = AudStageBgmWrap::getStageBgmVolumeIDAtIndex(getCurrentStageName(), volumeIndex);
        if (volumeID < 0) return;
        AudBgm* pStageBgm = AudWrap::getStageBgm();
        if (pStageBgm != nullptr) pStageBgm->changeTrackMuteState(volumeID, 0xF0);
    }

    // was fn_8001B510
    void startStageBGMForIndex(s32 index, s32 volumeIndex) {
        if (index < 0) {
            return;
        }

        s32 scenarioNo = getCurrentScenarioNo();
        JAISoundID id = AudStageBgmWrap::getStageBgmIDAtIndex(getCurrentStageName(), scenarioNo, index);

        if (id.isAnonymous()) {
            return;
        }

        if (!(AudWrap::getBgmMgr()->mCurrentBGM[0] == id && isPlayingStageBgm())) {
            AudWrap::startStageBgm(id, false, -1);
        }

        if (volumeIndex < 0) {
            return;
        }

        s32 volumeID = AudStageBgmWrap::getStageBgmVolumeIDAtIndex(getCurrentStageName(), volumeIndex);
        if (volumeID < 0) return;
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->changeTrackMuteState(volumeID, 0);
        }
    }

    JAISoundHandle* startSubBGM(const char* pName, bool param2) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return AudWrap::startSubBgm(id, param2);
    }

    void stopSubBGM(u32 param1) {
        AudBgm* pSubBgm = AudWrap::getSubBgm();

        if (pSubBgm != nullptr) {
            pSubBgm->stop(param1);
        }
    }

    void moveVolumeSubBGM(f32 param1, u32 param2) {
        AudBgm* pSubBgm = AudWrap::getSubBgm();

        if (pSubBgm != nullptr) {
            pSubBgm->moveVolume(param1, param2);
        }
    }

    bool isPlayingSubBgmID(u32 id) {
        AudBgm* pSubBgm = AudWrap::getSubBgm();

        if (pSubBgm != nullptr && !pSubBgm->isStopping() && pSubBgm->getSoundID() == id) {
            return true;
        }

        return false;
    }

    JAISoundHandle* startBossBGM(s32 id) {
        JAISoundHandle* pSoundHandle = nullptr;

        // Case order follows the target jump table (SMG2 names, not the Petari BossBgmID enum names).
        switch (id) {
        case 0:
            pSoundHandle = startStageBGM("MBGM_SMG2_BOSS_08A", 0);
            break;
        case 1:
            pSoundHandle = startStageBGM("MBGM_SMG2_BOSS_01A", 0);
            break;
        case 2:
            pSoundHandle = startStageBGM("MBGM_SMG2_BOSS_03A", 0);
            break;
        case 3:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_03A", 0);
            break;
        case 4:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_03B", 0);
            break;
        case 5:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_05A", 0);
            break;
        case 6:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_05B", 0);
            break;
        case 7:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_01A", 0);
            break;
        case 8:
            pSoundHandle = startStageBGM("MBGM_SMG_BOSS_01B", 0);
            break;
        }

        return pSoundHandle;
    }

    JAISoundHandle* startDepartBGM() {
        return startStageBGMIfNotPlaying("BGM_SMG2_EV_DEPART01", false);
    }

    JAISoundHandle* startGliderBGM() {
        if (isEqualStageName("JungleGliderGalaxy")) {
            return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("MBGM_GLIDER"), false, -1);
        }

        if (isEqualStageName("ChallengeGliderGalaxy")) {
            return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("MBGM_SMG2_GALAXY_28"), false, -1);
        }

        return nullptr;
    }

    JAISoundHandle* startMinigameBGM() {
        bool isGlider;

        if (isEqualStageName("JungleGliderGalaxy")) {
            isGlider = true;
        } else {
            isGlider = isEqualStageName("ChallengeGliderGalaxy");
        }

        if (isGlider) {
            return startGliderBGM();
        }

        if (isEqualStageName("AbekobeGalaxy")) {
            return nullptr;
        }

        return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("MBGM_MINI_GAME"), false, -1);
    }

    // NONMATCHING: regalloc/order (target hoists li r31,1 above the first getSceneMgr call).
    void stopStageBgmForDemo() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->stop(10);
        }

        AudWrap::getSceneMgr()->_1D = true;
        AudWrap::getSceneMgr()->mIsNewPlayerMode = true;
    }

    void startBossRushBGM() {
        AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("BGM_EV_BOSSRUSH"), false, -1);
    }

    void startAfterBossRushBGM() {
        if (!isPlayingStageBgmID(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("BGM_EV_BOSSRUSH02"))) {
            AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("BGM_EV_BOSSRUSH02"), false, -1);
        }
    }

    void stopStageBGMImmediately() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->stop(0);
        }
    }

    // NONMATCHING: regalloc/order (target keeps the string base in r31 and hoists the sceneMgr store).
    JAISoundHandle* startStarChanceBGM(bool param1) {
        AudWrap::getSceneMgr()->mIsNewPlayerMode = true;
        clearBgmQueue();

        if (isPlayerDead()) {
            return nullptr;
        }

        if (isEqualStageName("VsKoopaLv1Galaxy") || isEqualStageName("VsKoopaGravityGalaxy") || isEqualStageName("VsKoopaLv3Galaxy") || (isEqualStageName("QuicksandGalaxy") && getCurrentScenarioNo() == 1)) {
            return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("STM_STAR_CHANCE_3"), false, -1);
        }

        if (param1) {
            return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("MBGM_STAR_CHANCE_2"), false, -1);
        }

        return AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("MBGM_STAR_CHANCE"), false, -1);
    }

    JAISoundHandle* startStarChanceBGM2(bool param1) {
        return startStarChanceBGM(param1);
    }

    void startAppearPlanetJingle() {
        AudWrap::startSubBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("BGM_JG_APPEAR_PLANET"), false);
    }

    bool isUseScenarioSelectMusicZeroStars() {
        return getPowerStarNum() == 0;
    }

    bool isUseScenarioSelectMusicOneStar() {
        return getPowerStarNum() - 1 == 0;
    }

    // The asm copies the incoming r3 into the state variable before the grand star checks, so the
    // parameter is a bool (callers pass true/false). symbols.txt has "__2MRFv": rename to "__2MRFb".
    JAISoundHandle* startMarioFaceShipBGM(bool param1) {
        s32 stateNo = param1;
        s32 grandStarNum = GameDataFunction::calcCurrentGrandStarNum();

        if (GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
            stateNo = 0;
        } else if (grandStarNum >= 4) {
            stateNo = 4;
        } else if (grandStarNum >= 2) {
            stateNo = 3;
        } else if (getPowerStarNum() >= 2) {
            stateNo = 2;
        }

        switch (stateNo) {
        case 1:
            return startStageBGMIfNotPlaying("MBGM_SMG2_GALAXY_INTER", false);
        case 2:
            return startStageBGMIfNotPlaying("MBGM_SMG2_MARIO_SHIP01", false);
        case 3:
            return startStageBGMIfNotPlaying("MBGM_SMG2_MARIO_SHIP02", false);
        case 4:
            return startStageBGMIfNotPlaying("MBGM_SMG2_MARIO_SHIP03", false);
        default:
            return nullptr;
        }
    }

#pragma dont_inline on
    bool isFirstGrandStarReturnBGMStage() {
        return isEqualString(GameSequenceFunction::getClearedStageName(), "VsKoopaJrLv1Galaxy");
    }

#pragma dont_inline reset
    const char* getGrandStarReturnBGMName() {
        if (isFirstGrandStarReturnBGMStage()) {
            return "STM_SMG2_FIRST_ASTRO";
        }

        return "STM_SMG2_SECOND_ASTRO";
    }

    void startGrandStarReturnBGM() {
        const char* pName = getGrandStarReturnBGMName();

        AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName), true, -1);
    }

    void startGrandStarReturnBGMIfNotUnlocked() {
        if (unlockStageBGM() == 0) {
            AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(getGrandStarReturnBGMName()), false, -1);
        }
    }

    void startFirstAstroStageBgm() {
        AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("STM_SMG2_FIRST_ASTRO2"), true, -1);
    }

    void startEmptyNameBGMIfNotUnlocked() {
        if (unlockStageBGM() == 0) {
            AudWrap::startStageBgm(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(""), false, -1);
        }
    }

    void stopStageAndSubBGM(u32 param1) {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->stop(param1);
        }

        AudBgm* pSubBgm = AudWrap::getSubBgm();

        if (pSubBgm != nullptr) {
            pSubBgm->stop(0);
        }
    }

    bool isGravityBGMPlaying() {
        return isPlayingStageBgmID(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID("STM_SMG2_GALAXY_03"));
    }

    // NONMATCHING: regalloc/order (target hoists li r31,1 above the getSceneMgr call).
    void setCubeBgmChangeInvalid() {
        AudWrap::getSceneMgr()->mIsNewPlayerMode = true;
    }

    bool isCubeBgmChangeInvalid() {
        return AudWrap::getSceneMgr()->mIsNewPlayerMode;
    }

    void clearBgmQueue() {
        AudWrap::getBgmMgr()->clearLastBGM(0);
        AudWrap::getBgmMgr()->clearLastBGM(1);
        AudWrap::getBgmMgr()->clearNextBGM(0);
        AudWrap::getBgmMgr()->clearNextBGM(1);
    }

};  // namespace MR
