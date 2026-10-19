#pragma auto_inline off
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/Menbo.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#define BATCH_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Menbo*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvMenbo {
    NEW_NERVE(MenboNrvWait, Menbo, Wait);
    NEW_NERVE(MenboNrvMove, Menbo, Move);
    NEW_NERVE(MenboNrvTurn, Menbo, Turn);

    BATCH_NERVE(MenboNrvTurnBack, exeMenboTurnBack);

    NEW_NERVE(MenboNrvFind, Menbo, Find);
    NEW_NERVE(MenboNrvAttack, Menbo, Attack);

    BATCH_NERVE(MenboNrvAttackEnd, exeMenboAttackEnd);

    NEW_NERVE(MenboNrvSwoonStart, Menbo, SwoonStart);
    NEW_NERVE(MenboNrvSwoon, Menbo, Swoon);
    NEW_NERVE(MenboNrvSwoonEnd, Menbo, SwoonEnd);
    NEW_NERVE(MenboNrvFreeze, Menbo, Freeze);
    NEW_NERVE(MenboNrvPressDown, Menbo, PressDown);
    NEW_NERVE(MenboNrvBlowDown, Menbo, BlownDown);
    NEW_NERVE(MenboNrvBreak, Menbo, Break);

    class MenboNrvEaten : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static MenboNrvEaten sInstance;
    };
    MenboNrvEaten MenboNrvEaten::sInstance ATTRIBUTE_WEAK;

    class MenboNrvFreezeBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MenboNrvFreezeBind sInstance;
    };
    MenboNrvFreezeBind MenboNrvFreezeBind::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMenbo

#define GET_MENBO_NERVE(name) (&NrvMenbo::name::sInstance)

extern const volatile f32 gMenboWaterProbeHeight;

namespace {
extern const f32 cDamp;
extern const f32 cTickBoost;
extern const f32 cTurnDamp;
extern const f32 cFriction;
extern const f32 cProbeAbove;
extern const f32 cProbeBelow;
struct MenboSightParam { f32 distance, horizontal, vertical; };
static const MenboSightParam cSight = {1000.0f, 160.0f, 30.0f};
}

Menbo::Menbo(const char* pName) : LiveActor(pName), mAnimScaleCtrl(nullptr), mStarPointBind(nullptr), mPartsModel(nullptr) {
    mSpawnPos.set(0.0f, 0.0f, 0.0f);
    mSpawnRot.set(0.0f, 0.0f, 0.0f);
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 0.0f, 0.0f);
    mCooldown = 0;
    mTurnDir = 0.0f;
    mTurnRate = 0.0f;
    mTurnTimer = 0.0f;
    mFreezeStep = 0;
    mRetryCount = 0;
    mGroundDist = 1000.0f;
    mIsInWater = false;
    mIsPlayerWet = false;
    mIsTurning = false;
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
}

void Menbo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Menbo", false);
    mSpawnPos = mPosition;
    mSpawnRot = mRotation;
    mBasePos = mPosition;
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    initNerve(GET_MENBO_NERVE(MenboNrvWait), 1);
    MR::getJMapInfoArg7NoInit(rIter, &mGroundDist);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_MENBO_NERVE(MenboNrvFreezeBind), "SupportFreeze");
    mPartsModel = new PartsModel(this, "氷モデル", "MenboFreezeIce", getBaseMtx(), 0x13, 0);
    mPartsModel->initWithoutIter();
    mPartsModel->mCheckHiddenHost = true;
    MR::onEmitEffectSyncClipping(this, "MenboRippleFrontL");
    MR::onEmitEffectSyncClipping(this, "MenboRippleFrontR");
    MR::onEmitEffectSyncClipping(this, "MenboRippleBackL");
    MR::onEmitEffectSyncClipping(this, "MenboRippleBackR");
    MR::tryRegisterDemoCast(this, "星の入り江惑星氷化", rIter);
    makeActorAppeared();
}

