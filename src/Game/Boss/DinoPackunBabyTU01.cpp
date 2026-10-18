#include "Game/Boss/DinoPackunBaby.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

// PLACEHOLDER(objects): unnamed core helper (jump toward the player along the front vector with a jump param); resolve when core names fn_800126F0
extern "C" void fn_800126F0(LiveActor*, const TVec3f*, const f32*);
// PLACEHOLDER(objects): unnamed core helper (jump with a jump param, same as fn_800126F0 but chase flavour); resolve when core names fn_800127A0
extern "C" void fn_800127A0(LiveActor*, const TVec3f*, const f32*);
// PLACEHOLDER(objects): unnamed core helper (= MR::setVelocityBlowAttack(actor, sender, receiver, f32, f32, stopFrames)); resolve when core names fn_80012B90
extern "C" void fn_80012B90(LiveActor*, f32, f32, HitSensor*, HitSensor*, s32);

void DinoPackunBaby::chasePlayer(const MR::ActorMoveParam* pParam, bool bAttack) {
    MR::moveAndTurnToPlayer(this, &mFront, pParam->_0, pParam->_4, pParam->_8, pParam->_C);
    if (bAttack) {
        MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Message"));
    }
}

void DinoPackunBaby::escapeFromPlayer(const MR::ActorMoveParam* pParam, bool bAttack) {
    MR::escapeFromPlayer(this, &mFront, pParam->_0, pParam->_4, pParam->_8, pParam->_C);
    if (bAttack) {
        MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Message"));
    }
}

bool DinoPackunBaby::updateJumpState(ActorStateBaseInterface* pState) {
    bool isHit = false;
    if (MR::isBinded(this) && MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"))) {
        isHit = true;
    }
    MR::updateActorState(pState);
    if (isHit) {
        return false;
    }
    return MR::isOnGround(this);
}

bool DinoPackunBaby::isInSight(const f32* pSight) {
    return MR::isInSightFanPlayer(this, mFront, pSight[0], pSight[1], pSight[2]);
}

bool DinoPackunBaby::isPlayerInFront() {
    TVec3f dir(*MR::getPlayerPos());
    dir.sub(mPosition);
    MR::normalizeOrZero(&dir);
    return dir.dot(mFront) > 0.0f;
}

void DinoPackunBaby::requestMovementOnParts() {
    MR::requestMovementOn(mShellLeft);
    MR::requestMovementOn(mShellRight);
    MR::requestMovementOn(mShellHip);
    MR::requestMovementOn(mShellCenter);
    mShellLeft->requestMovementOnModel();
    mShellRight->requestMovementOnModel();
    mShellHip->requestMovementOnModel();
    mShellCenter->requestMovementOnModel();
    if (!MR::isDead(mPetal)) {
        MR::requestMovementOn(mPetal);
    }
    if (!MR::isDead(mTail)) {
        MR::requestMovementOn(mTail);
    }
}

void DinoPackunBaby::startActionShells(const char* pName) {
    MR::startAction(mShellLeft, pName);
    MR::startAction(mShellRight, pName);
    MR::startAction(mShellHip, pName);
    MR::startAction(mShellCenter, pName);
}

