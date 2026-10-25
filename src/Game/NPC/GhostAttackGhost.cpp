#include "Game/NPC/GhostAttackGhost.hpp"
#include "Game/NPC/GhostAttackGhostFootprint.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <cstdio>

extern const char lbl_807CFEB0[4], lbl_807CF430[4];

namespace NrvGhostAttackGhost {
    NEW_NERVE(GhostAttackGhostNrvStart, GhostAttackGhost, Start);
    NEW_NERVE(GhostAttackGhostNrvAppear, GhostAttackGhost, Appear);
    NEW_NERVE(GhostAttackGhostNrvRun, GhostAttackGhost, Run);
    NEW_NERVE(GhostAttackGhostNrvWait, GhostAttackGhost, Wait);
    NEW_NERVE(GhostAttackGhostNrvBind, GhostAttackGhost, Bind);
    NEW_NERVE(GhostAttackGhostNrvStarWaitStart, GhostAttackGhost, StarWaitStart);
    NEW_NERVE(GhostAttackGhostNrvStarWait, GhostAttackGhost, StarWait);
    class GhostAttackGhostNrvStarWaitAir : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            GhostAttackGhost* actor = static_cast<GhostAttackGhost*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) actor->startSleep();
        }
        static GhostAttackGhostNrvStarWaitAir sInstance;
    };
    GhostAttackGhostNrvStarWaitAir GhostAttackGhostNrvStarWaitAir::sInstance ATTRIBUTE_WEAK;
    class GhostAttackGhostNrvSleep : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            GhostAttackGhost* actor = static_cast<GhostAttackGhost*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) actor->startSleep();
        }
        static GhostAttackGhostNrvSleep sInstance;
    };
    GhostAttackGhostNrvSleep GhostAttackGhostNrvSleep::sInstance ATTRIBUTE_WEAK;
}
#define GHOST_NERVE(name) (&NrvGhostAttackGhost::GhostAttackGhostNrv##name::sInstance)

GhostAttackGhost::GhostAttackGhost(const char* name) : LiveActor(name), mRaceDataReader(nullptr), mFootprint(nullptr), mHostSensor(nullptr), mXanimeResTable(MR::createPlayerXanimeResource(true)), mObjArg0(0), mMovementDelta(TVec3f(0.0f)), mPreviousPosition(TVec3f(0.0f)), mHandJointCtrl(nullptr), mHeadJointCtrl(nullptr) {
    MR::registerGhostAttackGhost(this);
}

// NONMATCHING: temporary register assignment.
void GhostAttackGhost::init(const JMapInfoIter& iter) {
    bool irregular;
    if (MR::isEqualStageName("StarCreekGalaxy") && MR::getCurrentScenarioNo() == 2) irregular = true;
    else irregular = MR::isEqualStageName("MagicGalaxy");
    if (irregular) {
        MR::initActor(this, iter, "GhostAttackGhost", "2DWater", false);
        MR::startAction(this, "Irregular");
    } else {
#ifdef GALAXY_NATIVE
        MR::initActor(this, iter, "GhostAttackGhost", "", false);  // lbl_807CFEB0 is this pooled empty string
#else
        MR::initActor(this, iter, "GhostAttackGhost", lbl_807CFEB0, false);
#endif
    }
    mHandJointCtrl = MR::createJointController(this, "HandL0", &GhostAttackGhost::calcHandJoint);
    mHeadJointCtrl = MR::createJointController(this, "HandR0", &GhostAttackGhost::calcHandJoint);
    MR::getJMapInfoArg0NoInit(iter, &mObjArg0);
    s32 scenario = GameSequenceFunction::getPlacedGreenStarScenarioNo(MR::getCurrentStageName(), MR::getCurrentOrSelectedScenarioNo());
    if (scenario <= 0) scenario = 0;
    char filename[256];
    snprintf(filename, sizeof(filename), "GhostAttackGhostData%02d.gst", scenario);
    mRaceDataReader = new RaceDataReader(this, filename, mXanimeResTable);
    if (mRaceDataReader->mPacketReader != nullptr) mFootprint = new GhostAttackGhostFootprint(this);
    initNerve(mRaceDataReader->mPacketReader != nullptr ? GHOST_NERVE(Start) : static_cast<const Nerve*>(GHOST_NERVE(Sleep)), 0);
    MR::useStageSwitchAwake(this, iter);
    if (MR::isGhostLuigiOpenInCurrentStage()) makeActorAppeared();
    else makeActorDead();
}
void GhostAttackGhost::makeActorAppeared() {
    if (MR::isGhostLuigiOpenInCurrentStage()) LiveActor::makeActorAppeared();
}
void GhostAttackGhost::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!tryAttachToHost(sender, receiver) && !tryStartRace(sender, receiver) && (isNerve(GHOST_NERVE(Wait)) || isNerve(GHOST_NERVE(StarWait)))) {
        if (MR::isSensorType(receiver, 0x61)) {
            mHostSensor = receiver;
            TPos3f mtx(MR::getSensorHost(receiver)->getBaseMtx());
            mtx.getTrans(mPosition);
            TVec3f rotation;
            mtx.getEuler(rotation);
            mRotation.set(rotation * 57.29578f);
            setNerve(GHOST_NERVE(Bind));
        }
    }
}
MtxPtr GhostAttackGhost::getBaseMtx() const {
    if (mHostSensor != nullptr) return MR::getSensorHost(mHostSensor)->getBaseMtx();
    return LiveActor::getBaseMtx();
}
const char* GhostAttackGhost::getRecordedAnimationName() const {
    return mRaceDataReader->mXanimePlayer->getCurrentAnimationName();
}
const char* GhostAttackGhost::getCurrentAnimationName() const {
    const char* name = mRaceDataReader->mXanimePlayer->getCurrentAnimationName();
    if (MR::isEqualString(name, "dup-non-group")) name = MR::getPlayingBckName(this);
    return name;
}
// NONMATCHING: scale component load scheduling.
bool GhostAttackGhost::calcHandJoint(TPos3f* mtx, const JointControllerInfo&) {
    TVec3f scale(1.0f);
    f32 z = scale.z, y = scale.y, x = scale.x;
    (*mtx)[0][0] = x; (*mtx)[0][1] = 0.0f; (*mtx)[0][2] = 0.0f;
    (*mtx)[1][0] = 0.0f; (*mtx)[1][1] = y; (*mtx)[1][2] = 0.0f;
    (*mtx)[2][0] = 0.0f; (*mtx)[2][1] = 0.0f; (*mtx)[2][2] = z;
    return true;
}
char sGhostHopperAnimationTag[] = "Hopper";
char sGhostHopperJapaneseAnimationTag[] = "ホッパー";

