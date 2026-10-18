#pragma auto_inline off
#include "Game/MapObj/MarioFaceShipNpcHolder.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/NPCUtil.hpp"
MarioFaceShipNpcHolder::MarioFaceShipNpcHolder(const char* name) : NameObj(name), mNpcs(nullptr), mCount(0) {}
MarioFaceShipNpcHolder::~MarioFaceShipNpcHolder() {}
void MarioFaceShipNpcHolder::init(const JMapInfoIter&) {
    MR::connectToSceneMapObjDecorationMovement(this);
    MR::registerDemoSimpleCastAll(this);
    mNpcs = new MarioFaceShipNpcInfo[40];
}
MarioFaceShipNpcInfo::MarioFaceShipNpcInfo() : mIndex(-1), mDemoCastId(-1), mIsEnabled(true), mActor(nullptr) {}
void MarioFaceShipNpcHolder::movement() {
    NameObj::movement();
    if (MR::isNormalTalking()) requestMovement();
}
void MarioFaceShipNpcHolder::registerActor(LiveActor* actor, s32 index, const JMapInfoIter& iter) {
    bool disabled = false;
    MR::getJMapInfoArg2NoInit(iter, &disabled);
    mNpcs[mCount].mActor = actor;
    mNpcs[mCount].mIndex = index;
    mNpcs[mCount].mDemoCastId = MR::getJMapInfoDemoCastID(iter);
    mNpcs[mCount].mIsEnabled = !disabled;
    ++mCount;
}
bool MarioFaceShipNpcHolder::isRegistered(const LiveActor* actor, s32 index) const {
    for (s32 i = 0; i < mCount; ++i) {
        if (actor == mNpcs[i].mActor && (index == -1 || index == mNpcs[i].mIndex)) return true;
    }
    return false;
}
void MarioFaceShipNpcHolder::requestMovement() {
    for (s32 i = 0; i < mCount; ++i) MR::requestMovementOn(mNpcs[i].mActor);
}
void MarioFaceShipNpcHolder::registerGroup(LiveActorGroup* group, const MarioFaceShipNpcGroupInfo* info) const {
    if (info) {
        for (s32 i = 0; i < info->mCount; ++i) {
            const char* name = info->mNames[i];
            s32 cast = info->mDemoCastIds[i];
            for (s32 j = 0; j < mCount; ++j) {
                if (MR::isEqualString(mNpcs[j].mActor->mName, name) && (cast == -1 || cast == mNpcs[j].mDemoCastId)) group->registerActor(mNpcs[j].mActor);
            }
        }
    }
}
void MR::createMarioFaceShipNpcHolder() { MR::createSceneObj(SceneObj_MarioFaceShipNpcHolder); }
void MR::registerMarioFaceShipNpcActor(LiveActor* actor, s32 index, const JMapInfoIter& iter) {
    MR::getSceneObj<MarioFaceShipNpcHolder*>(SceneObj_MarioFaceShipNpcHolder)->registerActor(actor, index, iter);
}
void MR::registerMarioFaceShipNpcGroup(LiveActorGroup* group, const MarioFaceShipNpcGroupInfo* info) {
    MR::getSceneObj<MarioFaceShipNpcHolder*>(SceneObj_MarioFaceShipNpcHolder)->registerGroup(group, info);
}
bool MR::isMarioFaceShipNpc(const LiveActor* actor, s32 index) {
    return MR::getSceneObj<MarioFaceShipNpcHolder*>(SceneObj_MarioFaceShipNpcHolder)->isRegistered(actor, index);
}
void MR::requestMarioFaceShipNpcMovement() { MR::getSceneObj<MarioFaceShipNpcHolder*>(SceneObj_MarioFaceShipNpcHolder)->requestMovement(); }
