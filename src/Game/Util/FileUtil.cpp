#include "Game/Util/FileUtil.hpp"
#include "Game/Util.hpp"
#include "Game/System/FileLoader.hpp"
#include "Game/System/FileRipper.hpp"
#include "Game/Scene/StageFileLoader.hpp"
#include "Game/System/Language.hpp"
#include "Game/System/ResourceHolderManager.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <cstdio>

namespace MR {
    bool isFileExist(const char* pFilePath, bool considerLanguage) {
        s32 entryNum;

        if (considerLanguage) {
            entryNum = convertPathToEntrynumConsideringLanguage(pFilePath);
        } else {
            entryNum = DVDConvertPathToEntrynum(pFilePath);
        }

        return entryNum >= 0;
    }

    u32 getFileSize(const char* pFilePath, bool considerLanguage) {
        s32 entryNum;

        if (considerLanguage) {
            entryNum = convertPathToEntrynumConsideringLanguage(pFilePath);
        } else {
            entryNum = DVDConvertPathToEntrynum(pFilePath);
        }

        DVDFileInfo fileInfo;
        DVDFastOpen(entryNum, &fileInfo);
        u32 size = fileInfo.length;
        DVDClose(&fileInfo);

        return size;
    }

    inline s32 convertPathToEntrynumConsideringLanguage(const char* pFilePath) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), pFilePath, false);

        return DVDConvertPathToEntrynum(filePath);
    }

    bool isFileExistOnDisc(const char* path, bool language) {
        return isFileExist(path, language);
    }

    void* loadToMainRAM(const char* path, u8* dst, JKRHeap* heap, int direction) {
        char loadPath[256];
        makeFileNameConsideringLanguage(loadPath, sizeof(loadPath), path, false);
        SingletonHolder<FileLoader>::get()->requestLoadToMainRAM(loadPath, dst, heap, static_cast<JKRDvdRipper::EAllocDirection>(direction), false);
        char receivePath[256];
        makeFileNameConsideringLanguage(receivePath, sizeof(receivePath), path, false);
        return SingletonHolder<FileLoader>::get()->receiveFile(receivePath);
    }

    void loadAsyncToMainRAM(const char* path, u8* dst, JKRHeap* heap, JKRDvdRipper::EAllocDirection direction) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), path, false);
        SingletonHolder<FileLoader>::get()->requestLoadToMainRAM(filePath, dst, heap, direction, false);
    }

    JKRMemArchive* mountArchive(const char* pFilePath, JKRHeap* pHeap, bool) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), pFilePath, false);

        SingletonHolder< FileLoader >::get()->requestMountArchive(filePath, pHeap, JKRDvdRipper::ALLOC_DIRECTION_FORWARD, false);

        char archivePath[256];
        makeFileNameConsideringLanguage(archivePath, sizeof(archivePath), pFilePath, false);

        return SingletonHolder< FileLoader >::get()->receiveArchive(archivePath);
    }



    void mountAsyncArchive(const char* path, JKRHeap* heap, bool, s32 allocDirection) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), path, false);
        SingletonHolder<FileLoader>::get()->requestMountArchive(filePath, heap, static_cast<JKRDvdRipper::EAllocDirection>(allocDirection), false);
    }

    void* receiveFile(const char* pFilePath) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), pFilePath, false);

        return SingletonHolder< FileLoader >::get()->receiveFile(filePath);
    }

    JKRMemArchive* receiveArchive(const char* pFilePath) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), pFilePath, false);

        return SingletonHolder< FileLoader >::get()->receiveArchive(filePath);
    }

    void receiveAllRequestedFile() {
        SingletonHolder< FileLoader >::get()->receiveAllRequestedFile();
    }

    void createAndAddArchive(void* pArcData, JKRHeap* pHeap, const char* pFilePath) {
        SingletonHolder< FileLoader >::get()->createAndAddArchive(pArcData, pHeap, pFilePath);
    }

    void getMountedArchiveAndHeap(const char* pFilePath, JKRArchive** ppArchive, JKRHeap** ppHeap) {
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), pFilePath, false);

        SingletonHolder< FileLoader >::get()->getMountedArchiveAndHeap(filePath, ppArchive, ppHeap);
    }

    void* getResourceFromMountedArchive(const char* path, const char* resource) {
        JKRArchive* archive = nullptr;
        JKRHeap* heap = nullptr;
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), path, false);
        SingletonHolder<FileLoader>::get()->getMountedArchiveAndHeap(filePath, &archive, &heap);
        return archive->getResource(resource);
    }

    bool isExistResourceInMountedArchive(const char* path, const char* resource) {
        JKRArchive* archive = nullptr;
        JKRHeap* heap = nullptr;
        char filePath[256];
        makeFileNameConsideringLanguage(filePath, sizeof(filePath), path, false);
        SingletonHolder<FileLoader>::get()->getMountedArchiveAndHeap(filePath, &archive, &heap);
        return archive->getResource(resource) != nullptr;
    }

    void removeFileConsideringLanguage(const char* path) {
        SingletonHolder<FileLoader>::get()->removeFile(path);
    }

    void removeResourceAndFileHolderIfIsEqualHeap(JKRHeap* pHeap) {
        SingletonHolder< ResourceHolderManager >::get()->removeIfIsEqualHeap(pHeap);
        SingletonHolder< FileLoader >::get()->removeHolderIfIsEqualHeap(pHeap);
    }

    void* decompressFileFromArchive(JKRArchive* pArchive, const char* pFilePath, JKRHeap* pHeap, int align) {
        u8* fileData = (u8*)pArchive->getResource(pFilePath);
        s32 compressionType = FileRipper::checkCompressed(fileData);
        u32 fileSize = pArchive->getResSize(fileData);

        u32 adjustedFileSize = fileSize;

        if (compressionType == 2)  // Yaz0
        {
            u32 decompressedSize = (fileData[4] << 24) | (fileData[5] << 16) | (fileData[6] << 8) | fileData[7];

            adjustedFileSize = decompressedSize;
        }

        u8* buffer = new (pHeap, align) u8[adjustedFileSize];

        JKRMemArchive::fetchResource_subroutine(fileData, fileSize, buffer, adjustedFileSize, compressionType);

        return buffer;
    }




    void makeFileNameConsideringLanguage(char* pDst, u32 size, const char* pFilePath) {
        makeFileNameConsideringLanguage(pDst, size, pFilePath, true);
    }

    void makeFileNameConsideringLanguage(char* pDst, u32 size, const char* pFilePath, bool checkLocalized) {
        char prefix[256];
        snprintf(prefix, sizeof(prefix), "/LocalizeData/%s", getCurrentLanguagePrefix());
        addFilePrefix(pDst, size, pFilePath, prefix);

        bool exists;
        if (checkLocalized) {
            exists = isFileExistOnDisc(pDst, false);
        } else {
            exists = isFileExist(pDst, false);
        }
        if (!exists) {
            snprintf(pDst, size, "%s", pFilePath);
        }
    }

    bool makeObjectArchiveFileName(char* dst, u32 size, const char* file) {
        snprintf(dst, size, "/ObjectData/%s", file);
        if (isFileExist(dst, true)) return true;
        snprintf(dst, size, "/LightData/%s", file);
        if (isFileExist(dst, true)) return true;
        snprintf(dst, size, "/DemoData/%s", file);
        if (isFileExist(dst, true)) return true;
        snprintf(dst, size, "/MapPartsData/%s", file);
        if (isFileExist(dst, false)) return true;
        snprintf(dst, size, "%s", file);
        return isFileExist(dst, false);
    }

    char sArchiveFileFormat[] = "%s.arc";

    inline bool makeObjectArchiveFileNameFromPrefix(char* pDst, u32 size, const char* pFilePrefix, bool) {
        char fileName[256];
        snprintf(fileName, sizeof(fileName), sArchiveFileFormat, pFilePrefix);

        return makeObjectArchiveFileName(pDst, size, fileName);
    }

    bool makeLayoutArchiveFileName(char* pDst, u32 size, const char* pFileName) {
        snprintf(pDst, size, "/Region/LayoutData/%s", pFileName);

        if (isFileExist(pDst, false)) {
            return true;
        }

        snprintf(pDst, size, "/LayoutData/%s", pFileName);

        if (isFileExist(pDst, true)) {
            return true;
        }

        snprintf(pDst, size, "%s", pFileName);

        return isFileExist(pDst, false);
    }

    bool makeLayoutArchiveFileNameFromPrefix(char* pDst, u32 size, const char* pFilePrefix, bool fallback) {
        char fileName[64];
        snprintf(fileName, sizeof(fileName), sArchiveFileFormat, pFilePrefix);
        bool isExistArc = makeLayoutArchiveFileName(pDst, size, fileName);

        const char* pAspectSuffix = isScreen16Per9() ? "16x9" : "4x3";
        snprintf(fileName, sizeof(fileName), "%s%s.arc", pFilePrefix, pAspectSuffix);
        bool isExistAspectArc = makeLayoutArchiveFileName(pDst, size, fileName);

        bool fileFound = isExistArc || isExistAspectArc;

        if (!fileFound && !fallback) {
            return false;
        }

        if (isExistAspectArc) {
            pAspectSuffix = isScreen16Per9() ? "16x9" : "4x3";
            snprintf(fileName, sizeof(fileName), "%s%s.arc", pFilePrefix, pAspectSuffix);

        } else {
            snprintf(fileName, sizeof(fileName), sArchiveFileFormat, pFilePrefix);
        }

        makeLayoutArchiveFileName(pDst, size, fileName);

        return true;
    }

    // Original pool strings at 8065DA04/8065DA0C have no surviving reference,
    // likely from code stripped from the shipped build.
    char sDesignArchiveName[] = "Design";
    char sSoundArchiveName[] = "Sound";

    void makeDemoArchiveFileName(char* dst, u32 size, const char* stage) {
        StageFileLoader::makeStageArchivePath(dst, size, stage, "Demo");
    }

    // Original pool strings at 8065DA1C/8065DA24 have no surviving reference,
    // likely from code stripped from the shipped build.
    char sLightArchiveName[] = "Light";
    char sZoneInfoArchiveName[] = "ZoneInfo";

    void makeScenarioArchiveFileName(char* pDst, u32 size, const char* pStageName) {
        snprintf(pDst, size, "/StageData/%s/%sScenario.arc", pStageName, pStageName);
    }
};  // namespace MR
