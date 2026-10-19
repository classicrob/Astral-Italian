#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/AudioLib/AudUtil.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

// NONMATCHING: .sdata2 literal order differs; instruction bytes and referenced scalar values match.
// Sound IDs are SMG2 values from the target switch tables; SMG1 names in AudSoundId.hpp do not apply.

namespace {
    static const s32 cMagicPntGVolMinPrm = 100;
    static const s32 cMagicPntGVolMaxPrm = 1500;
    static const f32 cMagicPntGVolMin = 0.8f;
    static const f32 cMagicPntGVolMax = 1.0f;
    static const s32 cMagicPntGPitMinPrm = 100;
    static const s32 cMagicPntGPitMaxPrm = 2000;
    static const f32 cMagicPntGPitMin = 0.5f;
    static const f32 cMagicPntGPitMax = 2.0f;
    static const s32 cUnizoRollVolMinPrm = 200;
    static const s32 cUnizoRollVolMaxPrm = 700;
    static const f32 cUnizoRollVolMin = 0.1f;
    static const f32 cUnizoRollVolMax = 1.0f;
    static const s32 cUnizoRollPitMinPrm = 50;
    static const s32 cUnizoRollPitMaxPrm = 1000;
    static const f32 cUnizoRollPitMin = 0.8f;
    static const f32 cUnizoRollPitMax = 1.0f;
    static const f32 cUnizoRollGroundPitMin = 0.9f;
    static const f32 cUnizoRollGroundPitMax = 1.0f;
    static const s32 cTorpedoPitMinPrm = 300;
    static const s32 cTorpedoPitMaxPrm = 2000;
    static const f32 cTorpedoPitMin = 1.27f;
    static const f32 cTorpedoPitMax = 1.2f;
    static const s32 cTorpedoWiatMinPrm = 200;
    static const s32 cTorpedoWiatMaxPrm = 3000;
    static const f32 cTorpedoWiatMin = 4.0f;
    static const f32 cTorpedoWiatMax = 24.0f;
};  // namespace

