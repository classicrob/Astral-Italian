#include "Game/Boss/KoopaJrRobotTU14.hpp"
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateMeteor.hpp"
#include "Game/Boss/KoopaBattleCamera.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

#pragma auto_inline off
// Attack sequences per stage/variant (0 = AttackFire, 1 = AttackMeteor, 2 = AttackQuake).
static s32 sSequenceLv1A[] = {1, 2};
static s32 sSequenceLv1B[] = {1, 0, 2};
static s32 sSequenceLv2A[] = {2, 2, 1};
static s32 sSequenceLv2B[] = {1, 0, 2};
static s32 sSequenceLv3A[] = {2, 2, 1};
static s32 sSequenceLv3B[] = {2, 1, 0};


namespace NrvKoopaBattleStage {
    class KoopaBattleStageNrvBattleStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaBattleStage* actor = reinterpret_cast< KoopaBattleStage* >(pSpine->mExecutor);
            if (MR::updateActorState(actor)) {
                actor->setNextSequence();
            }
        }
        static KoopaBattleStageNrvBattleStart sInstance;
    };
    KoopaBattleStageNrvBattleStart KoopaBattleStageNrvBattleStart::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KoopaBattleStageNrvWait, KoopaBattleStage, Wait);

    class KoopaBattleStageNrvAttackMeteor : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaBattleStage* actor = reinterpret_cast< KoopaBattleStage* >(pSpine->mExecutor);
            if (MR::updateActorState(actor)) {
                actor->setNextSequence();
            }
        }
        static KoopaBattleStageNrvAttackMeteor sInstance;
    };
    KoopaBattleStageNrvAttackMeteor KoopaBattleStageNrvAttackMeteor::sInstance ATTRIBUTE_WEAK;

    class KoopaBattleStageNrvAttackQuake : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaBattleStage* actor = reinterpret_cast< KoopaBattleStage* >(pSpine->mExecutor);
            if (MR::updateActorState(actor)) {
                actor->setNextSequence();
            }
        }
        static KoopaBattleStageNrvAttackQuake sInstance;
    };
    KoopaBattleStageNrvAttackQuake KoopaBattleStageNrvAttackQuake::sInstance ATTRIBUTE_WEAK;

    class KoopaBattleStageNrvAttackFire : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaBattleStage* actor = reinterpret_cast< KoopaBattleStage* >(pSpine->mExecutor);
            if (MR::updateActorState(actor)) {
                actor->setNextSequence();
            }
        }
        static KoopaBattleStageNrvAttackFire sInstance;
    };
    KoopaBattleStageNrvAttackFire KoopaBattleStageNrvAttackFire::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KoopaBattleStageNrvDamage, KoopaBattleStage, Damage);
};  // namespace NrvKoopaBattleStage

KoopaBattleStage::KoopaBattleStage(KoopaBattle* pHost, s32 stage, s32 variant) : ActorStateBase< KoopaBattle >("クッパ戦闘段階", pHost) {
    mStateBattleStart = nullptr;
    mStateAttackFire = nullptr;
    mStateAttackQuake = nullptr;
    mStateAttackMeteor = nullptr;
    mStateDamage = nullptr;
    mStateExtra = nullptr;
    mSequence = nullptr;
    mSequenceCount = 0;
    mSequenceIndex = 0;
    mUnk34 = 2;
    mHp = 2;
    mStage = stage;
    mVariant = 0;
    setSequence(variant);
    initNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvWait::sInstance, 8);

    mStateBattleStart = new KoopaStateBattleStart(mHost);
    MR::initActorState(this, mStateBattleStart, &NrvKoopaBattleStage::KoopaBattleStageNrvBattleStart::sInstance, "BattleStart");
    if (isInSequence(1)) {
        mStateAttackMeteor = new KoopaStateMeteor(mHost);
        MR::initActorState(this, mStateAttackMeteor, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackMeteor::sInstance, "AttackMeteor");
    }
    if (isInSequence(2)) {
        mStateAttackQuake = new KoopaStateAttackQuake(mHost);
        MR::initActorState(this, mStateAttackQuake, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackQuake::sInstance, "AttackQuake");
    }
    if (isInSequence(0)) {
        mStateAttackFire = new KoopaStateFireAttack(mHost);
        MR::initActorState(this, mStateAttackFire, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackFire::sInstance, "AttackFire");
    }
    mStateDamage = new KoopaStateDamage(mHost, variant);
    MR::initActorState(this, mStateDamage, &NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance, "Damage");
    changeSubs(variant);
    mStateExtra = new KoopaBattleCamera(mHost);
}

KoopaBattleStage::~KoopaBattleStage() {}

void KoopaBattleStage::start(bool pFlag) {
    MR::showPlayer();
    MR::resetKoopaBattleToPoseA(getHost());
    MR::appearKoopaBattlePlanet();
    if (mVariant == 0) {
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvBattleStart::sInstance);
        mStateBattleStart->initPosition();
    } else {
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvWait::sInstance);
    }
    if (pFlag) {
        mStateExtra->startFacePlayer();
        return;
    }
    mStateExtra->startFromCamera();
}

void KoopaBattleStage::appear() {
    mSequenceIndex = 0;
    MR::killKoopaDemoChamber();
    MR::startKoopaBattleDynamicCtrl(getHost(), -1);
    MR::setKoopaBattleNo0();
    MR::validateCollisionParts(getHost());
    mIsDead = false;
}

void KoopaBattleStage::kill() {
    mStateExtra->end();
    mIsDead = true;
}

void KoopaBattleStage::control() {
    mStateExtra->update();
}

