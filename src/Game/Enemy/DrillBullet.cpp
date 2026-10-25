#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/DrillBullet.hpp"
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace {
    char sDrillBulletModelName[] = "Horino";
    char sDrillBulletHillName[] = "ホリーノ塚の道";
    char sDrillBulletBodySensor[] = "Body";
    char sDrillBulletAttackSensor[] = "Attack";
    char sDrillBulletScaleAction[] = "Scale";
    char sDrillBulletFreezeState[] = "SupportFreeze";
    char sDrillBulletGroundAction[] = "Ground";
    char sDrillBulletDigInAction[] = "DigWallIn";
    char sDrillBulletDigOutSignAction[] = "DigWallOutSign";
    char sDrillBulletDigOutAction[] = "DigWallOut";
    char sDrillBulletStepDownAction[] = "StepDown";
    char sDrillBulletSpinAction[] = "SpinReaction";
    char sDrillBulletAttackSuccessAction[] = "AttackSuccess";
    char sDrillBulletExplosionEffect[] = "Explosion";
}

namespace NrvDrillBullet {
    NEW_NERVE(DrillBulletNrvMove, DrillBullet, Move);
    NEW_NERVE(DrillBulletNrvDigWallIn, DrillBullet, DigWallIn);
    NEW_NERVE(DrillBulletNrvDigWallOut, DrillBullet, DigWallOut);
    NEW_NERVE(DrillBulletNrvStepDown, DrillBullet, StepDown);
    NEW_NERVE(DrillBulletNrvSpinReaction, DrillBullet, Reaction);
    NEW_NERVE(DrillBulletNrvAttackSuccess, DrillBullet, Reaction);

    // Star-pointer bind: updates the state, then goes back to Move unless it can dig again.
    class DrillBulletNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            DrillBullet* pActor = reinterpret_cast<DrillBullet*>(pSpine->mExecutor);
            if (MR::updateActorState(pActor) && !pActor->tryDigWall()) {
                pActor->setNerve(&DrillBulletNrvMove::sInstance);
            }
        }
        static DrillBulletNrvBindStarPointer sInstance;
    };
    DrillBulletNrvBindStarPointer DrillBulletNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(DrillBulletNrvEaten, DrillBullet, Eaten);
};  // namespace NrvDrillBullet

#define GET_DRILL_BULLET_NERVE(name) (&NrvDrillBullet::name::sInstance)



DrillBullet::DrillBullet(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mYoshiLockOn = nullptr;
    mPartsModel = nullptr;
    mRailClip.set(0.0f, 0.0f, 0.0f);
    mIsNoGravity = -1;
}

DrillBullet::~DrillBullet() {
}

void DrillBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, sDrillBulletModelName, false);
    MR::addEffectHitNormal(this, nullptr);
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::initAndSetRailClipping(&mRailClip, this, 100.0f, 500.0f);

    mPartsModel = new HorinoHill(this, 7, sDrillBulletHillName, 150.0f);
    mPartsModel->initWithoutIter();
    mScale.x = 1.2f;
    mScale.y = 1.2f;
    mScale.z = 1.2f;
    MR::setSensorRadius(this, sDrillBulletBodySensor, 1.2f * MR::getSensorRadius(this, sDrillBulletBodySensor));
    MR::setSensorRadius(this, sDrillBulletAttackSensor, 1.2f * MR::getSensorRadius(this, sDrillBulletAttackSensor));
    MR::startBtkAndSetFrameAndStop(this, sDrillBulletScaleAction, 1.0f);
    MR::invalidateShadowAll(this);

    MR::getJMapInfoArg0NoInit(rIter, &mIsNoGravity);
    if (mIsNoGravity == 0) {
        MR::offCalcGravity(this);
    }

    initNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_DRILL_BULLET_NERVE(DrillBulletNrvBindStarPointer), sDrillBulletFreezeState);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f, 75.0f, 0.0f));

    if (MR::useStageSwitchAwake(this, rIter)) {
        makeActorDead();
        return;
    }
    makeActorAppeared();
}

void DrillBullet::control() {
    mAnimScaleCtrl->updateNerve();
    if (isTargetable()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    tryBindStarPointer();
}

// SMG2 base matrix: rail-aligned when the rail is used; otherwise gravity/Z-axis aligned.
void DrillBullet::calcAndSetBaseMtx() {
    if (mIsNoGravity == 0) {
        TPos3f mtx;
        mtx.identity();
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::calcUpVec(&up, this);
        TVec3f front = MR::getRailDirection(this);
        if (MR::isSameDirection(front, up, 0.01f) == false) {
            MR::makeMtxFrontUpPos(&mtx, front, up, mPosition);
        } else {
            MR::makeMtxFrontNoSupportPos(&mtx, front, mPosition);
        }
        MR::setBaseTRMtx(this, mtx);
    } else {
        MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, MR::getRailDirection(this));
    }
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void DrillBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvStepDown)) &&
        !isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvEaten)) && MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorEnemy(pReceiver)) {
            if (MR::sendMsgToEnemyAttackBlow(pReceiver, pSender)) {
                MR::emitItemHit(this);
                kill();
                return;
            }
        }

        if (MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
            if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvBindStarPointer))) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender)) {
                MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
                setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvAttackSuccess));
            }
        }
    }
}

