#include "Game/Enemy/PukupukuWaterSurface.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/Pukupuku.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"

extern const f32 sWaterProbeTop;
extern const f32 sWaterProbeBottom;
extern const f32 sWaterGravity;

// The eight water-surface states (SMG2 0x807D2FE8..0x807D3004). Their execute bodies are the
// fn_ members of PukupukuWaterSurface, in the same order as the target's wrappers.
namespace NrvPukupukuWaterSurface {
    class NrvMoveRail : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exePukuMoveRail(); }
        static NrvMoveRail sInstance;
    };
    NrvMoveRail NrvMoveRail::sInstance ATTRIBUTE_WEAK;

    class NrvMoveReverse : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exeMoveReverse(); }
        static NrvMoveReverse sInstance;
    };
    NrvMoveReverse NrvMoveReverse::sInstance ATTRIBUTE_WEAK;

    class NrvBlownOff : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exePukuBlownOff(); }
        static NrvBlownOff sInstance;
    };
    NrvBlownOff NrvBlownOff::sInstance ATTRIBUTE_WEAK;

    class NrvTrampled : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exePukuTrampled(); }
        static NrvTrampled sInstance;
    };
    NrvTrampled NrvTrampled::sInstance ATTRIBUTE_WEAK;

    class NrvLanding : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exePukuLanding(); }
        static NrvLanding sInstance;
    };
    NrvLanding NrvLanding::sInstance ATTRIBUTE_WEAK;

    class NrvDead : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< PukupukuWaterSurface* >(pSpine->mExecutor)->exePukupukuDead(); }
        static NrvDead sInstance;
    };
    NrvDead NrvDead::sInstance ATTRIBUTE_WEAK;

    class NrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static NrvEaten sInstance;
    };
    NrvEaten NrvEaten::sInstance ATTRIBUTE_WEAK;

    class NrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            PukupukuWaterSurface* pActor = static_cast< PukupukuWaterSurface* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(pActor, pActor->mBindStarPointer, pActor->mRestoreNerve);
        }
        static NrvBindStarPointer sInstance;
    };
    NrvBindStarPointer NrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvPukupukuWaterSurface

#define WS_NRV(name) (&NrvPukupukuWaterSurface::name::sInstance)

PukupukuWaterSurface::PukupukuWaterSurface(const char* pName) : LiveActor(pName) {
    mWaterInfo = nullptr;
    mFrontVec.set(0.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 0.0f, 0.0f);
    mNerveRate = 0.0f;
    mUnusedB4 = 0.0f;
    mUnusedB8 = 0.0f;
    mNerveBlendRate = 0.0f;
    mLandingCount = 0;
    mYoshiLockOn = nullptr;
    mScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
    mRestoreNerve = nullptr;
}

PukupukuWaterSurface::~PukupukuWaterSurface() {}

void PukupukuWaterSurface::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Pukupuku", "WaterSurface", false);
    bool isNoShadow = false;
    MR::getJMapInfoArg0NoInit(rIter, &isNoShadow);
    if (!isNoShadow) {
        MR::invalidateShadowAll(this);
    }
    initNerve(WS_NRV(NrvMoveRail), 1);
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::calcFrontVec(&mFrontVec, this);
    MR::calcUpVec(&mUpVec, this);
    mWaterInfo = new WaterInfo();
    mScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
    MR::declareStarPiece(this, 3);
    MR::declareCoin(this, 1);
    mPartsModel = new PartsModel(this, "氷モデル", "PukupukuFreezeIce", getBaseMtx(), 0x13, false);
    mPartsModel->initWithoutIter();
#ifdef GALAXY_NATIVE
    mPartsModel->mCheckHiddenHost = true;
#else
    reinterpret_cast< u8* >(mPartsModel)[0x9D] = 1;
#endif
    mPartsModel->makeActorDead();
    MR::startAction(mPartsModel, "Normal");
    makeActorAppeared();
}

void PukupukuWaterSurface::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
}

void PukupukuWaterSurface::calcAndSetBaseMtx() {
    bool isMoveNerve = isNerve(WS_NRV(NrvEaten)) || isNerve(WS_NRV(NrvBlownOff)) || isNerve(WS_NRV(NrvTrampled)) || isNerve(WS_NRV(NrvDead));
    if (!isMoveNerve) {
        TPos3f sp8;
        sp8.identity();
        calcSurfaceMtx(&sp8, this, mUpVec, mFrontVec);
        MR::setBaseTRMtx(this, sp8);
        MR::updateBaseScale(this, mScaleCtrl);
        return;
    }
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleCtrl);
}

