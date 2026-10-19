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
