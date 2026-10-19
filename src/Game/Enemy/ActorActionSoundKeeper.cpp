#pragma auto_inline off
#include "Game/Enemy/ActorActionSoundKeeper.hpp"
#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/GameAudio/ActionSound.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Effect/SyncBckEffectChecker.hpp"
#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

JointSoundInfo::JointSoundInfo() {
    mName = nullptr;
    mJointMtx = nullptr;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mPosition.z = 0.0f;
    mSoundObject = nullptr;
}

ActorActionSoundKeeper::ActorActionSoundKeeper(LiveActor* actor, AudSoundObject* soundObject, const ActionSoundData* data)
    : mActor(actor), mData(data), _8(0), mSounds(nullptr), mChecker(nullptr), mSoundCount(0), mJointCount(0), mJoints(nullptr) {
    mSoundCount = data->mCount;
    s32 count = 0;
    for (s32 i = 0; i < mSoundCount; i++) {
        if (mData->getInfo(i)->mJointName) count++;
    }
    if (count > 0) mJoints = new JointSoundInfo[count];
    mSounds = new ActionSound*[mSoundCount];
    for (s32 i = 0; i < mSoundCount; i++) {
        const ActionSoundInfo* info = mData->getInfo(i);
        AudSoundObject* object = soundObject;
        const char* jointName = info->mJointName;
        if (jointName) {
            JointSoundInfo* joint = &mJoints[mJointCount];
            joint->mName = jointName;
            joint->mJointMtx = MR::getJointMtx(actor, jointName);
            joint->mSoundObject = new AudSoundObject(&joint->mPosition, 1, nullptr);
            object = joint->mSoundObject;
            mJointCount++;
        }
        mSounds[i] = new ActionSound(info, object);
        if (mSounds[i]->getId() == 0) {
            s32 index = 0;
            if (actor->mActionKeeper) index = reinterpret_cast<s32>(actor->mActionKeeper->mAnimKeeper); // PLACEHOLDER(core): SyncBckEffectChecker/ActionSound still declare this pointer as s32; resolve when effect/audio owners migrate their APIs.
            if (!mChecker) mChecker = new SyncBckEffectChecker(actor->mModelManager->mXanimePlayer, reinterpret_cast<ActorAnimKeeper*>(index));
            mSounds[i]->createSyncBckParam(actor->mModelManager->mXanimePlayer, index);
        }
    }
    MR::fn_800885B0(mData, false);
}

#pragma dont_inline on
void ActorActionSoundKeeper::updateSounds() {
    if (mChecker) {
        mChecker->updateBefore();
        for (s32 i = 0; i < mSoundCount; i++) updateSoundState(mSounds[i]);
        mChecker->updateAfter();
    }
    JointSoundInfo* joint;
    for (s32 i = 0; i < mJointCount; i++) {
        joint = &mJoints[i];
        if (joint->mSoundObject->hasActiveSound()) {
            joint->mPosition.set<f32>(joint->mJointMtx[0][3], joint->mJointMtx[1][3], joint->mJointMtx[2][3]);
            joint->mSoundObject->process();
        }
    }
}

#pragma dont_inline off

void ActorActionSoundKeeper::updateSoundState(ActionSound* sound) {
    SyncBckEffectInfo* info = sound->mParam;
    if (info) {
        bool create = mChecker->isCreate(info, true);
        bool createNoFrame = mChecker->isCreate(info, false);
        if (create) sound->startSoundNow();
        if (createNoFrame) sound->markStartPending();
        sound->restartSoundWithRate(mChecker->fn_8006F510());
        if (mChecker->isDelete(info)) sound->mUnk10 = false;
    }
}

void ActorActionSoundKeeper::playActionSoundKeeper(const char* name, s32 pitch, s32 velocity, s32 volume) const {
    tryPlaySound(0, -2, name, pitch, velocity, volume);
}

void ActorActionSoundKeeper::playSoundWithMode(s32 mode, const char* name, s32 pitch, s32 velocity, s32 volume) const {
    tryPlaySound(mode, -2, name, pitch, velocity, volume);
}

void ActorActionSoundKeeper::startSoundByName(const char* name) const {
    tryPlaySound(0, 1, name, -1, -1, -1);
}

bool ActorActionSoundKeeper::tryPlaySound(s32 mode, s32 id, const char* name, s32 pitch, s32 velocity, s32 volume) const {
    ActionSound* sound;
    bool played = false;
    for (s32 i = 0; i < mSoundCount; i++) {
        sound = mSounds[i];
        if ((id == -2 || id == sound->getId()) && MR::isEqualString(sound->getName(), name)) {
            sound->play(mode, pitch, velocity, volume);
            played = true;
        }
    }
    return played;
}

void ActorActionSoundKeeper::stopActionSoundKeeper(const char* name, u32 frames) {
    ActionSound* sound;
    for (s32 i = 0; i < mSoundCount; i++) {
        sound = mSounds[i];
        if (MR::isEqualString(sound->getName(), name)) sound->stopSoundFrames(frames);
    }
}

void ActorActionSoundKeeper::releaseActionSoundKeeper(const char* name) {
    ActionSound* sound;
    for (s32 i = 0; i < mSoundCount; i++) {
        sound = mSounds[i];
        if (MR::isEqualString(sound->getName(), name)) sound->releaseActionSoundHandle();
    }
}

void ActorActionSoundKeeper::resetChecker() {
    if (mChecker) mChecker->reset();
}

void ActorActionSoundKeeper::updateChecker() {
    if (mChecker) mChecker->fn_8006F1F0();
}
