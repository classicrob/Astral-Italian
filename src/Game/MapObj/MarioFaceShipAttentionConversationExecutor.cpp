#include "Game/MapObj/MarioFaceShipNpcHolder.hpp"
#include "Game/MapObj/MarioFaceShipAttentionConversationExecutor.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/Util.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/MarioFaceShipAttentionConversationDirector.hpp"
#include "Game/Util/EventUtil.hpp"

const char* sDefaultCameraName = "デフォルト注目カメラ";
const char* sTalkCameraName = "前振り会話カメラ";
const char* sTalkCameraUpName = "前振り会話カメラ[上]";
const char* sItemPlanetCameraName = "前振り会話カメラ[アイテム惑星]";

#define GET_EXE_NERVE(nerve) GET_NERVE(MarioFaceShipAttentionConversationExecutor, MarioFaceShipAttentionConversationExecutorNrv##nerve)

namespace NrvMarioFaceShipAttentionConversationExecutor {
    class MarioFaceShipAttentionConversationExecutorNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvWait sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvDemoEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvDemoEnd sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvDemoStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvDemoStart sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvTalkBegin : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvTalkBegin sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvCameraEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvCameraEnd sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvTalkEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvTalkEnd sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvDemoClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvDemoClose sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvWaitDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvWaitDemo sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvTalkHide : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvTalkHide sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvTalkShow : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvTalkShow sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvCameraClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvCameraClose sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvWaitTimeKeep : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvWaitTimeKeep sInstance;
    };

    class MarioFaceShipAttentionConversationExecutorNrvResetPose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MarioFaceShipAttentionConversationExecutorNrvResetPose sInstance;
    };
};  // namespace NrvMarioFaceShipAttentionConversationExecutor

MarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutor(const char* pName) : LiveActor(pName) {
    mMeister = nullptr;
    mTalkIndex = -1;
    mTargetActor = nullptr;
    mActorGroup = nullptr;
    mDemoIndex = 0;
    mCameraInfo = nullptr;
    mTalkCtrl = nullptr;
    mCameraInfoDefault = nullptr;
    mCameraInfoTalk = nullptr;
    mCameraInfoTalkUp = nullptr;
    mCameraInfoItemPlanet = nullptr;
    mIsDefaultCamera = false;
    mIsNpcIntro = false;
    mIsItemPlanet = false;
    mIsArg4 = false;
    mIsArg5 = false;
}

void MarioFaceShipAttentionConversationExecutor::init(const JMapInfoIter&) {
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(GET_EXE_NERVE(Wait), 0);
    MR::registerDemoSimpleCastAll(this);
    mCameraInfoTalk = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mCameraInfoTalk, sTalkCameraName);
    mCameraInfoTalkUp = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mCameraInfoTalkUp, sTalkCameraUpName);
    mCameraInfoItemPlanet = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mCameraInfoItemPlanet, sItemPlanetCameraName);
    mCameraInfoDefault = new ActorCameraInfo(0, 1);
    MR::initMultiActorCameraNoInit(this, mCameraInfoDefault, sDefaultCameraName);
    makeActorAppeared();
}

