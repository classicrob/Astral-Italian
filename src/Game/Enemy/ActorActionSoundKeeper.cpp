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
            if (actor->mActionKeeper) index = actor->mActionKeeper->mActionIndex;
            if (!mChecker) mChecker = new SyncBckEffectChecker(actor->mModelManager->mXanimePlayer, index);
            mSounds[i]->fn_8021F970(actor->mModelManager->mXanimePlayer, index);
        }
    }
    MR::fn_800885B0(mData, false);
}

#pragma dont_inline on
void ActorActionSoundKeeper::fn_8021F2D0() {
    if (mChecker) {
        mChecker->updateBefore();
        for (s32 i = 0; i < mSoundCount; i++) fn_8021F3B0(mSounds[i]);
        mChecker->updateAfter();
    }
    JointSoundInfo* joint;
    for (s32 i = 0; i < mJointCount; i++) {
        joint = &mJoints[i];
        if (joint->mSoundObject->fn_80596A60()) {
            joint->mPosition.set<f32>(joint->mJointMtx[0][3], joint->mJointMtx[1][3], joint->mJointMtx[2][3]);
            joint->mSoundObject->process();
        }
    }
}

#pragma dont_inline off

void ActorActionSoundKeeper::fn_8021F3B0(ActionSound* sound) {
    SyncBckEffectInfo* info = sound->mParam;
    if (info) {
        bool create = mChecker->isCreate(info, true);
        bool createNoFrame = mChecker->isCreate(info, false);
        if (create) sound->fn_8021FA80();
        if (createNoFrame) sound->fn_8021FA40();
        sound->fn_8021FC10(mChecker->fn_8006F510());
        if (mChecker->isDelete(info)) sound->mUnk10 = false;
    }
}

void ActorActionSoundKeeper::fn_8021F470(const char* name, s32 pitch, s32 velocity, s32 volume) const {
    fn_8021F4D0(0, -2, name, pitch, velocity, volume);
}

void ActorActionSoundKeeper::fn_8021F490(s32 mode, const char* name, s32 pitch, s32 velocity, s32 volume) const {
    fn_8021F4D0(mode, -2, name, pitch, velocity, volume);
}

void ActorActionSoundKeeper::fn_8021F4B0(const char* name) const {
    fn_8021F4D0(0, 1, name, -1, -1, -1);
}

bool ActorActionSoundKeeper::fn_8021F4D0(s32 mode, s32 id, const char* name, s32 pitch, s32 velocity, s32 volume) const {
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

void ActorActionSoundKeeper::fn_8021F5A0(const char* name, u32 frames) {
    ActionSound* sound;
    for (s32 i = 0; i < mSoundCount; i++) {
        sound = mSounds[i];
        if (MR::isEqualString(sound->getName(), name)) sound->fn_8021FAE0(frames);
    }
}

void ActorActionSoundKeeper::fn_8021F630(const char* name) {
    ActionSound* sound;
    for (s32 i = 0; i < mSoundCount; i++) {
        sound = mSounds[i];
        if (MR::isEqualString(sound->getName(), name)) sound->fn_8021FB80();
    }
}

void ActorActionSoundKeeper::fn_8021F6B0() {
    if (mChecker) mChecker->reset();
}

void ActorActionSoundKeeper::fn_8021F6D0() {
    if (mChecker) mChecker->fn_8006F1F0();
}
