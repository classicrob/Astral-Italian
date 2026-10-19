#pragma auto_inline off
#include "Game/MapObj/MarioFaceShipNormalConversationExecutor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/Util.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/Util/EventUtil.hpp"

namespace {
    const char* sDefaultCameraName = "デフォルト会話カメラ";
    const char* sUpperCameraName = "デフォルト会話カメラ[上]";
    const char* sItemPlanetCameraName = "デフォルト会話カメラ[アイテム惑星]";
    s32 sFadeTime = 30;
    s32 sCameraTime = 60;
}

namespace NrvMarioFaceShipNormalConversationExecutor {
    class Wait : public Nerve {
    public:
        void execute(Spine*) const {}
        static Wait sInstance;
    };
    Wait Wait::sInstance ATTRIBUTE_WEAK;
    class DemoStart : public Nerve {
    public:
        void execute(Spine*) const {}
        static DemoStart sInstance;
    };
    DemoStart DemoStart::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(CameraOpen, MarioFaceShipNormalConversationExecutor, CameraOpen);
    NEW_NERVE(Talk, MarioFaceShipNormalConversationExecutor, Talk);
    NEW_NERVE(CameraClose, MarioFaceShipNormalConversationExecutor, CameraClose);
    class DemoEnd : public Nerve {
    public:
        void execute(Spine*) const {}
        static DemoEnd sInstance;
    };
    DemoEnd DemoEnd::sInstance ATTRIBUTE_WEAK;
}
#define CONV_NERVE(n) (&NrvMarioFaceShipNormalConversationExecutor::n::sInstance)

MarioFaceShipNormalConversationExecutor::MarioFaceShipNormalConversationExecutor(const char* name)
    : LiveActor(name), mMeister(nullptr), mTalkIndex(-1), mActorGroup(nullptr), _9C(nullptr),
      mTalkCtrl(nullptr), mDefaultCamera(nullptr), mUpperCamera(nullptr), mItemPlanetCamera(nullptr),
      mIsNpcIntro(false), mIsItemPlanet(false) {}

void MarioFaceShipNormalConversationExecutor::init(const JMapInfoIter& iter) {
    const char* name = nullptr;
    MR::getObjectName(&name, iter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(CONV_NERVE(Wait));
    MR::registerDemoSimpleCastAll(this);
    mDefaultCamera = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mDefaultCamera, sDefaultCameraName);
    mUpperCamera = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mUpperCamera, sUpperCameraName);
    mItemPlanetCamera = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mItemPlanetCamera, sItemPlanetCameraName);
    makeActorAppeared();
}
void MarioFaceShipNormalConversationExecutor::exeCameraOpen() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mActorGroup->getObjNum(); ++i)
            MR::requestMovementOn(mActorGroup->getActor(i));
        MR::openWipeFade(sFadeTime);
        startCamera();
        MR::startBckPlayer("Watch", nullptr);
        mMeister->startWait();
    }
    if (MR::isGreaterStep(this, sCameraTime)) setNerve(CONV_NERVE(Talk));
}
void MarioFaceShipNormalConversationExecutor::exeCameraClose() {
    if (MR::isStep(this, sCameraTime - sFadeTime)) MR::closeWipeFade(sFadeTime);
    if (MR::isGreaterStep(this, sCameraTime)) {
        if (mIsItemPlanet)
            MR::endMultiActorCamera(this, mItemPlanetCamera, sItemPlanetCameraName, false, -1);
        else if (mIsNpcIntro)
            MR::endMultiActorCamera(this, mDefaultCamera, sDefaultCameraName, false, -1);
        else
            MR::endMultiActorCamera(this, mUpperCamera, sUpperCameraName, false, -1);
        MR::endDemo(this, "会話デモ");
        mMeister->resetPose();
        setNerve(CONV_NERVE(DemoEnd));
    }
}
void MarioFaceShipNormalConversationExecutor::initConversation(MarioFaceShipNormalConversationParam* param, Meister* meister) {
    mTalkIndex = param->mTalkIndex;
    mActorGroup = param->mActorGroup;
    mMeister = meister;
    mTalkCtrl = param->mTalkCtrl;
    mIsNpcIntro = MarioFaceShipEventDataHolder::hasEventAtResultStatic();
    mIsItemPlanet = MR::hasMarioFaceShipItemEventAtResult();
}
bool MarioFaceShipNormalConversationExecutor::tryStartDemo() {
    if (isNerve(CONV_NERVE(Wait)))
        return MR::requestStartDemoMarioPuppetable(this, "会話デモ", CONV_NERVE(CameraOpen), CONV_NERVE(DemoStart));
    return false;
}
bool MarioFaceShipNormalConversationExecutor::isDemoActive() const {
    return !isNerve(CONV_NERVE(Wait)) && !isNerve(CONV_NERVE(DemoEnd));
}
bool MarioFaceShipNormalConversationExecutor::isDemoEnd() const {
    return isNerve(CONV_NERVE(DemoEnd));
}
void MarioFaceShipNormalConversationExecutor::startCamera() {
    TVec3f position(0.0f);
    TVec3f rotation(0.0f);
    if (mIsItemPlanet) {
        MR::startMultiActorCameraTargetOther(this, mItemPlanetCamera, sItemPlanetCameraName, CameraTargetArg(mMeister), -1);
        if (MR::tryFindNamePos("マイスター位置[帰還後アイテム惑星]", &position, &rotation)) {
            MR::setPosition(mMeister, position);
            MR::setRotation(mMeister, rotation);
        }
        MR::setPlayerPos("マリオ位置[帰還後アイテム惑星]");
    }
    else if (mIsNpcIntro) {
        MR::startMultiActorCameraTargetOther(this, mDefaultCamera, sDefaultCameraName, CameraTargetArg(mMeister), -1);
        if (MR::tryFindNamePos("マイスター位置[NPC紹介]", &position, &rotation)) {
            MR::setPosition(mMeister, position);
            MR::setRotation(mMeister, rotation);
        }
        MR::setPlayerPos("マリオ位置[NPC紹介]");
    }
    else {
        MR::startMultiActorCameraTargetOther(this, mUpperCamera, sUpperCameraName, CameraTargetArg(mMeister), -1);
        MR::setPlayerPos("マリオ位置[帰還後位置上]");
    }
}
MarioFaceShipNormalConversationExecutor::~MarioFaceShipNormalConversationExecutor() {}

void MarioFaceShipNormalConversationExecutor::exeTalk() {
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) setNerve(CONV_NERVE(CameraClose));
}
