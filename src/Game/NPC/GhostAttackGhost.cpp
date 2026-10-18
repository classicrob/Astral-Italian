#pragma auto_inline off

#include "Game/NPC/GhostAttackGhost.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimePlayer.hpp"

class GhostAttackGhostFootprint : public LiveActor {
public:
    GhostAttackGhostFootprint(LiveActor* pHost);
    virtual void appear();
    virtual void control();
    virtual MtxPtr getBaseMtx() const;
};

namespace MR {
    void registerGhostAttackGhost(GhostAttackGhost*);
    bool isGhostLuigiOpenInCurrentStage();
}  // namespace MR

extern "C" XanimeResourceTable* fn_8004F900(s32);
extern "C" int snprintf(char*, unsigned long, const char*, ...);
extern "C" void fn_800345E0(LiveActor*, const char*);

namespace NrvGhostAttackGhost {
    NEW_NERVE(GhostAttackGhostNrvStart, GhostAttackGhost, Start);
    NEW_NERVE(GhostAttackGhostNrvWait, GhostAttackGhost, Wait);
    NEW_NERVE(GhostAttackGhostNrvAppear, GhostAttackGhost, Appear);
    NEW_NERVE(GhostAttackGhostNrvNear, GhostAttackGhost, Near);
    NEW_NERVE(GhostAttackGhostNrvAttack, GhostAttackGhost, Attack);
    NEW_NERVE(GhostAttackGhostNrvHit, GhostAttackGhost, Hit);
    NEW_NERVE(GhostAttackGhostNrvStop, GhostAttackGhost, Stop);
    NEW_NERVE(GhostAttackGhostNrvDemo, GhostAttackGhost, Demo);
    NEW_NERVE(GhostAttackGhostNrvTerminate, GhostAttackGhost, Terminate);
};  // namespace NrvGhostAttackGhost

GhostAttackGhost::GhostAttackGhost(const char* pName) : LiveActor(pName) {
    mRaceDataReader = NULL;
    mFootprint = NULL;
    mHostSensor = NULL;
    mXanimeResTable = fn_8004F900(1);
    mObjArg0 = 0;
    mPrevPos = TVec3f(0.0f);
    mPrevPos2 = TVec3f(0.0f);
    mHandJointCtrl = NULL;
    mHeadJointCtrl = NULL;
    MR::registerGhostAttackGhost(this);
}

/* NONMATCHING: init stage-branch order and joint-controller wiring are unverified */
void GhostAttackGhost::init(const JMapInfoIter& rIter) {
    if (MR::isEqualStageName("StarCreekGalaxy") && MR::getCurrentScenarioNo() == 2) {
        MR::initActor(this, rIter, "GhostAttackGhost", "2DWater", NULL, false);
        MR::startAction(this, "Irregular");
    } else if (MR::isEqualStageName("MagicGalaxy")) {
        MR::initActor(this, rIter, "GhostAttackGhost", "2DWater", NULL, false);
        MR::startAction(this, "Irregular");
    } else {
        MR::initActor(this, rIter, "GhostAttackGhost", "2DWater", NULL, false);
    }

    mHandJointCtrl = MR::createJointController(this, this, "Hand", &GhostAttackGhost::calcHandJoint, &GhostAttackGhost::calcHandJoint);
    mHeadJointCtrl = MR::createJointController(this, this, "Head", &GhostAttackGhost::calcHandJoint, &GhostAttackGhost::calcHandJoint);

    MR::getJMapInfoArg0NoInit(rIter, &mObjArg0);
    s32 scenarioNo = MR::getCurrentOrSelectedScenarioNo();
    if (scenarioNo <= 0) {
        scenarioNo = 0;
    }
    char nameBuf[0x100];
    snprintf(nameBuf, 0x100, "GhostAttackGhostData%02d.gst", scenarioNo);

    mRaceDataReader = new RaceDataReader(this, nameBuf, mXanimeResTable);
    if (mRaceDataReader->mIsEnable != 0) {
        mFootprint = new GhostAttackGhostFootprint(this);
    }

    if (mRaceDataReader->mIsEnable != 0) {
        initNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvWait), 0);
    } else {
        initNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvStop), 0);
    }

    if (MR::isGhostLuigiOpenInCurrentStage()) {
        makeActorAppeared();
    } else {
        makeActorDead();
    }
}