bool DinoPackunBaby::breakShellLeft(HitSensor* pSensor) {
    if (MR::isDead(mShellLeft)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellLeft")) {
        mShellLeft->kill();
        MR::invalidateHitSensor(this, "ShellLeft");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakShellRight(HitSensor* pSensor) {
    if (MR::isDead(mShellRight)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellRight")) {
        mShellRight->kill();
        MR::invalidateHitSensor(this, "ShellRight");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakShellHip(HitSensor* pSensor) {
    if (MR::isDead(mShellHip)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellHip")) {
        mShellHip->kill();
        mTail->appear();
        MR::invalidateHitSensor(this, "ShellHip");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakAllShells() {
    bool isLeft = breakShellLeft(nullptr);
    bool isRight = breakShellRight(nullptr);
    bool isHip = breakShellHip(nullptr);
    mTail->kill();
    if (!MR::isDead(mShellCenter)) {
        mShellCenter->kill();
    }
    if (isLeft || isRight || isHip) {
        return true;
    }
    return false;
}

void DinoPackunBaby::startBreakShells(const char* pName) {
    mShellLeft->startBreak(pName);
    mShellRight->startBreak(pName);
    mShellHip->startBreak(pName);
    mShellCenter->startBreak(pName);
}

void DinoPackunBaby::updateShellsMtx() {
    mShellLeft->updateModelMtx();
    mShellRight->updateModelMtx();
    mShellHip->updateModelMtx();
    mShellCenter->updateModelMtx();
}

void DinoPackunBaby::restoreShells() {
    mShellLeft->appear();
    mShellRight->appear();
    mShellHip->appear();
    mShellCenter->appear();
    mTail->kill();
    MR::validateHitSensor(this, "ShellLeft");
    MR::validateHitSensor(this, "ShellRight");
    MR::validateHitSensor(this, "ShellHip");
}

bool DinoPackunBaby::isShellsAllAlive() {
    if (!MR::isDead(mShellLeft) && !MR::isDead(mShellRight) && !MR::isDead(mShellHip)) {
        return true;
    }
    return false;
}

bool DinoPackunBaby::isShellLeftDead() {
    return MR::isDead(mShellLeft);
}

bool DinoPackunBaby::isShellRightDead() {
    return MR::isDead(mShellRight);
}

bool DinoPackunBaby::isShellHipDead() {
    return MR::isDead(mShellHip);
}

bool DinoPackunBaby::isShellsAllDead() {
    if (MR::isDead(mShellLeft) && MR::isDead(mShellRight) && MR::isDead(mShellHip)) {
        return true;
    }
    return false;
}

bool DinoPackunBaby::isShellSensor(HitSensor* pSensor) {
    if (MR::isEqualSensor(pSensor, this, "ShellLeft") || MR::isEqualSensor(pSensor, this, "ShellRight") ||
        MR::isEqualSensor(pSensor, this, "ShellHip") || MR::isEqualSensor(pSensor, this, "ShellCenter")) {
        return true;
    }
    return false;
}

void DinoPackunBaby::appearPetalDownLand() {
    mPetal->makeActorAppeared();
    MR::startAction(mPetal, "DownLand");
    MR::hideMaterial(this, "DinoPackunBabyPetalMat1_v");
}

void DinoPackunBaby::appearPetalDeath() {
    mPetal->makeActorAppeared();
    MR::startAction(mPetal, "DemoDeath");
    MR::hideMaterial(this, "DinoPackunBabyPetalMat2_v");
}

void DinoPackunBaby::killPetal() {
    mPetal->makeActorDead();
}

void DinoPackunBaby::hitReaction() {
    MR::zeroVelocity(this);
    mScaleController->startHitReaction();
}

void DinoPackunBaby::initDemoCamera(const char* pName) {
    MR::initAnimCamera(this, mCameraInfo, pName);
}

bool DinoPackunBaby::tryStartDemoWithCamera(const char* pName, const char* pPlayerBck, s32 frame) {
    bool isStart;
    if (pPlayerBck != nullptr) {
        isStart = MR::tryStartDemoMarioPuppetable(this, pName);
    } else {
        isStart = MR::tryStartDemo(this, pName);
    }
    if (isStart) {
        MR::zeroVelocity(this);
        TPos3f pos;
        MR::findNamePos("ベビーディノパックンデモ位置", pos);
        mFront.set(pos.mMtx[0][2], pos.mMtx[1][2], pos.mMtx[2][2]);
        MR::setPosition(this, pos.mMtx[0][3], pos.mMtx[1][3], pos.mMtx[2][3]);
        if (pPlayerBck != nullptr) {
            MR::setPlayerPos("ベビーディノパックンデモ位置");
        } else {
            MR::setPlayerPos("プレイヤーデモ位置（ベビーディノパックン戦）");
        }
        MR::startAnimCameraTargetSelf(this, mCameraInfo, pName, frame, false, 1.0f);
        return true;
    }
    return false;
}

bool DinoPackunBaby::tryEndDemoWithCamera(const char* pName, bool isPuppetEnd) {
    bool isEnd;
    if (isPuppetEnd) {
        isEnd = MR::tryEndDemoAndAnimCamera(this, mCameraInfo, pName, 0);
    } else {
        isEnd = MR::endDemoAndAnimCameraIfActionEnd(this, mCameraInfo, pName, 0);
    }
    if (isEnd) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::tryForceDeleteEffectAll(mShellLeft);
            MR::tryForceDeleteEffectAll(mShellRight);
            MR::tryForceDeleteEffectAll(mShellHip);
            MR::tryForceDeleteEffectAll(mShellCenter);
            MR::tryForceDeleteEffectAll(mPetal);
            MR::tryForceDeleteEffectAll(mTail);
            mShellLeft->killModel();
            mShellRight->killModel();
            mShellHip->killModel();
            mShellCenter->killModel();
        }
        return true;
    }
    return false;
}

void DinoPackunBaby::initMultiCamera(const char* pName) {
    MR::initMultiActorCameraNoInit(this, mCameraInfo, pName);
}

void DinoPackunBaby::startMultiCamera(const char* pName) {
    MR::startMultiActorCameraTargetPlayer(this, mCameraInfo, pName, -1);
}

void DinoPackunBaby::endMultiCamera(const char* pName) {
    MR::endMultiActorCamera(this, mCameraInfo, pName, false, -1);
}

#define DECL_NRV(name)                                                                                                                               \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                                   \
        static name sInstance;                                                                                                                       \
    }
#define DEF_NRV(name, exe_name)                                                                                                                      \
    inline void name::execute(Spine* pSpine) const {                                                                                                 \
        reinterpret_cast< DinoPackunBabyBattle* >(pSpine->mExecutor)->exe##exe_name();                                                               \
    }
#define GET_NRV(name) (&NrvDinoPackunBabyBattle::DinoPackunBabyBattleNrv##name::sInstance)

namespace NrvDinoPackunBabyBattle {
    DECL_NRV(DinoPackunBabyBattleNrvWander);
    DECL_NRV(DinoPackunBabyBattleNrvSearch);
    DECL_NRV(DinoPackunBabyBattleNrvNotice);
    DECL_NRV(DinoPackunBabyBattleNrvChase);
    DECL_NRV(DinoPackunBabyBattleNrvAttackJumpSign);
    DECL_NRV(DinoPackunBabyBattleNrvAttackJump);
    DECL_NRV(DinoPackunBabyBattleNrvAttackJumpLand);
    DECL_NRV(DinoPackunBabyBattleNrvAttack2ndJumpSign);
    DECL_NRV(DinoPackunBabyBattleNrvAttack2ndJump);
    DECL_NRV(DinoPackunBabyBattleNrvAttack2ndJumpLand);
    DECL_NRV(DinoPackunBabyBattleNrvAttackSuccess);
    DECL_NRV(DinoPackunBabyBattleNrvDamageShell);
    DECL_NRV(DinoPackunBabyBattleNrvDamageShellLand);
    DECL_NRV(DinoPackunBabyBattleNrvEscapeStart);
    DECL_NRV(DinoPackunBabyBattleNrvEscapeLand);
    DECL_NRV(DinoPackunBabyBattleNrvEscape);
    DECL_NRV(DinoPackunBabyBattleNrvEscapeTurnStart);
    DECL_NRV(DinoPackunBabyBattleNrvEscapeTurn);
    DECL_NRV(DinoPackunBabyBattleNrvDown);
    DECL_NRV(DinoPackunBabyBattleNrvDownLand);
};  // namespace NrvDinoPackunBabyBattle

DinoPackunBabyBattle::DinoPackunBabyBattle(DinoPackunBaby* pHost, bool isFirstHalf)
    : ActorStateBase< DinoPackunBaby >(isFirstHalf ? "Phase[戦闘前半]" : "Phase[戦闘後半]", pHost), mIsFirstHalf(isFirstHalf),
      mIsAttackHit(false), mWanderStep(120), mWanderTime(nullptr), mJumpTime(nullptr), mChaseJumpSpeed(nullptr), mLandChaseTime(nullptr),
      mLandAttackTime(nullptr), mDownDeathTime(nullptr), mShellDamageSpeed(nullptr), mEscapeJumpSpeed(nullptr), mDownSpeed(nullptr),
      mWanderSight(nullptr), mChaseSight(nullptr), mEscapeSight(nullptr) {
}

void DinoPackunBabyBattle::init() {
    initNerve(GET_NRV(Chase), 24);
    if (mIsFirstHalf) {
        setNerve(GET_NRV(Wander));
    }
    if (mIsFirstHalf) {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Wander"), GET_NRV(Wander), "Wander");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Search"), GET_NRV(Search), "Search");
    }
    MR::initActorState(this, new DinoPackunBabyState(mHost, "Notice"), GET_NRV(Notice), "Notice");
    if (mIsFirstHalf) {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Chase1st"), GET_NRV(Chase), "Chase");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJumpSign1st"), GET_NRV(AttackJumpSign), "AttackJumpSign");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJump1st"), GET_NRV(AttackJump), "AttackJump");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJumpLand1st"), GET_NRV(AttackJumpLand), "AttackJumpLand");
    } else {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Chase2nd"), GET_NRV(Chase), "Chase");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJumpSign2nd"), GET_NRV(AttackJumpSign), "AttackJumpSign");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJump2nd"), GET_NRV(AttackJump), "AttackJump");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJumpLand2nd"), GET_NRV(AttackJumpLand), "AttackJumpLand");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Attack2ndJumpSign2nd"), GET_NRV(Attack2ndJumpSign), "Attack2ndJumpSign");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJump2nd"), GET_NRV(Attack2ndJump), "Attack2ndJump");
        MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackJumpLand2nd"), GET_NRV(Attack2ndJumpLand), "Attack2ndJumpLand");
    }
    MR::initActorState(this, new DinoPackunBabyState(mHost, "AttackSuccess"), GET_NRV(AttackSuccess), "AttackSuccess");
    MR::initActorState(this, new DinoPackunBabyState(mHost, "DamageShell"), GET_NRV(DamageShell), "DamageShell");
    MR::initActorState(this, new DinoPackunBabyState(mHost, "DamageShellLand"), GET_NRV(DamageShellLand), "DamageShellLand");
    if (mIsFirstHalf) {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "EscapeStart1st"), GET_NRV(EscapeStart), "EscapeStart");
    } else {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "EscapeStart2nd"), GET_NRV(EscapeStart), "EscapeStart");
    }
    MR::initActorState(this, new DinoPackunBabyState(mHost, "EscapeLand"), GET_NRV(EscapeLand), "EscapeLand");
    if (mIsFirstHalf) {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Escape1st"), GET_NRV(Escape), "Escape");
    } else {
        MR::initActorState(this, new DinoPackunBabyState(mHost, "Escape2nd"), GET_NRV(Escape), "Escape");
    }
    MR::initActorState(this, new DinoPackunBabyState(mHost, "EscapeTurnStart"), GET_NRV(EscapeTurnStart), "EscapeTurnStart");
    MR::initActorState(this, new DinoPackunBabyState(mHost, "EscapeTurn"), GET_NRV(EscapeTurn), "EscapeTurn");
    MR::initActorState(this, new DinoPackunBabyState(mHost, "Down"), GET_NRV(Down), "Down");
    MR::initActorState(this, new DinoPackunBabyState(mHost, "DownLand"), GET_NRV(DownLand), "DownLand");
    if (mIsFirstHalf) {
        mWanderTime = MR::getActorParamS32(mHost, "うろつき時の発見までの時間");
    }
    if (mIsFirstHalf) {
        mJumpTime = MR::getActorParamS32(mHost, "ジャンプまでの時間1st");
        mChaseJumpSpeed = MR::getActorParamJump(mHost, "追いかけジャンプ速度1st");
        mLandChaseTime = MR::getActorParamS32(mHost, "着地から追いかけまでの時間1st");
    } else {
        mJumpTime = MR::getActorParamS32(mHost, "ジャンプまでの時間2nd");
        mChaseJumpSpeed = MR::getActorParamJump(mHost, "追いかけジャンプ速度2nd");
        mLandChaseTime = MR::getActorParamS32(mHost, "着地から追いかけまでの時間2nd");
        mLandAttackTime = MR::getActorParamS32(mHost, "着地から連続攻撃までの時間2nd");
        mDownDeathTime = MR::getActorParamS32(mHost, "ダウンから死亡デモ時間2nd");
    }
    mShellDamageSpeed = MR::getActorParamJump(mHost, "シェルダメージ速度");
    mEscapeJumpSpeed = MR::getActorParamJump(mHost, "逃げ時のジャンプ速度");
    mDownSpeed = MR::getActorParamJump(mHost, "ダウン速度");
    mWanderSight = MR::getActorParamSight(mHost, "WanderSight");
    mChaseSight = MR::getActorParamSight(mHost, "ChaseSight");
    mEscapeSight = MR::getActorParamSight(mHost, "EscapeSight");
    mHost->initMultiCamera("戦闘開始のうろつき");
    kill();
}

