#include "Game/MapObj/CapsuleCage.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/PartsModel.hpp"

namespace {
    const char* cDemoCameraName = "注目カメラ";
    static const s32 sStepForStartCamera = 50;
    static const s32 sStepForEndCamera = 60;
};  // namespace

namespace NrvCapsuleCage {
    NEW_NERVE(CapsuleCageNrvWait, CapsuleCage, Wait);
    NEW_NERVE(CapsuleCageNrvStartCamera, CapsuleCage, StartCamera);
    NEW_NERVE(CapsuleCageNrvOpen, CapsuleCage, Open);
    NEW_NERVE(CapsuleCageNrvEndCamera, CapsuleCage, EndCamera);
};  // namespace NrvCapsuleCage

CapsuleCage::CapsuleCage(const char* pName) : MapObjActor(pName), mCameraInfo(), mNoKeyHole(false), mRestartBgm(false), mRockCover(false), mFrame(nullptr), mPlayRiddle(false) {
}

void CapsuleCage::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MR::getJMapInfoArg6NoInit(rIter, &mRestartBgm);
    MR::getJMapInfoArg7NoInit(rIter, &mNoKeyHole);
    s32 riddle = -1;
    MR::getJMapInfoArg4NoInit(rIter, &riddle);
    mPlayRiddle = riddle == 1;
    mRockCover = MR::isEqualString("GoroRockCoverCage", mObjectName);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    if (mNoKeyHole) info.setupModelName("CapsuleCageNoKeyHole");
    info.setupProjmapMtx(false);
    info.setupNerve(GET_NERVE(CapsuleCage, CapsuleCageNrvWait));
    info.setupSound(4, "CapsuleCage");
    initialize(rIter, info);
    if (mRockCover) {
        mFrame = new PartsModel(this, "外枠", "GoroRockCoverCageFrame", getBaseMtx(), 9, false);
        mFrame->initWithoutIter();
        MR::registerDemoSimpleCastAll(mFrame);
    }
    MR::setBodySensorType(this, 0x54 /* ATYPE_KEY_SWITCH_AVOID */);

    if (!MR::initActorCamera(this, rIter, &mCameraInfo)) {
        mCameraInfo = nullptr;
    }
    if (mNoKeyHole) {
        MR::startBtk(this, "Move");
        MR::setBtkFrameAndStop(this, 0.0f);
    }
}

void CapsuleCage::exeWait() {
}

void CapsuleCage::exeStartCamera() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraTargetSelf(this, mCameraInfo);
    }

    if (MR::isStep(this, ::sStepForStartCamera)) {
        setNerve(GET_NERVE(CapsuleCage, CapsuleCageNrvOpen));
    }
}

bool CapsuleCage::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (!mRockCover) return false;
    if (MR::isMsgRockAttack(msg)) {
        MR::startActionSound(this, "HitByRock");
        return false;
    }
    return false;
}

void CapsuleCage::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
        if (mFrame) MR::startAction(mFrame, "Move");
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::shakeCameraWeak();
        if (mRestartBgm) MR::stopStageBGM(90);
    }

    if (mNoKeyHole ? MR::isBtkStopped(this) : MR::isActionEnd(this)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::shakeCameraWeak();

        if (mCameraInfo != nullptr) {
            setNerve(GET_NERVE(CapsuleCage, CapsuleCageNrvEndCamera));
        } else {
            kill();
        }
        if (mPlayRiddle) MR::startActionSound(this, "RiddleSE");
    }
}

void CapsuleCage::exeEndCamera() {
    if (MR::isStep(this, ::sStepForEndCamera)) {
        kill();
    }
}

void CapsuleCage::kill() {
    if (mRestartBgm) MR::startStageBGM("STM_STAR_CHANCE_3", false);
    if (mCameraInfo != nullptr) {
        MR::endActorCamera(this, mCameraInfo, false);
        MR::endDemo(this, ::cDemoCameraName);
    }

    MapObjActor::kill();
}

void CapsuleCage::connectToScene(const MapObjActorInitInfo& rInfo) {
    MR::connectToSceneIndirectMapObj(this);
}

void CapsuleCage::initCaseUseSwitchB(const MapObjActorInitInfo& rInfo) {
    MR::listenStageSwitchOnB(this, MR::FunctorV0M<CapsuleCage*, void (CapsuleCage::*)()>(this, &CapsuleCage::startOpen));
}

void CapsuleCage::startOpen() {
    MR::invalidateClipping(this);

    if (mCameraInfo != nullptr) {
        MR::requestStartDemoWithoutCinemaFrame(this, ::cDemoCameraName, GET_NERVE(CapsuleCage, CapsuleCageNrvStartCamera), nullptr);
    } else {
        setNerve(GET_NERVE(CapsuleCage, CapsuleCageNrvOpen));
    }
}
