#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudStageUseResourceList.hpp"
#include "Game/AudioLib/AudBgmSetting.hpp"
#include "Game/GameAudio/ActionSoundSystem.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceCsvGroupHolder.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/RhythmLib/AudRhythmWrap.hpp"
#include "Game/Speaker/SpkSystem.hpp"
#include <JSystem/JAudio2/JAUSectionHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>

bool AudSceneSoundInfo::hasScenarioResource() const {
    return mScenarioId >= 0;
}

namespace {
    static const AudSceneSoundInfo cSceneInfo[] = {
        {"dummy", "dummy", 0, -1},
        {"Game", "BigWaterFallGalaxy", 1, -1},
        {"Game", "TamakoroSliderGalaxy", 2, -1},
        {"Game", "BigTree2Galaxy", 3, -1},
        {"Game", "TropicalResortGalaxy", 5, -1},
        {"Game", "WorldMap01Galaxy", 0, -1},
        {"Game", "WorldMap02Galaxy", 0, -1},
        {"Game", "WorldMap03Galaxy", 0, -1},
        {"Game", "WorldMap04Galaxy", 0, -1},
        {"Game", "WorldMap05Galaxy", 0, -1},
        {"Game", "WorldMap06Galaxy", 0, -1},
        {"Game", "WorldMap07Galaxy", 0, -1},
        {"Game", "WorldMap08Galaxy", 0, -1},
    };
    static const s8 cScenarioNoToWaveSet[][6] = {{0, -1, 1, 0, -1, -1}};
    static const s8 cWaveSetInfo[][2] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};
    static const s8 cBgmWaveSet[][2] = {{-1, -1}, {50, -1}, {50, -1}, {50, -1}, {23, -1}, {50, -1}};
    static const s8 cSeWaveSet[][4] = {{-1, -1, -1, -1}};
    static const s8 cSeScenarioWaveSet[][4] = {{27, -1, -1, -1}};
    const char* cStaticSound1st = "StaticSound1st";
    const char* cStaticSound2nd[] = {"StaticSound", "Mario", "StarPiece", "ModelWipeScene"};
}

AudSceneMgr::AudSceneMgr(JAUSectionHeap* heap) : _4(), mPlayerMode(), mIsNewPlayerMode(), _1D() {
    mSectionHeap = heap;
}

bool AudSceneMgr::isLoadDoneSystemInit() {
    return mSectionHeap->isWaveLoaded(7, 0);
}

void AudSceneMgr::loadSystemResource() {
    AudWrap::getInstListMgr()->requestWaveForSystem(7, 0, true);
}

void AudSceneMgr::requestResource() {
    JKRHeap* heap = MR::getStationedHeapGDDR3();
    MR::mountArchive(AudBgmSetting::getBgmParamResourceName(), heap, true);
    MR::mountArchive(AudBgmSetting::getMultiBgmResourceName(), heap, true);
    MR::mountArchive(getArchivePath(), heap, true);
    MR::mountArchive(AudBgmSetting::getStageBgmResourceName(), heap, true);
    MR::mountArchive(AudStageUseResourceList::getFileSelectResourceName(), heap, true);
    MR::mountArchive(AudStageUseResourceList::getFaceShipResourceName(), heap, true);
    MR::mountArchive(AudStageUseResourceList::getPeachCastleResourceName(), heap, true);
    MR::mountArchive(getEffectArchivePath(), heap, true);
}

void AudSceneMgr::loadStaticResource() {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->requestWave(11, -1, true);
    mgr->requestWave(2, 20, true);
    mgr->requestWave(3, 27, true);
    AudBgmSetting::loadBgmParamResource();
    AudBgmSetting::loadMultiBgmResource();
    AudBgmSetting::loadStageBgmResource();
    mgr->fn_802204A0(cStaticSound1st, true);
    mgr->loadRequestedWaves(false, true);
    mgr->_281C = true;
    AudWrap::getStageUseResourceList()->loadStaticSound();
}

void AudSceneMgr::loadStaticResource2nd() {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->_281C = false;
    mgr->requestWave(0, -1, true);
    mgr->requestWave(1, -1, true);
    mgr->requestWave(8, -1, true);
    mgr->requestWave(2, 15, true);
    mgr->requestWave(3, 30, true);
    mgr->requestWave(59, 9, true);
    mgr->requestWave(49, 1, true);
    mgr->requestWave(49, 2, true);
    mgr->requestWave(49, 3, true);
    mgr->requestWave(49, 4, true);
    mgr->requestWave(49, 5, true);
    mgr->requestWave(49, 6, true);
    mgr->requestWave(49, 7, true);
    mgr->requestWave(49, 8, true);
    mgr->requestWave(49, 10, true);
    for (s32 i = 0; i < 4; i++) {
        mgr->fn_802204A0(cStaticSound2nd[i], true);
    }
    mgr->loadRequestedWaves(false, true);
}

