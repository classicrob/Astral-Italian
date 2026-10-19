#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

// NONMATCHING: .sdata2 literal order differs; instruction bytes and referenced scalar values match.
// Sound IDs are SMG2 values from the target switch tables; SMG1 names in AudSoundId.hpp do not apply.

void AudSoundObject::modifySe_Gohara(JAISoundHandle* pHandle, f32* pVolume, u32* pVolumeTime, f32* pPitch, u32* pPitchTime, s32 levelA,
                                     s32 levelB) {
    switch (pHandle->getSound()->getID()) {
    case 0x400AD: {
        *pVolume = levelA / 100.0f;
        break;
    }
    case 0x400D3: {
        if (levelA >= 50) {
            *pVolume = levelA / 100.0f;
        } else {
            *pVolume = 0.0f;
        }
        break;
    }
    case 0x400B6:
    case 0x400B7:
    case 0x400B8:
    case 0x400D4:
    case 0x400EC:
    case 0x400ED:
    case 0x400FA:
    case 0x400FB: {
        if (levelB == -1) {
            levelB = 0x7F - 1;
        }
        *pVolume = levelA / 1000.0f;
        writePort(pHandle, 10, levelB);
        break;
    }
    case 0x87:
    case 0x88:
    case 0x200A8:
    case 0x200AB:
    case 0x200AC:
    case 0x200B5:
    case 0x200B6:
    case 0x200B8:
    case 0x200B9:
    case 0x200BA:
    case 0x200BC:
    case 0x401B0:
    case 0x503A9:
    case 0x503AA:
    case 0x503FB:
    case 0x503FC:
    case 0x503FD:
    case 0x50403:
    case 0x50409:
    case 0xB008A:
    case 0xB008B:
    case 0xB008C:
    case 0xB0093:
    case 0xB0094:
    case 0xB0096:
    case 0xB0097:
    case 0xB009C:
    case 0xB009D:
    case 0xB010D:
    case 0xB010E: {
        if (levelA >= 0) {
            *pVolume = levelA / 1000.0f;
        }
        if (levelB >= 0) {
            *pPitch = levelB / 1000.0f;
        }
        break;
    }
    case 0x503BC: {
        *pPitch = levelA / 1000.0f;
        break;
    }
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x92:
    case 0xEE:
    case 0xEF:
    case 0xFF:
    case 0x100:
    case 0x101:
    case 0x102:
    case 0x103:
    case 0x104:
    case 0x105:
    case 0x106: {
        writePort(pHandle, 11, levelA);
        break;
    }
    case 0x128: {
        levelA = MR::max(levelA, 0);
        levelA = MR::min(levelA, 1000);
        levelB = MR::max(levelB, 10);
        levelB = MR::min(levelB, 1000);
        *pPitch = levelA / 1000.0f;
        f32 tempo = levelB / 100.0f;
        pHandle->getSound()->getTempoMgr()->setTempo(tempo);
        break;
    }
    }
}
