#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/GameAudio/AudStageBgmTable.hpp"
#include "Game/MapObj/PlayerFollowPass.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Demo/ScenarioStarter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/SpinDriverCamera.hpp"
#include "Game/MapObj/SpinDriverShootPath.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Screen/ScenarioTitle.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"


namespace {
    class ScenarioStarterFollowValidater : public BaseMatrixFollowValidater {
    public:
        typedef bool (ScenarioStarter::*Func)() const;
        ScenarioStarterFollowValidater(ScenarioStarter* host, Func func) : mHost(host), mFunc(func) {}
        virtual bool isValid(s32) const { return (mHost->*mFunc)(); }
        ScenarioStarter* mHost;
        Func mFunc;
    };
}

namespace NrvScenarioStarter {
    NEW_NERVE(ScenarioStarterWaitScenarioCameraEnd, ScenarioStarter, WaitScenarioCameraEnd);
    NEW_NERVE(ScenarioStarterWaitToStart, ScenarioStarter, WaitToStart);
    NEW_NERVE(ScenarioStarterCinemaFrameBlank, ScenarioStarter, CinemaFrameBlank);
    NEW_NERVE(ScenarioStarterRailMove, ScenarioStarter, RailMove);
    NEW_NERVE(ScenarioStarterRailMoveCanceled, ScenarioStarter, RailMoveCanceled);
    NEW_NERVE(ScenarioStarterShowWelcomeLayout, ScenarioStarter, ShowWelcomeLayout);
};  // namespace NrvScenarioStarter

ScenarioStarter::ScenarioStarter(const char* pName)
    : LiveActor(pName), _8C(), _90(0.0f, 1.0f, 0.0f), _9C(0.0f, 0.0f, 0.0f), _B8(), mSpinDriverCamera(), _C0(0.0f, 1.0f, 0.0f), _CC(0.0f, 0.0f, 1.0f),
      _D8(1.0f, 0.0f, 0.0f), _E4(), _E8(), _EC(), mFlightTime(300), mFlyMotionStartStep(), _F8(), mFallMotionStartStep(), mInitalPlayerRotation(),
      mWelcomeLayout(), mTitle(), _10C(0.0f, 0.0f, 0.0f), _118(0.0f, 0.0f, 0.0f), _124() {
    _A8.set(0.0f, 0.0f, 0.0f, 1.0f);
    mFollowMtx.identity();
}

void ScenarioStarter::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg0NoInit(rIter, &mFlightTime);
    initShootPath(rIter);

    mSpinDriverCamera = new SpinDriverCamera();
    mSpinDriverCamera->init(rIter, this);

    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x67, 8, 200.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::invalidateClipping(this);
    mWelcomeLayout = MR::createSimpleLayout("Welcome表示", "Welcome", 1);
    mWelcomeLayout->kill();
    MR::registerDemoSimpleCastAll(mWelcomeLayout);
    mTitle = new ScenarioTitle();
    mTitle->initWithoutIter();
    mTitle->kill();
    _10C.set(*MR::getPlayerPos());
    _118.set(*MR::getPlayerRotate());
    MR::setPosition(this, *MR::getPlayerPos());
    initNerve(GET_NERVE(ScenarioStarter, ScenarioStarterWaitScenarioCameraEnd));
    MR::makeMtxTR(mFollowMtx, this);
    ScenarioStarterFollowValidater::Func valid = &ScenarioStarter::isFollowValid;
    BaseMatrixFollowValidater* validater = new ScenarioStarterFollowValidater(this, valid);
    MR::addBaseMatrixFollowTarget(this, rIter, &mFollowMtx, validater);
    if (MR::isBeginScenarioStarter()) {
        appear();
    } else {
        kill();
    }
    mFirstIslandFleetVisit = MR::isIslandFleetGalaxy1FirstTime();
}

void ScenarioStarter::exeWaitToStart() {
    if (MR::isFirstStep(this)) {
        _124 = false;
        MR::forceToBlankCinemaFrame();
    }
    MR::setPosition(this, *MR::getPlayerPos());
}

void ScenarioStarter::exeCinemaFrameBlank() {
    if (MR::isFirstStep(this)) {
        if (!AudStageBgmTable::isStartOnWelcome(MR::getCurrentStageName(), MR::getCurrentScenarioNo())) {
            isStartBgmOnWelcome();
        } else if (mFirstIslandFleetVisit && AudStageBgmTable::isFirstVisitStart(MR::getCurrentStageName(), MR::getCurrentScenarioNo())) {
            s32 scenario = MR::getCurrentScenarioNo();
            const char* stage = MR::getCurrentStageName();
            // The target passes the controller pointer in the unused scene-name slot.
            MR::startStageBGMFromStageName(reinterpret_cast< const char* >(MR::getCurrentGameSystemSceneController()), stage, scenario, false, true);
        }
        MR::deactivateDefaultGameLayout();
        MR::killDestroyCounter();
    }
    MR::setNerveAtStep(this, GET_NERVE(ScenarioStarter, ScenarioStarterRailMove), 0);
}