void AudSoundObject::modifySe_Takezawa(JAISoundHandle* pHandle, f32* pVolume, u32* pVolumeTime, f32* pPitch, u32* pPitchTime, s32 levelA,
                                       s32 levelB) {
    switch (pHandle->getSound()->getID()) {
    case 0xB0000:
    case 0xB0006: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 10000);
        *pVolume = AudUtil::linerMax(levelA, 100, 1.0f);
        break;
    }
    case 0xB0009: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 1000000);
        *pVolume = AudUtil::linerMax(levelA, 10000, 1.0f);
        break;
    }
    case 0xB0007:
    case 0xB000E: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = AudUtil::linerMax(levelA, 20, 1.0f);
        break;
    }
    case 0x50079: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        *pVolume = AudUtil::linerMinMax(levelA, ::cMagicPntGVolMinPrm, ::cMagicPntGVolMaxPrm, ::cMagicPntGVolMax, ::cMagicPntGVolMin);
        if (levelA <= ::cMagicPntGPitMinPrm) {
            *pPitch = ::cMagicPntGPitMin;
        } else if (levelA >= ::cMagicPntGPitMaxPrm) {
            *pPitch = ::cMagicPntGPitMax;
        } else {
            f32 d = ::cMagicPntGPitMaxPrm - ::cMagicPntGPitMinPrm;
            f32 t = levelA - ::cMagicPntGPitMinPrm;
            *pPitch = (t * t) / (d * d) * (::cMagicPntGPitMax - ::cMagicPntGPitMin) + ::cMagicPntGPitMin;
        }
        break;
    }
    case 0x500B1: {
        s32 pitchLevel = (u32)levelA >> 16;
        s32 volumeLevel = levelA & 0xFFFF;
        pitchLevel = MR::max(pitchLevel, 0);
        pitchLevel = MR::min(pitchLevel, 100000);
        volumeLevel = MR::max(volumeLevel, 0);
        volumeLevel = MR::min(volumeLevel, 100000);
        *pVolume = AudUtil::linerMinMax(volumeLevel, 10, 600, 0.0f, 1.0f);
        *pPitch = AudUtil::linerMinMax(pitchLevel, 10, 500, 1.0f, 1.5f);
        break;
    }
    case 0xA9:
    case 0xAA:
    case 0x500D3:
    case 0x500D4: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 12);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0x9018F: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        if (levelA >= 200) {
            *pVolume = AudUtil::linerMinMax(levelA, ::cUnizoRollVolMinPrm, ::cUnizoRollVolMaxPrm, ::cUnizoRollVolMin, ::cUnizoRollVolMax);
        } else {
            *pVolume = 0.0f;
        }
        *pPitch = AudUtil::linerMinMax(levelA, ::cUnizoRollPitMinPrm, ::cUnizoRollPitMaxPrm, ::cUnizoRollGroundPitMin, ::cUnizoRollGroundPitMax);
        break;
    }
    case 0x900BD:
    case 0x9016C: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        if (levelA >= 200) {
            *pVolume = AudUtil::linerMinMax(levelA, ::cUnizoRollVolMinPrm, ::cUnizoRollVolMaxPrm, ::cUnizoRollVolMin, ::cUnizoRollVolMax);
        } else {
            *pVolume = 0.0f;
        }
        *pPitch = AudUtil::linerMinMax(levelA, ::cUnizoRollPitMinPrm, ::cUnizoRollPitMaxPrm, ::cUnizoRollPitMin, ::cUnizoRollPitMax);
        break;
    }
    case 0x900C0:
    case 0x90191: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        if (levelA >= 50) {
            *pVolume = AudUtil::linerMinMax(levelA, 50, 200, 0.3f, 1.0f);
        } else {
            *pVolume = 0.0f;
        }
        *pPitch = AudUtil::linerMinMax(levelA, 5, 100, 0.9f, 1.0f);
        break;
    }
    case 0x900BF:
    case 0x90190: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        if (levelA >= 100) {
            *pVolume = AudUtil::linerMinMax(levelA, 100, 800, 0.5f, 1.0f);
        } else {
            *pVolume = 0.0f;
        }
        *pPitch = AudUtil::linerMinMax(levelA, 100, 800, 0.9f, 1.0f);
        break;
    }
    case 0x40081: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        *pVolume = AudUtil::linerMinMax(levelA, 100, 2000, 0.3f, 1.0f);
        *pPitch = AudUtil::linerMinMax(levelA, 100, 2000, 0.6f, 1.0f);
        break;
    }
    case 0x900F3: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100000);
        *pPitch = AudUtil::linerMinMax(levelA, ::cTorpedoPitMinPrm, ::cTorpedoPitMaxPrm, ::cTorpedoPitMin, ::cTorpedoPitMax);
        f32 wait = AudUtil::linerMinMax(levelA, ::cTorpedoWiatMinPrm, ::cTorpedoWiatMaxPrm, ::cTorpedoWiatMin, ::cTorpedoWiatMax);
        writePort(pHandle, 11, wait);
        break;
    }
    case 0x9001A: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 10000);
        if (levelA >= 100) {
            *pVolume = AudUtil::linerMinMax(levelA, 100, 1000, 0.6f, 1.0f);
        } else {
            *pVolume = 0.0f;
        }
        break;
    }
    case 0xC0013:
    case 0xC0014:
    case 0xC0015: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 127);
        writePort(pHandle, 13, levelA);
        break;
    }
    case 0xB0082:
    case 0xB0084: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x90164:
    case 0x90165: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x40075: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0xB00A3: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 127);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0xB00A8: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 127);
        writePort(pHandle, 13, levelA);
        break;
    }
    case 0xB00A9: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0xA007F:
    case 0xB00BA:
    case 0xB00BB: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        if (levelB > 0) {
            levelB = MR::min(levelB, 100);
            pHandle->getSound()->getAuxiliary().moveFxMix(levelB / 100.0f, 0);
        }
        break;
    }
    case 0xA0084: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        if (levelB > 0) {
            levelB = MR::min(levelB, 100);
            pHandle->getSound()->getAuxiliary().moveFxMix(levelB / 100.0f, 0);
        }
        break;
    }
    case 0xB00B8:
    case 0xB00B9: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 127);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0x901AE: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x901AF: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        levelB = MR::max(levelB, 0);
        levelB = MR::min(levelB, 100);
        *pVolume = levelA / 100.0f;
        writePort(pHandle, 13, levelB);
        break;
    }
    case 0xAC: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 6);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0x901F4: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x901FA: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 200);
        *pPitch = levelA / 100.0f;
        break;
    }
    case 0x40187: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x5037A: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pPitch = levelA / 100.0f;
        break;
    }
    case 0x5037B: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 12);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0xE3: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        levelB = MR::max(levelB, 10);
        levelB = MR::min(levelB, 1000);
        *pPitch = levelA / 100.0f;
        f32 tempo = levelB / 100.0f;
        pHandle->getSound()->getTempoMgr()->setTempo(tempo);
        break;
    }
    case 0x401A6: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 12);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0xEC: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 7);
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0x50590:
    case 0x50591: {
        const TVec3f* pPos = getPos();
        f32 dist = 35000.0f;
        if (pPos != nullptr) {
            dist = PSVECDistance(&AudMicWrap::getMicPos(), pPos);
            dist = MR::max(dist, 0.0f);
            dist = MR::min(dist, 35000.0f);
        }
        *pVolume = AudUtil::linerMinMax(35000.0f - dist, 0.0f, 35000.0f, 0.0f, 1.3f);
        break;
    }
    case 0x118: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 100);
        *pVolume = AudUtil::linerMinMax(levelA, 0, 100, 0.0f, 1.0f);
        break;
    }
    case 0x504F3:
    case 0x50096:
    case 0x50548:
    case 0x50549:
    case 0x5054C:
    case 0x5054D:
    case 0x5054F:
    case 0x50550: {
        if (MR::isExecScenarioStarter() || MR::isStageStateScenarioOpeningCamera()) {
            *pVolume = 0.0f;
        }
        break;
    }
    }
}

bool AudSoundObject::modifyLimitedSound_Takezawa(JAISoundID soundID) {
    if (!isPlayingID(soundID)) {
        switch (soundID) {
        case 0xC0013:
        case 0xC0014:
        case 0xC0015: {
            if (isPlayingID(0xC0013) || isPlayingID(0xC0014) || isPlayingID(0xC0015)) {
                return true;
            }
            break;
        }
        default: {
            return false;
        }
        }
    }

    return false;
}
