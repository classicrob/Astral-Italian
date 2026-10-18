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

namespace {
    AudBgmConductor* getAudBgmConductor() {
        return MR::getSceneObj< AudBgmConductor >(SceneObj_AudBgmConductor);
    }
};  // namespace

namespace MR {
    JAISoundHandle* startStageBGM(const char* pName, bool param2) {
        JAISoundID id = AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName);

        return AudWrap::startStageBgm(id, param2);
    }

    JAISoundHandle* startStageBGMFromStageName(const char* pSceneName, const char* pStageName, s32 scenarioNo) {
        JAISoundID id = AudStageBgmWrap::changeStageNameToSoundID(pSceneName, pStageName, scenarioNo);

        if (id.isAnonymous()) {
            return nullptr;
        }

        JAISoundHandle* pSoundHandle = AudWrap::startStageBgm(id, false);

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

    void unlockStageBGM() {
        AudBgm* pStageBgm = AudWrap::getStageBgm();

        if (pStageBgm != nullptr) {
            pStageBgm->playAfterPrepared();
        }
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

    void setCubeBgmChangeInvalid() {
        AudSceneMgr* pSceneMgr = AudWrap::getSceneMgr();

        pSceneMgr->_1D = 1;
    }

    bool isCubeBgmChangeInvalid() {
        return AudWrap::getSceneMgr()->_1D;
    }

    void clearBgmQueue() {
        AudWrap::getBgmMgr()->clearLastBGM(0);
        AudWrap::getBgmMgr()->clearLastBGM(1);
        AudWrap::getBgmMgr()->clearNextBGM(0);
        AudWrap::getBgmMgr()->clearNextBGM(1);
    }

    void startCurrentStageBGM() {
        startStageBGMFromStageName("Game", getCurrentStageName(), getCurrentScenarioNo());
    }

    JAISoundHandle* startBossBGM(s32 id) {
        JAISoundHandle* pSoundHandle = nullptr;

        switch (id) {
        case BossBgmID_01_A:
            pSoundHandle = startStageBGM("MBGM_BOSS_01_A", 0);
            break;
        case BossBgmID_01_B:
            pSoundHandle = startStageBGM("MBGM_BOSS_01_B", 0);
            break;
        case BossBgmID_02_A:
            pSoundHandle = startStageBGM("MBGM_BOSS_02_A", 0);
            break;
        case BossBgmID_02_B:
            pSoundHandle = startStageBGM("MBGM_BOSS_02_B", 0);
            break;
        case BossBgmID_03_A:
            pSoundHandle = startStageBGM("MBGM_BOSS_03_A", 0);
            break;
        case BossBgmID_03_B:
            pSoundHandle = startStageBGM("MBGM_BOSS_03_B", 0);
            break;
        case BossBgmID_05_A:
            pSoundHandle = startStageBGM("MBGM_BOSS_05_A", 0);
            break;
        case BossBgmID_05_B:
            pSoundHandle = startStageBGM("MBGM_BOSS_05_B", 0);
            break;
        }

        return pSoundHandle;
    }

};  // namespace MR
