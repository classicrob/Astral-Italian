#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/Util/StringUtil.hpp"

SyncBckEffectInfo::SyncBckEffectInfo(const char* pGroupName, const XanimePlayer* pPlayer, s32 index, const char* pName, s32 capacity,
                                     f32 startFrame, f32 endFrame, bool continueAnimEnd)
    : mBckResources(), mCapacity(), mCount(), mStartFrame(startFrame), mEndFrame(endFrame), mContinueAnimEnd(continueAnimEnd) {
    mBckResources = new BckResourceInfo*[capacity];
    mCapacity = capacity;
    addBck(pName);
}

void SyncBckEffectInfo::addBck(const char* pName) {
    BckResourceInfo* pInfo = new BckResourceInfo(pName);
    const s32 index = mCount++;
    mBckResources[index] = pInfo;
}

bool SyncBckEffectInfo::isRegisteredBck(const char* pName) const {
    if (pName == nullptr) {
        return false;
    }

    for (BckResourceInfo** pInfo = mBckResources; pInfo != mBckResources + mCount; pInfo++) {
        if (MR::isEqualStringCase(pName, (*pInfo)->mName)) {
            return true;
        }
    }

    return false;
}

namespace MR {
    namespace Effect {
        bool isExistSyncBckDeleteFrame(const SyncBckEffectInfo* pInfo) {
            const f32 endFrame = pInfo->mEndFrame;
            return 0.0f <= endFrame;
        }
    };  // namespace Effect
};  // namespace MR

void SyncBckEffectInfo::addBck(const char* pName, const XanimePlayer* pPlayer, s32 index, const char* pGroupName) {
    addBck(pName);
}
