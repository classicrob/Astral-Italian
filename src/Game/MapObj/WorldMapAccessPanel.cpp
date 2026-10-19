#include "Game/MapObj/WorldMapAccessPanel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapFunction.hpp"

#include "JSystem/JGeometry/TVec.hpp"


// file-static step values (sbss2 in the target)
static s32 sStepBase;
static s32 sStepLimit;
static s32 sStepCheck;

namespace NrvWorldMapAccessPanel {
    NEW_NERVE(WorldMapAccessPanelNrvWait, WorldMapAccessPanel, Wait);
    NEW_NERVE(WorldMapAccessPanelNrvStand, WorldMapAccessPanel, Stand);
    NEW_NERVE(WorldMapAccessPanelNrvDemoStart, WorldMapAccessPanel, DemoStart);
    NEW_NERVE(WorldMapAccessPanelNrvDemoStep, WorldMapAccessPanel, DemoStep);

    // nerves whose execute is empty in the target (fn_8032DA10 / fn_8032D9D0)
    class WorldMapAccessPanelNrvNopA : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static WorldMapAccessPanelNrvNopA sInstance;
    };
    WorldMapAccessPanelNrvNopA WorldMapAccessPanelNrvNopA::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(WorldMapAccessPanelNrvAccess, WorldMapAccessPanel, Access);
    NEW_NERVE(WorldMapAccessPanelNrvAccessEnd, WorldMapAccessPanel, Access);
    NEW_NERVE(WorldMapAccessPanelNrvReturn, WorldMapAccessPanel, Return);

    class WorldMapAccessPanelNrvNopB : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static WorldMapAccessPanelNrvNopB sInstance;
    };
    WorldMapAccessPanelNrvNopB WorldMapAccessPanelNrvNopB::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWorldMapAccessPanel

static TVec3f sZeroVec(0.0f, 0.0f, 0.0f);

#define WMAP_NRV(name) (&NrvWorldMapAccessPanel::WorldMapAccessPanelNrv##name::sInstance)

WorldMapAccessPanel::WorldMapAccessPanel(const char* pName) : LiveActor(pName) {
    mWorldIdA = -1;
    mWorldIdB = -1;
    mCounterA = 0;
    mCounterB = 0;
    mJointCtrl = nullptr;
    mIsHipDropped = 0;
    mCameraInfo = nullptr;
    mCamPos.x = 0.0f;
    mCamPos.y = 0.0f;
    mCamPos.z = 0.0f;
}

void WorldMapAccessPanel::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::getJMapInfoArg0NoInit(rIter, &mWorldIdA);
    MR::getJMapInfoArg1NoInit(rIter, &mWorldIdB);

    const char* pPanel = "Panel";
    bool (WorldMapAccessPanel::*pCalc)(TPos3f*, const JointControllerInfo&) = &WorldMapAccessPanel::calcJointMtx;
    JointControlDelegator< WorldMapAccessPanel >* joint =
        new JointControlDelegator< WorldMapAccessPanel >(this, pCalc, nullptr);
    MR::setJointControllerParam(joint, this, pPanel);
    mJointCtrl = joint;

    MR::initActorCamera(this, rIter, &mCameraInfo);
    initNerve(WMAP_NRV(Wait), 0);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::createCenterScreenBlur();

    if (MR::tryRegisterDemoCast(this, "ワールドマップからの開始", rIter)) {
        MR::tryRegisterDemoActionFunctor(this, MR::Functor_Inline(this, &WorldMapAccessPanel::startWorldMapWipe), nullptr);
        MR::tryRegisterDemoActionNerve(this, WMAP_NRV(Access), nullptr);
    }
    MR::invalidateClipping(this);

    if (MR::tryRegisterDemoCast(this, "マリオ顔惑星変化", rIter)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
    MR::startBrkAndSetFrameAndStop(this, "Access", 0.0f);
}

void WorldMapAccessPanel::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    if (mJointCtrl != nullptr) {
        mJointCtrl->registerCallBack();
    }
}

void WorldMapAccessPanel::exeWait() {
    if (isPlayerOnAccessSensor()) {
        MR::startActionSound(this, "RideOnTrg", -1, -1, -1);
        MR::tryHitReaction(this, "パネルオン");
        setNerve(WMAP_NRV(Stand));
    }
}

