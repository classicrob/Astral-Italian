#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleCamera.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util.hpp"

#pragma auto_inline off

namespace NrvKoopaStateLastBattle {
    NEW_NERVE(KoopaStateLastBattleNrvStart, KoopaStateLastBattle, Start);

    class KoopaStateLastBattleNrvChase : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaStateLastBattleNrvChase sInstance;
    };
    KoopaStateLastBattleNrvChase KoopaStateLastBattleNrvChase::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KoopaStateLastBattleNrvDamage, KoopaStateLastBattle, Damage);
    NEW_NERVE(KoopaStateLastBattleNrvAttack, KoopaStateLastBattle, Attack);

    class KoopaStateLastBattleNrvAttackRush : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaStateLastBattleNrvAttackRush sInstance;
    };
    KoopaStateLastBattleNrvAttackRush KoopaStateLastBattleNrvAttackRush::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaStateLastBattle

#define SINST(name) NrvKoopaStateLastBattle::KoopaStateLastBattleNrv##name::sInstance

KoopaStateLastBattle::KoopaStateLastBattle(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ最終バトルフェーズ", pHost), mChase(nullptr), mDamage(nullptr), mAttack(nullptr), mAttackRush(nullptr),
      mCamera(nullptr), mPhase(1) {
    initNerve(&SINST(Chase), 4);
    mChase = new KoopaStateLastChase(mHost);
    MR::initActorState(this, mChase, &SINST(Chase), "Chase");
    mDamage = new KoopaStateLastDamage(mHost);
    MR::initActorState(this, mDamage, &SINST(Damage), "Damage");
    mAttack = new KoopaStateLastAttack(mHost);
    MR::initActorState(this, mAttack, &SINST(Attack), "Attack");
    mAttackRush = new KoopaStateLastAttack(mHost);
    MR::initActorState(this, mAttackRush, &SINST(AttackRush), "AttackRush");
    mAttack->mIsChaseRoll = false;
    mAttackRush->mIsChaseRoll = true;
    mCamera = new KoopaLastBattleCamera(mHost);
}

void KoopaStateLastBattle::appear() {
    MR::sendMsgKoopaFinalBattleStart();
    MR::killKoopaDemoChamber();
    MR::killKoopaBattlePlanet();
    MR::setKoopaBattleNo2();
    MR::startKoopaBattleDynamicCtrl(mHost, -1);
    MR::setKoopaBattleVolumeScaleDownRate(mHost, 0);
    MR::validateCollisionParts(mHost);
    TQuat4f quat;
    TVec3f pos;
    MR::getKoopaBattlePoseB(&quat, &pos);
    mHost->setPose(quat, pos);
    TVec3f* pPos = &mHost->mPosition;
    MR::calcKoopaBattleWorldPos(pPos, TVec3f(0.0f, 0.0f, -20000.0f));
    MR::setPlayerPos("マリオ最終戦開始位置");
    mIsDead = false;
    MR::zeroVelocity(mHost);
    MR::showPlayer();
    mPhase = 1;
    mCamera->start(0.0f);
    MR::invalidateKoopaBattleCollision(mHost);
    mHost->getDefenseForm()->appear();
    MR::hideModelAndOnCalcAnim(mHost);
    MR::startKoopaBattleFaceCtrl(mHost, 30);
    mChase->setPatternMode(mPhase);
    mChase->_64 = false;
    setNerve(&SINST(Start));
}

void KoopaStateLastBattle::kill() {
    mCamera->end();
    mIsDead = true;
}

void KoopaStateLastBattle::control() {
    mCamera->update();
}

void KoopaStateLastBattle::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(Attack))) {
        mAttack->attackSensor(pSender, pReceiver);
    }
    if (isNerve(&SINST(AttackRush))) {
        mAttackRush->attackSensor(pSender, pReceiver);
    }
}

bool KoopaStateLastBattle::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool KoopaStateLastBattle::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, mHost, "Shell")) {
        if (MR::isMsgAttackRockReflect(msg)) {
            MR::startAction(mHost->mShell, "Hit");
            MR::tryHitReaction(mHost, "甲羅ヒット");
            return true;
        }
        return MR::isMsgKoopaC7(msg);
    }
    if (MR::isEqualSensor(pReceiver, mHost, "Shell2")) {
        if (MR::isMsgAttackRockReflect(msg)) {
            MR::startAction(mHost->getDefenseForm(), "Hit");
            MR::tryHitReaction(mHost, "甲羅ヒット");
            return true;
        }
        return MR::isMsgKoopaC7(msg);
    }
    if (MR::isMsgAttackRockReflect(msg) && isAttackable()) {
        mDamage->setDamageType(pReceiver);
        mDamage->mIsLongDamage = mPhase >= 3;
        mDamage->mIsFinal = mPhase >= 4;
        MR::emitEffectHitBetweenSensors(mHost, pSender, pReceiver, 0.0f, "Hit");
        setNerve(&SINST(Damage));
        return true;
    }
    return false;
}

void KoopaStateLastBattle::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ShellRotate");
    }
    f32 rate = MR::calcNerveEaseOutRate(this, 120);
    TVec3f pos(0.0f);
    MR::vecBlend(&pos, TVec3f(0.0f, 0.0f, -20000.0f), TVec3f(0.0f, 0.0f, -15000.0f), rate);
    MR::calcKoopaBattleWorldPos(&mHost->mPosition, pos);
    MR::startSoundInCameraDistance(mHost, "LastChaseRoll", -1, -1, 31000.0f);
    f32 angle = MR::calcNerveValue(this, 120, 0.0f, 2520.0f);
    MR::rotateQuatXAxisDegree(&mHost->mQuatA, mHost->mQuatB, angle);
    if (MR::isGreaterEqualStep(this, 120)) {
        mChase->_64 = true;
        setNerve(&SINST(Chase));
    }
}

void KoopaStateLastBattle::exeDamage() {
    if (MR::updateActorState(this)) {
        s32 phase = mPhase + 1;
        mPhase = phase;
        if (phase <= 2) {
            mChase->_64 = false;
            mChase->setPatternMode(mPhase);
            setNerve(&SINST(Chase));
            return;
        }
        if (phase <= 4) {
            setNerve(&SINST(AttackRush));
            return;
        }
        kill();
    }
}

void KoopaStateLastBattle::exeAttack() {
    if (MR::updateActorState(this)) {
        mChase->_64 = false;
        setNerve(&SINST(Chase));
    }
}

bool KoopaStateLastBattle::isAttackable() const {
    if (isNerve(&SINST(Chase))) {
        return mChase->isChasing();
    }
    if (isNerve(&SINST(Attack))) {
        return mAttack->isAttacking();
    }
    if (isNerve(&SINST(AttackRush))) {
        return mAttackRush->isAttacking();
    }
    return false;
}

KoopaStateLastBattle::~KoopaStateLastBattle() {}

void NrvKoopaStateLastBattle::KoopaStateLastBattleNrvAttackRush::execute(Spine* pSpine) const {
    MR::updateActorState(reinterpret_cast< KoopaStateLastBattle* >(pSpine->mExecutor));
}

void NrvKoopaStateLastBattle::KoopaStateLastBattleNrvChase::execute(Spine* pSpine) const {
    MR::updateActorStateAndNextNerve(reinterpret_cast< KoopaStateLastBattle* >(pSpine->mExecutor), &SINST(Attack));
}
