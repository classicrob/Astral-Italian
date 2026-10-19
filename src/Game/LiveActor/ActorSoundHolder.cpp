#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/LiveActor/ActorSoundHolder.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"

AudAnmSoundObject* ActorSoundHolder::getSoundObj() {
    return mSoundObject;
}

void ActorSoundHolder::resetSyncBck() {
    if (mActionSoundKeeper != nullptr) {
        mActionSoundKeeper->resetChecker();
    }
}

void ActorSoundHolder::resetSyncBckFrame() {
    if (mActionSoundKeeper != nullptr) {
        mActionSoundKeeper->updateChecker();
    }
}

bool ActorSoundHolder::setSoundPositionPtr(TVec3f* pPosition) {
    if (mLocalPosition != nullptr) {
        return false;
    }
    mSoundObject->setTrans(pPosition);
    return true;
}

void ActorSoundHolder::update() {
    if (mLocalPosition != nullptr) {
        PSMTXMultVec(mPositionMtx, mLocalPosition, mPosition);
    }
    if (mActionSoundKeeper != nullptr) {
        mActionSoundKeeper->updateSounds();
    }
}

void ActorSoundHolder::startAction(const char* name) {
    if (mActionSoundKeeper)
        mActionSoundKeeper->startSoundByName(name);
}
namespace MR {
    AudAnmSoundObject* getActorSoundObject(const LiveActor* actor) {
        return actor->mSoundHolder == nullptr ? nullptr : actor->mSoundHolder->getSoundObj();
    }
    s32 getActorSoundObjectCount(const LiveActor* actor) {
        if (!actor->mSoundHolder || !actor->mSoundHolder->mActionSoundKeeper)
            return 0;
        return actor->mSoundHolder->mActionSoundKeeper->mJointCount;
    }
    AudSoundObject* getActorSoundObjectAt(const LiveActor* actor, s32 index) {
        return actor->mSoundHolder->mActionSoundKeeper->mJoints[index].mSoundObject;
    }
}  // namespace MR
