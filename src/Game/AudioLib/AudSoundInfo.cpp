#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include <JSystem/JAudio2/JAUSoundTable.hpp>

u16 AudSoundInfo::getSoundSw(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_BGM:
            return reinterpret_cast< JAUSoundTableBgm* >(data)->mSoundSw;
        case DATA_SE:
            return reinterpret_cast< JAUSoundTableSe* >(data)->mSoundSw;
        case DATA_STREAM:
            return 0;
        }
    }
    return 0;
}

s32 AudSoundInfo::getLimitNum(JAISoundID soundID) const {
    return (getSoundSw(soundID) >> 3) & 3;
}

bool AudSoundInfo::isLimitNumPlaying(JAISoundID soundID) const {
    s32 numOfPlaying = AudWrap::getSystem()->getNumOfPlaying(soundID);
    u16 sw = (getSoundSw(soundID) >> 3) & 3;
    if (sw == 0) {
        return false;
    }
    return sw <= numOfPlaying;
}

u32 AudSoundInfo::getPlayCountLimit(u32 limitSw) const {
    switch (limitSw) {
    case 0x20:
        return 1;
    case 0x40:
        return 5;
    case 0x60:
        return 10;
    }
    return 0;
}

namespace {
    inline u32 getSoundSwitches(const AudSoundInfo* info, JAISoundID soundID) {
        return info->getSoundSw(soundID);
    }
}

bool AudSoundInfo::isLimitCountPlaying(JAISoundID soundID) const {
    u32 limitSw = getSoundSwitches(this, soundID) & 0x60;
    u32 max = getPlayCountLimit(limitSw);
    if (limitSw == 0) {
        return false;
    }
    return AudWrap::getSystem()->getPlayCountMin(soundID) < max;
}

u16 AudSoundInfo::getChordResIdBgm(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_BGM:
            return reinterpret_cast< JAUSoundTableBgm* >(data)->mChordResId;
        case DATA_SE:
        case DATA_STREAM:
            return -1;
        }
    }
    return -1;
}

bool AudSoundInfo::isRhythmBgm(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_BGM:
            return getSoundSw(soundID) & 1;
        case DATA_SE:
        case DATA_STREAM:
            return false;
        }
    }
    return false;
}

bool AudSoundInfo::isFanfareSe(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_SE:
            return (getSoundSw(soundID) >> 8) & 1;
        case DATA_BGM:
        case DATA_STREAM:
            return false;
        }
    }
    return false;
}

bool AudSoundInfo::isUseFxMix(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_SE:
            return (reinterpret_cast< JAUSoundTableSe* >(data)->mSoundSw >> 9) & 1;
        case DATA_BGM:
        case DATA_STREAM:
            return false;
        }
    }
    return false;
}

bool AudSoundInfo::isUseLpf(JAISoundID soundID) const {
    void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);
    u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);
    if (data != nullptr) {
        switch (typeID & 0xF0) {
        case DATA_SE:
            return (reinterpret_cast< JAUSoundTableSe* >(data)->mSoundSw >> 10) & 1;
        case DATA_BGM:
        case DATA_STREAM:
            return false;
        }
    }
    return false;
}
