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
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace NrvMenbo {
    NEW_NERVE(MenboNrvWait, Menbo, Wait);
    NEW_NERVE(MenboNrvMove, Menbo, Move);
    NEW_NERVE(MenboNrvTurn, Menbo, Turn);

    // Turn-again nerve: execute hands off to the turn-back helper.
    NEW_NERVE(MenboNrvTurnBack, Menbo, TurnBack);

    NEW_NERVE(MenboNrvFind, Menbo, Find);
    NEW_NERVE(MenboNrvAttack, Menbo, Attack);

    // Attack end nerve (custom execute in SMG2 as well).
    NEW_NERVE(MenboNrvAttackEnd, Menbo, AttackEnd);

    NEW_NERVE(MenboNrvSwoonStart, Menbo, SwoonStart);
    NEW_NERVE(MenboNrvSwoon, Menbo, Swoon);
    NEW_NERVE(MenboNrvSwoonEnd, Menbo, SwoonEnd);
    NEW_NERVE(MenboNrvFreeze, Menbo, Freeze);
    NEW_NERVE(MenboNrvPressDown, Menbo, PressDown);
    NEW_NERVE(MenboNrvBlowDown, Menbo, BlownDown);
    NEW_NERVE(MenboNrvBreak, Menbo, Break);

    // Yoshi tongue nerve: empty execute in SMG2.
    class MenboNrvEaten : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static MenboNrvEaten sInstance;
    };
    MenboNrvEaten MenboNrvEaten::sInstance ATTRIBUTE_WEAK;

    // Star-pointer bind state nerve: runs the state, then goes to Wait.
    class MenboNrvFreezeBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static MenboNrvFreezeBind sInstance;
    };
    MenboNrvFreezeBind MenboNrvFreezeBind::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMenbo

#define GET_MENBO_NERVE(name) (&NrvMenbo::name::sInstance)

// Blob of Menbo strings (SMG2 lbl_8068C660 + offsets); used as literal names below.
// unk8 "SupportFreeze", unk18 "氷モデル", unk24 "MenboFreezeIce", unk34/48/5C/70 ripple effects,
// unk84 "星の入り江惑星氷化" (demo cast), unk98 "Normal", unkE8 "Freeze", unkF0 "IceEnd".


// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern f32 lbl_807E3048;

// Water normal stored in the SMG2 WaterInfo area at this+0xB0 (WaterInfo + 0x14).
static inline TVec3f& waterNormalRef(Menbo* pActor) {
    return *reinterpret_cast<TVec3f*>(reinterpret_cast<char*>(pActor) + 0xB0);
}

// PartsModel byte at +0x9D (NameObj draw-flag position in SMG2).
static inline void setPartsModelVisible(Menbo* pActor, bool b) {
    *reinterpret_cast<bool*>(reinterpret_cast<char*>(pActor) + 0x9D) = b;
}

Menbo::Menbo(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mPartsModel = nullptr;
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
    TVec3f zero(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), zero);
}

Menbo::~Menbo() {
}

void Menbo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Menbo", nullptr, false);
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
    setPartsModelVisible(this, true);
    MR::onEmitEffectSyncClipping(this, "MenboRippleFrontL");
    MR::onEmitEffectSyncClipping(this, "MenboRippleFrontR");
    MR::onEmitEffectSyncClipping(this, "MenboRippleBackL");
    MR::onEmitEffectSyncClipping(this, "MenboRippleBackR");
    MR::tryRegisterDemoCast(this, "星の入り江惑星氷化", rIter);
    makeActorAppeared();
}

void Menbo::initAfterPlacement() {
    TVec3f pos = mPosition;
    TVec3f grav = mGravity;
    TVec3f scaledGrav;
    scaledGrav = grav;
    scaledGrav.scale(150.0f);
    pos.sub(scaledGrav);
    TVec3f ray = mGravity;
    ray.scale(mGroundDist);

    Triangle* pTri = nullptr;
    CollisionParts* pParts = nullptr;
    if (MR::fn_80037E40(0, &pTri, &pos, &ray)) {
        pParts = reinterpret_cast<CollisionParts*>(pTri);
    } else {
        if (MR::getFirstPolyOnLineToMap(nullptr, pTri, pos, ray)) {
            pParts = nullptr;  // SMG2 searches the same-host parts through the collision director here
        }
    }

    if (pParts != nullptr) {
        mBinder->setExCollisionParts(pParts);
    }
}

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
    mIsInWater = false;
    mIsPlayerWet = false;
    mIsTurning = false;
}