void DinoPackunBabyBattle::appear() {
    mIsDead = false;
    if (mIsFirstHalf) {
        MR::startAction(mHost, "WanderNoInterpole");
        setNerve(GET_NRV(Wander));
        return;
    }
    MR::startAction(mHost, "ChaseNoInterpole");
    setNerve(GET_NRV(Chase));
}

void DinoPackunBabyBattle::exeWander() {
    if (MR::isFirstStep(this) && mWanderStep > 0) {
        mHost->startMultiCamera("戦闘開始のうろつき");
    }
    if (MR::isStep(this, 60)) {
        endChaseCamera();
    }
    MR::updateActorState(this);
    if (MR::isStep(this, 20)) {
        MR::startActionSound(mHost, "WanderVoice", -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, *mWanderTime) && MR::isGreaterEqualStep(this, mWanderStep) && mHost->isInSight(mWanderSight)) {
        endChaseCamera();
        setNerve(GET_NRV(Notice));
        return;
    }
    if (MR::isActorStateDead(this)) {
        mWanderStep = 0;
        setNerve(GET_NRV(Search));
    }
}

void DinoPackunBabyBattle::exeSearch() {
    MR::updateActorState(this);
    if (mHost->isInSight(mWanderSight)) {
        setNerve(GET_NRV(Notice));
        return;
    }
    if (MR::isActorStateDead(this)) {
        setNerve(GET_NRV(Wander));
    }
}

