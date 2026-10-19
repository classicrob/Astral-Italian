#include "Game/LiveActor/ActorSoundHolder.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/VectorUtil.hpp"

ActorSoundHolder::ActorSoundHolder(LiveActor* pActor, s32 count, const char* pName,
                                 const TVec3f* pPosition, const TVec3f& rLocalPosition)
    : mSoundObject(nullptr), mActionSoundKeeper(nullptr), mPosition(nullptr), mPositionMtx(nullptr),
      mLocalPosition(nullptr) {
    if (pPosition != nullptr) {
        mSoundObject = new AudAnmSoundObject(const_cast<TVec3f*>(pPosition), count, MR::getCurrentHeap());
    } else if (!MR::isNearZero(rLocalPosition, 0.001f)) {
        mLocalPosition = new TVec3f(rLocalPosition);
        mPosition = new TVec3f(0.0f);
        mPositionMtx = pActor->getBaseMtx();
        PSMTXMultVec(mPositionMtx, mLocalPosition, mPosition);
        mSoundObject = new AudAnmSoundObject(mPosition, count, MR::getCurrentHeap());
    } else {
        mSoundObject = new AudAnmSoundObject(&pActor->mPosition, count, MR::getCurrentHeap());
    }

    const ActionSoundData* data = MR::getActionSoundData(pActor, pName);
    if (data != nullptr) {
        mActionSoundKeeper = new ActorActionSoundKeeper(pActor, mSoundObject, data);
    }

    ResourceHolder* holder = MR::getResourceHolder(pActor);
    if (holder != nullptr && holder->mBasResTable != nullptr && holder->mBasResTable->mCount != 0) {
        ResTable* table = holder->mBasResTable;
        AudInstListMgr* instList = AudWrap::getInstListMgr();
        for (u32 i = 0; i < table->mCount; i++) {
            const JAUSoundAnimation* animation = static_cast<const JAUSoundAnimation*>(table->getRes(i));
            for (s32 j = 0; j < animation->mNumSounds; j++) {
                instList->requestSoundWaves((&animation->mSounds)[j].mSoundID, false);
            }
        }
    }
}
