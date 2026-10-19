#include "Game/AreaObj/SoundEmitterSphere.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <cstdio>
#include <new>


SoundEmitterSphere::~SoundEmitterSphere() {
}

SoundEmitterSphere::SoundEmitterSphere(int formType, const char* pName) : AreaObj(formType, pName), mSoundObj(), mSoundPos(0.0f, 0.0f, 0.0f) {
}

void SoundEmitterSphere::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    char name[0x100];
    snprintf(name, 0x100, "SoundEmitterSphere%d", mObjArg0);
    mSoundObj = new ActionSoundKeeper(4, name, &mSoundPos);

    MR::connectToSceneAreaObj(this);
    MR::registerDemoSimpleCastAll(this);
}

void SoundEmitterSphere::movement() {
    TVec3f pos(0.0f);
    if (mObjArg1 == 0) {
        pos.set(MR::getCameraWatchPos());
    } else {
        pos.set(AudMicWrap::getMicPos());
    }

    if (!isInVolume(pos)) {
        TVec3f center;
        MR::calcSpherePos(&center, this);
        f32 radius = MR::getSphereRadius(this);
        f32 rate = radius / (mSoundPos - center).length();
        mSoundPos.x = center.x + rate * (mSoundPos.x - center.x);
        mSoundPos.y = center.y + rate * (mSoundPos.y - center.y);
        mSoundPos.z = center.z + rate * (mSoundPos.z - center.z);
    } else {
        mSoundPos = pos;
    }

    if (PSVECDistance(&mSoundPos, MR::getPlayerCenterPos()) >= 20000.0f) {
        return;
    }

    mSoundObj->update();
    mSoundObj->startLevelSound("PlaySound", -1, -1, -1);
}