void PukupukuWaterSurface::control() {
    mScaleCtrl->updateNerve();
    if (canBeEaten() != 0) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    bool isRailNerve = isNerve(WS_NRV(NrvMoveRail)) || isNerve(WS_NRV(NrvMoveReverse));
    if (isRailNerve && mBindStarPointer->tryStartPointBind()) {
        if (isNerve(WS_NRV(NrvMoveRail))) {
            mRestoreNerve = WS_NRV(NrvMoveRail);
        } else {
            mRestoreNerve = WS_NRV(NrvMoveReverse);
        }
        setNerve(WS_NRV(NrvBindStarPointer));
    }
}

void PukupukuWaterSurface::exePukuMoveRail() {
    if (MR::isFirstStep(this)) {
        mNerveBlendRate = 0.0f;
        if (!MR::isAnyAnimPlaying(this, "Swim")) {
            const char* eye = "OpenEye";
            MR::startAction(this, "Swim");
            MR::startBtp(this, eye);
        }
        MR::emitEffect(this, "Ripple");
    }
    f32 rate = MR::calcNerveRate(this, 10);
    mNerveBlendRate = MR::getLinerValue(rate, 0.5f, 5.0f, 1.0f);
    updateRailMove(mNerveBlendRate, 5.0f);
    if (MR::isEqualStringCase(MR::getPlayingBckName(this), "BodyAttack") && MR::isActionEnd(this)) {
        const char* eye = "OpenEye";
        MR::startAction(this, "Swim");
        MR::startBtp(this, eye);
    }
    if (probeSurfaceBelow() != 0) {
        setNerve(WS_NRV(NrvLanding));
        return;
    }
    if (!MR::isLoopRail(this) && MR::isRailReachedGoal(this)) {
        setNerve(WS_NRV(NrvMoveReverse));
        MR::reverseRailDirection(this);
    }
}

void PukupukuWaterSurface::exeMoveReverse() {
    updateRailMove(0.5f, 5.0f);
    if (probeSurfaceBelow() != 0) {
        setNerve(WS_NRV(NrvLanding));
        return;
    }
    TVec3f dir;
    MR::calcRailDirection(&dir, this);
    if (!MR::isRailGoingToEnd(this)) {
        dir = -dir;
    }
    if (mFrontVec.dot(dir) > 0.9f) {
        setNerve(WS_NRV(NrvMoveRail));
    }
}

void PukupukuWaterSurface::exePukuBlownOff() {
    if (MR::isFirstStep(this)) {
        const char* eye = "CloseEye";
        MR::startAction(this, "Rotate");
        MR::startBtp(this, eye);
        MR::emitEffect(this, "SpinDamageWater");
        MR::startBlowHitSound(this);
    } else if (!MR::isBinded(this)) {
        TVec3f grav(mGravity);
        grav.scale(sWaterGravity * getWaterScale());
        mVelocity.add(grav);
    }
    MR::stopSceneAtStep(this, 2, 4);
    if (MR::isStep(this, 15)) {
        MR::validateClipping(this);
        MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
        kill();
    }
}

void PukupukuWaterSurface::exePukuTrampled() {
    if (MR::isFirstStep(this)) {
        TVec3f vel(mGravity);
        vel.scale(10.0f);
        mVelocity.set(vel);
        const char* eye = "CloseEye";
        MR::startAction(this, "Flat");
        MR::startBtp(this, eye);
        MR::startActionSound(this, "Stomped", -1, -1, -1);
    } else if (!MR::isBinded(this)) {
        mVelocity.scale(0.89f);
    }
    if (MR::isStep(this, 30)) {
        TVec3f pos(mPosition - mGravity * 200.0f);
        MR::startActionSound(this, "Vanish", -1, -1, -1);
        MR::appearCoinPop(this, pos, 1);
        kill();
    }
}