#pragma dont_inline on
// NONMATCHING: collision-parts pointer and actor register allocation differ; typed probe and keeper calls agree.
void Menbo::initAfterPlacement() {
    f32 groundDist = mGroundDist;
    TVec3f pos(mPosition);
    TVec3f ray(mGravity);
    Triangle triangle;
    TVec3f offset(ray);
    offset *= 150.0f;
    pos.sub(offset);
    ray *= groundDist;
    CollisionParts* parts;
    if (MR::getFirstPolyOnLineToMoveLimit(static_cast<TVec3f*>(nullptr), &triangle, pos, ray)) {
        parts = triangle.mParts;
    } else if (MR::getFirstPolyOnLineToMap(nullptr, &triangle, pos, ray)) {
        CollisionParts* same = nullptr;
        CollisionParts* hitParts = triangle.mParts;
        MR::getCollisionDirector()->mCategoryKeeper[3]->searchSameHostParts(&same, hitParts);
        parts = same;
    } else parts = nullptr;
    if (parts) {
        Binder* binder = mBinder;
        binder->mExCollisionParts = parts;
        if (parts == nullptr) binder->_1EC._2 = false;
        else binder->_1EC._2 = true;
    }
}
#pragma dont_inline off

void Menbo::makeActorAppeared() {
    setNerve(GET_MENBO_NERVE(MenboNrvMove));
    MR::setPosition(this, mSpawnPos);
    MR::setRotation(this, mSpawnRot);
    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx.toMtxPtr(), TVec3f(0.0f), mSpawnRot);
    mtx.getYDir(mUp);
    mtx.getZDir(mFront);
    mCooldown = 0;
    LiveActor::makeActorAppeared();
    MR::startBrk(this, "Normal");
    mPartsModel->kill();
    MR::showModelIfHidden(this);
    mIsTurning = false;
    mIsInWater = false;
    mIsPlayerWet = false;
}

void Menbo::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    LiveActor::kill();
}

void Menbo::control() {
    mAnimScaleCtrl->updateNerve();
    if (isMenboActive()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (canMenboBindStarPointer() && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreezeBind));
    }
}

void Menbo::startClipped() {
    MR::deleteEffectAll(this);
    LiveActor::startClipped();
}

void Menbo::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
        mIsTurning = false;
    }

    if (MR::isGreaterStep(this, 70)) {
        if (isPlayerInSightWet()) {
            setNerve(GET_MENBO_NERVE(MenboNrvFind));
        } else {
            setNerve(GET_MENBO_NERVE(MenboNrvMove));
        }
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }
    MR::attenuateVelocity(this, 0.97f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
}

void Menbo::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
        MR::validateClipping(this);
        mCooldown = 2;
    }

    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
        setNerve(GET_MENBO_NERVE(MenboNrvTurn));
        return;
    }

    if (MR::isGreaterEqualStep(this, static_cast<s32>(MR::getBckFrameMax(this))) || MR::isGreaterStep(this, 600)) {
        if (isMenboFarFromHome() && isMenboFacingAwayFromHome(50.0f)) {
            mRetryCount = 0;
            setNerve(GET_MENBO_NERVE(MenboNrvTurnBack));
            return;
        }

        if (mIsTurning) {
            setNerve(GET_MENBO_NERVE(MenboNrvWait));
            return;
        }

        setNerve(GET_MENBO_NERVE(MenboNrvTurn));
        return;
    }

    if (isPlayerInSightWet()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFind));
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    MR::attenuateVelocity(this, 0.985f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    accelerateMenboForward(7.0f);
}

void Menbo::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Turn");
        mTurnTimer = 0.0f;
        mTurnRate = 0.0f;
        mTurnDir = MR::getSignHalfProbability();
        mIsTurning = true;
    }

    if (MR::isGreaterEqualStep(this, static_cast<s32>(MR::getBckFrameMax(this))) || MR::isGreaterStep(this, 110)) {
        setNerve(GET_MENBO_NERVE(MenboNrvMove));
    }

    if (!isMenboFarFromHome() && isPlayerInSightWet()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFind));
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    MR::attenuateVelocity(this, 0.99f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    updateMenboTurn();
}