void Menbo::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    LiveActor::kill();
}

void Menbo::control() {
    mAnimScaleCtrl->updateNerve();
    if (isYoshiEatable()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (isStarPointerBindable() && mStarPointBind->tryStartPointBind()) {
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
        if (isMovedFromBase() && isWallFreezeReady(50.0f)) {
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
    calcFrontFromUp(7.0f);
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

    if (!isMovedFromBase() && isPlayerInSightWet()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFind));
    }

    if (isGroundIceEdge()) {
        setNerve(GET_MENBO_NERVE(MenboNrvFreeze));
    }

    MR::attenuateVelocity(this, 0.99f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    rotateFrontGround();
}

// SMG2 fn_801C43C0: turn-again nerve (retries the turn up to 3 times, then attack end).
void Menbo::exeTurnBack() {
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
        if (isWallFreezeReady(30.0f)) {
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

    MR::attenuateVelocity(this, 0.97f * 0.99f);
    updateWaterInfo();
    blendUpVec();
    updateGravityWater();
    rotateFrontGround();
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
    blendFrontToPlayer();
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
        // NONMATCHING: SMG2 frame clamp approximated (the target reads a constant pool value here).
        f32 frameMax = MR::getBckFrameMax(this);
        if (frameMax < 600.0f) {
            MR::setBckFrameAndStop(this, frameMax);
        }
        TVec3f front = mFront;
        TVec3f projected;
        MR::normalize(&projected, front);
        projected.scale(1.0f);
        mVelocity.scale(mVelocity.dot(projected), projected);
        MR::fn_80010CB0(this, projected, 27.0f);
    }
}

// SMG2 fn_801C4720: attack end nerve (goes to Turn once the timer runs out).
void Menbo::exeAttackEnd() {
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

    MR::attenuateVelocity(this, 0.98f * 0.98f);
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

    MR::attenuateVelocity(this, 0.98f * 0.98f);
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

    MR::attenuateVelocity(this, 0.98f * 0.98f);
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
            MR::zeroVelocity(this);
            MR::emitEffect(this, "IceEnd");
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

void Menbo::exeBlownDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
    }

    if ((MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) || MR::isGreaterStep(this, 35)) {
        TVec3f side = mUp;
        side.scale(100.0f);
        TVec3f pos = side;
        pos.add(mPosition);
        MR::emitItemHitAtPos(this, &pos);
        MR::tryHitReactionDead(this);
        kill();
        return;
    }

    updateFallVelocity();
}

void Menbo::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(mPartsModel, "Break");
        TVec3f side = mUp;
        side.scale(100.0f);
        TVec3f pos = side;
        pos.add(mPosition);
        MR::emitItemHitAtPos(this, &pos);
        MR::hideModelIfShown(this);
    }

    if (MR::isActionEnd(mPartsModel)) {
        MR::startBva(mPartsModel, "Normal");
        setPartsModelVisible(this, false);
        mPartsModel->kill();
        kill();
    }
}

