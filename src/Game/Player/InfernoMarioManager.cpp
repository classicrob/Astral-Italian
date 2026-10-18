#include "Game/Player/InfernoMarioManager.hpp"
#include "Game/Player/InfernoMario.hpp"
#include "Game/Player/GhostPacketHistory.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
namespace NrvInfernoMarioManager {
    NEW_NERVE(InfernoMarioManagerNrvManage, InfernoMarioManager, Manage);
}
InfernoMarioManager::InfernoMarioManager(const char* name) : LiveActor(name), mHistory(nullptr), mActors(nullptr), mCount(0), mCapacity(0) {}
InfernoMarioManager::~InfernoMarioManager() {}
void InfernoMarioManager::init(const JMapInfoIter&) {
    MR::connectToSceneEnemyMovement(this);
    mHistory = new GhostPacketHistory(0xE11, 0x63FE);
    mActors = new InfernoMario*[0x40];
    mCapacity = 0x40;
    initNerve(&NrvInfernoMarioManager::InfernoMarioManagerNrvManage::sInstance);
    MR::invalidateClipping(this);
    makeActorAppeared();
}
void InfernoMarioManager::exeManage() {
    mHistory->update();
    s32 count = 0;
    for (s32 i = 0; i < mCount; i++) {
        if (!MR::isDead(mActors[i]))
            mActors[count++] = mActors[i];
    }
    mCount = count;
    s32 delay = 0;
    for (s32 i = 0; i < mCount; i++) {
        delay += mActors[i]->mFrameSpacing;
        mActors[i]->setFrameDelay(delay);
    }
}
bool InfernoMarioManager::add(InfernoMario* actor) {
    if (mCount >= mCapacity)
        return false;
    actor->setHistory(mHistory);
    mActors[mCount] = actor;
    mCount++;
    return true;
}
bool InfernoMarioManager::addFirst(InfernoMario* actor) {
    if (mCount >= mCapacity)
        return false;
    for (s32 i = mCount; i > 0; i--)
        mActors[i] = mActors[i - 1];
    actor->setHistory(mHistory);
    mActors[0] = actor;
    mCount++;
    return true;
}
void InfernoMarioManager::remove(InfernoMario* actor) {
    if (actor == nullptr)
        return;
    s32 count = 0;
    for (s32 i = 0; i < mCount; i++) {
        if (mActors[i] != actor)
            mActors[count++] = mActors[i];
    }
    mCount = count;
}
namespace MR {
    void createInfernoMarioManager() { createSceneObj(0x7D); }
    bool addInfernoMario(InfernoMario* actor) { return getSceneObj<InfernoMarioManager>(0x7D)->add(actor); }
    bool addInfernoMarioFirst(InfernoMario* actor) { return getSceneObj<InfernoMarioManager>(0x7D)->addFirst(actor); }
    void removeInfernoMario(InfernoMario* actor) { getSceneObj<InfernoMarioManager>(0x7D)->remove(actor); }
}
