#pragma once

#include <revolution/types.h>
#ifdef GALAXY_NATIVE
#include <petari/save_wire.hpp>
using SaveDataWireU32 = PetariNative::SaveWire::Value<u32>;
#else
typedef u32 SaveDataWireU32;
#endif

struct SaveDataFileHeader {
    /* 0x00 */ SaveDataWireU32 mCheckSum;
    /* 0x04 */ SaveDataWireU32 mVersion;
    /* 0x08 */ SaveDataWireU32 mUserFileInfoNum;
    /* 0x0C */ SaveDataWireU32 mFileSize;
};

struct SaveDataFileInfo {
    /* 0x00 */ char mName[12];
    /* 0x0C */ SaveDataWireU32 mOffset;
};

struct SaveDataFile {
    /* 0x00 */ SaveDataFileHeader mHeader;
    /* 0x10 */ SaveDataFileInfo mInfo[1];
};

struct SaveDataUserFileInfo {
    /* 0x00 */ u8* mData;
    /* 0x04 */ u32 mDataSize;
    /* 0x08 */ u8 mKind;
};

class SaveDataFileAccessor {
public:
    SaveDataFileAccessor(u8*);

    SaveDataFileHeader* getHeader();
    SaveDataFileInfo* getFileInfo(int);
    void makeUserFileInfo(SaveDataUserFileInfo*, const char*);

private:
    /* 0x00 */ SaveDataFile* mFile;
};
