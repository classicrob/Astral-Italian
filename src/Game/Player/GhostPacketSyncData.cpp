#include "Game/Player/GhostPacketSyncData.hpp"
#include "Game/Player/GhostPlayer.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

GhostPacketSyncData::GhostPacketSyncData() : mPosition(0.0f), mPackedPosition(0, 0, 0), mRotation(0), mScale(0), mVelocity(0) {
    mActionName = nullptr;
    mActionHash = 0;
    mUsesActionHash = false;
    mPositionOverflow = false;
    mBckFrame = 0;
    mBckRate = 0;
    for (s32 index = 0; index < 4; ++index) mTrackWeights[index] = 0;
}

void GhostPacketSyncData::syncWithPlayer() { syncWithActor(MarioAccess::getPlayerActor(), true); }

// NONMATCHING: track-weight loop address registers.
void GhostPacketSyncData::syncWithActor(const LiveActor* actor, bool syncAction) {
    XanimePlayer* player = actor->mModelManager->mXanimePlayer;
    MR::extractMtxTrans(actor->getBaseMtx(), &mPosition);
    if (MR::fn_8037FA30(&mPackedPosition, actor->getBaseMtx())) mPositionOverflow = true;
    else mPositionOverflow = false;
    MR::fn_8037FAF0(&mRotation, actor->getBaseMtx());
    MR::fn_8037FC30(&mScale, actor->mScale);
    MR::fn_8037FC40(&mVelocity, actor->mVelocity);
    if (syncAction) {
        if (player->isAnimationRunSimple()) {
            mUsesActionHash = false;
            mActionName = player->getCurrentBckName();
            mActionHash = 0;
        } else {
            mUsesActionHash = true;
            mActionName = player->getCurrentAnimationName();
            mActionHash = MR::getHashCode(mActionName);
        }
    }
    f32 frame;
    if (player->isTerminate()) frame = player->mFrameCtrl->mEnd;
    else frame = player->mFrameCtrl->mFrame;
    MR::fn_8037FC50(&mBckFrame, frame);
    MR::fn_8037FC70(&mBckRate, player->mFrameCtrl->mRate);
    for (u32 index = 0; index < 4; ++index) MR::fn_8037FC60(&mTrackWeights[index], player->mWeights[index]);
}