void Menbo::exeMenboTurnBack() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Turn");
        mTurnTimer = 0.0f;
        mTurnRate = 0.0f;
        mIsTurning = true;
        if (mTurnDir == 0.0f) {
            mTurnDir = 1.0f;
        }
    }

    if (MR::isGreaterStep(this, 110)) {
        if (isMenboFacingAwayFromHome(30.0f)) {
            if (mRetryCount < 3) {
                setNerve(GET_MENBO_NERVE(MenboNrvTurnBack));
                mRetryCount++;
            } else {
                setNerve(GET_MENBO_NERVE(MenboNrvAttackEnd));
            }
        } else {
            setNerve(GET_MENBO_NERVE(MenboNrvMove));
        }
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    MR::attenuateVelocity(this, cTurnDamp * cFriction);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    updateMenboTurn();
}

void Menbo::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Find");
        MR::invalidateClipping(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_MENBO_NERVE(MenboNrvAttack));
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    turnMenboTowardWetPlayer();
}

void Menbo::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    if (MR::isGreaterEqualStep(this, static_cast<s32>(MR::getBckFrameMax(this))) || MR::isGreaterStep(this, 600)) {
        setNerve(GET_MENBO_NERVE(MenboNrvAttackEnd));
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    MR::attenuateVelocity(this, 0.985f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();

    if (MR::isFirstStep(this)) {
        s32 frameLimit = 600;
        f32 limit = frameLimit;
        if (MR::getBckFrameMax(this) < limit) MR::getBckFrameMax(this);
        TVec3f front(mFront);
        JMAVECScaleAdd(&mUp, &front, &front, -mUp.dot(front));
        MR::normalize(&front);
        mVelocity.scale(front.dot(mVelocity), front);
        MR::addVelocityMoveToDirection3D(this, front, 27.0f);
    }
}

void Menbo::exeMenboAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
        mIsTurning = false;
        mBasePos = mPosition;
    }

    MR::attenuateVelocity(this, 0.98f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();

    if (MR::isGreaterStep(this, 70)) {
        setNerve(GET_MENBO_NERVE(MenboNrvTurn));
        return;
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }
}

void Menbo::exeSwoonStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonStart");
        MR::invalidateClipping(this);
    }

    MR::attenuateVelocity(this, cDamp * cDamp);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();

    if (MR::isActionEnd(this)) {
        setNerve(GET_MENBO_NERVE(MenboNrvSwoon));
        return;
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }
}

void Menbo::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
    }

    MR::attenuateVelocity(this, cDamp * cDamp);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();

    if (MR::isGreaterStep(this, 150)) {
        setNerve(GET_MENBO_NERVE(MenboNrvSwoonEnd));
        return;
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }
}

void Menbo::exeSwoonEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonEnd");
    }

    MR::attenuateVelocity(this, cDamp * cDamp);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();

    if (MR::isActionEnd(this)) {
        setNerve(GET_MENBO_NERVE(MenboNrvWait));
        return;
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }
}

void Menbo::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Freeze");
        MR::startBrk(this, "Ice");
        MR::zeroVelocity(this);
        mPartsModel->appear();
        MR::startBva(mPartsModel, "Normal");
        MR::validateClipping(this);
        mFreezeStep = 0;
    }

    MR::zeroVelocity(this);
    updateWaterInfo();

    if (!isGroundIceEdge()) {
        mFreezeStep++;
        if (mFreezeStep > 3) {
            setNerve(GET_MENBO_NERVE(MenboNrvWait));
            MR::startBrk(this, "Normal");
            MR::tryDeleteEffect(this, "Ice");
            MR::emitEffect(this, "IceEnd");
            MR::zeroVelocity(this);
            mPartsModel->kill();
        }
    } else {
        mFreezeStep = 0;
    }
}

void Menbo::exePressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PressDown");
        return;
    }

    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        MR::tryHitReactionDead(this);
        kill();
    }
}

#pragma dont_inline on
// NONMATCHING: negative-velocity and item-position temporaries occupy different stack slots.
void Menbo::exeBlownDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
    }

    MR::turnDirectionDegree(this, &mFront, -mVelocity, 30.0f);
    if ((MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) || MR::isGreaterStep(this, 35)) {
        TVec3f side(mUp);
        side *= 100.0f;
        TVec3f pos = mPosition + side;
        MR::emitItemHitAtPos(this, &pos);
        MR::tryHitReactionDead(this);
        kill();
        return;
    }

    updateMenboBlownVelocity();
}
#pragma dont_inline off