void DinoPackunBabyBattle::exeNotice() {
    MR::updateActorState(this);
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NRV(Chase));
    }
}

void DinoPackunBabyBattle::exeChase() {
    MR::updateActorState(this);
    if (MR::isStep(this, 3)) {
        MR::startActionSound(mHost, "ChaseVoice", -1, -1, -1);
    }
    if (mHost->isInSight(mChaseSight)) {
        if (MR::isGreaterEqualStep(this, *mJumpTime)) {
            setNerve(GET_NRV(AttackJumpSign));
        }
    } else if (MR::isActorStateDead(this)) {
        setNerve(GET_NRV(AttackJumpSign));
    }
}

void DinoPackunBabyBattle::exeAttackJumpSign() {
    if (MR::updateActorState(this)) {
        if (isNerve(GET_NRV(Attack2ndJumpSign))) {
            setNerve(GET_NRV(Attack2ndJump));
            return;
        }
        setNerve(GET_NRV(AttackJump));
    }
}

void DinoPackunBabyBattle::exeAttackJump() {
    if (MR::isFirstStep(this)) {
        fn_800127A0(mHost, &mHost->mFront, mChaseJumpSpeed);
    }
    if (mHost->updateJumpState(this)) {
        MR::tryHitReaction(mHost, "追いかけジャンプ着地");
        if (mIsAttackHit) {
            setNerve(GET_NRV(AttackSuccess));
            return;
        }
        if (isNerve(GET_NRV(Attack2ndJump))) {
            setNerve(GET_NRV(Attack2ndJumpLand));
            return;
        }
        setNerve(GET_NRV(AttackJumpLand));
    }
}

