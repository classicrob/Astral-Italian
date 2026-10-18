#include "Game/AreaObj/SoundPlayArea.hpp"
#include "Game/Util.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <cstdio>
#include <new>

extern "C" void* fn_80222860(void*, s32, const char*, TVec3f*);
extern "C" void* fn_80222AC0(void*, const char*);
extern "C" bool fn_80220830(void*);
extern "C" void fn_80222960(void*);
extern "C" void fn_80222A60(void*, const char*, s32, s32);
extern "C" void fn_80222A80(void*, const char*, s32, s32, s32);

static const char* sSoundName = "PlaySound";

SoundPlayArea::SoundPlayArea(const char* pName) : AreaObj(pName) {
    setupSoundPos();
}

void SoundPlayArea::setupSoundPos() {
    mCenter.x = 0.0f;
    mCenter.y = 0.0f;
    mCenter.z = 0.0f;
    _54 = false;
    _55 = false;
    mSoundObj = nullptr;
}

void SoundPlayArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::calcCubePos(this, &mCenter);

    char name[0x100];
    snprintf(name, 0x100, "SoundPlayArea%d", mObjArg0);

    void* sound = operator new(0x10);
    if (sound) {
        sound = fn_80222860(sound, 4, name, &mCenter);
    }
    mSoundObj = sound;

    sound = fn_80222AC0(sound, sSoundName);
    if (sound) {
        _55 = fn_80220830(sound);
    }

    MR::connectToSceneAreaObj(this);
    MR::registerDemoSimpleCastAll(this);
}

void SoundPlayArea::movement() {
    if (isInVolume(*MR::getPlayerPos())) {
        MR::getAreaCenterPos(&mCenter, this);
        fn_80222960(mSoundObj);

        if (!_55) {
            if (!_54) {
                fn_80222A60(mSoundObj, sSoundName, -1, -1);
                _54 = true;
            }
        } else {
            fn_80222A80(mSoundObj, sSoundName, -1, -1, -1);
        }
    } else {
        _54 = false;
    }
}