void MarioFaceShipAttentionConversationExecutor::exeDemoStart() {
    if (MR::isFirstStep(this)) {
        MR::requestMarioFaceShipNpcMovement();
        for (s32 i = 0; i < mActorGroup->getObjectCount(); i++) {
            MR::requestMovementOn(mActorGroup->getActor(i));
        }
        MR::openWipeFade(30);
        ActorCameraInfo* pInfo;
        const char* pName;
        if (mIsItemPlanet) {
            pInfo = mCameraInfoItemPlanet;
            pName = sItemPlanetCameraName;
            setPositions("マイスター位置[帰還後アイテム惑星]", "マリオ位置[帰還後アイテム惑星]");
        } else if (mIsNpcIntro) {
            pInfo = mCameraInfoTalk;
            pName = sTalkCameraName;
            setPositions("マイスター位置[NPC紹介]", "マリオ位置[NPC紹介]");
        } else {
            pInfo = mCameraInfoTalkUp;
            pName = sTalkCameraUpName;
            setPositions(nullptr, "マリオ位置[帰還後位置上]");
        }
        if (mIsArg4) {
            if (mIsDefaultCamera) {
                MR::startMultiActorCameraTargetOther(this, mCameraInfoDefault, sDefaultCameraName, CameraTargetArg(mTargetActor), -1);
            } else {
                MR::startMultiActorCameraTargetSelf(mTargetActor, mCameraInfo, MarioFaceShipAttentionConversationDirector::getKindName(), -1);
            }
        } else {
            MR::startMultiActorCameraTargetOther(this, pInfo, pName, CameraTargetArg(mMeister), -1);
        }
        MR::startBckPlayer("Watch", nullptr);
        mMeister->startWait();
    }
    if (MR::isGreaterStep(this, 0x3c)) {
        if (mIsArg4) {
            setNerve(GET_EXE_NERVE(WaitDemo));
        } else {
            setNerve(GET_EXE_NERVE(TalkBegin));
        }
    }
}

