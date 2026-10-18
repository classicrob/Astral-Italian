#include "Game/MapObj/WorldMapAccessPanel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "JSystem/JGeometry/TVec.hpp"

extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_8001C030(s32);
extern "C" s32 fn_80059390();
extern "C" void fn_80059210();
extern "C" void fn_800592D0();
extern "C" void fn_80088630();
extern "C" s32 fn_804D8290();
extern "C" s32 fn_804E8E90();
extern "C" u32 fn_804D72A0(s32);
extern "C" void fn_804D6520();

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

#define WMAP_NRV(name) (&NrvWorldMapAccessPanel::WorldMapAccessPanelNrv##name::sInstance)

WorldMapAccessPanel::WorldMapAccessPanel(const char* pName) : LiveActor(pName) {
    mWorldIdA = -1;
    mWorldIdB = -1;
    mCounterA = 0;
    mCounterB = 0;
    mJointCtrl = nullptr;
    mIsHipDropped = 0;
    mCameraInfo = nullptr;
    mFloatAC = 0.0f;
    mFloatB0 = 0.0f;
    mFloatB4 = 0.0f;
}

void WorldMapAccessPanel::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::getJMapInfoArg0NoInit(rIter, &mWorldIdA);
    MR::getJMapInfoArg1NoInit(rIter, &mWorldIdB);

    JointControlDelegator< WorldMapAccessPanel >* joint =
        new JointControlDelegator< WorldMapAccessPanel >(this, &WorldMapAccessPanel::calcJointMtx, nullptr);
    MR::setJointControllerParam(joint, this, "Panel");
    mJointCtrl = joint;

    MR::initActorCamera(this, rIter, &mCameraInfo);
    initNerve(WMAP_NRV(Wait), 0);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::createCenterScreenBlur();

    if (MR::tryRegisterDemoCast(this, "ワールドマップからの開始", rIter)) {
        MR::tryRegisterDemoActionFunctor(this, MR::Functor(this, &WorldMapAccessPanel::startWorldMapWipe), nullptr);
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
        fn_800345E0(this, "パネルオン");
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
            fn_800345E0(this, "パネルオフ");
            MR::startActionSound(this, "RideOff", -1, -1, -1);
            setNerve(WMAP_NRV(Wait));
        }
    }
}

void WorldMapAccessPanel::exeDemoStart() {
    s32 stepLimit = sStepBase + 0x1E;

    if (MR::isGreaterEqualStep(this, sStepLimit)) {
        if (MR::isLessEqualStep(this, 1)) {
            fn_800345E0(this, "移動開始");
        }
    }

    if (MR::isFirstStep(this)) {
        MR::setPlayerPosAndWait(*MR::getPlayerPos(), false);
        TVec3f camPos = MR::getCamPos();
        mFloatB4 = camPos.z;
        MR::startAction(this, "Access");
        fn_8001C030(0x1E);
    }

    if (MR::isStep(this, sStepBase)) {
        MR::startCenterScreenBlur(0x3C, 15.0f, 0x50, 5, 0x1E);
        MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
    }

    if (MR::isStep(this, stepLimit - 0xA)) {
        // NONMATCHING: world id query chain (fn_804E8E90 / fn_800592D0 / fn_80059210)
        if (fn_804E8E90() != 0) {
            fn_800592D0();
        } else {
            fn_80059210();
        }
    }

    if (MR::isGreaterEqualStep(this, stepLimit)) {
        if (fn_804D8290() != 0) {
            fn_80088630();
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
                fn_800345E0(this, "帰還開始");
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
        if (fn_80059390() != 0) {
            MR::resumeTimeKeepDemo(this);
            MR::startCenterScreenBlur(0x2D, 15.0f, 0x50, 5, 0x1E);
            setNerve(WMAP_NRV(AccessEnd));
        }
    }
}

void WorldMapAccessPanel::exeReturn() {
    if (!MR::isLessEqualStep(this, 0xA)) {
        if (!isPlayerOutOfRange()) {
            MR::startBrkAndSetFrameAndStop(this, "Access", 0.0f);
            MR::startActionSound(this, "RideOff", -1, -1, -1);
            fn_800345E0(this, "パネルオフ");
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
    f32 radius = MR::getSensorRadius(this, "Hit");
    f32 dist = MR::calcDistanceToPlayer(hitPos);

    bool inRange = dist < radius;
    if (!inRange) {
        return false;
    }
    if (!MR::isOnPlayer(this)) {
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

    if (!(dist < radius) && !(dist2 < radius2)) {
        return false;
    }
    return MR::isOnGroundPlayer();
}

bool WorldMapAccessPanel::isPlayerOutOfRange() {
    // NONMATCHING: func_8003DEF0 (plane/projection helper chain) not identified; distances approximated
    MR::calcGravity(this);
    TVec3f hitPos(*MR::getSensorPos(this, "Hit"));
    TVec3f hitPos2(*MR::getSensorPos(this, "Hit2"));
    f32 radius = MR::getSensorRadius(this, "Hit");
    f32 radius2 = MR::getSensorRadius(this, "Hit2");
    f32 dist = MR::calcDistanceToPlayer(hitPos);
    f32 dist2 = MR::calcDistanceToPlayer(hitPos2);
    return dist < radius || dist2 < radius2;
}

void WorldMapAccessPanel::setWorldIdsAndReset() {
    // NONMATCHING: GameDataFunction::setCurrentWorldId call not resolved
    u32 worldId = 0;
    if (mWorldIdB >= 0) {
        worldId = fn_804D72A0(mWorldIdB);
    }
    if (mWorldIdA >= 0) {
        (void)worldId;
    }
    fn_804D6520();
}

void WorldMapAccessPanel::startWorldMapWipe() {
    MR::forceOpenWipeCircle();
    MR::startCenterScreenBlur(0x2D, 15.0f, 0x50, 5, 0x1E);
}

WorldMapAccessPanel::~WorldMapAccessPanel() {
}

// NrvWorldMapAccessPanel nop nerve instances
