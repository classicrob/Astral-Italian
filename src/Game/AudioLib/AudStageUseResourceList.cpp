#include "Game/AudioLib/AudStageUseResourceList.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Util/ObjUtil.hpp"

AudStageUseResourceList::AudStageUseResourceList() {
    mParsers[0] = 0;
    mParsers[1] = 0;
    mParsers[2] = 0;
    mParsers[3] = 0;
    mParsers[4] = 0;
}
void AudStageUseResourceList::load() {
    mParsers[0] = MR::tryCreateStageUseResourceCsvParser("FileSelect", "sound_scenario_1");
    mParsers[1] = MR::tryCreateStageUseResourceCsvParser("MarioFaceShipGalaxy", "sound_scenario_1");
    mParsers[2] = MR::tryCreateStageUseResourceCsvParser("MarioFaceShipGalaxy", "sound_scenario_2");
    mParsers[3] = MR::tryCreateStageUseResourceCsvParser("MarioFaceShipGalaxy", "sound_scenario_4");
    mParsers[4] = MR::tryCreateStageUseResourceCsvParser("PeachCastleGalaxy", "sound_scenario_2");
}
void AudStageUseResourceList::loadStaticSound() {
    const JMapInfo* parser = mParsers[0];
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->requestInstList(parser);
    mgr->loadRequestedWaves(false, false);
}
void AudStageUseResourceList::loadIslandFleetClear() {
    loadResource(mParsers[1], true, true);
}
void AudStageUseResourceList::loadGalaxyClear() {
    loadResource(mParsers[2], true, true);
}
void AudStageUseResourceList::loadEndingClear() {
    loadResource(mParsers[3], true, true);
}
void AudStageUseResourceList::loadKoopaClear() {
    loadResource(mParsers[4], true, true);
}
const char* AudStageUseResourceList::getFileSelectResourceName() {
    return "/StageData/FileSelect/FileSelectUseResource.arc";
}
const char* AudStageUseResourceList::getFaceShipResourceName() {
    return "/StageData/MarioFaceShipGalaxy/MarioFaceShipGalaxyUseResource.arc";
}
const char* AudStageUseResourceList::getPeachCastleResourceName() {
    return "/StageData/PeachCastleGalaxy/PeachCastleGalaxyUseResource.arc";
}
void AudStageUseResourceList::loadResource(const JMapInfo* parser, bool eraseOld, bool reset) {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->requestInstList(parser);
    mgr->loadRequestedWaves(eraseOld, false);
    if (reset) {
        mgr->resetStageWaveSlots();
        mgr->clearWaveRequests();
    }
}