void KoopaBattleStage::setNextSequence() {
    s32 seq = mSequence[mSequenceIndex];
    switch (seq) {
    case 0:
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvAttackFire::sInstance);
        break;
    case 1:
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvAttackMeteor::sInstance);
        break;
    case 2:
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvAttackQuake::sInstance);
        break;
    }
    s32 next = mSequenceIndex + 1;
    mSequenceIndex = next;
    if (next >= mSequenceCount) {
        mSequenceIndex = 0;
    }
}

void KoopaBattleStage::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "Wait");
        setNextSequence();
    }
}

void KoopaBattleStage::exeDamage() {
    if (MR::updateActorState(this)) {
        if (mHp <= 0) {
            kill();
            return;
        }
        setNextSequence();
    }
}

bool KoopaBattleStage::isAttackable() const {
    if (getHost()->mIsDefenseForm || isNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance)) {
        return false;
    }
    return true;
}

void KoopaBattleStage::setSequence(s32 variant) {
    switch (mStage) {
    case 1:
        setSequenceLv1(variant);
        break;
    case 2:
        setSequenceLv2(variant);
        break;
    case 3:
        setSequenceLv3(variant);
        break;
    }
    mSequenceIndex = 0;
    mVariant = variant;
}

void KoopaBattleStage::setSequenceLv1(s32 variant) {
    switch (variant) {
    case 0:
        mSequence = sSequenceLv1A;
        mSequenceCount = 2;
        return;
    case 1:
        mSequence = sSequenceLv1B;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::setSequenceLv2(s32 variant) {
    switch (variant) {
    case 0:
        mSequence = sSequenceLv2A;
        mSequenceCount = 3;
        return;
    case 1:
        mSequence = sSequenceLv2B;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::setSequenceLv3(s32 variant) {
    switch (variant) {
    case 0:
        mSequence = sSequenceLv3A;
        mSequenceCount = 3;
        return;
    case 1:
        mSequence = sSequenceLv3B;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::changeSub1(s32 variant) {
    KoopaStateAttackQuake* pQuake = mStateAttackQuake;
    if (pQuake != nullptr) {
        pQuake->setMode(1);
    }
    if (mStateAttackFire != nullptr) {
        mStateAttackFire->setFireCount(1);
    }
    if (mStateAttackMeteor != nullptr) {
        mStateAttackMeteor->setMode(1);
    }
    switch (variant) {
    case 0:
        mHp = 2;
        return;
    case 1:
        mHp = 2;
        return;
    }
}

void KoopaBattleStage::changeSub2(s32 variant) {
    KoopaStateAttackQuake* pQuake = mStateAttackQuake;
    if (pQuake != nullptr) {
        pQuake->setMode(2);
    }
    if (mStateAttackFire != nullptr) {
        mStateAttackFire->setFireCount(2);
    }
    if (mStateAttackMeteor != nullptr) {
        mStateAttackMeteor->setMode(2);
    }
    switch (variant) {
    case 0:
        mHp = 2;
        return;
    case 1:
        mHp = 2;
        return;
    }
}

void KoopaBattleStage::changeSub3(s32 variant) {
    switch (variant) {
    case 0:
        mHp = 2;
        if (mStateAttackQuake != nullptr) {
            mStateAttackQuake->setMode(3);
        }
        if (mStateAttackFire != nullptr) {
            mStateAttackFire->setFireCount(3);
        }
        if (mStateAttackMeteor != nullptr) {
            mStateAttackMeteor->setMode(3);
        }
        return;
    case 1:
        mHp = 2;
        if (mStateAttackQuake != nullptr) {
            mStateAttackQuake->setMode(4);
        }
        if (mStateAttackFire != nullptr) {
            mStateAttackFire->setFireCount(3);
        }
        if (mStateAttackMeteor != nullptr) {
            mStateAttackMeteor->setMode(3);
        }
        break;
    }
}

void KoopaBattleStage::changeSubs(s32 variant) {
    switch (mStage) {
    case 1:
        changeSub1(variant);
        break;
    case 2:
        changeSub2(variant);
        break;
    case 3:
        changeSub3(variant);
        break;
    }
}

bool KoopaBattleStage::isInSequence(s32 id) const {
    for (s32 i = 0; i < mSequenceCount; i++) {
        if (id == mSequence[i]) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: the target compares the (unused) result of MR::sendMsgKoopaPunchAttack (cmpwi, no branch); ours drops it
void KoopaBattleStage::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, getHost(), "HandR")) {
        if (MR::isSensorPlayer(pReceiver)) {
            MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender);
        } else if (MR::sendMsgKoopaPunchAttack(pReceiver, pSender)) {
            return;
        }
    }
}

bool KoopaBattleStage::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool KoopaBattleStage::receiveOtherMsg(u32 msg, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (MR::isEqualSensor(pSensorB, getHost(), "Shell")) {
        if (MR::isMsgAttackRockReflect(msg)) {
            MR::startAction(reinterpret_cast< LiveActor* >(getHost()->mShell), "Hit");
            MR::tryHitReaction(getHost(), "甲羅ヒット");
            return true;
        }
        return MR::isMsgKoopaC7(msg);
    }
    if (MR::isMsgAttackRockReflect(msg)) {
        if (isAttackable()) {
            mHp -= 1;
            if (mHp == 0) {
                MR::stopStageBGM(0);
            }
            MR::emitEffectHitBetweenSensors(getHost(), pSensorA, pSensorB, 0.0f, "Hit");
            mStateDamage->setDamageType(pSensorB);
            mStateDamage->setFinal(mHp == 0);
            MR::emitItemBossDamageAtPos(getHost(), MR::getSensorPos(pSensorA));
            setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance);
            return true;
        }
    }
    return false;
}
