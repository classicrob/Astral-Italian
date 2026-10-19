#include "Game/Enemy/StinkBugParent.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace {
    // sGravity
    static const f32 sTurnRateSearch = 2.0f;
    static const f32 sTurnRateDashSign = 4.0f;
    static const s32 sStepForWait = 1;
    static const s32 sStepForDashSignEnd = 10;
    static const s32 sStepForShakeStart = 40;
    static const s32 sStepForEnableShake = 25;
    static const s32 sStepForRecover = 60;
    static const s32 sStepToJumpBackPlayer = 22;
    // mSearchAngleOffset;
    // sTerritorySizeX
    // sTerritorySizeY
    // sTerritorySizeZPlus
    // sChildRadius
    // sAttackCylinderRadius
    static const s32 sStepForChildDeathSound = 48;
    static const s32 sStepForSurpriseSound = 65;
};  // namespace

namespace NrvStinkBugParent {
    class StinkBugParentNrvAppearFall : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static StinkBugParentNrvAppearFall sInstance;
    };
    StinkBugParentNrvAppearFall StinkBugParentNrvAppearFall::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(StinkBugParentNrvWait, StinkBugParent, Wait);
    NEW_NERVE(StinkBugParentNrvSearch, StinkBugParent, Search);
    NEW_NERVE(StinkBugParentNrvDashSign, StinkBugParent, DashSign);
    NEW_NERVE(StinkBugParentNrvDashSignEnd, StinkBugParent, DashSignEnd);
    NEW_NERVE(StinkBugParentNrvDash, StinkBugParent, Dash);
    NEW_NERVE(StinkBugParentNrvDashEnd, StinkBugParent, DashEnd);
    NEW_NERVE(StinkBugParentNrvBack, StinkBugParent, Back);
    NEW_NERVE(StinkBugParentNrvHipDropDown, StinkBugParent, HipDropDown);
    NEW_NERVE(StinkBugParentNrvAttack, StinkBugParent, Attack);
    NEW_NERVE(StinkBugParentNrvSpinReaction, StinkBugParent, SpinReaction);
    NEW_NERVE(StinkBugParentNrvPanic, StinkBugParent, Panic);
    NEW_NERVE(StinkBugParentNrvShakeStart, StinkBugParent, ShakeStart);
    NEW_NERVE(StinkBugParentNrvShake, StinkBugParent, Shake);
    NEW_NERVE(StinkBugParentNrvRecover, StinkBugParent, Recover);
    NEW_NERVE(StinkBugParentNrvFreeze, StinkBugParent, Freeze);
    NEW_NERVE(StinkBugParentNrvDemoPrepare, StinkBugParent, DemoPrepare);
    NEW_NERVE(StinkBugParentNrvDemoChildDown, StinkBugParent, DemoChildDown);
    NEW_NERVE(StinkBugParentNrvDemoAnger, StinkBugParent, DemoAnger);
};  // namespace NrvStinkBugParent

namespace {
    const Param sParam[] = {{20.0f, 5.0f, 30}, {25.0f, 10.0f, 20}};

    const Param* getParam(bool isChildAlive) {
        if (!isChildAlive) {
            return &sParam[1];
        }
        return &sParam[0];
    }
};  // namespace

StinkBugParent::StinkBugParent(const char* pName) : StinkBugBase(pName), mAnimScaleCtrl(), mStarPointBind(), mParent() {
    mIsChildAlive = true;
}

void StinkBugParent::init(const JMapInfoIter& rIter) {
    StinkBugBase::init(rIter);
    initSound(4, "StinkBugParent", nullptr, TVec3f(0.0f));
    initHitSensor(2);
    MR::addHitSensorEye(this, "Body", 4, 250.0f, TVec3f(0.0f, 150.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "head", "Face", 8, 160.0f, TVec3f(0.0f, -50.0f, 80.0f));
    MR::initCollisionParts(this, "SwitchChildJ", getSensor("Body"), nullptr);
    mParent = MR::createCollisionPartsFromLiveActor(this, "Switch", getSensor("Body"), getBaseMtx(), MR::CollisionScaleType_Unk2);
    initBinder(300.0f, 300.0f, 0);
    MR::setBinderExceptActor(this, this);
    MR::setBinderIgnoreMovingCollision(this);
    MR::declarePowerStar(this);
    MR::declareStarPiece(this, 8);
    MR::initStarPointerTarget(this, 250.0f, TVec3f(0.0f, 100.0f, 0.0f));
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionNerve(this, GET_NERVE(StinkBugParent, StinkBugParentNrvDemoAnger), "ダメージ中");
    }

    mMaxTurnDegree = 180.0f;
    mRadius = 1200.0f;
    MR::startBrk(this, "Anger");
    MR::setBrkFrameAndStop(this, 0.0f);
    initNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvAppearFall), 1);
    MR::initActorState(this, mStarPointBind, GET_NERVE(StinkBugParent, StinkBugParentNrvFreeze), "SupportFreeze");
    makeActorDead();
}

void StinkBugParent::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::resetPosition(this, mHomePos);
    }

    fixInitPos();

    if (isPlayerInTerritory(400.0, 200.0f, 600.0f, 200.0f)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashSign));
    } else if (MR::isGreaterStep(this, ::sStepForWait)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvSearch));
    }
}

void StinkBugParent::exeSearch() {
    if (MR::isFirstStep(this)) {
        if (mIsChildAlive) {
            MR::startAction(this, "Search");
        } else {
            MR::startAction(this, "AngerSearch");
        }
    }

    fixInitPos();

    if (tryTurnSearch(::sTurnRateSearch)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvWait));
    } else if (isPlayerInTerritory(400.0f, 200.0f, 600.0f, 200.0f)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashSign));
    }
}

void StinkBugParent::exeDashSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RushStart");
        MR::startActionSound(this, "EvStinkbugLFind");
    }

    MR::startActionSound(this, "EmLvStinkbugLDashSign");
    fixInitPos();
    tryTurnDashSign(::sTurnRateDashSign);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashSignEnd));
    }
}

void StinkBugParent::exeDashSignEnd() {
    if (MR::isStep(this, ::sStepForDashSignEnd)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDash));
    }
}

void StinkBugParent::exeDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rush");
        MR::validateHitSensors(this);
    }

    MR::startActionSound(this, "EmLvStinkbugLDash", -1, -1, -1);

    if (!MR::isNear(this, mHomePos, mRadius) || MR::isBindedWall(this)) {
        mVelocity.zero();
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashEnd));
    } else {
        setDashVelocity(::getParam(mIsChildAlive)->mDashVelocity);
    }
}

void StinkBugParent::exeDashEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RushStop");
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
    }
}

void StinkBugParent::exeBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Back");
        MR::invalidateHitSensor(this, "head");
    }

    MR::startActionSound(this, "EmLvStinkbugLBack");

    if (MR::isNear(this, mHomePos, (2.0f * ::getParam(mIsChildAlive)->mDashDistance))) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvWait));
    } else {
        TVec3f a1;
        a1.sub(mHomePos, mPosition);
        MR::normalize(&a1);
        MR::turnVecToPlane(&a1, a1, mGravity);
        mVelocity.scale(::getParam(mIsChildAlive)->mDashDistance, a1);
    }
}

void StinkBugParent::exeHipDropDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Death");
        MR::startBrk(this, "Death");
        MR::invalidateCollisionParts(mParent);
        MR::invalidateHitSensors(this);
        MR::startActionSound(this, "EmStinkbugStompedS");
        MR::startActionSound(this, "EvStinkbugLStomped");
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void StinkBugParent::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
    }
}

void StinkBugParent::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinAction");
        MR::startActionSound(this, "EmGuardS");
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
    }
}

void StinkBugParent::exePanic() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Loss");
        MR::invalidateHitSensor(this, "head");
        MR::startActionSound(this, "EvStinkbugLPanic");
    }

    MR::startActionSound(this, "EmLvStinkbugLPanic");
    mVelocity.zero();

    if (!MR::isOnPlayer(getSensor("Body"))) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvRecover));
    } else if (MR::isStep(this, ::getParam(mIsChildAlive)->mPanicStep)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvShakeStart));
    }
}

void StinkBugParent::exeRecover() {
    mVelocity.zero();
    MR::startActionSound(this, "EmLvStinkbugLPanic");

    if (MR::isStep(this, ::sStepForRecover)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
    }
}



void StinkBugParent::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
        MR::startAction(this, "Back");
    }

    mVelocity.zero();

    if (MR::isEffectValid(this, "RushSmoke")) {
        MR::deleteEffect(this, "RushSmoke");
    }

    if (MR::isEffectValid(this, "BackSmoke")) {
        MR::deleteEffect(this, "BackSmoke");
    }

    MR::updateActorStateAndNextNerve(this, GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
}

void StinkBugParent::exeShakeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Repel");
    }

    if (MR::isStep(this, ::sStepForShakeStart)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvShake));
    }
}

void StinkBugParent::exeShake() {
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
    }
}

void StinkBugParent::exeDemoPrepare() {
    if (MR::tryStartDemoRegisteredMarioPuppetable(this, "ため")) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDemoChildDown));
    }
}

void StinkBugParent::exeDemoChildDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChildDeath");
        MR::startBva(this, "ChildDeath");
        MR::startBrk(this, "ChildDeath");
        MR::startActionSound(this, "EvStinkbugSStomped");
        MR::startActionSound(this, "EmStinkbugStompedS");
        mIsChildAlive = false;
        MR::invalidateCollisionParts(this);
        MR::validateCollisionParts(mParent);
    }

    if (MR::isStep(this, ::sStepToJumpBackPlayer)) {
        jumpBackPlayerFromChild();
    }

    if (MR::isStep(this, ::sStepForChildDeathSound)) {
        MR::startActionSound(this, "EmExplodeS");
    }

    if (MR::isStep(this, ::sStepForSurpriseSound)) {
        MR::startActionSound(this, "EvStinkbugLSurprise");
    }

    mVelocity.zero();
}

void StinkBugParent::exeDemoAnger() {
    if (MR::isFirstStep(this)) {
        TVec3f v1;
        MR::overlayWithPreviousScreen(2);
        MR::startAction(this, "Anger");
        MR::startBrk(this, "Anger");
        MR::resetPosition(this, mHomePos);
        MR::setPlayerPosAndWait("子連れカメムシデモ後ポイント", false);
        MR::findNamePos("子連れカメムシデモ後ポイント", &v1, nullptr);
        MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, v1, 180.0f);
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::fn_803B6830(false);
        }
    }

    MR::startActionSound(this, "EvLvStinkbugLAngry");
    MR::startActionSound(this, "EmLvStinkbugLAngry");

    if (MR::isActionEnd(this) || MR::isDemoEndWaitCameraInterpolating()) {
        TVec3f v2, v3;
        v2.scaleAdd(200.0f, mFront, mPosition);
        v3.scaleAdd(-1.0f, mGravity, mFront);
        MR::normalize(&v3);
        MR::appearStarPieceToDirection(this, v2, v3, 8, 25.0f, 40.0f, false);
        MR::startActionSound(this, "OjStarPieceBurstF");
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvWait));
        MR::startActionSound(this, "EmStinkbugLAngryEnd");
    }
}

void StinkBugParent::appear() {
    MR::startStageBGM("BGM_PINCH_01", false);
    LiveActor::appear();
}

void StinkBugParent::kill() {
    MR::startActionSound(this, "EmExplodeS");
    MR::requestAppearPowerStar(this, mPosition);
    LiveActor::kill();
}

void StinkBugParent::control() {
    if (!mIsChildAlive) {
        mParent->setMtx();
    }

    mAnimScaleCtrl->updateNerve();
    isShakeChance();

    if (!isNrvMoving()) {
        MR::offBind(this);
    } else {
        MR::onBind(this);
        TVec3f& vel = mVelocity;
        TVec3f& grav = mGravity;
        vel.scaleAdd(-grav.dot(vel), grav, vel);
        mVelocity.scaleAdd(2.0f, mGravity, mVelocity);
    }
}

void StinkBugParent::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFront);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// SMG2 unnamed helper: the Petari isShakeChance body (used by attackSensor).
bool StinkBugParent::fn_801FEDA0() const {
    if (this->isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown))) {
        return false;
    }

    if (this->isNrvDemo()) {
        return false;
    }

    if (!this->isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvShake))) {
        return false;
    }

    if (MR::isGreaterStep(this, 25)) {
        return false;
    }

    return MR::isOnPlayer(this->getSensor("Body"));
}

