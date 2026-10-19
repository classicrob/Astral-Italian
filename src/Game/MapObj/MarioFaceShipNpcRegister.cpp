#include "Game/MapObj/MarioFaceShipNpcRegister.hpp"
#include "Game/MapObj/MarioFaceShipNpcHolder.hpp"
#include "Game/System/MarioFaceShipAttentionConversationDirector.hpp"
#include "Game/Util.hpp"

MarioFaceShipNpcRegister::MarioFaceShipNpcRegister(LiveActor* actor, const char* actorName, const char* name)
    : NameObj(name), mActor(actor), mActorName(actorName), mTalkIndex(-1) {}
void MarioFaceShipNpcRegister::init(const JMapInfoIter& iter) {
    s32 talkIndex = -1;
    MR::getJMapInfoArg0NoInit(iter, &talkIndex);
    mTalkIndex = talkIndex;
    bool intro = false;
    MR::getJMapInfoArg1NoInit(iter, &intro);
    bool disabled = false;
    MR::getJMapInfoArg2NoInit(iter, &disabled);
    ActorCameraInfo* camera = nullptr;
    if (!MR::initMultiActorCamera(mActor, iter, &camera, MarioFaceShipAttentionConversationDirector::getKindName()))
        camera = nullptr;
    MarioFaceShipConversationNpcInfo info;
    info.mActor = mActor;
    info.mDemoName = mActorName;
    info.mEventNo = mTalkIndex;
    info.mCameraInfo = camera;
    info.mCastId = MR::getJMapInfoDemoCastID(iter);
    info.mFlag14 = intro;
    info.mFlag15 = !disabled;
    MR::createMarioFaceShipNpcHolder();
    MR::registerMarioFaceShipNpcActor(mActor, mTalkIndex, iter);
    MarioFaceShipAttentionConversationDirector::registerNpcToConversation(&info, iter);
}
MarioFaceShipNpcRegister::~MarioFaceShipNpcRegister() {}
namespace MR {
void registerMarioFaceShipNpc(LiveActor* actor, const JMapInfoIter& iter) {
    s32 count = getChildObjNum(iter);
    const char* actorName = nullptr;
    getObjectName(&actorName, iter);
    for (s32 i = 0; i < count; ++i) {
        const char* childName = nullptr;
        getChildObjName(&childName, iter, i);
        if (isEqualString(childName, "MarioFaceShipNpcRegister")) {
            MarioFaceShipNpcRegister* obj = new MarioFaceShipNpcRegister(actor, actorName, "マリオ顔船Npc紹介デモ登録者");
            initChildObj(obj, iter, i);
        }
    }
}
}