void DinoPackunBabyBattle::exeAttackJumpLand() {
    MR::updateActorState(this);
    if (!mIsFirstHalf && !isNerve(GET_NRV(Attack2ndJumpLand)) && MR::isGreaterEqualStep(this, *mLandAttackTime)) {
        setNerve(GET_NRV(Attack2ndJumpSign));
        return;
    }
    if (MR::isGreaterEqualStep(this, *mLandChaseTime) && mHost->isInSight(mWanderSight)) {
        setNerve(GET_NRV(Notice));
        return;
    }
    if (MR::isActorStateDead(this)) {
        if (mIsFirstHalf) {
            setNerve(GET_NRV(Wander));
            return;
        }
        setNerve(GET_NRV(Chase));
    }
}

void DinoPackunBabyBattle::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        mIsAttackHit = false;
    }
    if (MR::updateActorState(this)) {
        if (mIsFirstHalf) {
            setNerve(GET_NRV(Wander));
            return;
        }
        setNerve(GET_NRV(Chase));
    }
}

void DinoPackunBabyBattle::exeDamageShellLand() {
    MR::updateActorState(this);
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NRV(Notice));
    }
}

void DinoPackunBabyBattle::exeEscapeStart() {
    if (MR::isFirstStep(this)) {
        MR::onSwitchB(mHost);
    }
    if (mHost->updateJumpState(this)) {
        setNerve(GET_NRV(EscapeLand));
    }
}