// NONMATCHING: equivalent integer clamp lowering
void ScenarioStarter::exeRailMove() {
    bool forceFrame = true;
    if (!mFirstIslandFleetVisit && !MR::isStageNoPauseMenuStars()) {
        forceFrame = false;
    }
    if (MR::isFirstStep(this)) {
        calcShootMotionTime();
        if (mFirstIslandFleetVisit) {
            MR::startAction(_8C, "CannonFlyStart");
        } else {
            MR::startAction(_8C, "SpaceFlyStart");
        }
        _90 = _C0;
        turnBindHead(_90, 1.0f);
        MR::setPosition(this, MR::getRailPointPosStart(this));
        MR::endStartPosCamera();
        mSpinDriverCamera->start(_C0, _CC, mPosition);
        TPos3f rotMtx;
        _EC = 0.0f;
        MR::makeMtxUpFront(&rotMtx, _C0, _CC);
        rotMtx.getQuat(_A8);
        if (forceFrame) {
            MR::forceToFrameCinemaFrame();
        }
    }

    if (mFirstIslandFleetVisit && MR::isStep(this, 15)) {
        MR::startSoundPlayer("SE_PM_SPINDRIVER_PANIC_2ND", -1, -1);
    }
    if (MR::isStep(this, 10) && !forceFrame) {
        MR::tryBlankToFrameCinemaFrame();
        mTitle->start();
    }

    updateBindPosition();
    if (!MR::isNearZero(_90)) {
        turnBindHead(_90, 0.4f);
    }

    updateShootMotion();
    mSpinDriverCamera->update(_90, mPosition);
    if (mFlyMotionStartStep <= getNerveStep() && getNerveStep() <= _F8) {
        _E4 = _E8 * MR::getEaseOutValue(MR::normalize(getNerveStep(), mFlyMotionStartStep, _F8), 0.0f, 1.0f, 1.0f);
    }

    _EC = MR::pi() * MR::normalize(getNerveStep(), _F8, mFallMotionStartStep);
    if (MR::isStageNoPauseMenuStars() && MR::isStep(this, 250)) {
        MR::startStageBGM("MBGM_SMG2_GALAXY_25", false);
    }
    if (trySkipTrigger()) {
        MR::stopSeCategories(30);
        MR::tryFrameToBlankCinemaFrame();
        _124 = true;
    }

    if (_124 && MR::isStopCinemaFrame()) {
        if (MR::isStageNoPauseMenuStars() && MR::isLessStep(this, 250)) {
            MR::startStageBGM("MBGM_SMG2_GALAXY_25", false);
        }
        setNerve(GET_NERVE(ScenarioStarter, ScenarioStarterRailMoveCanceled));
        return;
    }

    s32 timing = AudStageBgmTable::getWelcomeStartOffset(MR::getCurrentStageName(), MR::getCurrentScenarioNo());
    timing = MR::clamp(timing, -mFlightTime, 0);
    if (MR::isStep(this, mFlightTime + timing)
        && AudStageBgmTable::isStartOnWelcome(MR::getCurrentStageName(), MR::getCurrentScenarioNo())) {
        isStartBgmOnWelcome();
    }

    if (MR::isGreaterEqualStep(this, mFlightTime)) {
        MR::endBindAndSpinDriverJump(this, _9C);
        _8C = nullptr;
        mSpinDriverCamera->end();
        setNerve(GET_NERVE(ScenarioStarter, ScenarioStarterShowWelcomeLayout));
    }
}

void ScenarioStarter::exeRailMoveCanceled() {
    if (MR::isFirstStep(this)) {
        MR::endBindAndPlayerWait(this);
        _8C = nullptr;
        mSpinDriverCamera->cancel();
        mTitle->kill();
        if (AudStageBgmTable::isStartOnWelcome(MR::getCurrentStageName(), MR::getCurrentScenarioNo())) {
            isStartBgmOnWelcome();
        }
        setNerve(GET_NERVE(ScenarioStarter, ScenarioStarterShowWelcomeLayout));
    }
}

