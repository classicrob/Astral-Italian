#include "Game/AudioLib/AudRemixMgr.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>

AudRemixMgr::AudRemixMgr(JKRHeap* pHeap) {
    mHeap = pHeap;
    mGroups = nullptr;
    mSoundObj = nullptr;
    mRemixSeq = nullptr;
}

void AudRemixMgr::init() {
    mSoundObj = new (mHeap, 0) AudSoundObject(nullptr, 0x10, mHeap);
    mRemixSeq = new (mHeap, 0) AudRemixSequencer();
}

void AudRemixMgr::update() {
    mRemixSeq->update();
}

#ifdef GALAXY_NATIVE
void AudRemixMgr::setRemixSeqResource(void* ptr) {
    u32* newPtr = (u32*)ptr;
    int groupCount = PetariNative::readU32BE(newPtr);
    newPtr++;
    mGroupCount = groupCount;
    newPtr += mGroupCount;
    mGroups = new (mHeap, 0) RemixNoteGroupData[mGroupCount];
    for (int i = 0; i < mGroupCount; i++) {
        RemixNoteGroupData* pGroupData = &mGroups[i];
        pGroupData->mIndex = i;
        pGroupData->mTrackCount = PetariNative::readU32BE(newPtr++);
        pGroupData->mNoteCount = PetariNative::readU32BE(newPtr++);
        pGroupData->mNoteTimings = newPtr;
        newPtr += pGroupData->mNoteCount;
        pGroupData->mRemixTracks = new (mHeap, 0) RemixNoteTrackData[pGroupData->mTrackCount];
        for (int j = 0; j < pGroupData->mTrackCount; j++) {
            pGroupData->mRemixTracks[j].mTrackNo = PetariNative::readU32BE(newPtr++);
            pGroupData->mRemixTracks[j].mNotes = reinterpret_cast< RemixNoteData* >(newPtr);
            newPtr += pGroupData->mNoteCount * 4;
        }
    }
}
#else
void AudRemixMgr::setRemixSeqResource(void* ptr) {
    u32* newPtr = (u32*)ptr;
    int groupCount = *(newPtr);
    newPtr++;
    mGroupCount = groupCount;
    newPtr += mGroupCount;
    mGroups = new (mHeap, 0) RemixNoteGroupData[mGroupCount];
    for (int i = 0; i < mGroupCount; i++) {
        RemixNoteGroupData* pGroupData = &mGroups[i];
        pGroupData->mIndex = i;
        pGroupData->mTrackCount = *newPtr++;
        pGroupData->mNoteCount = *newPtr++;
        pGroupData->mNoteTimings = newPtr;
        newPtr += pGroupData->mNoteCount;
        pGroupData->mRemixTracks = new (mHeap, 0) RemixNoteTrackData[pGroupData->mTrackCount];
        for (int j = 0; j < pGroupData->mTrackCount; j++) {
            pGroupData->mRemixTracks[j].mTrackNo = *newPtr++;
            pGroupData->mRemixTracks[j].mNotes = reinterpret_cast< RemixNoteData* >(newPtr);
            newPtr += pGroupData->mNoteCount * 4;
        }
    }
}
#endif

RemixNoteGroupData* AudRemixMgr::getRemixNoteGroupDataFromMelodyNo(s32 melodyNo) const {
    for (int i = 0; i < mGroupCount; i++) {
        if (melodyNo == mGroups[i].mIndex) {
            return &mGroups[i];
        }
    }
    return nullptr;
}
