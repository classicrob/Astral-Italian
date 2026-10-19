#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/AudioLib/AudUtil.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/MathUtil.hpp"

// NONMATCHING: .sdata2 literal order differs; instruction bytes and referenced scalar values match.
// Sound IDs are SMG2 values from the target switch tables; SMG1 names in AudSoundId.hpp do not apply.

void AudSoundObject::modifySe_Kawamura(JAISoundHandle* pHandle, f32* pVolume, u32* pVolumeTime, f32* pPitch, u32* pPitchTime, s32 level) {
    switch (pHandle->getSound()->getID()) {
    case 0x50017:
    case 0x50211: {
        *pVolume = level / 100.0f;
        break;
    }
    case 0x501D9: {
        f32 t = level / 100.0f;
        *pVolume = t * t;
        break;
    }
    case 0x5009C:
    case 0x5009D: {
        *pVolume = level / 100.0f;
        *pVolume *= *pVolume;
        *pVolume *= 2.0f;
        if (*pVolume > 1.0f) {
            *pVolume = 1.0f;
        }
        break;
    }
    case 0x6001B: {
        f32 t = level / 100.0f;
        *pPitch = 1.0f + 0.5f * (t * t);
        break;
    }
    case 0x50036: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        t *= t;
        *pPitch = 0.9f + 0.5f * t;
        *pVolume = t;
        break;
    }
    case 0x50037: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        if (level < 40) {
            *pPitch = 1.0f;
            *pVolume = 0.0f;
        } else {
            f32 t = (level - 40) / 60.0f;
            t *= t;
            *pPitch = 0.9f + 0.4f * t;
            *pVolume = t;
        }
        break;
    }
    case 0xB: {
        if (level < 0) {
            level = 0;
        }
        if (level > 12) {
            level = 12;
        }
        writePort(pHandle, 11, level);
        break;
    }
    case 0x25: {
        if (level < 0) {
            level = 0;
        }
        while (level > 12 * 3) {
            level -= 12;
        }
        writePort(pHandle, 11, level);
        break;
    }
    case 0x5009E: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        *pPitch = 0.5f + t / 2.0f;
        *pVolume = t * t;
        break;
    }
    case 0x5009F: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        *pVolume = t * t;
        *pPitch = 0.75f + t / 4.0f;
        break;
    }
    case 0x500A9: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        t *= t;
        *pPitch = 1.0f + t;
        *pVolume = 0.7f + 0.3f * t;
        break;
    }
    case 0x400BD: {
        f32 t = level / 100.0f;
        *pPitch = 1.0f + t / 2.0f;
        *pVolume = 0.2f + t * t;
        if (*pVolume > 1.0f) {
            *pVolume = 1.0f;
        }
        break;
    }
    case 0x400BC: {
        f32 t = level / 100.0f;
        *pPitch = 0.9f + t / 2.0f;
        break;
    }
    case 0x500FC: {
        f32 t = level / 100.0f;
        *pPitch = 0.9f + t / 3.0f;
        break;
    }
    case 0x400C0:
    case 0x5027A:
    case 0x5027B: {
        f32 t = level / 100.0f;
        *pPitch = 0.75f + t / 2.0f;
        *pVolume = t * t;
        break;
    }
    case 0x40123: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        t *= t;
        *pPitch = 1.0f + t / 2.0f;
        break;
    }
    case 0xBF: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        t *= t;
        *pPitch = 0.75f + t / 4.0f;
        *pVolume = 0.5f + t / 2.0f;
        break;
    }
    case 0x40152: {
        level = MR::max(level, 0);
        level = MR::min(level, 100000);
        *pPitch = AudUtil::linerMinMax(level, 300, 2000, 2.0f, 1.85f);
        f32 wait = AudUtil::linerMinMax(level, 200, 3000, 4.0f, 24.0f);
        writePort(pHandle, 11, (u16)wait);
        break;
    }
    case 0x50354: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        t *= t;
        *pPitch = 0.25f + t;
        *pVolume = 0.3f + t / 2.0f;
        break;
    }
    case 0x20037: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 50.0f;
        *pPitch = 0.75f + t * t;
        break;
    }
    case 0x20061: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        *pVolume = t * t;
        if (*pVolume < 0.2f) {
            *pVolume = 0.2f;
        }
        *pPitch = 0.5f + t / 2.0f;
        break;
    }
    case 0x2007C: {
        if (level < 0) {
            level = 0;
        }
        if (level > 100) {
            level = 100;
        }
        f32 t = level / 100.0f;
        *pPitch = 0.75f + t / 2.0f;
        *pVolume = t * t;
        break;
    }
    }
}

bool AudSoundObject::modifyLimitedSound_Kawamura(JAISoundID soundID) {
    u32 id = soundID;
    switch (id) {
    case 0x57: {
        if (isPlayingID(0x58)) {
            stopSound(0x58, 0);
            return false;
        }
        break;
    }
    case 0x58: {
        if (isPlayingID(0x57)) {
            return true;
        }
        break;
    }
    case 0x17: {
        if (isPlayingID(0x43)) {
            return true;
        }
        break;
    }
    case 0x1004E: {
        if (isPlayingID(0x10033)) {
            return true;
        }
        break;
    }
    case 0x50448:
    case 0x50449: {
        if (AudWrap::getSystem()->isAlreadyPlayingSoundNear(id, getPos(), 800.0f)) {
            return true;
        }
        break;
    }
    case 0x50250:
    case 0x503DB:
    case 0x503DC:
    case 0x503DD:
    case 0x503DE:
    case 0x503DF:
    case 0x503E0: {
        if (AudWrap::getSystem()->isAlreadyPlayingSoundNear(id, getPos(), 600.0f)) {
            return true;
        }
        break;
    }
    case 0x50252: {
        if (AudWrap::getSystem()->isAlreadyPlayingSoundNear(id, getPos(), 500.0f)) {
            return true;
        }
        break;
    }
    case 0x50001:
    case 0x50002:
    case 0x50003:
    case 0x501EA:
    case 0x502F9: {
        if (AudWrap::getSystem()->isAlreadyPlayingSoundNear(id, getPos(), 400.0f)) {
            return true;
        }
        break;
    }
    case 0x5000C:
    case 0x50207: {
        if (AudWrap::getSystem()->isAlreadyPlayingSoundNear(id, getPos(), 350.0f)) {
            return true;
        }
        break;
    }
    case 0x50570: {
        if (MR::isPlayingStageBgmName("BGM_GAMBLE")) {
            return true;
        }
        break;
    }
    default: {
        return false;
    }
    }

    return false;
}
