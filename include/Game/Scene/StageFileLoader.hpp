#pragma once

#include <revolution/types.h>

class StageFileLoader {
public:
    StageFileLoader(const char*);

    void startLoadingStageFile();
    void waitLoadedStageFile();
    void makeStageArchiveNameList(const char*);
    void makeStageArchiveName(const char*);
    bool addStageArchiveName(const char*, const char*);
    static void makeStageArchivePath(char*, u32, const char*, const char*);   // SMG2 0x8045B050 (in the StageDataHolder gap)

    /* 0x000 */ char* mStageFiles[0x40];
    /* 0x100 */ s32 mStageFileNum;
};
