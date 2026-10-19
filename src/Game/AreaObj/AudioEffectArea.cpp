#include "Game/AreaObj/AudioEffectArea.hpp"
#include "Game/Util.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

AudioEffectArea::AudioEffectArea(const char* pName) : AreaObj(pName), mEffectType(), _40() {
}

void AudioEffectArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);

    mEffectType = mObjArg0;
    _40 = mObjArg1;

    if (mObjArg1 < 0) {
        _40 = 64;
    }
}

void AudioEffectArea::movement() {
    if (isInVolume(*MR::getPlayerPos())) {
        MR::setAudioEffectType(mEffectType, _40, 0);
    } else {
        MR::fn_8005ADA0();
    }
}