#pragma dont_inline on
// NONMATCHING: item-position vector temporaries occupy different stack slots.
void Menbo::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(mPartsModel, "Break");
        TVec3f side(mUp);
        side *= 100.0f;
        TVec3f pos = mPosition + side;
        MR::emitItemHitAtPos(this, &pos);
        MR::hideModelIfShown(this);
    }

    if (MR::isActionEnd(mPartsModel)) {
        MR::startBva(mPartsModel, "Normal");
        mPartsModel->mCheckHiddenHost = false;
        mPartsModel->kill();
        kill();
    }
}
#pragma dont_inline off

void Menbo::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    makeMenboSurfaceMatrix(&mtx, this, mUp, mFront);
    MR::setBaseTRMtx(this, mtx);
    MR::calcFrontVec(&mFront, this);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// NONMATCHING: compiler folds one redundant branch to the shared epilogue.
void Menbo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
            if (isNerve(GET_MENBO_NERVE(MenboNrvFreezeBind)) || isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart)) ||
            isNerve(GET_MENBO_NERVE(MenboNrvSwoon)) || isNerve(GET_MENBO_NERVE(MenboNrvSwoonEnd))) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            bool blocked = false;
            if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
            isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) blocked = true;
            if (!blocked) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                    return;
                }
                return;
            }
        }
        return;
    } else if (MR::isSensorEnemy(pSender) &&
    ((!MR::isSensorEnemy(pReceiver) && !MR::isSensorMapObj(pReceiver) && !MR::isSensorNpc(pReceiver) && !MR::isSensorYoshi(pReceiver)) ||
    !MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender))) {
        if (MR::isSensorPlayerOrRide(pReceiver) && MR::isPlayerDamaging()) MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Menbo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver)) {

        if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgExplosionAttack(msg) ||
        MR::isMsgYoshiEat(msg)) {
            return tryMenboBlowDown(pSender, pReceiver);
        }

        if (MR::isMsgLockOnStarPieceShoot(msg)) {
            return true;
        }

        if (MR::isMsgStarPieceAttack(msg)) {
            if (canMenboStartSwoon()) {
                setNerve(GET_MENBO_NERVE(MenboNrvSwoonStart));
                return true;
            }
            return true;
        }

        if (MR::isMsgPlayerTrample(msg)) {
            return tryMenboPressDown(pSender, pReceiver);
        }

        if (MR::isMsgPlayerHipDrop(msg)) {
            return tryMenboPressDown(pSender, pReceiver);
        }

        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg)) {
            return tryMenboBlowDown(pSender, pReceiver);
        }

        return false;
    }
    return false;
}

bool Menbo::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
    isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        blocked = true;
    }
    if (blocked) return false;

    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender) || MR::isSensorNpc(pSender) ||
    MR::isSensorRide(pSender)) {
        MR::addVelocityFromPush(this, 1.0f, pSender, pReceiver);
        return true;
    }

    return false;
}

bool Menbo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg)) {
        if (isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) || isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvSwoon))) {
            return tryMenboBlowDown(pSender, pReceiver);
        }
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        return tryMenboBlowDown(pSender, pReceiver);
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isMenboActive()) {
            setNerve(GET_MENBO_NERVE(MenboNrvEaten));
            return true;
        }
        return false;
    }

    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }

    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) {
        kill();
        return true;
    }

    return false;
}

bool Menbo::isMenboActive() const {
    bool result = false;
    if (!isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvBreak)) && !isNerve(GET_MENBO_NERVE(MenboNrvEaten))) result = true;
    return result;
}

bool Menbo::canMenboStartSwoon() const {
    bool result = false;
    if (!isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvBreak)) && !isNerve(GET_MENBO_NERVE(MenboNrvEaten)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart))) result = true;
    return result;
}

