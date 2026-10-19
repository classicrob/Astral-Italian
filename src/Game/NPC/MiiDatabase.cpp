#include "Game/NPC/MiiDatabase.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Array.hpp"
#include <RVLFaceLib.h>

struct MiiDatabaseStoreData {
    /* 0x00 */ RFLStoreData* mData;
    /* 0x04 */ MR::AssignableArray< RFLStoreData* > mEntries;
};

MiiDatabase::~MiiDatabase() {
    delete[] mRawBuffer;
    delete mStoreData->mData;
    delete mStoreData;
}

template MR::AssignableArray< RFLStoreData* >::~AssignableArray();