bool DrillBullet::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvStepDown))) {
        return false;
    }
    if (!MR::isSensorPlayerOrRide(pSender)) {
        return false;
    }
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (MR::isFaceToPlayerHorizontalDegree(this, MR::getRailDirection(this), 25.0f)) {
            return false;
        }
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvSpinReaction));
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvStepDown));
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerHitProjectiles(msg)) {
        MR::emitItemHit(this);
        kill();
        return true;
    }

    return false;
}

bool DrillBullet::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvStepDown))) {
        return false;
    }
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (MR::isMsgToEnemyAttackBlow(msg)) {
        MR::emitItemHit(this);
        kill();
        return true;
    }
    return false;
}

bool DrillBullet::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        return tryEaten();
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        makeActorDead();
        return true;
    }
    return false;
}

void DrillBullet::makeActorDead() {
    if (!MR::isDead(mPartsModel)) {
        mPartsModel->kill();
    }
    LiveActor::makeActorDead();
}

void DrillBullet::kill() {
    MR::tryRumbleDefaultHit(this, 0);
    MR::emitEffect(this, sDrillBulletExplosionEffect);
    MR::startActionSound(this, sDrillBulletExplosionEffect, -1, -1, -1);
    LiveActor::kill();
}

// Nerve exes ------------------------------------------------------------------------------------

void DrillBullet::exeMove() {
    if (MR::isFirstStep(this)) {
        mPartsModel->appear();
        MR::emitEffect(this, sDrillBulletGroundAction);
        MR::startAction(this, sDrillBulletModelName);
    }
    MR::moveCoordAndFollowTrans(this, 8.0f);
    tryDigWall();
}

void DrillBullet::exeDigWallIn() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, sDrillBulletDigInAction);
    }
    MR::moveCoordAndFollowTrans(this, 1.6f);
    if (!MR::isEffectValid(this, sDrillBulletDigInAction)) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove));
    }
}

void DrillBullet::exeDigWallOut() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, sDrillBulletDigOutSignAction);
    }
    MR::moveCoordAndFollowTrans(this, 1.6f);
    if (!MR::isEffectValid(this, sDrillBulletDigOutSignAction)) {
        MR::emitEffect(this, sDrillBulletDigOutAction);
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove));
    }
}

void DrillBullet::exeStepDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, sDrillBulletStepDownAction);
    }
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void DrillBullet::exeReaction() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvSpinReaction))) {
            MR::startAction(this, sDrillBulletSpinAction);
        } else {
            MR::startAction(this, sDrillBulletAttackSuccessAction);
        }
    }
    MR::moveCoordAndFollowTrans(this, 8.0f);
    if (!tryDigWall() && MR::isActionEnd(this)) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove));
    }
}

void DrillBullet::exeEaten() {
    if (MR::isFirstStep(this) && !MR::isDead(mPartsModel)) {
        mPartsModel->kill();
    }
}

// Helpers ---------------------------------------------------------------------------------------

bool DrillBullet::tryDigWall() {
    s32 nextPoint = MR::getNextRailPointNo(this);
    if (!MR::isRailReachedNearNextPoint(this, 130.0f)) {
        return false;
    }

    s32 digType = -1;
    MR::getRailPointArg0NoInit(this, nextPoint, &digType);
    if (digType == 0) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvDigWallIn));
        return true;
    }
    if (digType == 1) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvDigWallOut));
        return true;
    }
    return false;
}

bool DrillBullet::tryBindStarPointer() {
    if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove)) || isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvSpinReaction))) {
        if (mStarPointBind->tryStartPointBind()) {
            setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvBindStarPointer));
            return true;
        }
    }
    return false;
}

bool DrillBullet::tryEaten() {
    if (isTargetable()) {
        setNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvEaten));
        return true;
    }
    return false;
}

bool DrillBullet::isTargetable() const {
    if (isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvMove)) || isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvSpinReaction)) ||
        isNerve(GET_DRILL_BULLET_NERVE(DrillBulletNrvBindStarPointer))) {
        return true;
    }
    return false;
}
