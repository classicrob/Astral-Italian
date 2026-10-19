#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/LiveActor.hpp"

void* getActorParamF32Ptr(const ActorParam*, const char*);
void* getActorParamS32Ptr(const ActorParam*, const char*);
void* getActorParamMovePtr(const ActorParam*, const char*);
void* getActorParamSightPtr(const ActorParam*, const char*);
void* getActorParamJumpPtr(const ActorParam*, const char*);

namespace {
    const s32 sDefaultS32 = 0;
    const f32 sDefaultF32 = 0.0f;
    const f32 sDefaultMoveBlock[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const f32 sDefaultSightBlock[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const f32 sDefaultJumpBlock[2] = {0.0f, 0.0f};
}  // namespace

namespace MR {
    bool hasActorParam(const LiveActor* pActor) {
        if (pActor->mActionKeeper != nullptr) {
            if (pActor->mActionKeeper->mActorParam != nullptr) {
                return true;
            }
        }

        return false;
    }

    const f32* getActorParamF32(const LiveActor* pActor, const char* pName) {
        if (hasActorParam(pActor)) {
            return static_cast< const f32* >(getActorParamF32Ptr(pActor->mActionKeeper->mActorParam, pName));
        }

        return &sDefaultF32;
    }

    const s32* getActorParamS32(const LiveActor* pActor, const char* pName) {
        if (hasActorParam(pActor)) {
            return static_cast< const s32* >(getActorParamS32Ptr(pActor->mActionKeeper->mActorParam, pName));
        }

        return &sDefaultS32;
    }

    const f32* getActorParamMove(const LiveActor* pActor, const char* pName) {
        if (hasActorParam(pActor)) {
            return static_cast< const f32* >(getActorParamMovePtr(pActor->mActionKeeper->mActorParam, pName));
        }

        return sDefaultMoveBlock;
    }

    const f32* getActorParamSight(const LiveActor* pActor, const char* pName) {
        if (hasActorParam(pActor)) {
            return static_cast< const f32* >(getActorParamSightPtr(pActor->mActionKeeper->mActorParam, pName));
        }

        return sDefaultSightBlock;
    }

    const f32* getActorParamJump(const LiveActor* pActor, const char* pName) {
        if (hasActorParam(pActor)) {
            return static_cast< const f32* >(getActorParamJumpPtr(pActor->mActionKeeper->mActorParam, pName));
        }

        return sDefaultJumpBlock;
    }
}  // namespace MR