void WorldMapAccessPanel::exeStand() {
    if (!MR::isValidSwitchA(this)) {
        MR::startActionSound(this, "EngineStarting", -1, -1, -1);
    }

    if (isPlayerNearAccessGround()) {
        mCounterB = 0;
        mCounterA++;
        if (mCounterA >= 0x14) {
            if (MR::isValidSwitchA(this)) {
                MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(this, "マリオ顔惑星発進デモ起動",
                                                                      WMAP_NRV(DemoStep),
                                                                      WMAP_NRV(NopA));
                return;
            }
            MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(this, "ワールドマップ移動開始",
                                                                  WMAP_NRV(DemoStart),
                                                                  WMAP_NRV(NopA));
            MR::startActionSound(this, "Activate", -1, -1, -1);
        }
    } else {
        mIsHipDropped = 0;
        mCounterB++;
        if (mCounterB >= 0xA) {
            mCounterB = 0;
            mCounterA = 0;
            MR::startBrkAndSetFrameAndStop(this, "Access", 0.0f);
            MR::tryHitReaction(this, "パネルオフ");
            MR::startActionSound(this, "RideOff", -1, -1, -1);
            setNerve(WMAP_NRV(Wait));
        }
    }
}

void WorldMapAccessPanel::exeDemoStart() {
    s32 stepLimit = sStepBase + 0x1E;

    if (MR::isGreaterEqualStep(this, sStepLimit)) {
        if (MR::isLessEqualStep(this, 1)) {
            MR::tryHitReaction(this, "移動開始");
        }
    }

    if (MR::isFirstStep(this)) {
        MR::setPlayerPosAndWait(*MR::getPlayerPos(), false);
        mCamPos = MR::getCamPos();
        MR::startAction(this, "Access");
        MR::stopStageAndSubBGM(0x1E);
    }

    if (MR::isStep(this, sStepBase)) {
        MR::startCenterScreenBlur(0x3C, 15.0f, 0x50, 5, 0x1E);
        MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
    }

    if (MR::isStep(this, stepLimit - 0xA)) {
        // NONMATCHING: world id query chain (WorldMapFunction::isWorldNoMid / startSpaceWipeBackward / startCloudWipeBackward)
        if (WorldMapFunction::isWorldNoMid(GameDataFunction::getCurrentWorldId()) != 0) {
            MR::startSpaceWipeBackward();
        } else {
            MR::startCloudWipeBackward();
        }
    }

    if (MR::isGreaterEqualStep(this, stepLimit)) {
        if (GameSequenceFunction::isGreenStarEventOnly() != 0) {
            AudInstListMgr::requestInstrumentRefresh();
        }
        MR::endActorCamera(this, mCameraInfo, false, -1);
        MR::endDemo(this, "ワールドマップ移動開始");
        setWorldIdsAndReset();
        setNerve(WMAP_NRV(NopB));
        return;
    }

    MR::startActionSound(this, "EngineStarting", -1, -1, -1);
}

void WorldMapAccessPanel::exeDemoStep() {
    if (MR::isFirstStep(this)) {
        if (mIsHipDropped == 0) {
            MR::setPlayerPosAndWait(*MR::getPlayerPos(), false);
        }
        MR::startAction(this, "Access");
    }
    if (MR::isGreaterEqualStep(this, 1)) {
        MR::endDemo(this, "マリオ顔惑星発進デモ起動");
        MR::onSwitchA(this);
        setNerve(WMAP_NRV(NopB));
    }
}

void WorldMapAccessPanel::exeAccess() {
    if (isNerve(WMAP_NRV(AccessEnd))) {
        if (MR::isGreaterEqualStep(this, sStepCheck)) {
            if (MR::isLessEqualStep(this, 1)) {
                MR::tryHitReaction(this, "帰還開始");
            }
        }
        if (!MR::isDemoActive()) {
            setNerve(WMAP_NRV(Return));
        }
    } else {
        if (MR::isFirstStep(this)) {
            MR::startBrk(this, "Access");
            MR::setAllAnimFrameAtEnd(this, "Access");
            MR::forceOpenSystemWipeCircle();
            MR::pauseTimeKeepDemo(this);
            return;
        }
        if (MR::isHideOrEndModelWipe()) {
            MR::resumeTimeKeepDemo(this);
            MR::startCenterScreenBlur(0x2D, 15.0f, 0x50, 5, 0x1E);
            setNerve(WMAP_NRV(AccessEnd));
        }
    }
}

