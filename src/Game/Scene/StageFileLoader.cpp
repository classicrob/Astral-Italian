#include "Game/Scene/StageFileLoader.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <cstdio>
#include <cstring>

StageFileLoader::StageFileLoader(const char* pName) {
    mZoneCount = 0;
    makeStageArchiveNameList();
}

void StageFileLoader::startLoadingStageFile() {
    for (int i = 0; i < mZoneCount; i++) {
        MR::mountAsyncArchive(mStageFiles[i], MR::getAproposHeapForSceneArchive(0.03f));
    }
}

void StageFileLoader::waitLoadedStageFile() {
    for (int i = 0; i < mZoneCount; i++) {
        const char* file = mStageFiles[i];
        MR::receiveFile(file);
        mountFilesInStageMapFile(file);
    }
}


void StageFileLoader::makeStageArchiveName(char* buf, u32 len, const char* pZoneName) {
    snprintf(buf, len, "/StageData/%s.arc", pZoneName);
}