// NONMATCHING: regalloc
namespace {
    struct CameraException {
        const char* stage;
        s32 scenario;
    };

    const CameraException sCameraExceptions[] = {
        {"DigMineGalaxy", 3},
        {"VsKoopaJrLv1Galaxy", 2},
        {"KoopaJrLv2Galaxy", 2},
    };

    inline bool isStartPosCameraEnabled(const char* pStage, s32 scenario) {
        for (u32 i = 0; i < 3; i++) {
            if (scenario == sCameraExceptions[i].scenario && MR::isEqualString(pStage, sCameraExceptions[i].stage)) {
                return false;
            }
        }

        return true;
    }
}  // namespace

void ScenarioStarter::exeShowWelcomeLayout() {
    if (MR::isFirstStep(this)) {
        MR::tryFrameToScreenCinemaFrame();
        GameSceneFunction::notifyEndScenarioStarter();
        const char* pStage = MR::getCurrentStageName();
        s32 scenario = MR::getCurrentScenarioNo();
        bool startCamera = isStartPosCameraEnabled(pStage, scenario);
        if (startCamera) {
            MR::startStartPosCamera(!_124);
        }
        if (!MR::isDead(mTitle)) {
            mTitle->end();
        }
        if (MR::isStageNoPauseMenuStars()) {
            kill();
            return;
        }
        mWelcomeLayout->appear();
        MR::startAnim(mWelcomeLayout, "Appear", 0);
        MR::executeOnWelcomeAndRetry();
        MR::resetPlayerFollowPass();
    }
    if (MR::isStep(this, 90)) {
        MR::forceAppearDefaultGameLayout();
        MR::appearDestroyCounter();
    }
    if (MR::isAnimStopped(mWelcomeLayout, 0)) {
        kill();
    }
}

void ScenarioStarter::initShootPath(const JMapInfoIter& rIter) {
    initRailRider(rIter);
    mInitalPlayerRotation = 0.0f;
    MR::getJMapInfoArg5NoInit(rIter, &mInitalPlayerRotation);
    _B8 = new SpinDriverShootPath();
    _B8->init(rIter);
    _B8->calcInitPose(&_C0, &_CC, &_D8, mInitalPlayerRotation);
}

void ScenarioStarter::updateBindPosition() {
    TVec3f pathPos;
    TVec3f pathDir;
    f32 nerveRate = MR::calcNerveRate(this, mFlightTime);
    _B8->calcPosition(&pathPos, nerveRate);
    _B8->calcDirection(&pathDir, nerveRate, 0.01f);
    if (!MR::isNearZero(pathDir)) {
        _90 = pathDir;
    }

    _9C.set(pathPos);
    _9C -= mPosition;
    MR::setPosition(this, pathPos);
}

void ScenarioStarter::updateBindActorMtx() {
    if (isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterRailMove))) {
        TPos3f rotateY;
        rotateY.identity();
        rotateY.setEulerY(_E4);
        rotateY.zeroTrans();

        TPos3f rotateX;
        rotateX.identity();
        rotateX.setEulerX(_EC);

        TPos3f mtx;
        mtx.setQT(_A8, mPosition);
        mtx.concat(mtx, rotateX);
        if (!mFirstIslandFleetVisit) {
            mtx.concat(mtx, rotateY);
        }
        MR::setBaseTRMtx(_8C, mtx);
        PSMTXCopy(mtx, mFollowMtx);
    } else {
        TPos3f mtx;
        MR::makeMtxTR(mtx, _10C, _118);
        MR::setBaseTRMtx(_8C, mtx);
        PSMTXCopy(mtx, mFollowMtx);
    }
}

void ScenarioStarter::updateShootMotion() {
    if (MR::isStep(this, mFlyMotionStartStep)) {
        if (mFirstIslandFleetVisit) {
            MR::tryStartActionNotPlaying(_8C, "CannonFlyStart");
        } else {
            MR::startBckWithInterpole(_8C, "SpaceFlyLoop", 5);
        }
    }

    if (MR::isLessStep(this, _F8)) {
        MR::startLevelSoundPlayer("SE_PM_LV_S_SPIN_DRV_FLY", -1);
    }

    if (MR::isStep(this, _F8)) {
        MR::startBckWithInterpole(_8C, "SpaceFlyEnd", 0);
        MR::startSoundPlayer("SE_PM_S_SPIN_DRV_COOL_DOWN", -1, -1);
        MR::startSoundPlayer("SE_PV_JUMP_S", -1, -1);
    }

    if (MR::isStep(this, mFallMotionStartStep)) {
        MR::startBckWithInterpole(_8C, "Fall", 20);
    }
}

