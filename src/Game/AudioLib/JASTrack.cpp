#include "Game/AudioLib/AudUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASBankTable.hpp>
#include <JSystem/JAudio2/JASBasicBank.hpp>
#include <JSystem/JAudio2/JASBasicInst.hpp>
#include <JSystem/JAudio2/JASChannel.hpp>
#include <JSystem/JAudio2/JASMutex.hpp>
#include <JSystem/JAudio2/JASTrack.hpp>
#include <JSystem/JAudio2/JASWaveArcLoader.hpp>
#include <JSystem/JSupport/JSupport.hpp>
#include <cstring>

JASChannel* JASTrack::channelStart(JASTrack::TChannelMgr* pChannelMgr, u32 pitch, u32 velocity, u32 updateTimer) {
    JASBank* bank = nullptr;
    if (mBankTable != nullptr) {
        bank = mBankTable->getBank(mBankNumber);
    }

    JASChannel* channel = JASBank::noteOn(bank, mProgNumber, pitch, velocity, mNoteOnPrio | mReleasePrio << 8, channelUpdateCallback, pChannelMgr);
    if (channel == nullptr) {
        if (AudUtil::isProperCurrentStageOrAudioDebugMode()) {
            JASInstParam params;
            if (!bank->getInstParam(mProgNumber, JSULoByte(pitch), JSULoByte(velocity), &params)) {
                return nullptr;
            }

            JASWaveBank* waveBank = bank->getWaveBank();
            if (!waveBank) {
                return nullptr;
            }

            JASWaveHandle* waveHandle = waveBank->getWaveHandle(params._18);
            if (!waveHandle) {
                return nullptr;
            }

            const JASWaveInfo* waveInfo = waveHandle->getWaveInfo();
            if (!waveInfo) {
                return nullptr;
            }

            int wavePtr = waveHandle->getWavePtr();
            if (!wavePtr) {
                return nullptr;
            }
        }
        return nullptr;
    }

    channel->setUpdateTimer(updateTimer);
    channel->setSkipSamples(mSkipSample);
    channel->setVibrateDelay(mVibDelay);
    channel->setTremoloDelay(mTremDelay);
    channel->setParams(pChannelMgr->mParams);

    for (u32 i = 0; i < 6; i++) {
        channel->setMixConfig(i, mMixConfig[i]);
    }

    overwriteOsc(channel);
    if (mDirectRelease != 0) {
        channel->setDirectRelease(mDirectRelease);
    }

    return channel;
}