bool Menbo::canMenboBindStarPointer() const {
    bool result = false;
    if (!isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) && !isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) && !isNerve(GET_MENBO_NERVE(MenboNrvBreak)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvEaten)) &&
    (!isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart)) || !MR::isLessStep(this, 30)) &&
    !isNerve(GET_MENBO_NERVE(MenboNrvFreezeBind))) result = true;
    return result;
}

bool Menbo::tryMenboPressDown(HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
    isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        blocked = true;
    }
    if (!blocked) {

        MR::zeroVelocity(this);
        if (isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
            setNerve(GET_MENBO_NERVE(MenboNrvBreak));
        } else {
            setNerve(GET_MENBO_NERVE(MenboNrvPressDown));
        }
        return true;
    }
    return false;
}

bool Menbo::tryMenboBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
    isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        blocked = true;
    }
    if (!blocked) {

        MR::zeroVelocity(this);
        if (isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
            MR::tryHitReactionBreak(this);
            setNerve(GET_MENBO_NERVE(MenboNrvBreak));
        } else {
            MR::tryHitReaction(this, "吹っ飛び");
            MR::setVelocityBlowAttack(this, pSender, pReceiver, 28.0f, 20.0f, 4);
            setNerve(GET_MENBO_NERVE(MenboNrvBlowDown));
            MR::invalidateExCollisionParts(this);
        }
        return true;
    }
    return false;
}

#pragma dont_inline on
// NONMATCHING: probe and position temporaries occupy different stack slots.
void Menbo::updateWaterInfo() {
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f probe = up;
    probe *= gMenboWaterProbeHeight;
    TVec3f position = mPosition + probe;
    if (MR::getWaterAreaObj(&mWaterInfo, position)) {
        MR::getWaterAreaInfo(&mWaterInfo, mPosition, mGravity, false);
        return;
    }
    if (MR::getWaterAreaObj(&mWaterInfo, mPosition)) {
        MR::getWaterAreaInfo(&mWaterInfo, mPosition, mGravity, false);
    }
}
#pragma dont_inline off

void Menbo::blendUpVec() {
    TVec3f target(0.0f);
    if (mWaterInfo.isInWater()) {
        target = mWaterInfo.mSurfaceNormal;
    } else {
        target = -mGravity;
    }
    MR::vecBlendSphere(&mUp, mUp, target, 0.3f);
}

void Menbo::accelerateMenboForward(f32 rate) {
    f32 cooled = calcMenboMoveImpulse(rate);
    if (cooled > 0.0f) {
        TVec3f front(mFront);
        JMAVECScaleAdd(&mUp, &front, &front, -mUp.dot(front));
        MR::normalize(&front);
        mVelocity.scale(front.dot(mVelocity), front);
        MR::addVelocityMoveToDirection3D(this, front, cooled);
    }
}

#pragma dont_inline on
// NONMATCHING: scaled water-normal and velocity-sum stack slots and argument setup order differ.
void Menbo::updateGravityWater() {
    if (mWaterInfo.isInWater()) {
        TVec3f& velocity = mVelocity;
        const TVec3f& water = mWaterInfo.mSurfaceNormal;
        Vec projected;
        JMAVECScaleAdd(&water, &velocity, &projected, -water.dot(velocity));
        TVec3f scaled(water);
        scaled *= 0.2f;
        MR::setVelocity(this, reinterpret_cast<const TVec3f&>(projected) + scaled);
        mIsInWater = true;
        return;
    }
    mIsInWater = false;
    MR::addVelocityToGravity(this, 0.2f);
}
#pragma dont_inline off

// NONMATCHING: operand floating-point registers are swapped for the damping multiply.
void Menbo::updateMenboTurn() {
    mTurnRate = mTurnRate * 0.985f;
    mTurnRate = mTurnRate + calcMenboTurnImpulse();
    MR::rotateVecDegree(&mFront, mFront, mUp, mTurnRate * mTurnDir);
}

void Menbo::turnMenboTowardWetPlayer() {
    if (isPlayerInSightWet()) {
        TVec3f toPlayer;
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        MR::vecBlendSphere(&mFront, mFront, toPlayer, 0.05f);
    }
}

