#include "Game/GameAudio/ActionSoundKeeper.hpp"

#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/GameAudio/ActionSound.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"


ActionSoundKeeper::ActionSoundKeeper(s32 pId, const char* pName, TVec3f* pPos)
    : mSoundObject(nullptr), mInfo(nullptr), mSounds(nullptr), mSoundCount(0) {
    mInfo = MR::getActionSoundData(pName);
    if (mInfo != nullptr) {
        mSoundObject = new AudSoundObject(pPos, (u8)pId, nullptr);
        mSoundCount = mInfo->mCount;
        mSounds = new ActionSound*[mSoundCount];
        for (s32 i = 0; i < mSoundCount; i++) {
            mSounds[i] = new ActionSound(mInfo->getInfo(i), mSoundObject);
        }
        MR::fn_800885B0(mInfo, false);
    }
}

void ActionSoundKeeper::startSoundAuto(const char* pName, s32 pitch, s32 velocity) {
    playSounds(0, pName, pitch, velocity, -1);
}

void ActionSoundKeeper::startSound(const char* pName, s32 pitch, s32 velocity) {
    playSounds(1, pName, pitch, velocity, -1);
}

s32 ActionSoundKeeper::playSounds(s32 mode, const char* pName, s32 pitch, s32 velocity, s32 volume) {
    s32 playCount = 0;
    for (s32 i = 0; i < mSoundCount; i++) {
        ActionSound* sound = mSounds[i];
        if (sound->getId() == -1 && MR::isEqualString(sound->getName(), pName)) {
            if (sound->play(mode, pitch, velocity, volume)) {
                playCount++;
            }
        }
    }
    return playCount;
}