void WorldMapAccessPanel::exeReturn() {
    if (!MR::isLessEqualStep(this, 0xA)) {
        MR::calcDistanceToPlayer(mPosition);
        if (!isPlayerOutOfRange()) {
            MR::startBrkAndSetFrameAndStop(this, "Access", 0.0f);
            MR::startActionSound(this, "RideOff", -1, -1, -1);
            MR::tryHitReaction(this, "パネルオフ");
            setNerve(WMAP_NRV(Wait));
        }
    }
}

bool WorldMapAccessPanel::calcJointMtx(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    if (isNerve(WMAP_NRV(Wait))) {
        return true;
    }

    TVec3f pos;
    TVec3f up;
    MR::copyJointPos(this, "Panel", &pos);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd((Vec*)&up, (Vec*)&pos, (Vec*)&pos, -20.0f);
    pMtx->mMtx[0][3] = pos.x;
    pMtx->mMtx[1][3] = pos.y;
    pMtx->mMtx[2][3] = pos.z;
    return true;
}

bool WorldMapAccessPanel::isPlayerOnAccessSensor() {
    TVec3f hitPos(*MR::getSensorPos(this, "Hit"));
    TVec3f hitPos2(*MR::getSensorPos(this, "Hit2"));
    f32 radius = MR::getSensorRadius(this, "Hit");
    f32 radius2 = MR::getSensorRadius(this, "Hit2");
    f32 dist = MR::calcDistanceToPlayer(hitPos);
    f32 dist2 = MR::calcDistanceToPlayer(hitPos2);
    bool inRange = dist < radius || dist2 < radius2;

    if (!MR::isOnPlayer(this) || !inRange) {
        return false;
    }
    if (MR::isPlayerHipDropLand()) {
        mIsHipDropped = 1;
    }
    return true;
}

bool WorldMapAccessPanel::isPlayerNearAccessGround() {
    TVec3f hitPos(*MR::getSensorPos(this, "Hit"));
    TVec3f hitPos2(*MR::getSensorPos(this, "Hit2"));
    f32 radius = MR::getSensorRadius(this, "Hit");
    f32 radius2 = MR::getSensorRadius(this, "Hit2");
    f32 dist = MR::calcDistanceToPlayer(hitPos);
    f32 dist2 = MR::calcDistanceToPlayer(hitPos2);
    bool inRange = dist < radius || dist2 < radius2;

    // NONMATCHING: branch layout (target emits the isOnGroundPlayer call block before the "return false" block)
    if (!inRange) {
        return false;
    }
    return MR::isOnGroundPlayer();
}

bool WorldMapAccessPanel::isPlayerOutOfRange() {
    MR::calcGravity(this);
    TVec3f hitPos(*MR::getSensorPos(this, "Hit"));
    TVec3f hitPos2(*MR::getSensorPos(this, "Hit2"));
    f32 radius = MR::getSensorRadius(this, "Hit");
    f32 radius2 = MR::getSensorRadius(this, "Hit2");
    f32 c = 0.0f;
    f32 b = 0.0f;
    f32 a = 0.0f;
    MR::func_8003DEF0(&b, &c, MR::getPlayerPos(), &hitPos, &mGravity);
    MR::func_8003DEF0(&a, &c, MR::getPlayerPos(), &hitPos2, &mGravity);
    return b < radius || a < radius2;
}

void WorldMapAccessPanel::setWorldIdsAndReset() {
    if (mWorldIdB >= 0) {
        GameSequenceFunction::setWorldMapCurrentNode(mWorldIdB);
    }
    s32 worldId = mWorldIdA;
    if (worldId >= 0) {
        GameDataFunction::setCurrentWorldId(worldId);
    }
    GameSequenceFunction::requestChangeStageWorldMapEnterType1();
}

void WorldMapAccessPanel::startWorldMapWipe() {
    MR::forceOpenWipeCircle();
    MR::startCenterScreenBlur(0x2D, 15.0f, 0x50, 5, 0x1E);
}

WorldMapAccessPanel::~WorldMapAccessPanel() {
}

// NrvWorldMapAccessPanel nop nerve instances
