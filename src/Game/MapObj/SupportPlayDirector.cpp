#include "Game/MapObj/SupportPlayDirector.hpp"
#include "Game/Screen/SupportPlayGuidance.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
SupportPlayDirector::SupportPlayDirector() : NameObj("サポートプレイ指揮"), mGuidance(0), mGroup(0), mActiveCount(0), mGuidanceFrames(0) {}
void SupportPlayDirector::init(const JMapInfoIter&) {
    mGroup = new LiveActorGroup("サポートプレイアクターリスト", 64);
    mGuidance = new SupportPlayGuidance();
    mGuidance->initWithoutIter();
    MR::connectToSceneMapObjMovement(this);
}
void SupportPlayDirector::movement() {
    if (GameSequenceFunction::isPadMode2()) {
        if (mActiveCount > 0) { mActiveCount = 0; sendMsgGroupStop(); }
    } else {
        if (MR::isConnectedWPad(1)) {
            if (mActiveCount == 0) sendMsgGroupStart();
            mActiveCount = 120;
        } else if (mActiveCount > 0) {
            --mActiveCount;
            if (mActiveCount <= 0) { mActiveCount = 0; sendMsgGroupStop(); }
        }
        updateGuidance();
    }
}
void SupportPlayDirector::registerActor(LiveActor* actor) { if (!mGroup->isFull()) mGroup->registerActor(actor); }
bool SupportPlayDirector::isActive() const { return mActiveCount > 0; }
void SupportPlayDirector::sendMsgGroupStart() {
    s32 count = mGroup->mObjectCount;
    for (s32 i = 0; i < count; ++i) MR::sendMsgSupportPlayGroupStart(mGroup->getActor(i));
}
void SupportPlayDirector::sendMsgGroupStop() {
    s32 count = mGroup->mObjectCount;
    for (s32 i = 0; i < count; ++i) MR::sendMsgSupportPlayGroupStop(mGroup->getActor(i));
}
void SupportPlayDirector::updateGuidance() {
    if (mActiveCount <= 0 || !MR::isNeedExplainSupportTico()) { mGuidanceFrames = 0; return; }
    ++mGuidanceFrames;
    if (mGuidanceFrames >= 100) { MR::onGameEventFlagSupportTicoAtFirst(); mGuidance->requestAppear(); }
}
SupportPlayDirector::~SupportPlayDirector() {}