void GhostAttackGhost::makeActorAppeared() {
    if (MR::isGhostLuigiOpenInCurrentStage()) {
        LiveActor::makeActorAppeared();
    }
}

MtxPtr GhostAttackGhost::getBaseMtx() const {
    if (mHostSensor != NULL) {
        return MR::getSensorHost(mHostSensor)->getBaseMtx();
    }
    return LiveActor::getBaseMtx();
}

bool GhostAttackGhost::calcHandJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    TVec3f scale(1.0f);
    (*pMtx)[0][0] = scale.x;
    (*pMtx)[0][1] = 0.0f;
    (*pMtx)[0][2] = 0.0f;
    (*pMtx)[1][0] = 0.0f;
    (*pMtx)[1][1] = scale.y;
    (*pMtx)[1][2] = 0.0f;
    (*pMtx)[2][0] = 0.0f;
    (*pMtx)[2][1] = 0.0f;
    (*pMtx)[2][2] = scale.z;
    return true;
}

/* NONMATCHING: matrix blend and rotate sequence not yet verified */
void GhostAttackGhost::calcAndSetBaseMtx() {
    if (mHostSensor != NULL) {
        TPos3f hostMtx;
        hostMtx.set(MR::getSensorHost(mHostSensor)->getBaseMtx());
        MR::setBaseTRMtx(this, hostMtx);
        return;
    }
    const char* pAnimName = MR::getPlayingBckName(this);
    if (MR::isEqualSubString(pAnimName, "Hopper")) {
        mHandJointCtrl->registerCallBack();
    }
}

/* NONMATCHING: control() sync of footprint and previous positions not verified */
void GhostAttackGhost::control() {
    if (mFootprint != NULL && !MR::isDead(mFootprint)) {
        mFootprint->movement();
    }
    mPrevPos = mPosition;
    mPrevPos2 = mPosition;
}

/* NONMATCHING: sensor filters are a best reading of the attackSensor asm */
void GhostAttackGhost::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvNear)) &&
        !isNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvAttack))) {
        return;
    }
    if (!MR::isSensorType(pReceiver, 0x61)) {
        return;
    }
    mHostSensor = pReceiver;
    MR::getSensorHost(pReceiver)->getBaseMtx();
    setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvHit));
}

/* NONMATCHING: exe bodies below are transcribed from m2c drafts; nerve mapping unverified */
void GhostAttackGhost::exeStart() {
    if (MR::isFirstStep(this)) {
        mRaceDataReader->startActionName("GhostStartWait");
        MR::startBtp(this, "Blink");
    }
}

void GhostAttackGhost::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        fn_800345E0(this, "Jn");
        setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvHit));
        if (mFootprint != NULL) {
            mFootprint->appear();
        }
    }
}

void GhostAttackGhost::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this, 0);
        mRaceDataReader->receivePacket();
    }
    if (mFootprint != NULL && !MR::isDead(mFootprint)) {
        mFootprint->movement();
    }
    if (mRaceDataReader->receivePacket()) {
        setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvNear));
    }
}

void GhostAttackGhost::exeNear() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        if (MR::isExecScenarioStarter()) {
            MR::validateHitSensors(this);
        }
    }
    if (mRaceDataReader->mXanimePlayer->isTerminate()) {
        setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvAttack));
        return;
    }
    if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}

void GhostAttackGhost::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        mRaceDataReader->startActionName("GhostStarWaitStart");
        if (mFootprint != NULL) {
            mFootprint->appear();
        }
    }
    if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}

void GhostAttackGhost::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
    }
    if (mRaceDataReader->mIsEnable == 0) {
        mRaceDataReader->startActionName("SleepLie");
        setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvStop));
        return;
    }
    mRaceDataReader->startActionName("Sleep");
    setNerve(GET_NERVE(GhostAttackGhost, GhostAttackGhostNrvHit));
}

void GhostAttackGhost::exeDemo() {
    if (MR::isFirstStep(this)) {
        mRaceDataReader->startActionName("GhostStarWait");
    }
    if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}

void GhostAttackGhost::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
        mRaceDataReader->startActionName("GhostStarWaitAir");
    }
    if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}

void GhostAttackGhost::exeTerminate() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
        mRaceDataReader->startActionName("GhostStarWaitStart");
    }
    if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}
