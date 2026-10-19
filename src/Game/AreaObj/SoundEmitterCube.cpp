#include "Game/AreaObj/SoundEmitterCube.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <cstdio>
#include <new>

// PLACEHOLDER(objects): unnamed target symbol fn_80222A80, declared extern "C" so the reloc name matches; resolve when fn_80222A80 gets its real name in symbols.txt
extern "C" void fn_80222A80(void*, const char*, s32, s32, s32);

static const char* sSoundName = "PlaySound";

SoundEmitterCube::~SoundEmitterCube() {
}

SoundEmitterCube::SoundEmitterCube(int formType, const char* pName) : AreaObj(formType, pName), mSoundObj(), mSoundPos(0.0f, 0.0f, 0.0f) {
}

SoundEmitterCubeWorldMapSync::SoundEmitterCubeWorldMapSync(int formType, const char* pName) : SoundEmitterCube(formType, pName) {
}

void SoundEmitterCube::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    char name[0x100];
    snprintf(name, 0x100, "SoundEmitterCube%d", mObjArg0);
    mSoundObj = new ActionSoundKeeper(4, name, &mSoundPos);

    MR::connectToSceneAreaObj(this);
    MR::registerDemoSimpleCastAll(this);
}

void SoundEmitterCube::movement() {
    TVec3f pos(0.0f);
    if (mObjArg1 == 0) {
        pos.set(MR::getCameraWatchPos());
    } else {
        pos.set(AudMicWrap::getMicPos());
    }

    TDirBox3f worldBox;
    MR::calcCubeWorldBox(&worldBox, this);
    TBox3f bounds;
    bounds.i.set(worldBox._24);
    bounds.f.set(worldBox._24);
    TVec3f cornerZ;
    TVec3f cornerY;
    TVec3f cornerX;
    cornerX.scaleAdd(worldBox._30.x, worldBox._0, worldBox._24);
    cornerY.scaleAdd(worldBox._30.y, worldBox._C, worldBox._24);
    cornerZ.scaleAdd(worldBox._30.z, worldBox._18, worldBox._24);
    bounds.extend(cornerX);
    bounds.extend(cornerY);
    bounds.extend(cornerZ);

    cornerX.scaleAdd(worldBox._30.y, worldBox._C, cornerX);
    cornerY.scaleAdd(worldBox._30.z, worldBox._18, cornerY);
    cornerZ.scaleAdd(worldBox._30.x, worldBox._0, cornerZ);
    bounds.extend(cornerX);
    bounds.extend(cornerY);
    bounds.extend(cornerZ);

    cornerX.scaleAdd(worldBox._30.z, worldBox._18, cornerX);
    bounds.extend(cornerX);

    mSoundPos = pos;

    if (mSoundPos.x < bounds.i.x) {
        mSoundPos.x = bounds.i.x;
    }

    if (mSoundPos.x > bounds.f.x) {
        mSoundPos.x = bounds.f.x;
    }

    if (mSoundPos.y < bounds.i.y) {
        mSoundPos.y = bounds.i.y;
    }

    if (mSoundPos.y > bounds.f.y) {
        mSoundPos.y = bounds.f.y;
    }

    if (mSoundPos.z < bounds.i.z) {
        mSoundPos.z = bounds.i.z;
    }

    if (mSoundPos.z > bounds.f.z) {
        mSoundPos.z = bounds.f.z;
    }

    if (PSVECDistance(&mSoundPos, MR::getPlayerCenterPos()) >= 20000.0f) {
        return;
    }

    if (isValid()) {
        mSoundObj->update();
        fn_80222A80(mSoundObj, sSoundName, -1, -1, -1);
    }
}