void Menbo::updateMenboBlownVelocity() {
    f32 decel = MR::isOnGround(this) ? 0.93f : 0.99f;
    MR::attenuateVelocity(this, decel);
    if (!MR::isOnGround(this)) {
        TVec3f zero(0.0f, 0.0f, 0.0f);
        if (!MR::isInWater(this, zero)) {
            MR::addVelocityToGravity(this, 1.3f);
        }
    }
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

bool Menbo::isMenboFarFromHome() const {
    TVec3f d = mBasePos;
    d.sub(mPosition);
    return d.length() > 1000.0f;
}

bool Menbo::isMenboFacingAwayFromHome(f32 angle) {
    TVec3f dir(0.0f);
    MR::calcDirToTarget(&dir, this, &mBasePos);
    f32 limit = MR::cosDegree(angle);
    return dir.dot(mFront) < limit;
}

#pragma dont_inline on
// NONMATCHING: probe origin and scaled-up temporaries occupy different stack slots.
bool Menbo::isGroundIceEdge() {
    if (MR::isBindedGroundIce(this)) {
        return true;
    }
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f s = up;
    s *= cProbeAbove;
    TVec3f origin = mPosition + s;
    TVec3f ray = up;
    ray *= -(cProbeAbove - cProbeBelow);
    return MR::isFirstPolyOnLineToMapIce(&origin, &ray);
}
#pragma dont_inline off

bool Menbo::isPlayerInSightWet() {
    if (MR::isPlayerSwimming()) {
        mIsPlayerWet = true;
    } else if (MR::isOnGroundPlayer()) {
        Triangle* pTri = MR::getPlayerGroundingPolygon();
        if (MR::isGroundCodeWater(pTri) || MR::isGroundCodeWet(MR::getPlayerGroundingPolygon())) {
            mIsPlayerWet = true;
        } else {
            mIsPlayerWet = false;
        }
    }
    bool result = false;
    if (MR::isInSightFanPlayer(this, mFront, cSight.distance, cSight.horizontal, cSight.vertical) && mIsPlayerWet) result = true;
    return result;
}

f32 Menbo::calcMenboMoveImpulse(f32 rate) {
    mCooldown -= 1;
    if (mCooldown <= 0) {
        mCooldown = 81;
        return rate;
    }
    return 0.0f;
}

f32 Menbo::calcMenboTurnImpulse() {
    f32 v = mTurnTimer - 1.0f;
    mTurnTimer = v;
    f32 ret = 0.0f;
    if (v <= 0.0f) {
        ret = cTickBoost;
        s32 interval = 81;
        mTurnTimer = interval;
    }
    return ret;
}

void Menbo::makeMenboSurfaceMatrix(TPos3f* pMtx, const LiveActor* actor, const TVec3f& rUp, const TVec3f& rFront) {
    TPos3f base;
    base.set(actor->getBaseMtx());
    TVec3f up;
    TVec3f front;
    if (!MR::isNearZero(rUp, 0.001f)) up = rUp;
    else base.getYDir(up);
    if (!MR::isNearZero(rFront, 0.001f)) front = rFront;
    else base.getZDir(front);
    TVec3f cross;
    PSVECCrossProduct(&front, &up, &cross);
    if (MR::isNearZero(cross, 0.001f)) {
        TVec3f side;
        base.getXDir(side);
        PSVECCrossProduct(&side, &up, &front);
    }
    MR::makeMtxUpFrontPos(pMtx, up, front, actor->mPosition);
}

Menbo::~Menbo() {
}

void NrvMenbo::MenboNrvFreezeBind::execute(Spine* pSpine) const {
    LiveActor* actor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
    MR::updateActorStateAndNextNerve(actor, GET_MENBO_NERVE(MenboNrvWait));
}

namespace {
const f32 cDamp = 0.98f;
const f32 cTurnDamp = 0.97f;
const f32 cFriction = 0.99f;
const f32 cProbeAbove = 200.0f;
const f32 cProbeBelow = -50.0f;

const f32 cTickBoost = 1.0f;
}

extern const volatile f32 gMenboWaterProbeHeight = 0.0f;
