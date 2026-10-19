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

JASChannel* JASBank::noteOn(JASBank const* pBank, int progNo, u8 pitch, u8 velocity, u16 priority, JASChannel::Callback pCallback,
                            void* pChannelMgr) {
    if (progNo >= 0xf0) {
        return noteOnOsc(progNo - 0xf0, pitch, velocity, priority, pCallback, pChannelMgr);
    }

    if (pBank == nullptr) {
        return nullptr;
    }

    JASInstParam params;
    if (!pBank->getInstParam(progNo, pitch, velocity, &params)) {
        return nullptr;
    }

    JASWaveBank* waveBank = pBank->getWaveBank();
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

    JASChannel* channel = new JASChannel(pCallback, pChannelMgr);
    if (!channel) {
        return nullptr;
    }

    channel->setPriority(priority);
    channel->mWaveInfo = *waveInfo;
    channel->mWavePtr = wavePtr;
    channel->mChannelType = params.mChannelType;
    channel->setBankDisposeID(pBank);
    channel->setInitPitch(params.mPitch * (waveInfo->mSampleRate / JASDriver::getDacRate()));
    if (params._24 == 0) {
        channel->setKey(pitch - waveInfo->mBaseKey);
    }
    channel->setInitVolume(params.mVolume);
    channel->setVelocity(velocity);
    channel->setInitPan(params.mPan);
    channel->setInitFxmix(params.mFxMix);
    channel->setInitDolby(params.mDolby);
    for (u32 i = 0; i < params.mNumOscillators; i++) {
        channel->setOscInit(i, params.mOscillatorData[i]);
    }
    channel->setDirectRelease(params.mDirectRelease);
    if (!channel->play()) {
        return nullptr;
    }
    return channel;
}

JASInstParam::JASInstParam()
    : mChannelType(JASChannel::CH_WAVE), _18(0), mOscillatorData(nullptr), mNumOscillators(0), _24(false), mDirectRelease(0) {
}
