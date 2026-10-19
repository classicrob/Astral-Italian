#include "Game/Speaker/SpkSystem.hpp"
#include "Game/Speaker/SpkSpeakerCtrl.hpp"

SpkSystem::SpkSystem(JKRHeap* pHeap) : JASGlobalInstance(true), mData() {
    if (pHeap == nullptr) {
        pHeap = JKRGetCurrentHeap();
    }

    mHeap = pHeap;
    mMixBuffer = new (pHeap, 0) SpkMixingBuffer(pHeap);
    mSoundHolder = new (pHeap, 0) SpkSoundHolder();
    SpkSpeakerCtrl::setMixingBuffer(mMixBuffer);
}

void SpkSystem::setup() {
    SpkSpeakerCtrl::setup();
}

void SpkSystem_DUMMY() {
    // NOTE: JSUList<SpkSound> dtor is being emitted here, as well as JASGlobalInstance<SpkSystem>. Unsure as to exactly why
    SpkSoundHolder dummy;
}

void SpkSystem::setResource(JKRArchive* pArchive, u16 tableOffset, u16 waveOffset) {
    mData = new (mHeap, 0) SpkData(pArchive);
    mData->loadTable(tableOffset);
    mData->loadWave(waveOffset);
}

void SpkSystem::framework() {
    SpkSpeakerCtrl::framework();
    mSoundHolder->framework();
}

void SpkSystem::startSound(s32 padChannel, s32 waveID, SpkSoundHandle* pHandle, s32 mode, f32 volume) {
    mSoundHolder->startSound(padChannel, waveID, pHandle, mode, volume);
}

void SpkSystem::startLevelSound(s32 padChannel, s32 waveID, SpkSoundHandle* pHandle) {
    mSoundHolder->startLevelSound(padChannel, waveID, pHandle);
}

void SpkSystem::stopSound(s32 padChannel, s32 frames) {
    if (padChannel < 0) {
        for (s32 i = 0; i < 4; i++) mSoundHolder->stopEachSound(i, frames);
    } else {
        mSoundHolder->stopEachSound(padChannel, frames);
    }
}

void SpkSystem::newSoundMemPool(s32 n) {
    JASPoolAllocObject< SpkSound >::memPool_.newMemPool(n);
}

void SpkSystem::connect(s32 padChannel) {
    SpkSpeakerCtrl::connect(padChannel);
}

void SpkSystem::disconnect(s32 padChannel) {
    SpkSpeakerCtrl::disconnect(padChannel);
}

void SpkSystem::reconnect(s32 padChannel) {
    if (padChannel < 0) {
        for (int i = 0; i < 4; i++) {
            SpkSpeakerCtrl::reconnect(i);
        }
    } else {
        SpkSpeakerCtrl::reconnect(padChannel);
    }
}

void SpkSystem::extensionProcess(s32 a1, s32 a2) {
    SpkSpeakerCtrl::extensionProcess(a1, a2);
}

f32 SpkSystem::getDeviceVolume(s32 padChannel) {
    return SpkSpeakerCtrl::getDeviceVolume(padChannel);
}