void Menbo::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    calcMtxUpFront(&mtx, mUp, mFront);
    MR::setBaseTRMtx(this, mtx);
    MR::calcFrontVec(&mFront, this);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void Menbo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
            if (isNerve(GET_MENBO_NERVE(MenboNrvFreezeBind)) || isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart)) ||
                isNerve(GET_MENBO_NERVE(MenboNrvSwoon)) || isNerve(GET_MENBO_NERVE(MenboNrvSwoonEnd))) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }

            bool isHit = isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
                         isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten));
            if (!isHit) {
                MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
            }
        }
    } else {
        bool isEnemyOrOther = MR::isSensorEnemy(pSender) && !MR::isSensorEnemy(pReceiver) &&
                              !MR::isSensorMapObj(pReceiver) && !MR::isSensorNpc(pReceiver) &&
                              !MR::isSensorYoshi(pReceiver);
        if (isEnemyOrOther || !MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) {
            if (MR::isSensorPlayerOrRide(pReceiver) && MR::isPlayerDamaging()) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

bool Menbo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgExplosionAttack(msg) ||
        MR::isMsgYoshiEat(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (isPlayerAttackable()) {
            setNerve(GET_MENBO_NERVE(MenboNrvSwoonStart));
        }
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        return tryPressDown();
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        return tryPressDown();
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }

    return false;
}

bool Menbo::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        return false;
    }

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
            return tryBlowDown(pSender, pReceiver);
        }
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isYoshiEatable()) {
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

// Helpers -----------------------------------------------------------------------------------------

bool Menbo::isYoshiEatable() const {
    return !isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) &&
           !isNerve(GET_MENBO_NERVE(MenboNrvBreak)) && !isNerve(GET_MENBO_NERVE(MenboNrvEaten));
}

bool Menbo::isPlayerAttackable() const {
    return !isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) && !isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) &&
           !isNerve(GET_MENBO_NERVE(MenboNrvBreak)) && !isNerve(GET_MENBO_NERVE(MenboNrvEaten)) &&
           !isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart));
}

bool Menbo::isStarPointerBindable() const {
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvFreeze)) || isNerve(GET_MENBO_NERVE(MenboNrvBreak)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        return false;
    }
    if (isNerve(GET_MENBO_NERVE(MenboNrvSwoonStart)) && MR::isLessStep(this, 30)) {
        return false;
    }
    return !isNerve(GET_MENBO_NERVE(MenboNrvFreezeBind));
}

bool Menbo::tryPressDown() {
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        return false;
    }

    MR::zeroVelocity(this);
    if (isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
        setNerve(GET_MENBO_NERVE(MenboNrvBreak));
    } else {
        setNerve(GET_MENBO_NERVE(MenboNrvPressDown));
    }
    return true;
}

bool Menbo::tryBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_MENBO_NERVE(MenboNrvPressDown)) || isNerve(GET_MENBO_NERVE(MenboNrvBlowDown)) ||
        isNerve(GET_MENBO_NERVE(MenboNrvBreak)) || isNerve(GET_MENBO_NERVE(MenboNrvEaten))) {
        return false;
    }

    MR::zeroVelocity(this);
    if (isNerve(GET_MENBO_NERVE(MenboNrvFreeze))) {
        MR::tryHitReactionBreak(this);
        setNerve(GET_MENBO_NERVE(MenboNrvBreak));
    } else {
        MR::tryHitReaction(this, "吹っ飛び");
        MR::fn_80012B90(this, 28.0f, 20.0f, pSender, pReceiver, 4);
        setNerve(GET_MENBO_NERVE(MenboNrvBlowDown));
        MR::invalidateExCollisionParts(this);
    }
    return true;
}

void Menbo::updateWaterInfo() {
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f probe = up;
    probe.scale(lbl_807E3048);
    probe.add(mPosition);
    if (MR::getWaterAreaObj(&mWaterInfo, probe)) {
        MR::getWaterAreaInfo(&mWaterInfo, mPosition, mGravity, false);
        return;
    }
    if (MR::getWaterAreaObj(&mWaterInfo, mPosition)) {
        MR::getWaterAreaInfo(&mWaterInfo, mPosition, mGravity, false);
    }
}

void Menbo::blendUpVec() {
    TVec3f target;
    if (mWaterInfo.isInWater()) {
        target = waterNormalRef(this);
    } else {
        target = mGravity;
        target.scale(-1.0f);
    }
    MR::vecBlendSphere(&mUp, mUp, target, 0.3f);
}

void Menbo::updateGravityWater() {
    if (mWaterInfo.isInWater()) {
        TVec3f water = waterNormalRef(this);
        TVec3f projected = mVelocity;
        TVec3f removed = water;
        removed.scale(-water.dot(mVelocity));
        projected.add(removed);
        TVec3f scaled = water;
        scaled.scale(0.2f);
        scaled.add(projected);
        MR::setVelocity(this, scaled);
        mIsInWater = true;
        return;
    }
    mIsInWater = false;
    MR::addVelocityToGravity(this, 0.2f);
}