void DinoPackunBabyBattle::exeEscapeLand() {
    MR::updateActorState(this);
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NRV(Escape));
    }
}

void DinoPackunBabyBattle::exeEscape() {
    MR::updateActorState(this);
    if (mHost->isInSight(mEscapeSight)) {
        setNerve(GET_NRV(EscapeTurnStart));
    }
}

void DinoPackunBabyBattle::exeEscapeTurnStart() {
    if (MR::isLessEqualStep(this, 30)) {
        MR::turnDirectionFromPlayerDegree(mHost, &mHost->mFront, 6.0f);
    }
    MR::updateActorState(this);
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NRV(EscapeTurn));
    }
}

void DinoPackunBabyBattle::exeDown() {
    if (MR::isFirstStep(this)) {
        fn_800126F0(mHost, &mHost->mFront, mDownSpeed);
    }
    MR::updateActorState(this);
    if (!mIsFirstHalf) {
        MR::stopStageBGM(0);
    }
    if (!mIsFirstHalf) {
        if (MR::isGreaterEqualStep(this, *mDownDeathTime)) {
            MR::zeroVelocity(mHost);
            kill();
            return;
        }
    }
    if ((!MR::isBinded(mHost) || !MR::sendMsgEnemyAttackToBindedSensor(mHost, mHost->getSensor("Body"))) &&
        MR::reboundVelocityFromCollision(mHost, 0.0f, 3.0f, 1.0f)) {
        MR::tryHitReactionAtPos(mHost, "ダウン着地", &mHost->mPosition);
        MR::zeroVelocity(mHost);
        setNerve(GET_NRV(DownLand));
    }
}

void DinoPackunBabyBattle::exeDownLand() {
    if (MR::isFirstStep(this)) {
        mHost->appearPetalDownLand();
    }
    MR::updateActorState(this);
    if (MR::isActionEnd(mHost)) {
        mHost->killPetal();
        kill();
    }
}