void PukupukuWaterSurface::exePukuLanding() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Freeze");
        MR::zeroVelocity(this);
        MR::invalidateClipping(this);
        MR::emitEffect(this, "IceStart");
        MR::emitEffect(this, "Ice");
        MR::tryDeleteEffect(this, "Ripple");
        mLandingCount = 0;
        mPartsModel->appear();
        MR::startBva(mPartsModel, "Normal");
    }
    updatePukuWaterInfo();
    if (probeSurfaceBelow() == 0) {
        mLandingCount++;
        if (mLandingCount > 3) {
            setNerve(WS_NRV(NrvMoveRail));
            MR::startAction(this, "Swim");
            MR::tryDeleteEffect(this, "Ice");
            MR::emitEffect(this, "IceEnd");
            MR::zeroVelocity(this);
            mPartsModel->kill();
        }
    } else {
        mLandingCount = 0;
    }
}

void PukupukuWaterSurface::exePukupukuDead() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(3);
        MR::tryRumblePadMiddle(this, 0);
        MR::zeroVelocity(this);
        MR::startAction(mPartsModel, "Break");
        MR::startBva(mPartsModel, "Break");
        TVec3f pos(mPosition - mGravity * 100.0f);
        MR::appearStarPiece(this, pos, 3, 10.0f, 40.0f, false);
        MR::hideModelIfShown(this);
    }
    if (MR::isActionEnd(mPartsModel)) {
        MR::startBva(mPartsModel, "Normal");
#ifdef GALAXY_NATIVE
        mPartsModel->mCheckHiddenHost = false;
#else
        reinterpret_cast< u8* >(mPartsModel)[0x9D] = 0;
#endif
        mPartsModel->kill();
        kill();
    }
}

void PukupukuWaterSurface::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(WS_NRV(NrvLanding))) {
        return;
    }
    {
        if (MR::isSensorPlayer(pReceiver)) {
            bool isMoveNerve = isNerve(WS_NRV(NrvEaten)) || isNerve(WS_NRV(NrvBlownOff)) || isNerve(WS_NRV(NrvTrampled)) || isNerve(WS_NRV(NrvDead));
            if (!isMoveNerve) {
                if (isNerve(WS_NRV(NrvBindStarPointer))) {
                    MR::sendMsgPush(pReceiver, pSender);
                    return;
                }
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                    bool isRailNerve = isNerve(WS_NRV(NrvMoveRail)) || isNerve(WS_NRV(NrvMoveReverse));
                    if (isRailNerve) {
                        const char* eye = "CloseEye";
                        MR::startAction(this, "BodyAttack");
                        MR::startBtp(this, eye);
                    }
                } else {
                    MR::sendMsgPush(pReceiver, pSender);
                }
                return;
            }
        }
    }
    if (isNerve(WS_NRV(NrvBindStarPointer))) {
        if (MR::isSensorType(pReceiver, 0x27) && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            setBlowVelocity(pReceiver, pSender);
            setNerve(WS_NRV(NrvBlownOff));
            return;
        }
    }
    if (MR::isSensorEnemy(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }
    if (MR::isSensorYoshi(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool PukupukuWaterSurface::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        if (!isNerve(WS_NRV(NrvLanding))) {
            mScaleCtrl->startHitReaction();
        }
        return true;
    }
    bool isMoveNerve = isNerve(WS_NRV(NrvEaten)) || isNerve(WS_NRV(NrvBlownOff)) || isNerve(WS_NRV(NrvTrampled)) || isNerve(WS_NRV(NrvDead));
    if (!isMoveNerve) {
    if (isNerve(WS_NRV(NrvLanding))) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerHitAll(msg)) {
            setNerve(WS_NRV(NrvDead));
            return true;
        }
        return false;
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        setNerve(WS_NRV(NrvTrampled));
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg) || MR::isMsgYoshiProjectile(msg)) {
        setBlowVelocity(pSender, pReceiver);
        setNerve(WS_NRV(NrvBlownOff));
        return true;
    }
    }
    return false;
}

bool PukupukuWaterSurface::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isMoveNerve = isNerve(WS_NRV(NrvEaten)) || isNerve(WS_NRV(NrvBlownOff)) || isNerve(WS_NRV(NrvTrampled)) || isNerve(WS_NRV(NrvDead));
    if (!isMoveNerve && MR::isSensorType(pSender, 0x27)) {
        setBlowVelocity(pSender, pReceiver);
        setNerve(WS_NRV(NrvBlownOff));
        return true;
    }
    return false;
}

