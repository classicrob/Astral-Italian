#pragma auto_inline off
#include "Game/MapObj/ChimneyEntryObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
namespace NrvChimneyEntryObj {
    class ChimneyEntryObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static ChimneyEntryObjNrvWait sInstance;
    };
    ChimneyEntryObjNrvWait ChimneyEntryObjNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(ChimneyEntryObjNrvWarpStart, ChimneyEntryObj, WarpStart);
    NEW_NERVE(ChimneyEntryObjNrvWarp, ChimneyEntryObj, Warp);
    NEW_NERVE(ChimneyEntryObjNrvWarpEnd, ChimneyEntryObj, WarpEnd);
}
namespace {
    const char* sExitName = "マリオ顔惑星煙突出口";
    TVec3f sExitOffset(0.0f, 0.0f, 0.0f);
}
ChimneyEntryObj::ChimneyEntryObj(const char* name) : LiveActor(name), mBindActor(nullptr), mCameraInfo(nullptr) {
    mEntranceMtx.identity();
    mPlayerMtx.identity();
    mPlayerStartMtx.identity();
    mPlayerTargetMtx.identity();
    mExitMtx.identity();
}
ChimneyEntryObj::~ChimneyEntryObj() {}
void ChimneyEntryObj::init(const JMapInfoIter& iter) {
    MR::calcGravity(this);
    MR::initDefaultPos(this, iter);
    MR::makeMtxTR(mEntranceMtx, mPosition, mRotation);
    mExitMtx.set(mEntranceMtx);
    MR::connectToSceneMapObjMovement(this);
    initSound(4, "ChimneyEntryObj", nullptr, TVec3f(0.0f));
    MR::setGroupClipping(this, iter, 32);
    MR::setClippingTypeSphere(this, 1000.0f);
    initHitSensor(1);
    MR::addHitSensorBinder(this, "binder", 8, 70.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initMultiActorCamera(this, iter, &mCameraInfo, "出現");
    initNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWait::sInstance);
    MR::useStageSwitchAwake(this, iter);
    makeActorAppeared();
    MR::invalidateClipping(this);
}
// NONMATCHING: temporary vector pointer allocation and load scheduling differ.
void ChimneyEntryObj::initAfterPlacement() {
    TVec3f position(0.0f), rotation(0.0f);
    if (MR::tryFindNamePos(sExitName, &position, &rotation)) {
        position = position + sExitOffset;
        MR::makeMtxTR(mExitMtx, position, rotation);
    }
}
void ChimneyEntryObj::control() { MR::startActionSound(this, "SmokeEmitting"); }
void ChimneyEntryObj::exeWarpStart() {
    if (MR::isFirstStep(this)) MR::startBckPlayer("EarthenPipeReady", nullptr);
    MR::blendMtx(mPlayerStartMtx, mPlayerTargetMtx, MR::calcNerveRate(this, (s32)MR::getBckFrameMaxPlayer()), mPlayerMtx);
    if (MR::isBckStoppedPlayer() && !MR::isPlayerDead()) setNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWarp::sInstance);
}
void ChimneyEntryObj::exeWarp() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeIn", nullptr);
        MR::startActionSound(this, "WarpStart");
    }
    if (MR::isBckStoppedPlayer() && !MR::isPlayerDead()) {
        MR::startMultiActorCameraTargetPlayer(this, mCameraInfo, "出現", -1);
        mPlayerMtx.set(mExitMtx);
        setNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWarpEnd::sInstance);
    }
}
void ChimneyEntryObj::exeWarpEnd() {
    if (MR::isFirstStep(this)) {
        MR::endMultiActorCamera(this, mCameraInfo, "出現", true, -1);
        MR::startActionSound(this, "WarpEnd");
        warpPlayer();
        MR::validateHitSensors(this);
        setNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWait::sInstance);
    }
}
bool ChimneyEntryObj::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor*) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (!isNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWait::sInstance)) return false;
        MR::calcGravity(this);
        TVec3f relative(*MR::getPlayerPos());
        relative.sub(mPosition);
        TVec3f tangent(relative);
        TVec3f up(0.0f);
        f32 z = mEntranceMtx.mMtx[2][1];
        f32 y = mEntranceMtx.mMtx[1][1];
        f32 x = mEntranceMtx.mMtx[0][1];
        up.set<f32>(x, y, z);
        MR::vecKillElement(tangent, mGravity, &tangent);
        if (PSVECMag(&tangent) > 50.0f && relative.dot(up) < -5.0f) return false;
        mBindActor = MR::getSensorHost(sender);
        MR::invalidateClipping(this);
        mPlayerStartMtx.set(MR::getPlayerBaseMtx());
        mPlayerMtx.set(mPlayerStartMtx);
        TVec3f front(MR::getCamPos());
        front.sub(mPosition);
        MR::normalize(&front);
        MR::makeMtxUpFrontPos(&mPlayerTargetMtx, up, front, mPosition);
        MR::invalidateHitSensors(this);
        setNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWarpStart::sInstance);
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mPlayerMtx);
        return true;
    }
    if (MR::isMsgRushCancel(msg)) {
        mBindActor = nullptr;
        setNerve(&NrvChimneyEntryObj::ChimneyEntryObjNrvWait::sInstance);
        return true;
    }
    return false;
}
void ChimneyEntryObj::warpPlayer() {
    TVec3f up(mExitMtx.mMtx[0][1], mExitMtx.mMtx[1][1], mExitMtx.mMtx[2][1]);
    TVec3f front(mExitMtx.mMtx[0][2], mExitMtx.mMtx[1][2], mExitMtx.mMtx[2][2]);
    up.mult(1.0f);
    TVec3f velocity(up);
    MR::setPlayerFrontTargetVec(front, 1);
    MR::startBckPlayer("Fall", nullptr);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, velocity);
    mBindActor = nullptr;
}
