#pragma once
#include <revolution/types.h>
class LiveActor;
class ActorCameraInfo;
class JMapInfoIter;

struct MarioFaceShipNpcConversationInfo {
    MarioFaceShipNpcConversationInfo()
        : mActor(nullptr), mTalkIndex(-1), mDemoCastId(-1), mIsIntro(false), mIsEnabled(true) {}
    LiveActor* mActor;
    ActorCameraInfo* mCameraInfo;
    const char* mActorName;
    s32 mTalkIndex;
    s32 mDemoCastId;
    bool mIsIntro;
    bool mIsEnabled;
};
namespace MR {
    void fn_804BFF30(const MarioFaceShipNpcConversationInfo*, const JMapInfoIter&);
    const char* getMarioFaceShipAttentionConversationCameraName();
}