bool AudSceneMgr::isLoadDoneStaticResource() {
    return AudWrap::getInstListMgr()->isLoadDone();
}

bool AudSceneMgr::isLoadDoneStaticResource2nd() {
    return AudWrap::getInstListMgr()->isLoadDone();
}

void AudSceneMgr::loadStageResource(const char* scene, const char* stage) {
    loadStageResource(findSceneSoundInfo(scene, stage));
}

void AudSceneMgr::loadStageResource(const AudSceneSoundInfo* info) {
    if (info == nullptr) {
        return;
    }
    s32 waveSetId = info->mStageId;
    loadWaveSet(cSeWaveSet[cWaveSetInfo[waveSetId][1]], 4);
    loadWaveSet(cBgmWaveSet[cWaveSetInfo[waveSetId][0]], 2);
    if (!MR::isNextStageFileSelect()) {
        loadPlayerResource();
    }
}

void AudSceneMgr::loadScenarioResource(const char* scene, const char* stage, s32 scenario) {
    const AudSceneSoundInfo* info = findSceneSoundInfo(scene, stage);
    if (info != nullptr && info->hasScenarioResource()) {
        loadScenarioResource(info, scenario);
    }
}

void AudSceneMgr::loadScenarioResource(const AudSceneSoundInfo* info, s32 scenario) {
    if (info == nullptr) {
        return;
    }
    if (!info->hasScenarioResource()) {
        return;
    }
    s32 waveSet = cScenarioNoToWaveSet[info->mScenarioId][scenario - 1];
    if (waveSet >= 0) {
        loadWaveSet(cSeScenarioWaveSet[waveSet], 4);
    }
}

void AudSceneMgr::resetForScene() {
    _4 = 0;
    AudWrap::getSystem()->mIsSceneSoundMuted = false;
    AudWrap::getSystem()->mIsStartSoundBlocked = false;
    AudWrap::getSystem()->mIsStartLevelSoundBlocked = false;
    AudWrap::getSystem()->initSceneVolume();
    initSceneAudioEffect();
    mIsNewPlayerMode = false;
    _1D = false;
    SpkSystem::reconnect(-1);
}

void AudSceneMgr::initSceneAudioEffect() {
    AudEffector* effector = AudWrap::getSystem()->mAudEffector;
    if (effector != nullptr) {
        if (MR::isUseStageAudioEffect()) {
            effector->setFxLineType(1);
            MR::setAudioEffectType(6, 64, 1);
        } else {
            effector->setFxLineType(0);
            MR::setAudioEffectType(0, 64, 1);
        }
    }
}

const AudSceneSoundInfo* AudSceneMgr::findSceneSoundInfo(const char* scene, const char* stage) {
    for (s32 i = 0; i < ARRAY_SIZE(cSceneInfo); i++) {
        if (strcmp(scene, cSceneInfo[i].mSceneName) == 0 && strcmp(stage, cSceneInfo[i].mStageName) == 0) {
            return &cSceneInfo[i];
        }
    }
    if (strcmp(scene, "Game") == 0) {
        return &cSceneInfo[0];
    }
    return nullptr;
}

void AudSceneMgr::loadWaveSet(const s8* set, s32 count) {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    for (s32 i = 0; i < count; i++) {
        if (set[i] >= 0) {
            mgr->requestWave(set[i], -1, false);
        }
    }
}

void AudSceneMgr::setPlayerModeMario() {
    mPlayerMode = PlayerMode_Mario;
}

void AudSceneMgr::setPlayerModeLuigi() {
    mPlayerMode = PlayerMode_Luigi;
}

void AudSceneMgr::loadPlayerResource() {
    switch (mPlayerMode) {
    case PlayerMode_Mario: {
        AudInstListMgr* mgr = AudWrap::getInstListMgr();
        mgr->requestWave(34, 1, false);
        mgr->requestWave(34, 3, false);
        break;
    }
    case PlayerMode_Luigi: {
        AudInstListMgr* mgr = AudWrap::getInstListMgr();
        mgr->requestWave(34, 2, false);
        mgr->requestWave(34, 4, false);
        break;
    }
    }
}