void DinoPackunBabyBattle::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NRV(Chase)) || isNerve(GET_NRV(Notice)) || isNerve(GET_NRV(AttackJumpSign)) || isNerve(GET_NRV(AttackJump)) ||
        isNerve(GET_NRV(Attack2ndJumpSign)) || isNerve(GET_NRV(Attack2ndJump)) || isNerve(GET_NRV(AttackJumpLand)) ||
        isNerve(GET_NRV(DamageShellLand))) {
        if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            MR::zeroVelocity(mHost);
            endChaseCamera();
            if (isNerve(GET_NRV(AttackJump)) || isNerve(GET_NRV(Attack2ndJump))) {
                mIsAttackHit = true;
                return;
            }
            setNerve(GET_NRV(AttackSuccess));
            return;
        }
    }
    if (MR::isSensorEnemyAttack(pSender) && MR::isSensorMapObj(pReceiver)) {
        MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
    if ((MR::isEqualSensor(pSender, mHost, "Body") || MR::isEqualSensor(pSender, mHost, "HeadAttack") ||
         MR::isEqualSensor(pSender, mHost, "LegL") || MR::isEqualSensor(pSender, mHost, "LegR")) &&
        !MR::isPlayerHipDropFalling()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool DinoPackunBabyBattle::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceAttack(msg)) {
        MR::tryHitReactionBetweenSensors(mHost, "スターピース反応", pSender, pReceiver);
        mHost->hitReaction();
        return true;
    }
    if (isNerve(GET_NRV(EscapeStart)) || isNerve(GET_NRV(DamageShell)) || isNerve(GET_NRV(Down))) {
        return false;
    }
    if (((MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !mHost->isPlayerInFront()) || MR::isMsgPlayerHipDrop(msg)) &&
        ((isNerve(GET_NRV(EscapeStart)) && MR::isGreaterEqualStep(this, 30)) || isNerve(GET_NRV(EscapeLand)) || isNerve(GET_NRV(Escape)) ||
         isNerve(GET_NRV(EscapeTurnStart)) || isNerve(GET_NRV(EscapeTurn)))) {
        if (MR::isEqualSensor(pReceiver, mHost, "Body")) {
            MR::tryHitReactionBetweenSensors(mHost, "ダウン開始", pSender, pReceiver);
            setNerve(GET_NRV(Down));
            return true;
        }
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !mHost->isPlayerInFront()) {
        if (mHost->breakShellLeft(pReceiver) || mHost->breakShellRight(pReceiver) || mHost->breakShellHip(pReceiver)) {
            if (mHost->isShellsAllDead()) {
                mHost->breakAllShells();
                MR::tryHitReactionBetweenSensors(mHost, "全シェル破壊（スピン）", pSender, pReceiver);
                fn_80012B90(mHost, mEscapeJumpSpeed[0], mEscapeJumpSpeed[1], pSender, pReceiver, 4);
                setNerve(GET_NRV(EscapeStart));
                return true;
            }
            MR::tryHitReactionBetweenSensors(mHost, "シェル破壊（スピン）", pSender, pReceiver);
            fn_80012B90(mHost, mShellDamageSpeed[0], mShellDamageSpeed[1], pSender, pReceiver, 4);
            endChaseCamera();
            setNerve(GET_NRV(DamageShell));
            return true;
        }
        if (((MR::isEqualSensor(pReceiver, mHost, "BodyLeft") && mHost->isShellLeftDead()) ||
             (MR::isEqualSensor(pReceiver, mHost, "BodyRight") && mHost->isShellRightDead()) ||
             (MR::isEqualSensor(pReceiver, mHost, "BodyHip") && mHost->isShellHipDead())) &&
            !isNerve(GET_NRV(Escape)) && !isNerve(GET_NRV(EscapeLand)) && !isNerve(GET_NRV(EscapeTurnStart)) &&
            !isNerve(GET_NRV(EscapeTurn)) && !isNerve(GET_NRV(Down)) && !isNerve(GET_NRV(DownLand)) && !mHost->isShellsAllAlive()) {
            mHost->breakAllShells();
            fn_80012B90(mHost, mEscapeJumpSpeed[0], mEscapeJumpSpeed[1], pSender, pReceiver, 4);
            MR::tryHitReactionBetweenSensors(mHost, "素肌ダメージ（スピン）", pSender, pReceiver);
            setNerve(GET_NRV(EscapeStart));
            return true;
        }
    }
    if (MR::isMsgPlayerHipDrop(msg) && mHost->isShellSensor(pReceiver) && mHost->breakAllShells()) {
        fn_80012B90(mHost, mEscapeJumpSpeed[0], mEscapeJumpSpeed[1], pSender, pReceiver, 4);
        MR::tryHitReactionBetweenSensors(mHost, "シェル破壊（ヒップドロップ）", pSender, pReceiver);
        endChaseCamera();
        setNerve(GET_NRV(EscapeStart));
        return true;
    }
    return false;
}

void DinoPackunBabyBattle::control() {
    if (!mIsFirstHalf) {
        if (isNerve(GET_NRV(Notice)) || isNerve(GET_NRV(Chase)) || isNerve(GET_NRV(Wander))) {
            MR::tryEmitEffect(mHost, "AngrySmoke");
            return;
        }
        MR::tryDeleteEffect(mHost, "AngrySmoke");
    }
}