void Menbo::calcFrontFromUp(f32 rate) {
    f32 cooled = tickCooldown(rate);
    if (cooled > 0.0f) {
        TVec3f front = mFront;
        TVec3f projected = mUp;
        projected.scale(-mUp.dot(front));
        projected.add(front);
        MR::normalize(&projected, projected);
        mVelocity.scale(mVelocity.dot(projected), projected);
        MR::fn_80010CB0(this, projected, cooled);
    }
}

void Menbo::rotateFrontGround() {
    mTurnRate = mTurnRate * 0.985f;
    mTurnRate = mTurnRate + tickTurnTimer();
    MR::rotateVecDegree(&mFront, mFront, mUp, mTurnRate * mTurnDir);
}

void Menbo::blendFrontToPlayer() {
    if (isPlayerInSightWet()) {
        TVec3f toPlayer;
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        MR::vecBlendSphere(&mFront, mFront, toPlayer, 0.05f);
    }
}

void Menbo::updateFallVelocity() {
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

bool Menbo::isMovedFromBase() const {
    TVec3f d = mBasePos;
    d.sub(mPosition);
    return d.length() > 0.0f;
}

// SMG2 fn_801C5C30: true when the facing is within the given cone of the player-side direction.
bool Menbo::isWallFreezeReady(f32 angle) {
    TVec3f dir(0.0f);
    MR::fn_8000E960(&dir, this, &mBasePos);
    if (angle < 0.0f) {
        angle = -angle;
    }
    f32 limit = MR::cosDegree(angle);
    return dir.dot(mFront) < limit;
}

// SMG2 fn_801C5CE0: true when the body is touching ice or a sloped ground edge.
bool Menbo::isGroundIceEdge() {
    if (MR::isBindedGroundIce(this)) {
        return true;
    }
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f s = up;
    s.scale(200.0f);
    TVec3f origin = s;
    origin.add(mPosition);
    TVec3f ray = up;
    ray.scale(-250.0f);
    return MR::fn_80038570(&origin, &ray);
}

bool Menbo::isPlayerInSightWet() {
    if (MR::isPlayerSwimming()) {
        mIsPlayerWet = true;
    } else if (MR::isOnGroundPlayer()) {
        Triangle* pTri = MR::getPlayerGroundingPolygon();
        if (MR::isGroundCodeWaterBottomH(pTri) || MR::isGroundCodeWaterBottomM(pTri)) {
            mIsPlayerWet = true;
        } else {
            mIsPlayerWet = false;
        }
    }
    return MR::isInSightFanPlayer(this, mFront, 1000.0f, 160.0f, 30.0f) && mIsPlayerWet;
}

f32 Menbo::tickCooldown(f32 rate) {
    mCooldown -= 1;
    if (mCooldown <= 0) {
        mCooldown = 81;
        return rate;
    }
    return 0.0f;
}

f32 Menbo::tickTurnTimer() {
    f32 v = mTurnTimer - 1.0f;
    mTurnTimer = v;
    f32 ret = 0.0f;
    if (v == 0.0f) {
        ret = 1.0f;
        mTurnTimer = 81.0f;
    }
    return ret;
}

void Menbo::calcMtxUpFront(TPos3f* pMtx, const TVec3f& rUp, const TVec3f& rFront) {
    TVec3f up = rUp;
    TVec3f front = rFront;
    if (MR::isNearZero(front, 0.001f)) {
        front = mFront;
    }
    if (MR::isNearZero(up, 0.001f)) {
        up = mUp;
    }
    MR::makeMtxUpFrontPos(pMtx, up, front, mPosition);
}

// Nerve execute bodies --------------------------------------------------------------------------

void NrvMenbo::MenboNrvFreezeBind::execute(Spine* pSpine) const {
    LiveActor* actor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
    MR::updateActorStateAndNextNerve(actor, GET_MENBO_NERVE(MenboNrvWait));
}

// Named nerve classes with custom exe are generated above by NEW_NERVE; exeEaten etc. are inline.
