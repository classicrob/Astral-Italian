#include "Game/GameAudio/AudBgmConductor.hpp"
#include "Game/AudioLib/AudBgm.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

AudBgmConductor::AudBgmConductor()
    : NameObj("AudBgmConductor"), _14(-1), mStateBits(0), mState(0), mStartDelay(-1), mFadeInDelay(-1), mStopImmediately(false) {
}

void AudBgmConductor::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, MR::MovementType_AudBgmConductor, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
    if (MR::isEqualStageName("IceVolcanoGalaxy") || MR::isEqualStageName("OceanRingGalaxy")) {
        MR::registerDemoSimpleCastAll(this);
    }
}

void AudBgmConductor::playPinchEventBgmLevel(bool secondPinch) {
    if (MR::isPlayingSubBgmID(BGM_MUTEKI_A) || MR::isPlayingSubBgmID(BGM_MUTEKI_B) || MR::isPlayingSubBgmID(BGM_FIRE_A) ||
        MR::isPlayingSubBgmID(BGM_FIRE_B) || MR::isPlayingSubBgmID(BGM_ICE_A) || MR::isPlayingSubBgmID(BGM_ICE_B)) {
        mStartDelay = -1;
        mFadeInDelay = -1;
        return;
    }

    if (!MR::isPlayerDead() && !MR::isPowerStarGetDemoActive()) {
        if (mStartDelay < 0) {
            if (secondPinch) {
                MR::startSubBGM("BGM_PINCH_2", false);
            } else {
                MR::startSubBGM("BGM_PINCH_1", false);
            }

            if (secondPinch) {
                MR::moveVolumeSubBGM(0.0f, 0);
                mFadeInDelay = 2;
            }

            mStopImmediately = false;
        }
        mStartDelay = 3;
    }
}

namespace AudBgmConductorFunction {
    s32 updatePlayerState(u32* time, s32 current, u32 bits) {
        s32 state;
        if (bits & 0x200) {
            *time = 20;
            state = 10;
        } else if (bits & 0x10) {
            *time = 20;
            state = 9;
        } else if (bits & 8) {
            *time = 20;
            state = 8;
        } else {
            *time = 20;
            state = 0;
        }
        if (state != current) {
            MR::setStageBGMState(state, *time);
            if (state == 10) {
                MR::moveStageBGMTrackVolume(15, 0.4f, *time);
            }
        }
        return state;
    }

    s32 updateSwitchState(u32* time, s32 current, u32 bits) {
        s32 state;
        if (bits & 0x40) {
            *time = 6;
            state = 2;
        } else {
            *time = 3;
            state = 1;
        }
        if (state != current) {
            MR::setStageBGMState(state, *time);
        }
        return state;
    }
}

void AudBgmConductor::movement() {
    movementMorphBgm();
    AudBgm* bgm = AudWrap::getStageBgm();
    if (bgm == nullptr) {
        return;
    }
    JAISoundID id = bgm->getSoundID();
    if (_14 != (u32)id) {
        mState = -1;
    }
    u32 time = 90;
    s32 state;
    switch (id) {
    case 0x02000039:
        if (mStateBits & 2) {
            if (mStateBits & 4) {
                time = 20;
                state = 6;
            } else {
                state = 5;
            }
        } else if (mStateBits & 4) {
            time = 20;
            state = 4;
        } else {
            if (mStateBits & 0x10) {
                state = 9;
            } else if (mStateBits & 8) {
                state = 8;
            } else {
                state = 1;
            }
            time = 20;
        }
        if (state != mState) {
            MR::setStageBGMState(state, time);
        }
        break;
    case 0x01000017: {
        f32 tempo;
        if (mStateBits & 0x10) {
            time = 20;
            tempo = 174.0f;
            state = 9;
        } else if (mStateBits & 8) {
            time = 20;
            tempo = 154.0f;
            state = 8;
        } else {
            time = 20;
            tempo = 154.0f;
            state = 0;
        }
        if (state != mState) {
            MR::setStageBGMState(state, time);
            bgm->moveTempo(tempo / 154.0f, time);
        }
        break;
    }
    case 0x0100001e: {
        f32 tempo = 142.0f;
        if (mStateBits & 0x20) {
            time = 6;
            state = 2;
        } else {
            time = 6;
            state = 0;
        }
        if (state != mState) {
            MR::setStageBGMState(state, time);
            bgm->moveTempo(tempo / 142.0f, time);
        }
        break;
    }
    case 0x0200000b:
    case 0x02000018:
        state = AudBgmConductorFunction::updateSwitchState(&time, mState, mStateBits);
        break;
    case 0x02000025:
        if (MR::isEqualStageName("MadnessOnimasuGalaxy")) {
            state = AudBgmConductorFunction::updateSwitchState(&time, mState, mStateBits);
        } else {
            state = AudBgmConductorFunction::updatePlayerState(&time, mState, mStateBits);
        }
        break;
    case 0x02000020:
    case 0x02000021:
        if (mStateBits & 0x80) {
            time = 30;
            state = 2;
        } else {
            time = 180;
            state = 1;
        }
        if (state != mState) {
            MR::setStageBGMState(state, time);
        }
        break;
    case 0x01000008:
    case 0x0100002c:
    case 0x0100002d:
    case 0x01000039:
    case 0x01000070:
    case 0x02000008:
    case 0x0200000d:
    case 0x0200000f:
    case 0x02000013:
    case 0x0200001b:
    case 0x0200001e:
    case 0x0200003a:
    case 0x02000046:
    case 0x02000054:
    case 0x02000055:
    case 0x02000058:
    case 0x02000059:
    case 0x0200005a:
        state = AudBgmConductorFunction::updatePlayerState(&time, mState, mStateBits);
        break;
    case 0x0200003b:
    case 0x0200003d:
    case 0x02000040:
    case 0x02000041:
    case 0x02000042:
    case 0x0200004a:
    case 0x02000056:
        if (mStateBits & 0x100) {
            time = 60;
            state = 2;
        } else {
            time = 90;
            state = 1;
        }
        if (state != mState) {
            MR::setStageBGMState(state, time);
        }
        break;
    default:
        state = 0;
        break;
    }
    _14 = id;
    mState = state;
    mStateBits = 0;
}

void AudBgmConductor::setStateBit(u32 bit) {
    mStateBits |= bit;
}

void AudBgmConductor::stopPinchEventBgmSoon() {
    mStartDelay = 0;
    mStopImmediately = true;
}

void AudBgmConductor::movementMorphBgm() {
    if (MR::isPlayingSubBgmID(0x0100000b) || MR::isPlayingSubBgmID(0x0100000c) || MR::isPlayingSubBgmID(0x0100000f) ||
        MR::isPlayingSubBgmID(0x01000010) || MR::isPlayingSubBgmID(0x0100000d) || MR::isPlayingSubBgmID(0x0100000e)) {
        return;
    }

    if (MR::isPowerStarGetDemoActive() || MR::isPlayerDead()) {
        mStartDelay = -1;
        mFadeInDelay = -1;
        mStopImmediately = false;
        return;
    }

    if (mStartDelay >= 0) {
        if (mFadeInDelay == 0) {
            MR::moveVolumeSubBGM(1.0f, 90);
            mFadeInDelay = -1;
        }

        if (mFadeInDelay > 0) {
            mFadeInDelay--;
        }

        mStartDelay--;
        if (mStartDelay <= 0) {
            if (mStopImmediately) {
                MR::stopSubBGM(1);
            } else {
                MR::stopSubBGM(90);
            }

            mStartDelay = -1;
            mFadeInDelay = -1;
            mStopImmediately = false;
        }
    }
}