bool PukupukuWaterSurface::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(WS_NRV(NrvLanding)) && MR::isMsgPlayerKick(msg)) {
        setNerve(WS_NRV(NrvDead));
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (canBeEaten() == 0) {
            return false;
        }
        MR::stopBck(this);
        setNerve(WS_NRV(NrvEaten));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        MR::zeroVelocity(this);
        makeActorDead();
        return true;
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        MR::zeroVelocity(this);
        makeActorDead();
        return true;
    }
    return false;
}

s32 PukupukuWaterSurface::canBeEaten() {
    if (isNerve(WS_NRV(NrvEaten))) {
        bool isMoveNerve = isNerve(WS_NRV(NrvEaten)) || isNerve(WS_NRV(NrvBlownOff)) || isNerve(WS_NRV(NrvTrampled)) || isNerve(WS_NRV(NrvDead));
        if (isMoveNerve) {
            return 0;
        }
    }
    return 1;
}

s32 PukupukuWaterSurface::probeSurfaceBelow() {
    if (MR::isBindedGroundIce(this)) {
        return 1;
    }
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f pos(mPosition + up * sWaterProbeTop);
    TVec3f direction(up);
    direction.scale(-(sWaterProbeTop - sWaterProbeBottom));
    return MR::isFirstPolyOnLineToMapIce(&pos, &direction);
}

#pragma dont_inline on
void PukupukuWaterSurface::setBlowVelocity(HitSensor* pSender, HitSensor* pReceiver) {
    f32 scale = getWaterScale();
    MR::setVelocityBlowAttack(this, pSender, pReceiver, 35.0f * scale, 20.0f * scale, 4);
}

#pragma dont_inline off

void PukupukuWaterSurface::updatePukuWaterInfo() {
    if (MR::getWaterAreaObj(mWaterInfo, mPosition)) {
        MR::getWaterAreaInfo(mWaterInfo, mPosition, mGravity, false);
    }
}

void PukupukuWaterSurface::updateUpVecFromWater() {
    TVec3f to(0.0f);
    if (mWaterInfo->isInWater()) {
        to = mWaterInfo->mSurfaceNormal;
    } else {
        to = -mGravity;
    }
    MR::vecBlendSphere(&mUpVec, mUpVec, to, 0.3f);
}

// NONMATCHING: vector locals occupy reversed stack slots; calls and field offsets match.
void PukupukuWaterSurface::snapToWaterSurface() {
    if (mWaterInfo->isInWater()) {
        TVec3f sp8(mUpVec);
        sp8.scale(-60.0f);
        TVec3f sp14(mWaterInfo->mSurfacePos);
        sp14.add(sp8);
        mPosition = sp14;
    }
}

void PukupukuWaterSurface::calcSurfaceMtx(TPos3f* pMtx, LiveActor* pActor, const TVec3f& up, const TVec3f& front) {
    TPos3f base;
    base.set(pActor->getBaseMtx());
    TVec3f upDir;
    TVec3f frontDir;
    TVec3f cross;
    TVec3f side;
    if (!MR::isNearZero(up, 0.001f)) {
        upDir = up;
    } else {
        base.getYDir(upDir);
    }
    if (!MR::isNearZero(front, 0.001f)) {
        frontDir = front;
    } else {
        base.getZDir(frontDir);
    }
    PSVECCrossProduct(&frontDir, &upDir, &cross);
    if (MR::isNearZero(cross, 0.001f)) {
        base.getXDir(side);
        PSVECCrossProduct(&side, &upDir, &frontDir);
    }
    MR::makeMtxUpFrontPos(pMtx, upDir, frontDir, pActor->mPosition);
}

void PukupukuWaterSurface::updateRailMove(f32 speed, f32 turn) {
    if (MR::isExistRail(this)) {
        MR::moveCoordAndFollowTrans(this, speed);
        TVec3f dir;
        MR::calcRailDirection(&dir, this);
        if (!MR::isRailGoingToEnd(this)) {
            dir = -dir;
        }
        MR::turnDirection(this, &mFrontVec, dir, MR::cosDegree(turn));
    }
    updatePukuWaterInfo();
    updateUpVecFromWater();
    snapToWaterSurface();
}

f32 PukupukuWaterSurface::getWaterScale() {
    TVec3f offset;
    offset.x = 0.0f;
    offset.y = 0.0f;
    offset.z = 0.0f;
    if (MR::isInWater(this, offset)) {
        return 0.6f;
    }
    return 1.0f;
}

const f32 sWaterProbeTop = 200.0f;
const f32 sWaterProbeBottom = -50.0f;
const f32 sWaterGravity = 1.0f;
