#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSoundObjHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include <JSystem/JGadget/hashcode.hpp>

AudSoundObject::AudSoundObject(TVec3f* pPos, u8 numHandles, JKRHeap* pHeap)
    : JAUSoundObject(pPos, numHandles, pHeap), JKRDisposer(), mSeVersion(0), mMapCode(0), mMapCodeExtra(0), mHashDatas(nullptr), mNumSounds(0) {
    mNumHandles = numHandles;
    if (mNumHandles > 0) {
        mHashDatas = new AudSoundObjHashData[numHandles];

        for (s32 idx = 0; idx < mNumHandles; idx++) {
            mHashDatas[idx].init();
        }
    }
}

AudSoundObject::~AudSoundObject() {
    AudWrap::getSoundObjHolder()->remove(this);
}

void AudSoundObject::addToSoundObjHolder() {
    AudWrap::getSoundObjHolder()->add(this);
}

void AudSoundObject::setTrans(TVec3f* pos) {
    mPos = pos;
}




bool AudSoundObject::writePort(JAISoundHandle* pHandle, u32 port, u16 value) {
    if (pHandle == nullptr) {
        return false;
    }

    if (!pHandle->isSoundAttached()) {
        return false;
    }

    BOOL en = OSDisableInterrupts();

    JASTrack* track = pHandle->getSound()->getTrack();
    if (track != nullptr) {
        track->writePort(port, value);
        OSRestoreInterrupts(en);
        return true;
    }
    OSRestoreInterrupts(en);
    return false;
}

bool AudSoundObject::isLimitedSound(JAISoundID soundID, bool isLevel) {
    if (soundID == -1) {
        return true;
    }

    if (AudWrap::getSystem()->isRegisteredLimitedSound(soundID)) {
        return true;
    }

    if (modifyLimitedSound(soundID)) {
        return true;
    }

    AudSoundInfo* info = AudWrap::getSoundInfo();
    if (info != nullptr && !isLevel && info->isLimitCountPlaying(soundID)) {
        return true;
    }

    return false;
}

JAISoundHandle* AudSoundObject::startSound(JAISoundID soundID) {
    soundID = convertSoundIdFromSeVersion(soundID);
    if (!isEnableStartSound(soundID)) {
        return nullptr;
    }

    if (soundID == -1) {
        return nullptr;
    }

    if (AudWrap::getSystem()->mIsStartSoundBlocked && soundID.getGroupID() != SE_HOME_BUTTON_MENU && soundID.getGroupID() != SE_SYSTEM) {
        return nullptr;
    }

    if (isLimitedSound(soundID, false)) {
        return nullptr;
    }

    limitVoiceOne(soundID);
    JAISoundHandle* handle = JAUSoundObject::startSound(soundID);
    setMapCodeToPort(handle, soundID);
    setCutoffToPort(handle, soundID);
    return handle;
}

JAISoundHandle* AudSoundObject::startLevelSound(JAISoundID soundID) {
    soundID = convertSoundIdFromSeVersion(soundID);
    if (!isEnableStartSound(soundID)) {
        return nullptr;
    }

    if (soundID == -1) {
        return nullptr;
    }

    if (AudWrap::getSystem()->mIsStartLevelSoundBlocked && soundID.getGroupID() != SE_HOME_BUTTON_MENU && soundID.getGroupID() != SE_SYSTEM) {
        return nullptr;
    }

    if (isLimitedSound(soundID, true)) {
        return nullptr;
    }

    JAISoundHandle* handle = JAUSoundObject::startLevelSound(soundID);
    setMapCodeToPort(handle, soundID);
    setCutoffToPort(handle, soundID);
    return handle;
}

JAISoundHandle* AudSoundObject::startLevelSound(const char* pName) {
    JAISoundID id = convertNameToLevelSEID(pName);
    return startLevelSound(id);
}

JAISoundHandle* AudSoundObject::startSoundParam(JAISoundID soundID, s32 volume, s32 param) {
    JAISoundHandle* handle = startSound(soundID);
    modifySoundParams(handle, volume, param);
    return handle;
}

JAISoundHandle* AudSoundObject::startLevelSoundParam(JAISoundID soundID, s32 volume, s32 param) {
    JAISoundHandle* handle = startLevelSound(soundID);
    modifySoundParams(handle, volume, param);
    return handle;
}

JAISoundHandle* AudSoundObject::startLevelSoundParam(const char* name, s32 volume, s32 param) {
    JAISoundID id = convertNameToLevelSEID(name);
    return startLevelSoundParam(id, volume, param);
}

void AudSoundObject::modifySoundParams(JAISoundHandle* handle, s32 volume, s32 param) {
    if (handle != nullptr && handle->isSoundAttached()) {
        f32 finalVolume = 1.0f;
        f32 pitch = 1.0f;
        u32 volumeTime = 0;
        u32 pitchTime = 0;
        modifySe_Kawamura(handle, &finalVolume, &volumeTime, &pitch, &pitchTime, volume);
        modifySe_Takezawa(handle, &finalVolume, &volumeTime, &pitch, &pitchTime, volume, param);
        modifySe_Gohara(handle, &finalVolume, &volumeTime, &pitch, &pitchTime, volume, param);
        AudWrap::getSystem()->mAudEffector->modifySoundPitch(handle, &pitch, &volumeTime);
        handle->getSound()->mParams.mMove.moveVolume(finalVolume, volumeTime);
        handle->getSound()->mParams.mMove.movePitch(pitch, pitchTime);
    }
}

bool AudSoundObject::modifyLimitedSound(JAISoundID soundID) {
    if (modifyLimitedSound_Kawamura(soundID)) {
        return true;
    }

    if (modifyLimitedSound_Takezawa(soundID)) {
        return true;
    }

    return false;
}


bool AudSoundObject::isPlayingID(JAISoundID soundID) {
    return getHandleSoundID(soundID) != nullptr;
}

void AudSoundObject::releaseHandle(JAISoundID soundID) {
    JAISoundHandle* handle = getHandleSoundID(soundID);
    if (handle != nullptr) {
        handle->getSound()->releaseHandle();
    }
}



void AudSoundObject::setMapCode(s32 code) {
    if (code < 0) {
        mMapCode = 0;
    } else {
        mMapCode = code;
    }
}

void AudSoundObject::setMapCodeExtra(s32 code) {
    if (code == 0) {
        code = 1;
    }

    if (code < 0) {
        mMapCodeExtra = 0;
    } else {
        mMapCodeExtra = code;
    }
}

s32 AudSoundObject::getMapCode() const {
    if (mMapCodeExtra > 0) {
        return mMapCodeExtra;
    } else {
        return mMapCode;
    }
}

void AudSoundObject::clearMapCode() {
    mMapCode = 0;
    mMapCodeExtra = 0;
}

JAISoundID AudSoundObject::convertNameToLevelSEID(const char* pName) {
    u32 hash = 0;
    bool found = false;

    for (s32 idx = 0; idx < mNumSounds; idx++) {
        if (mHashDatas[idx].mName == pName) {
            hash = mHashDatas[idx].mHash;
            found = true;
        }
    }

    if (!found) {
        hash = JGadget::getHashCode(pName);

        for (s32 idx = 0; idx < mNumSounds; idx++) {
            mHashDatas[idx].mID = (u32)mHashDatas[idx].mID + 1;
        }

        if (mNumSounds < mNumHandles) {
            AudSoundObjHashData* data = &mHashDatas[mNumSounds];
            data->mName = pName;
            data->mHash = hash;
            data->mID = 0;
            mNumSounds++;
        } else {
            s32 i = 0;
            for (s32 idx = 0; idx < mNumSounds; idx++) {
                if (mHashDatas[i].mID < mHashDatas[idx].mID) {
                    i = idx;
                }
            }
            AudSoundObjHashData* data = &mHashDatas[i];
            data->mName = pName;
            data->mHash = hash;
            data->mID = 0;
        }
    }

    return AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName, hash);
}

void AudSoundObject::limitVoiceOne(JAISoundID soundID) {
    u8 group = soundID.getGroupID();

    if (group == SE_ENEMY_VOICE) {
        stopCategorySound(SE_ENEMY_VOICE, 0);
        return;
    }

    if (group == SE_BOSS_VOICE) {
        stopCategorySound(SE_BOSS_VOICE, 0);
        return;
    }

    if (group == SE_PLAYER_VOICE) {
        stopCategorySound(SE_PLAYER_VOICE, 0);
        return;
    }

    if (group == SE_SUPPORTER_VOICE) {
        stopCategorySound(SE_SUPPORTER_VOICE, 0);
        return;
    }
}


// NONMATCHING: regalloc; equality comparison operands are reversed.
void AudSoundObject::stopCategorySound(u32 category, u32 time) {
    for (s32 i = 0; i < JAISoundHandles::mNumHandles; i++) {
        JAISound* sound = getSound(i);
        if (sound != nullptr && sound->getID().getGroupID() == category) {
            stopSound(sound->getID(), time);
        }
    }
}

// NONMATCHING: regalloc; floating-point temporaries use different registers.
void AudSoundObject::setMapCodeToPort(JAISoundHandle* pHandle, JAISoundID soundID) {
    if (pHandle == nullptr) {
        return;
    }

    if (!pHandle->isSoundAttached()) {
        return;
    }

    s32 mapCode = getMapCode();
    if (mapCode <= 0) {
        return;
    }
    if (mapCode == 17) {
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiLight()) {
            mapCode = 8;
        } else {
            mapCode = 4;
        }
    }
    if (mapCode == 18) {
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiLight()) {
            mapCode = 1;
        } else {
            mapCode = 4;
        }
    }
    if (mapCode == 19) {
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiLight()) {
            mapCode = 1;
        } else {
            mapCode = 7;
        }
    }
    if (mapCode == 21) {
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiLight()) {
            mapCode = 12;
        } else {
            mapCode = 4;
        }
    }
    if (mapCode == 22) {
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiLight()) {
            mapCode = 11;
        } else {
            mapCode = 4;
        }
    }
    s32 portMapCode = mapCode;
    if (mapCode >= 20) {
        portMapCode = mapCode - 3;
    }
    switch (soundID) {
    case 0x20006:
    case 0x20013:
    case 0x20017:
    case 0x20025:
    case 0x20026:
    case 0x20027:
    case 0x20028:
    case 0x20029:
    case 0x2002a:
    case 0x2002b:
    case 0x20038:
    case 0x902c8:
    case 0x902c9:
    case 0x902ca:
    case 0x902cb:
        s32 divisor = 7;
        writePort(pHandle, 12, (portMapCode - 1) / divisor);
        writePort(pHandle, 11, ((portMapCode - 1) % divisor) * 12);
        break;
    }
    switch (soundID) {
    case 0x20025:
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiDash()) {
            MR::startActionSoundPlayer("DashFootNoteAddL", -1, -1);
        } else if (MR::isPlayerElementModeYoshi() && MR::isYoshiTongueReadyToEat()) {
            MR::startActionSoundPlayer("HoldFootNoteAddL", -1, -1);
        } else if (MR::isPlayerElementModeRock()) {
            PSVECMag(reinterpret_cast<const Vec*>(MR::getPlayerVelocity()));
            f32 x = MR::getPlayerStickX() * MR::getPlayerStickX();
            f32 y = MR::getPlayerStickY() * MR::getPlayerStickY();
            f32 stick = y + x;
            MR::startSoundPlayer("SE_PM_ROCK_MR_FOOT_ADD_L", 300.0f + 700.0f * stick, 700.0f + 300.0f * stick);
        }
        break;
    case 0x20026:
        if (MR::isPlayerElementModeYoshi() && MR::isPlayerElementModeYoshiDash()) {
            MR::startActionSoundPlayer("DashFootNoteAddR", -1, -1);
        } else if (MR::isPlayerElementModeYoshi() && MR::isYoshiTongueReadyToEat()) {
            MR::startActionSoundPlayer("HoldFootNoteAddR", -1, -1);
        } else if (MR::isPlayerElementModeRock()) {
            PSVECMag(reinterpret_cast<const Vec*>(MR::getPlayerVelocity()));
            f32 x = MR::getPlayerStickX() * MR::getPlayerStickX();
            f32 y = MR::getPlayerStickY() * MR::getPlayerStickY();
            f32 stick = y + x;
            MR::startSoundPlayer("SE_PM_ROCK_MR_FOOT_ADD_R", 300.0f + 700.0f * stick, 700.0f + 300.0f * stick);
        }
        break;
    case 0x20029:
        if (MR::isPlayerElementModeRock()) {
            MR::startSoundPlayer("SE_PM_ROCK_MR_FOOT_ADD_R", -1, -1);
        }
        break;
    case 0x2002a:
        if (mapCode == 20) {
            MR::startSoundPlayer("SE_PM_CLOUD_STEP_RIDE_ON", -1, -1);
        }
        if (MR::isPlayerElementModeRock()) {
            MR::startSoundPlayer("SE_PM_ROCK_MR_FOOT_ADD_LAND", -1, -1);
        }
        break;
    case 0x20038:
        if (MR::isPlayerElementModeRock()) {
            MR::startSoundPlayer("SE_PM_ROCK_MR_FOOT_ADD_LAND", -1, -1);
        }
        break;
    }
}

void AudSoundObject::setCutoffToPort(JAISoundHandle* pHandle, JAISoundID soundID) {
    if (pHandle == nullptr) {
        return;
    }

    if (!pHandle->isSoundAttached()) {
        return;
    }

    AudSoundInfo* info = AudWrap::getSoundInfo();
    if (info == nullptr) {
        return;
    }
    if (info->isUseLpf(soundID)) {
        s32 value = AudWrap::getSystem()->mAudEffector->mCutoff;
        if (value < 0 || value > 127) {
            value = 127;
        }
        writePort(pHandle, 10, value);
    } else {
        writePort(pHandle, 10, 127);
    }
}


AudSoundObjHashData::AudSoundObjHashData() {
    init();
}

void AudSoundObjHashData::init() {
    mName = nullptr;
    mHash = 0;
    mID = 0;
}