void StinkBugParent::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown)) && !isNrvDemo() &&
        !isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvAttack)) &&
        !isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvFreeze)) && MR::isSensorPlayer(pReceiver)) {
        if (MR::isSensorEnemyAttack(pSender)) {
            if (!isHitHorn(pSender, pReceiver, 25.0f)) {
                return;
            }

            if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
                if (!isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDash))) {
                    setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvAttack));
                    return;
                }
            }
        }

        if (MR::isSensorEye(pSender) && this->fn_801FEDA0() && !MR::isPlayerHipDropFalling()) {
            MR::sendMsgEnemyAttackFlipToDir(pReceiver, pSender, mFront);
            return;
        }
    }
}

bool StinkBugParent::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown))) {
        return false;
    }

    if (isNrvDemo()) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceReflect(msg) && pReceiver == getSensor("Body")) {
        if (isNrvEnableStarPieceAttack()) {
            setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvSpinReaction));
        }
        return true;
    } else if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (mIsChildAlive) {
            if (!isHitChild()) {
                return false;
            } else {
                setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDemoPrepare));
            }
        } else {
            setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown));
        }
        return true;
    } else if (!isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvSpinReaction)) && !isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDash)) &&
               MR::isMsgPlayerSpinAttackOrSupportTico(msg) && pReceiver == getSensor("Body")) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvSpinReaction));
        return true;
    }

    return false;
}

bool StinkBugParent::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown))) {
        return false;
    }

    if (isNrvDemo()) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvPanic))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvShakeStart))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvShake))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvFreeze))) {
        return false;
    }

    if (MR::isMsgFloorTouch(msg) && MR::isOnPlayer(getSensor("Body"))) {
        setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvPanic));
        return true;
    }

    return false;
}

void StinkBugParent::jumpBackPlayerFromChild() const {
    TVec3f v10, v9, v8;
    MR::findNamePos("子連れカメムシデモ後ポイント", &v10, nullptr);
    v9.sub(v10, *MR::getPlayerPos());
    MR::normalize(v9, &v8);
    TVec3f v7 = -v8;
    MR::offBind(reinterpret_cast<LiveActor*>(MR::getPlayerDemoActor()));
    MR::unlockPlayerAnimation();
    MR::forceFlyPlayer(v10, v7, 0);
}

bool StinkBugParent::isShakeChance() const {
    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvHipDropDown))) {
        return false;
    }

    if (isNrvDemo()) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvAttack))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvFreeze))) {
        return false;
    }

    if (!mStarPointBind->tryStartPointBind()) {
        return false;
    }

    const_cast< StinkBugParent* >(this)->setNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvFreeze));
    return true;
}

bool StinkBugParent::isHitChild() const {
    if (!mIsChildAlive) {
        return false;
    }

    TVec3f v1, v2, v3, v4;
    MR::copyJointPos(this, "ChildCenter", &v1);
    v2.sub(*MR::getPlayerPos(), v1);
    MR::calcUpVec(&v3, this);
    f32 a = v3.dot(v2);
    v4.scaleAdd(-a, v3, v2);
    return v4.length() <= 150.0f;
}

bool StinkBugParent::isNrvEnableStarPieceAttack() const {
    return isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvWait)) || isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvSearch)) ||
           isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashSign)) || isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDashSignEnd)) ||
           isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
}

inline bool StinkBugParent::isNrvMoving() const {
    return isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDash)) || isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvBack));
}

inline bool StinkBugParent::isNrvDemo() const {
    return isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDemoPrepare)) || isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDemoChildDown)) ||
           isNerve(GET_NERVE(StinkBugParent, StinkBugParentNrvDemoAnger));
}

namespace NrvStinkBugParent {
    inline void StinkBugParentNrvAppearFall::execute(Spine* pSpine) const {
        LiveActor* pActor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
        if (!MR::isDemoActiveRegistered(pActor)) {
            MR::invalidateClipping(pActor);
            pActor->setNerve(&StinkBugParentNrvWait::sInstance);
        }
    }
}