// NONMATCHING: string relocation grouping differs.
void GhostAttackGhost::calcAndSetBaseMtx() {
    if (mHostSensor != nullptr) {
        TPos3f hostMtx(MR::getSensorHost(mHostSensor)->getBaseMtx());
        MR::setBaseTRMtx(this, hostMtx);
        return;
    }
    const char* name = getCurrentAnimationName();
    bool hopper;
    if (MR::isEqualSubString(name, sGhostHopperAnimationTag)) hopper = true;
    else hopper = MR::isEqualSubString(name, sGhostHopperJapaneseAnimationTag);
    if (hopper) {
        mHandJointCtrl->registerCallBack();
        mHeadJointCtrl->registerCallBack();
    }
    MtxPtr modelMtx = MR::getJ3DModel(this)->mBaseTransformMtx.mMtx;
    TPos3f mtx(&mRaceDataReader->mBaseMtx);
    if (MR::isEqualSubString(getCurrentAnimationName(), "チューブスライダー")) {
        TVec3f front(mMovementDelta);
        TVec3f up(0.0f, 1.0f, 0.0f);
        TVec3f position(0.0f);
        f32 upZ=mtx[2][1], upY=mtx[1][1], upX=mtx[0][1];
        up.set<f32>(upX, upY, upZ);
        mtx.getTrans(position);
        MR::vecKillElement(front, up, &front);
        if (!MR::isNearZero(front, 0.001f) && !MR::isNearZero(up, 0.001f)) {
            MR::normalize(&front);
            MR::normalize(&up);
            if (!MR::isSameDirection(front, up, 0.01f) && !MR::isSameDirection(front, -up, 0.01f)) MR::makeMtxFrontUpPos(&mtx, front, up, position);
        }
    }
    if (MR::isEqualSubString(getCurrentAnimationName(), "Trapeze") && !MR::isEqualStageName("FlowerHighlandGalaxy")) MR::rotateMtxLocalYDegree(mtx, 90.0f);
    MR::blendMtxRotate(getBaseMtx(), mtx, 0.2f, modelMtx);
    MR::setMtxTrans(modelMtx, mPosition.x, mPosition.y, mPosition.z);
    MR::getJ3DModel(this)->mBaseScale = mScale;
}
// NONMATCHING: temporary address register allocation.
void GhostAttackGhost::control() {
    if (mFootprint != nullptr && !MR::isDead(mFootprint)) mFootprint->movement();
    mMovementDelta.set(mPosition - mPreviousPosition);
    mPreviousPosition.set(mPosition);
}
bool GhostAttackGhost::tryAttachToHost(HitSensor* sender, HitSensor* receiver) {
    if (!MR::isEqualSensor(sender, this, "Body")) return false;
    if (!MR::isSensorType(receiver, 0x61)) return false;
    if (!isNerve(GHOST_NERVE(Appear))) return false;
    if (!isNerve(GHOST_NERVE(Run))) return false;
    mHostSensor = receiver;
    TPos3f mtx(MR::getSensorHost(receiver)->getBaseMtx());
    mtx.getTrans(mPosition);
    TVec3f rotation;
    mtx.getEuler(rotation);
    mRotation.set(rotation * 57.29578f);
    setNerve(GHOST_NERVE(Bind));
    return true;
}
bool GhostAttackGhost::tryStartRace(HitSensor*, HitSensor* receiver) {
    if (MR::isStageStateScenarioOpeningCamera()) return false;
    if (MR::isExecScenarioStarter()) return false;
    if (!MR::isSensorPlayerOrRide(receiver)) return false;
    if (!isNerve(GHOST_NERVE(Start))) return false;
    setNerve(GHOST_NERVE(Appear));
    return true;
}
// NONMATCHING: animation controller load and parameter store scheduling.
void GhostAttackGhost::startSleep() {
    mRaceDataReader->startActionName("SleepLie");
    MR::startBtp(this, "Sleep");
    XanimePlayer* player = mRaceDataReader->mXanimePlayer;
    BckCtrlData data;
#ifndef GALAXY_NATIVE
    data.mName = lbl_807CF430;
#endif
    // Native BckCtrlData already retains the constructor's canonical empty name.
    data.mStartFrame = 0;
    data.mLoopMode = 2;
    data.mRepeatFrame = 134;
    data.mEndFrame = 254;
    data.mInterpole = 20;
    BckCtrlFunction::reflectBckCtrlData(data, player);
    MR::invalidateShadow(this, nullptr);
}
void GhostAttackGhost::exeStart() {
    if (MR::isFirstStep(this)) {
        mRaceDataReader->startActionName("GhostStartWait");
        MR::startBtp(this, "Blink");
    }
}
void GhostAttackGhost::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::tryHitReaction(this, "開始");
        setNerve(GHOST_NERVE(Run));
        if (mFootprint != nullptr) mFootprint->appear();
    }
}
void GhostAttackGhost::exeRun() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this, 0);
        mRaceDataReader->receivePacket();
    }
    if (mFootprint != nullptr && !MR::isDead(mFootprint)) mFootprint->updatePrint();
    if (mRaceDataReader->receivePacket()) {
        mFootprint->stop();
        setNerve(GHOST_NERVE(Wait));
    }
}
namespace {
    f32 getGhostSensorRadius() NO_INLINE {
        if ((MR::isEqualStageName("MemoryRoadGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) || (MR::isEqualStageName("MemoryRoadGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 4)) return 200.0f;
        return 800.0f;
    }
    bool isGhostSensorEnabled() NO_INLINE {
        if (MR::isEqualStageName("HomeCenterGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("BigWaterFallGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("VsKoopaLv1Galaxy") && MR::getCurrentOrSelectedScenarioNo() == 1) return false;
        if (MR::isEqualStageName("WanwanFactoryGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("KachikochiLavaGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("MagicGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("VsKoopaGravityGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 3) return false;
        if (MR::isEqualStageName("VsKoopaGravityGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 4) return false;
        if (MR::isEqualStageName("YosshiHomeGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 4) return false;
        if (MR::isEqualStageName("YosshiHomeGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 5) return false;
        if (MR::isEqualStageName("YosshiHomeGalaxy") && MR::getCurrentOrSelectedScenarioNo() == 6) return false;
        return true;
    }
}
void GhostAttackGhost::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        if (isGhostSensorEnabled()) {
            MR::validateHitSensors(this);
            MR::setSensorRadius(this, "Body", getGhostSensorRadius());
        }
    } else {
        if (mObjArg0 == 0) {
            mRaceDataReader->startActionName("GhostStarWaitAir");
            setNerve(GHOST_NERVE(StarWait));
            return;
        }
        mRaceDataReader->startActionName("SleepLie");
        setNerve(GHOST_NERVE(StarWaitAir));
    }
}
void GhostAttackGhost::exeBind() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::zeroVelocity(this);
        mRaceDataReader->startActionName("GhostStarWaitStart");
        mFootprint->stop();
    }
    if (mRaceDataReader->mXanimePlayer->isTerminate()) {
        setNerve(GHOST_NERVE(StarWaitStart));
        return;
    }
    if (MR::isPowerStarGetDemoActive()) kill();
}
void GhostAttackGhost::exeStarWaitStart() {
    if (MR::isFirstStep(this)) mRaceDataReader->startActionName("GhostStarWait");
    updateSensorRadius();
    if (MR::isPowerStarGetDemoActive()) kill();
}
void GhostAttackGhost::updateSensorRadius() {
    if (isGhostSensorEnabled() && getNerveStep() % 5 == 0) {
        f32 radius = getGhostSensorRadius();
        if (MR::getSensorRadius(this, "Body") < radius) {
            MR::setSensorRadius(this, "Body", getGhostSensorRadius());
            return;
        }
        MR::setSensorRadius(this, "Body", 100.0f);
    }
}
void GhostAttackGhost::exeStarWait() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
        mRaceDataReader->startActionName("GhostStarWaitAir");
    }
    updateSensorRadius();
    if (MR::isPowerStarGetDemoActive()) kill();
}
GhostAttackGhost::~GhostAttackGhost() {}
