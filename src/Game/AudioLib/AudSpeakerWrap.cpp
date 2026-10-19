#include "Game/AudioLib/AudSpeakerWrap.hpp"
#include "Game/Speaker/SpkSound.hpp"
#include "Game/Speaker/SpkSystem.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include <JSystem/JAudio2/JASGlobal.hpp>
#include <revolution/wpad.h>

namespace {
    static SpkSoundHandle sSoundHandle[WPAD_MAX_CONTROLLERS];
    static SpkSoundHandle sSoundLevelHandle[12];
    static SpkSoundHandle sLoopSoundHandle[WPAD_MAX_CONTROLLERS];
};  // namespace

const char* AudSpeakerWrap::getResName() {
    return "/AudioRes/SpkRes/SpkRes.arc";
}

u32 AudSpeakerWrap::getRequiredHeapSize() {
    return 0x40000;
}

void AudSpeakerWrap::start(s32 waveID, s32 padChannel, s32 mode, f32 volume) {
    SpkSystem* system = JASGlobalInstance<SpkSystem>::getInstance();
    if (!system) return;
    if (mode == 0) {
        if (padChannel < 0) {
            for (s32 i = 0; i < 4; i++) {
                system->startSound(i, waveID, &sSoundHandle[i], mode, volume);
            }
        } else {
            system->startSound(padChannel, waveID, &sSoundHandle[padChannel], mode, volume);
        }
    } else {
        if (padChannel < 0) {
            for (s32 i = 0; i < 4; i++) {
                s32 index = findFreeSoundHandle();
                if (index >= 0) system->startSound(i, waveID, &sSoundLevelHandle[index], mode, volume);
            }
        } else {
            s32 index = findFreeSoundHandle();
            if (index >= 0) system->startSound(padChannel, waveID, &sSoundLevelHandle[index], mode, volume);
        }
    }
}

void AudSpeakerWrap::startLevel(s32 waveID, s32 padChannel) {
    SpkSystem* system = JASGlobalInstance<SpkSystem>::getInstance();
    if (!system) return;
    if (padChannel < 0) {
        for (s32 i = 0; i < 4; i++) {
            system->startLevelSound(i, waveID, &sLoopSoundHandle[i]);
        }
    } else {
        system->startLevelSound(padChannel, waveID, &sLoopSoundHandle[padChannel]);
    }
}

void AudSpeakerWrap::stop(s32 padChannel, s32 frames) {
    if (padChannel < 0) {
        for (s32 i = 0; i < 4; i++) {
            if (sSoundHandle[i].getSound()) sSoundHandle[i].getSound()->stop(frames);
        }
    } else {
        if (sSoundHandle[padChannel].getSound()) sSoundHandle[padChannel].getSound()->stop(frames);
    }
    for (s32 i = 0; i < 12; i++) {
        if (sSoundLevelHandle[i].getSound()) sSoundLevelHandle[i].getSound()->stop(frames);
    }
}

bool AudSpeakerWrap::isPlayable(s32 padChannel) {
    if (AudSystem::msBasic->isHomeButtonMenuActive()) return false;
    SpkSystem* system = JASGlobalInstance< SpkSystem >::getInstance();

    if (system == nullptr) {
        return false;
    }

    return system->getDeviceVolume(padChannel) != 0.0f;
}

s32 AudSpeakerWrap::findFreeSoundHandle() {
    for (s32 i = 0; i < 12; i++) {
        if (!sSoundLevelHandle[i].getSound()) return i;
    }
    return -1;
}
