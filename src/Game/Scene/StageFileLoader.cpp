#include "Game/Scene/StageFileLoader.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <cstdio>
#include <cstring>

StageFileLoader::StageFileLoader(const char* pStageName) {
    mStageFileNum = 0;
    makeStageArchiveNameList(pStageName);
}

void StageFileLoader::startLoadingStageFile() {
    if (MR::getWorldMapHeap() != nullptr) {
        for (s32 i = 0; i < mStageFileNum; i++) {
            MR::mountAsyncArchive(mStageFiles[i], MR::getWorldMapHeap(), true, true);
        }
    } else {
        for (s32 i = 0; i < mStageFileNum; i++) {
            MR::mountAsyncArchive(mStageFiles[i], MR::getFileCacheHeap(), true, true);
        }
    }
}

void StageFileLoader::waitLoadedStageFile() {
    for (s32 i = 0; i < mStageFileNum; i++) {
        MR::receiveFile(mStageFiles[i]);
    }
}

void StageFileLoader::makeStageArchiveNameList(const char* pStageName) {
    GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pStageName);

    for (s32 i = 0; i < accessor.getZoneNum(); i++) {
        makeStageArchiveName(accessor.getZoneName(i));
    }
}

void StageFileLoader::makeStageArchiveName(const char* pZoneName) {
    addStageArchiveName(pZoneName, "Map");
    addStageArchiveName(pZoneName, "Sound");
    addStageArchiveName(pZoneName, "Design");
    addStageArchiveName(pZoneName, "ZoneInfo");
    addStageArchiveName(pZoneName, "UseResource");
    addStageArchiveName(pZoneName, "Ghost");
}

bool StageFileLoader::addStageArchiveName(const char* pZoneName, const char* pSuffix) {
    char path[256];
    makeStageArchivePath(path, sizeof(path), pZoneName, pSuffix);

    if (!MR::isFileExist(path, false)) {
        return false;
    }

    u32 len = strlen(path) + 1;
    mStageFiles[mStageFileNum] = new char[len];
    MR::copyString(mStageFiles[mStageFileNum], path, len);
    mStageFileNum++;
    return true;
}
