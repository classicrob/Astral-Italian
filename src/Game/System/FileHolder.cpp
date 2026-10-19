#include "Game/System/FileHolder.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Util/StringUtil.hpp"
#include <revolution/dvd.h>
#include <cstring>
#endif

#define FILE_HOLDER_STATE_NONE 0
#define FILE_HOLDER_STATE_READING 1
#define FILE_HOLDER_STATE_DONE 2

FileHolderFileEntry::FileHolderFileEntry(const char* pFilePath, JKRHeap* pHeap, void* pContext)
    : mHashCode(MR::getHashCodeLower(pFilePath)), mContext(nullptr), mHeap(pHeap), mState(FILE_HOLDER_STATE_NONE), mContextSet(true) {
    OSInitMessageQueue(&mQueue, &mMessage, 1);
#ifdef GALAXY_NATIVE
    mNativePath = nullptr;
    if (pFilePath != nullptr && DVDConvertPathToEntrynum(pFilePath) < 0) {
        const s32 length = strlen(pFilePath) + 1;
        mNativePath = new (pHeap, 0) char[length];
        MR::copyString(mNativePath, pFilePath, length);
    }
#endif

    if (pContext) {
        mContextSet = false;
        mContext = pContext;
    }
}

FileHolderFileEntry::~FileHolderFileEntry() {
    if (mContextSet && (mContext != nullptr)) {
        delete (u8*)mContext;
    }
#ifdef GALAXY_NATIVE
    delete[] mNativePath;
#endif
}

void FileHolderFileEntry::waitReadDone() {
    if (mState != FILE_HOLDER_STATE_DONE) {
        OSMessage msg;
        OSReceiveMessage(&mQueue, &msg, OS_MESSAGE_BLOCK);
        mState = FILE_HOLDER_STATE_DONE;
    }
}

void FileHolderFileEntry::setContext(void* pContext, JKRHeap* pHeap) {
    mContext = pContext;
    mHeap = pHeap;

    OSSendMessage(&mQueue, nullptr, OS_MESSAGE_NOBLOCK);
    mState = FILE_HOLDER_STATE_READING;
}

FileHolder::FileHolder() {
    mEntries.init(0x220);
}

FileHolderFileEntry* FileHolder::add(const char* pFilePath, JKRHeap* pHeap, void* pContext) {
    FileHolderFileEntry* pEntry = new (pHeap, 0) FileHolderFileEntry(pFilePath, pHeap, pContext);
    mEntries.push_back(pEntry);
    return pEntry;
}

bool FileHolder::isExist(const char* pFilePath) const {
    return findEntry(pFilePath);
}

void* FileHolder::getContext(const char* pFilePath) const {
    return findEntry(pFilePath)->mContext;
}

void FileHolder::removeIfIsEqualHeap(JKRHeap* pHeap) {
    if (pHeap == nullptr) {
        return;
    }

    for (FileHolderFileEntry** i = mEntries.begin(); i != mEntries.end();) {
        if ((*i)->mHeap == pHeap || MR::getHeapNapa((*i)->mHeap) == pHeap || MR::getHeapGDDR3((*i)->mHeap) == pHeap) {
            delete *i;
            mEntries.erase(i);
        } else {
            i++;
        }
    }
}

void FileHolder::removeFile(const char* pFilePath) {
    FileHolderFileEntry* entry = findEntry(pFilePath);
    if (entry != nullptr) {
        FileHolderFileEntry** p = mEntries.begin();
        while (p != mEntries.end() && *p != entry) {
            p++;
        }

        delete *p;
        mEntries.erase(p);
    }
}

FileHolderFileEntry* FileHolder::findEntry(const char* pFilePath) const {
    u32 hashCode = MR::getHashCodeLower(pFilePath);
    for (FileHolderFileEntry* const* pEntry = mEntries.begin(); pEntry != mEntries.end(); pEntry++) {
        if (hashCode == (*pEntry)->mHashCode) {
            return *pEntry;
        }
    }

    return nullptr;
}