void ScenarioStarter::turnBindHead(const TVec3f& rHead, f32 rate) {
    MR::turnQuatYDirRate(&_A8, _A8, _90, 1.0f);
    TVec3f head;
    _A8.getYDir(head);
    TQuat4f rotation;
    rotation.setRotate(head, rHead, rate);
    _A8.mult(rotation);
    _A8.normalize();
}

void ScenarioStarter::calcShootMotionTime() {
    s32 startTime = MR::getBckFrameMax(_8C, "SpaceFlyStart");
    MR::getBckFrameMax(_8C, "SpaceFlyLoop");
    s32 endTime = MR::getBckFrameMax(_8C, "SpaceFlyEnd");

    if (mFlightTime < 20) {
        mFlyMotionStartStep = -1;
        _F8 = -1;
        mFallMotionStartStep = 0;
        return;
    }

    if (mFlightTime < endTime + 20) {
        mFlyMotionStartStep = -1;
        _F8 = 0;
        mFallMotionStartStep = -1;
        return;
    }

    _F8 = mFlightTime - (endTime + 20);
    mFallMotionStartStep = mFlightTime - 20;
    s32 flyTime = 0.2f * mFlightTime;
    if (flyTime > 90) {
        flyTime = 90;
    }

    mFlyMotionStartStep = startTime * (flyTime / startTime);
    if (_F8 <= mFlyMotionStartStep) {
        mFlyMotionStartStep = 0;
    }

    f32 duration = 0.05f * (_F8 - mFlyMotionStartStep);
    _E4 = 0.0f;
    _E8 = static_cast< s32 >(duration / TWO_PI) * TWO_PI;
}

bool ScenarioStarter::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (_8C == nullptr && MR::isMsgAutoRushBegin(msg) && isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterWaitToStart))) {
        _8C = MR::getSensorHost(pSender);
        setNerve(GET_NERVE(ScenarioStarter, ScenarioStarterCinemaFrameBlank));
        return true;
    }

    if (_8C != nullptr && MR::isMsgUpdateBaseMtx(msg)) {
        updateBindActorMtx();
        return true;
    }

    if (_8C && (MR::isMsgRushForceCancel(msg) || MR::isMsgRushCancel(msg))
        && (isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterRailMove)) || isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterRailMoveCanceled)))) {
        MR::endBindAndPlayerWait(this);
        _8C = nullptr;
        GameSceneFunction::notifyEndScenarioStarter();
        MR::executeOnWelcomeAndRetry();
        MR::resetPlayerFollowPass();
        kill();
        return true;
    }
    if (MR::isMsgStartDemo(msg) && isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterShowWelcomeLayout))) {
        if (MR::isLessStep(this, 1)) {
            GameSceneFunction::notifyEndScenarioStarter();
            MR::executeOnWelcomeAndRetry();
            MR::resetPlayerFollowPass();
        }
        kill();
        return true;
    }
    return false;
}

bool ScenarioStarter::trySkipTrigger() const {
    if (MR::isStageNoPauseMenuStars()) {
        return false;
    }
    if (mFirstIslandFleetVisit) {
        return false;
    }
    bool result;
    if (!MR::isAlreadyVisitedCurrentStageAndScenario()) {
        result = false;
    } else {
        result = false;
        if (MR::isGreaterStep(this, 10) && MR::isLessStep(this, _F8) && MR::testSystemPadTriggerDecide()) {
            result = true;
        }
    }

    return result;
}

void ScenarioStarter::isStartBgmOnWelcome() {
    MR::resetStageBGMInterrupted();
    if (MR::unlockStageBGM() == 0) {
        MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo(), false, false);
    }
}


ScenarioStarter::~ScenarioStarter() {
}

void ScenarioStarter::kill() {
    mWelcomeLayout->kill();
    mTitle->kill();
    LiveActor::kill();
}

void ScenarioStarter::getStartPos(TVec3f* pos) {
    pos->set(MR::getRailPointPosStart(this));
}

bool ScenarioStarter::isFollowValid() const {
    bool valid = false;
    if (!MR::isDead(this) && !isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterWaitScenarioCameraEnd))
        && !isNerve(GET_NERVE(ScenarioStarter, ScenarioStarterShowWelcomeLayout))) {
        valid = true;
    }
    return valid;
}

void ScenarioStarter::exeWaitScenarioCameraEnd() {
    if (!MR::isStageStateScenarioOpeningCamera()) {
        setNerve(GET_NERVE(ScenarioStarter, ScenarioStarterWaitToStart));
    }
}