void DinoPackunBabyBattle::endChaseCamera() {
    if (mWanderStep > 0) {
        mHost->endMultiCamera("戦闘開始のうろつき");
        mWanderStep = 0;
    }
}

DinoPackunBabyBattle::~DinoPackunBabyBattle() {
}

// Executes the target inlines into the nerve (state update + transition).
inline void DinoPackunBabyBattle::exeDamageShell() {
    if (mHost->updateJumpState(this)) {
        setNerve(GET_NRV(DamageShellLand));
    }
}

inline void DinoPackunBabyBattle::exeEscapeTurn() {
    MR::updateActorState(this);
    if (MR::isActorStateDead(this)) {
        setNerve(GET_NRV(Escape));
    }
}

namespace NrvDinoPackunBabyBattle {
    DEF_NRV(DinoPackunBabyBattleNrvWander, Wander)
    DEF_NRV(DinoPackunBabyBattleNrvSearch, Search)
    DEF_NRV(DinoPackunBabyBattleNrvNotice, Notice)
    DEF_NRV(DinoPackunBabyBattleNrvChase, Chase)
    DEF_NRV(DinoPackunBabyBattleNrvAttackJumpSign, AttackJumpSign)
    DEF_NRV(DinoPackunBabyBattleNrvAttackJump, AttackJump)
    DEF_NRV(DinoPackunBabyBattleNrvAttackJumpLand, AttackJumpLand)
    DEF_NRV(DinoPackunBabyBattleNrvAttack2ndJumpSign, AttackJumpSign)
    DEF_NRV(DinoPackunBabyBattleNrvAttack2ndJump, AttackJump)
    DEF_NRV(DinoPackunBabyBattleNrvAttack2ndJumpLand, AttackJumpLand)
    DEF_NRV(DinoPackunBabyBattleNrvAttackSuccess, AttackSuccess)
    DEF_NRV(DinoPackunBabyBattleNrvDamageShell, DamageShell)
    DEF_NRV(DinoPackunBabyBattleNrvDamageShellLand, DamageShellLand)
    DEF_NRV(DinoPackunBabyBattleNrvEscapeStart, EscapeStart)
    DEF_NRV(DinoPackunBabyBattleNrvEscapeLand, EscapeLand)
    DEF_NRV(DinoPackunBabyBattleNrvEscape, Escape)
    DEF_NRV(DinoPackunBabyBattleNrvEscapeTurnStart, EscapeTurnStart)
    DEF_NRV(DinoPackunBabyBattleNrvEscapeTurn, EscapeTurn)
    DEF_NRV(DinoPackunBabyBattleNrvDown, Down)
    DEF_NRV(DinoPackunBabyBattleNrvDownLand, DownLand)

    DinoPackunBabyBattleNrvWander DinoPackunBabyBattleNrvWander::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvSearch DinoPackunBabyBattleNrvSearch::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvNotice DinoPackunBabyBattleNrvNotice::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvChase DinoPackunBabyBattleNrvChase::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttackJumpSign DinoPackunBabyBattleNrvAttackJumpSign::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttackJump DinoPackunBabyBattleNrvAttackJump::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttackJumpLand DinoPackunBabyBattleNrvAttackJumpLand::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttack2ndJumpSign DinoPackunBabyBattleNrvAttack2ndJumpSign::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttack2ndJump DinoPackunBabyBattleNrvAttack2ndJump::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttack2ndJumpLand DinoPackunBabyBattleNrvAttack2ndJumpLand::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvAttackSuccess DinoPackunBabyBattleNrvAttackSuccess::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvDamageShell DinoPackunBabyBattleNrvDamageShell::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvDamageShellLand DinoPackunBabyBattleNrvDamageShellLand::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvEscapeStart DinoPackunBabyBattleNrvEscapeStart::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvEscapeLand DinoPackunBabyBattleNrvEscapeLand::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvEscape DinoPackunBabyBattleNrvEscape::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvEscapeTurnStart DinoPackunBabyBattleNrvEscapeTurnStart::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvEscapeTurn DinoPackunBabyBattleNrvEscapeTurn::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvDown DinoPackunBabyBattleNrvDown::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyBattleNrvDownLand DinoPackunBabyBattleNrvDownLand::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvDinoPackunBabyBattle
