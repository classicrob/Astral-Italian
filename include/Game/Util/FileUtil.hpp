#pragma once

#include <JSystem/JKernel/JKRMemArchive.hpp>

namespace MR {
    void* loadToMainRAM(const char*, u8*, JKRHeap*, int);
    bool isFileExistOnDisc(const char*, bool);
    void makeDemoArchiveFileName(char*, u32, const char*);
    /// @brief Determines if a file exists within the disc file system.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the file.
    /// @param[in] considerLanguage Determines if the file is localized.
    /// @return `true` if the file exists within the disc file system, `false` otherwise.
    bool isFileExist(const char* pFilePath, bool considerLanguage) NO_INLINE;

    /// @brief Returns the size of a file, in bytes.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the file.
    /// @param[in] considerLanguage Determines if the file is localized.
    /// @return The size of the file, in bytes.
    u32 getFileSize(const char* pFilePath, bool considerLanguage);

    /// @brief Returns the archive after it has been asynchronously mounted.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the archive file.
    /// @return A pointer to the mounted archive.
    JKRMemArchive* receiveArchive(const char* pFilePath);

    /// @brief Creates an archive from existing memory data.
    /// @param[in] pArcData A pointer to the archive file data.
    /// @param[in,out] pHeap A pointer to the target heap.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the file.
    void createAndAddArchive(void* pArcData, JKRHeap* pHeap, const char* pFilePath);

    /// @brief Returns the mounted archive and heap for a given file by pointer.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the archive file.
    /// @param[out] ppArchive A pointer to the archive pointer to initialize.
    /// @param[out] ppHeap A pointer to the heap pointer to initialize.
    void getMountedArchiveAndHeap(const char* pFilePath, JKRArchive** ppArchive, JKRHeap** ppHeap);

    /// @brief Decompresses a file from an archive file.
    /// @param[in] pArchive A pointer to the target archive.
    /// @param[in] pFilePath A pointer to the null-terminated absolute path of the file within the archive file.
    /// @param[in,out] pHeap A pointer to the target heap.
    /// @param[in] align The alignment for the allocation.
    /// @return A pointer to the decompressed file data.
    void* decompressFileFromArchive(JKRArchive* pArchive, const char* pFilePath, JKRHeap* pHeap, int align);

    /// @brief Creates an absolute path to the given layout's archive file.
    /// @param[out] pDst A pointer to the path buffer to initialize.
    /// @param[in] size The size of the path buffer, in bytes.
    /// @param[in] pFilePrefix A pointer to the null-terminated name of the archive file, without the extension.
    /// @param[in] fallback Whether to fall back if none found.
    /// @return `true` if a valid archive file exists in the file system, `false` otherwise.
    bool makeLayoutArchiveFileNameFromPrefix(char* pDst, u32 size, const char* pFilePrefix, bool fallback);

    /// @brief Creates an absolute path to the given galaxy's mission archive file.
    /// @param[out] pDst A pointer to the path buffer to initialize.
    /// @param[in] size The size of the path buffer, in bytes.
    /// @param[in] pStageName A pointer to the null-terminated name of the galaxy.
    void makeScenarioArchiveFileName(char* pDst, u32 size, const char* pStageName);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <JSystem/JKernel/JKRDvdRipper.hpp>
#include <revolution/types.h>
class JKRArchive;
class JKRHeap;
class JKRMemArchive;

namespace MR {
    s32 convertPathToEntrynumConsideringLanguage(const char* pFilePath);
    void loadAsyncToMainRAM(const char* pFilePath, u8* pDst, JKRHeap* pHeap, JKRDvdRipper::EAllocDirection allocDir);
    JKRMemArchive* mountArchive(const char* pFilePath, JKRHeap* pHeap, bool);
    void mountAsyncArchive(const char* pFilePath, JKRHeap* pHeap);
    void mountAsyncArchive(const char* pFilePath, JKRHeap* pHeap, bool, bool allocFromTail);   // SMG2 0x80025AF0 (3rd arg unused)
    void mountAsyncArchiveByObjectOrLayoutName(const char* pFilePrefix, JKRHeap* pHeap);
    void removeFileConsideringLanguage(const char* pFilePath);
    void removeResourceAndFileHolderIfIsEqualHeap(JKRHeap* pHeap);
    bool isLoadedFile(const char* pFilePath);
    bool isMountedArchive(const char* pFilePath);
    bool isLoadedObjectOrLayoutArchive(const char* pFilePrefix);
    void makeFileNameConsideringLanguage(char* pDst, u32 size, const char* pFilePath);
    // SMG2 fn_80025460 (unnamed): the bool selects no distinct code path in the target (both branches are identical).
    void makeFileNameConsideringLanguage(char* pDst, u32 size, const char* pFilePath, bool);
    bool makeObjectArchiveFileName(char* pDst, u32 size, const char* pFileName);
    bool makeLayoutArchiveFileName(char* pDst, u32 size, const char* pFileName);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void* receiveFile(const char* pFilePath);
    void receiveAllRequestedFile();
};  // namespace MR

namespace MR {
    // SMG2 fn_80025C60 (core4 name): resource `pResName` of the mounted archive `pArchivePath`.
    void* getResourceFromMountedArchive(const char* pArchivePath, const char* pResName);
    // SMG2 fn_80025CE0 (core4 name)
    bool isExistResourceInMountedArchive(const char* pArchivePath, const char* pResName);
}
