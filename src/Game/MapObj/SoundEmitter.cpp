#include "Game/MapObj/SoundEmitter.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/RailRider.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include <cstdio>
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

namespace NrvSoundEmitter {
    NEW_NERVE(SoundEmitterNrvStandBy, SoundEmitter, StandBy);
    NEW_NERVE(SoundEmitterNrvPlaySound, SoundEmitter, PlaySound);
};  // namespace NrvSoundEmitter

SoundEmitter::SoundEmitter(const char* pName) : LiveActor(pName) {
    _8C = -1;
    _94 = -1;
    _98 = false;
    _99 = false;
    _9A = false;
}

void SoundEmitter::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &_8C);
    MR::getJMapInfoArg1NoInit(rIter, &_94);
    s32 demoType = -1;
    MR::getJMapInfoArg2NoInit(rIter, &demoType);
    if (demoType >= 0) _98 = true; else _98 = false;
    if (demoType == 1) _99 = true; else _99 = false;
    s32 repeat = -1;
    MR::getJMapInfoArg3NoInit(rIter, &repeat);
    if (repeat >= 0) _9A = true; else _9A = false;
    if (!_98) MR::registerDemoSimpleCastAll(this);
    if (_99) MR::registerStoryBookDemoCast(this);

    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
    }

    char soundName[256];
    snprintf(soundName, sizeof(soundName), "SoundEmitterPoint%d", _8C);
    initSound(2, soundName, nullptr, TVec3f(0.0f));
    MR::setClippingTypeSphere(this, 20000.0f);
    MR::initUseStageSwitchReadAppear(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);

    if (MR::isValidSwitchAppear(this)) {
        initNerve(GET_NERVE(SoundEmitter, SoundEmitterNrvStandBy));
    } else {
        initNerve(GET_NERVE(SoundEmitter, SoundEmitterNrvPlaySound));
    }

    makeActorAppeared();
}

void SoundEmitter::control() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            if (_9A) setNerve(GET_NERVE(SoundEmitter, SoundEmitterNrvPlaySound));
            else kill();
        }
    }
}

void SoundEmitter::exePlaySound() {
    if (MR::isFirstStep(this)) {
        MR::startActionSoundOneShot(this, "PlaySound");
    }
    MR::startActionSoundLevel(this, "PlaySound");
    RailRider* rider = mRailRider;
    if (rider != nullptr) {
        if (_94 == 0) rider->moveToNearestPos(MR::getCameraWatchPos());
        else rider->moveToNearestPos(MR::getCamPos());
        mPosition.x = mRailRider->mCurPos.x;
        mPosition.y = mRailRider->mCurPos.y;
        mPosition.z = mRailRider->mCurPos.z;
    }
}

inline void SoundEmitter::exeStandBy() {
    if (MR::isValidSwitchAppear(this)) {
        if (MR::isOnSwitchAppear(this)) {
            setNerve(GET_NERVE(SoundEmitter, SoundEmitterNrvPlaySound));
        }
    }
}

SoundEmitter::~SoundEmitter() {
}
