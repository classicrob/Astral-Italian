#pragma once

#include "Game/Speaker/SpkData.hpp"
#include "Game/Speaker/SpkMixingBuffer.hpp"
#include "Game/Speaker/SpkSound.hpp"
#include <JSystem/JAudio2/JASGlobal.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>

class SpkSystem : JASGlobalInstance< SpkSystem > {
public:
    SpkSystem(JKRHeap*);

    void setResource(JKRArchive*, u16, u16);
    void framework();
    void startSound(s32, s32, SpkSoundHandle*);
    void startSound(s32, s32, SpkSoundHandle*, s32, f32);
    void startLevelSound(s32, s32, SpkSoundHandle*);
    void stopSound(s32 padChannel, s32 fadeFrames);  // padChannel < 0: all channels; fn_804AC160
    static void newSoundMemPool(s32);
    static void setup();
    static void connect(s32);
    static void disconnect(s32);
    static void reconnect(s32);
    static void extensionProcess(s32, s32);
    static f32 getDeviceVolume(s32);

    /* 0x00 */ JKRHeap* mHeap;
    /* 0x04 */ SpkData* mData;
    /* 0x08 */ SpkMixingBuffer* mMixBuffer;
    /* 0x0C */ SpkSoundHolder* mSoundHolder;
};