void MarioFaceShipAttentionConversationExecutor::exeTalkBegin() {
    if (MR::isFirstStep(this)) {
        MR::showBubbleBeak(mTalkCtrl);
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        MR::overlayWithPreviousScreen(10);
        setNerve(GET_EXE_NERVE(CameraEnd));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeCameraEnd() {
    if (MR::isFirstStep(this)) {
        if (mIsItemPlanet) {
            MR::endMultiActorCamera(this, mCameraInfoItemPlanet, sItemPlanetCameraName, false, -1);
        } else if (mIsNpcIntro) {
            MR::endMultiActorCamera(this, mCameraInfoTalk, sTalkCameraName, false, -1);
        } else {
            MR::endMultiActorCamera(this, mCameraInfoTalkUp, sTalkCameraUpName, false, -1);
        }
        if (mIsDefaultCamera) {
            MR::startMultiActorCameraTargetOther(this, mCameraInfoDefault, sDefaultCameraName, CameraTargetArg(mTargetActor), -1);
        } else {
            MR::startMultiActorCameraTargetSelf(mTargetActor, mCameraInfo, MarioFaceShipAttentionConversationDirector::getKindName(), -1);
        }
    }
    if (MR::isGreaterStep(this, 0x3c)) {
        setNerve(GET_EXE_NERVE(TalkEnd));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeTalkEnd() {
    if (MR::isFirstStep(this)) {
        MR::hideBubbleBeak(mTalkCtrl);
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        setNerve(GET_EXE_NERVE(DemoClose));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeDemoClose() {
    if (MR::isStep(this, 0x1e)) {
        MR::closeWipeFade(0x1e);
    }
    if (MR::isGreaterStep(this, 0x3c)) {
        if (mIsDefaultCamera) {
            MR::endMultiActorCamera(this, mCameraInfoDefault, sDefaultCameraName, false, -1);
        } else {
            MR::endMultiActorCamera(mTargetActor, mCameraInfo, MarioFaceShipAttentionConversationDirector::getKindName(), false, -1);
        }
        MR::endDemo(this, "注目会話デモ");
        setNerve(GET_EXE_NERVE(ResetPose));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeTalkHide() {
    if (MR::isFirstStep(this)) {
        MR::hideBubbleBeak(mTalkCtrl);
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        setNerve(GET_EXE_NERVE(TalkShow));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeTalkShow() {
    if (MR::isFirstStep(this)) {
        if (mIsDefaultCamera) {
            MR::endMultiActorCamera(this, mCameraInfoDefault, sDefaultCameraName, false, -1);
        } else {
            MR::endMultiActorCamera(mTargetActor, mCameraInfo, MarioFaceShipAttentionConversationDirector::getKindName(), false, -1);
        }
        if (mIsItemPlanet) {
            MR::startMultiActorCameraTargetOther(this, mCameraInfoItemPlanet, sItemPlanetCameraName, CameraTargetArg(mMeister), -1);
        } else if (mIsNpcIntro) {
            MR::startMultiActorCameraTargetOther(this, mCameraInfoTalk, sTalkCameraName, CameraTargetArg(mMeister), -1);
        } else {
            MR::startMultiActorCameraTargetOther(this, mCameraInfoTalkUp, sTalkCameraUpName, CameraTargetArg(mMeister), -1);
        }
        MR::showBubbleBeak(mTalkCtrl);
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        setNerve(GET_EXE_NERVE(CameraClose));
    }
}

void MarioFaceShipAttentionConversationExecutor::exeCameraClose() {
    if (MR::isStep(this, 0x1e)) {
        MR::closeWipeFade(0x1e);
    }
    if (MR::isGreaterStep(this, 0x3c)) {
        if (mIsItemPlanet) {
            MR::endMultiActorCamera(this, mCameraInfoItemPlanet, sItemPlanetCameraName, false, -1);
        } else if (mIsNpcIntro) {
            MR::endMultiActorCamera(this, mCameraInfoTalk, sTalkCameraName, false, -1);
        } else {
            MR::endMultiActorCamera(this, mCameraInfoTalkUp, sTalkCameraUpName, false, -1);
        }
        MR::endDemo(this, "注目会話デモ");
        setNerve(GET_EXE_NERVE(ResetPose));
    }
}

void MarioFaceShipAttentionConversationExecutor::initConversation(Param* pParam, Meister* pMeister) {
    mTalkIndex = pParam->mTalkIndex;
    mTargetActor = pParam->mTargetActor;
    mActorGroup = pParam->mActorGroup;
    mCameraInfo = pParam->mCameraInfo;
    mMeister = pMeister;
    mDemoIndex = pParam->mDemoIndex;
    mTalkCtrl = pParam->mTalkCtrl;
    mIsDefaultCamera = false;
    mIsNpcIntro = MarioFaceShipEventDataHolder::hasEventAtResultStatic();
    mIsItemPlanet = MR::hasMarioFaceShipItemEventAtResult();
    mIsArg4 = pParam->mIsArg4;
    mIsArg5 = pParam->mIsArg5;
    if (pParam->mCameraInfo == nullptr) {
        mCameraInfo = mCameraInfoDefault;
        mIsDefaultCamera = true;
    }
}

bool MarioFaceShipAttentionConversationExecutor::tryStartDemo() {
    if (!isNerve(GET_EXE_NERVE(Wait))) {
        return false;
    }
    if (mIsArg5) {
        char name[0x80];
        MarioFaceShipAttentionConversationDirector::makeEventName(name, 0x80, mTalkIndex);
        setNerve(GET_EXE_NERVE(WaitTimeKeep));
        mMeister->startWait();
        if (mIsItemPlanet) {
            setPositions("マイスター位置[帰還後アイテム惑星]", "マリオ位置[帰還後アイテム惑星]");
        } else if (mIsNpcIntro) {
            setPositions("マイスター位置[NPC紹介]", "マリオ位置[NPC紹介]");
        } else {
            setPositions(nullptr, "マリオ位置[帰還後位置上]");
        }
        return MR::tryStartTimeKeepDemoMarioPuppetable(this, name, nullptr);
    }
    return MR::requestStartDemoMarioPuppetable(this, "注目会話デモ", GET_EXE_NERVE(DemoStart), GET_EXE_NERVE(DemoEnd));
}

bool MarioFaceShipAttentionConversationExecutor::isWaitingNerve() const {
    bool ret = false;
    if (!isNerve(GET_EXE_NERVE(Wait))) {
        if (!isNerve(GET_EXE_NERVE(ResetPose))) {
            ret = true;
        }
    }
    return ret;
}

bool MarioFaceShipAttentionConversationExecutor::isNerveDemoEnd() const {
    return isNerve(GET_EXE_NERVE(ResetPose));
}

void MarioFaceShipAttentionConversationExecutor::setPositions(const char* pMeisterName, const char* pMarioName) {
    TVec3f pos(0.0f);
    TVec3f rot(0.0f);
    MR::setPlayerPos(pMarioName);
    if (pMeisterName != nullptr) {
        if (MR::tryFindNamePos(pMeisterName, &pos, &rot)) {
            MR::setPosition(mMeister, pos);
            MR::setRotation(mMeister, rot);
        }
    }
}

MarioFaceShipAttentionConversationExecutor::~MarioFaceShipAttentionConversationExecutor() {
}

namespace NrvMarioFaceShipAttentionConversationExecutor {
    MarioFaceShipAttentionConversationExecutorNrvWait MarioFaceShipAttentionConversationExecutorNrvWait::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvDemoEnd MarioFaceShipAttentionConversationExecutorNrvDemoEnd::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvDemoStart MarioFaceShipAttentionConversationExecutorNrvDemoStart::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvTalkBegin MarioFaceShipAttentionConversationExecutorNrvTalkBegin::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvCameraEnd MarioFaceShipAttentionConversationExecutorNrvCameraEnd::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvTalkEnd MarioFaceShipAttentionConversationExecutorNrvTalkEnd::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvDemoClose MarioFaceShipAttentionConversationExecutorNrvDemoClose::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvWaitDemo MarioFaceShipAttentionConversationExecutorNrvWaitDemo::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvTalkHide MarioFaceShipAttentionConversationExecutorNrvTalkHide::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvTalkShow MarioFaceShipAttentionConversationExecutorNrvTalkShow::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvCameraClose MarioFaceShipAttentionConversationExecutorNrvCameraClose::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvWaitTimeKeep MarioFaceShipAttentionConversationExecutorNrvWaitTimeKeep::sInstance ATTRIBUTE_WEAK;
    MarioFaceShipAttentionConversationExecutorNrvResetPose MarioFaceShipAttentionConversationExecutorNrvResetPose::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMarioFaceShipAttentionConversationExecutor

inline void MarioFaceShipAttentionConversationExecutor::exeResetPose() {
    if (MR::isFirstStep(this)) {
        mMeister->resetPose();
    }
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvResetPose::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeResetPose();
}

inline void MarioFaceShipAttentionConversationExecutor::exeWaitTimeKeep() {
    if (!MR::isTimeKeepDemoActive()) {
        setNerve(GET_EXE_NERVE(ResetPose));
    }
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvWaitTimeKeep::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeWaitTimeKeep();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvCameraClose::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeCameraClose();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvTalkShow::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeTalkShow();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvTalkHide::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeTalkHide();
}

inline void MarioFaceShipAttentionConversationExecutor::exeWaitDemo() {
    if (MR::isGreaterStep(this, 0x3c)) {
        setNerve(GET_EXE_NERVE(TalkHide));
    }
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvWaitDemo::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeWaitDemo();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvDemoClose::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeDemoClose();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvTalkEnd::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeTalkEnd();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvCameraEnd::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeCameraEnd();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvTalkBegin::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeTalkBegin();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvDemoStart::execute(Spine* pSpine) const {
    MarioFaceShipAttentionConversationExecutor* actor = reinterpret_cast< MarioFaceShipAttentionConversationExecutor* >(pSpine->mExecutor);
    actor->exeDemoStart();
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvDemoEnd::execute(Spine* pSpine) const {
}

inline void NrvMarioFaceShipAttentionConversationExecutor::MarioFaceShipAttentionConversationExecutorNrvWait::execute(Spine* pSpine) const {
}
