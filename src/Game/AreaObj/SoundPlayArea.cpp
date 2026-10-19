#include "Game/AreaObj/SoundPlayArea.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <cstdio>
#include <new>

// PLACEHOLDER(objects): unnamed target symbol fn_80222AC0, declared extern "C" so the reloc name matches; resolve when fn_80222AC0 gets its real name in symbols.txt
extern "C" void* fn_80222AC0(void*, const char*);
extern "C" void fn_80222A80(void*, const char*, s32, s32, s32);

static const char* sSoundName = "PlaySound";

SoundPlayArea::SoundPlayArea(const char* pName) : AreaObj(pName) {
    setupSoundPos();
}

void SoundPlayArea::setupSoundPos() {
    mCenter.x = 0.0f;
    mCenter.y = 0.0f;
    mCenter.z = 0.0f;
    mIsSoundStarted = false;
    _55 = false;
    mSoundObj = nullptr;
}

void SoundPlayArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::calcCubePos(this, &mCenter);

    char name[0x100];
    snprintf(name, 0x100, "SoundPlayArea%d", mObjArg0);

    mSoundObj = new ActionSoundKeeper(4, name, &mCenter);
    void* sound = mSoundObj;

    sound = fn_80222AC0(sound, sSoundName);
    if (sound) {
        _55 = reinterpret_cast< ActionSoundInfo* >(sound)->fn_80220830();
    }

    MR::connectToSceneAreaObj(this);
    MR::registerDemoSimpleCastAll(this);
}

void SoundPlayArea::movement() {
    if (isInVolume(*MR::getPlayerPos())) {
        MR::getAreaCenterPos(&mCenter, this);
        mSoundObj->update();

        if (!_55) {
            if (!mIsSoundStarted) {
                mSoundObj->startSound(sSoundName, -1, -1);
                mIsSoundStarted = true;
            }
        } else {
            fn_80222A80(mSoundObj, sSoundName, -1, -1, -1);
        }
    } else {
        mIsSoundStarted = false;
    }
}
