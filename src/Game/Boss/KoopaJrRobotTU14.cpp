#pragma auto_inline off
#include "Game/Boss/KoopaJrRobotTU14.hpp"
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

// Data labels of the sequence tables, names and action strings (SB4E01 .data 0x80676980..0x80676B70).
extern "C" s32 lbl_80676980[];
extern "C" s32 lbl_80676988[];
extern "C" s32 lbl_80676994[];
extern "C" s32 lbl_806769A0[];
extern "C" s32 lbl_806769AC[];
extern "C" s32 lbl_806769B8[];
extern "C" const char lbl_80676A10[];
extern "C" const char lbl_80676A18[];
extern "C" const char lbl_80676A20[];
extern "C" const char lbl_80676A28[];
extern "C" const char lbl_807CF5E0;
extern "C" f32 lbl_807D8630;

// PLACEHOLDER(objects): unnamed core helpers used by the battle stage (names not resolved yet)
extern "C" void fn_800CBBE0();
extern "C" void fn_800CBCA0();
extern "C" void fn_800CBBB0(LiveActor*, s32);
extern "C" LiveActor* fn_800CBFE0();
extern "C" void fn_800CB7D0(LiveActor*);
extern "C" void fn_800C9980(void*);
extern "C" void fn_800C9A30(void*);
extern "C" void fn_800C9AC0(void*);
extern "C" void fn_800C9B00(void*);
extern "C" void* fn_800C9900(void*, LiveActor*);
extern "C" bool fn_800CC250(u32);
extern "C" bool fn_800CC210(HitSensor*, HitSensor*);
extern "C" void fn_8022BBC0(LiveActor*, const TVec3f*);
// PLACEHOLDER(objects): unnamed son-koopa state ctors / methods (KoopaStateBase family, other TUs), resolve when named
extern "C" ActorStateBaseInterface* fn_800E75B0(void*, KoopaBattle*);
extern "C" ActorStateBaseInterface* fn_800E68C0(void*, KoopaBattle*);
extern "C" ActorStateBaseInterface* fn_800E6CE0(void*, KoopaBattle*);
extern "C" ActorStateBaseInterface* fn_800E93A0(void*, KoopaBattle*);
extern "C" ActorStateBaseInterface* fn_800E7950(void*, KoopaBattle*, s32);
extern "C" void fn_800E6F80(ActorStateBaseInterface*, s32);
extern "C" void fn_800E6960(ActorStateBaseInterface*, s32);
extern "C" void fn_800E9520(ActorStateBaseInterface*, s32);
extern "C" void fn_800E7A20(ActorStateBaseInterface*, HitSensor*);
extern "C" void fn_800E7AD0(ActorStateBaseInterface*, bool);

// PLACEHOLDER(objects): vslot 0x20 (no args) of the BattleStart state object; resolve when that state class is named
class KoopaBattleStageStartSlot : public ActorStateBaseInterface {
public:
    virtual void slot20();
};

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

// NONMATCHING: the SB4E01 ctor keeps lbl_80676980 in r31 across the whole body (one lis/addi, CSE) and stores the host before the vptr; ours recomputes the base per use and the register assignment differs (0x268 vs 0x22c)
KoopaBattleStage::KoopaBattleStage(KoopaBattle* pHost, s32 stage, s32 variant) : ActorStateBase< KoopaBattle >((const char*)lbl_80676980 + 0x44, pHost) {
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

    void* mem = ::operator new(0x28);
    mStateBattleStart = mem ? fn_800E75B0(mem, mHost) : (ActorStateBaseInterface*)mem;
    MR::initActorState(this, mStateBattleStart, &NrvKoopaBattleStage::KoopaBattleStageNrvBattleStart::sInstance, (const char*)lbl_80676980 + 0x54);
    if (isInSequence(1)) {
        mem = ::operator new(0x20);
        mStateAttackMeteor = mem ? fn_800E68C0(mem, mHost) : (ActorStateBaseInterface*)mem;
        MR::initActorState(this, mStateAttackMeteor, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackMeteor::sInstance, (const char*)lbl_80676980 + 0x60);
    }
    if (isInSequence(2)) {
        mem = ::operator new(0x44);
        mStateAttackQuake = mem ? fn_800E6CE0(mem, mHost) : (ActorStateBaseInterface*)mem;
        MR::initActorState(this, mStateAttackQuake, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackQuake::sInstance, (const char*)lbl_80676980 + 0x70);
    }
    if (isInSequence(0)) {
        mem = ::operator new(0x28);
        mStateAttackFire = mem ? fn_800E93A0(mem, mHost) : (ActorStateBaseInterface*)mem;
        MR::initActorState(this, mStateAttackFire, &NrvKoopaBattleStage::KoopaBattleStageNrvAttackFire::sInstance, (const char*)lbl_80676980 + 0x7C);
    }
    mem = ::operator new(0x1C);
    mStateDamage = mem ? fn_800E7950(mem, mHost, variant) : (ActorStateBaseInterface*)mem;
    MR::initActorState(this, mStateDamage, &NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance, (const char*)lbl_80676980 + 0x88);
    changeSubs(variant);
    mem = ::operator new(0x4C);
    mStateExtra = mem ? fn_800C9900(mem, mHost) : mem;
}

KoopaBattleStage::~KoopaBattleStage() {}

void KoopaBattleStage::start(bool pFlag) {
    MR::showPlayer();
    fn_800CB7D0(getHost());
    fn_800CBBE0();
    if (mVariant == 0) {
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvBattleStart::sInstance);
        static_cast< KoopaBattleStageStartSlot* >(mStateBattleStart)->slot20();
    } else {
        setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvWait::sInstance);
    }
    if (pFlag) {
        fn_800C9980(mStateExtra);
        return;
    }
    fn_800C9A30(mStateExtra);
}

void KoopaBattleStage::appear() {
    mSequenceIndex = 0;
    fn_800CBCA0();
    fn_800CBBB0(getHost(), -1);
    fn_800CBFE0();
    MR::validateCollisionParts(getHost());
    mIsDead = false;
}

void KoopaBattleStage::kill() {
    fn_800C9AC0(mStateExtra);
    mIsDead = true;
}

void KoopaBattleStage::control() {
    fn_800C9B00(mStateExtra);
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
        MR::startAction(getHost(), lbl_80676A10);
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
    if (getHost()->_10C || isNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance)) {
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
        mSequence = lbl_80676980;
        mSequenceCount = 2;
        return;
    case 1:
        mSequence = lbl_80676988;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::setSequenceLv2(s32 variant) {
    switch (variant) {
    case 0:
        mSequence = lbl_80676994;
        mSequenceCount = 3;
        return;
    case 1:
        mSequence = lbl_806769A0;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::setSequenceLv3(s32 variant) {
    switch (variant) {
    case 0:
        mSequence = lbl_806769AC;
        mSequenceCount = 3;
        return;
    case 1:
        mSequence = lbl_806769B8;
        mSequenceCount = 3;
        return;
    }
}

void KoopaBattleStage::changeSub1(s32 variant) {
    ActorStateBaseInterface* pQuake = mStateAttackQuake;
    if (pQuake != nullptr) {
        fn_800E6F80(pQuake, 1);
    }
    if (mStateAttackFire != nullptr) {
        fn_800E9520(mStateAttackFire, 1);
    }
    if (mStateAttackMeteor != nullptr) {
        fn_800E6960(mStateAttackMeteor, 1);
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
    ActorStateBaseInterface* pQuake = mStateAttackQuake;
    if (pQuake != nullptr) {
        fn_800E6F80(pQuake, 2);
    }
    if (mStateAttackFire != nullptr) {
        fn_800E9520(mStateAttackFire, 2);
    }
    if (mStateAttackMeteor != nullptr) {
        fn_800E6960(mStateAttackMeteor, 2);
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
            fn_800E6F80(mStateAttackQuake, 3);
        }
        if (mStateAttackFire != nullptr) {
            fn_800E9520(mStateAttackFire, 3);
        }
        if (mStateAttackMeteor != nullptr) {
            fn_800E6960(mStateAttackMeteor, 3);
        }
        return;
    case 1:
        mHp = 2;
        if (mStateAttackQuake != nullptr) {
            fn_800E6F80(mStateAttackQuake, 4);
        }
        if (mStateAttackFire != nullptr) {
            fn_800E9520(mStateAttackFire, 3);
        }
        if (mStateAttackMeteor != nullptr) {
            fn_800E6960(mStateAttackMeteor, 3);
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

// NONMATCHING: the target compares the (unused) result of fn_800CC210 (cmpwi, no branch); ours drops it
void KoopaBattleStage::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, getHost(), lbl_80676A18)) {
        if (MR::isSensorPlayer(pReceiver)) {
            MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender);
        } else {
            fn_800CC210(pReceiver, pSender);
        }
    }
}

bool KoopaBattleStage::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool KoopaBattleStage::receiveOtherMsg(u32 msg, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (MR::isEqualSensor(pSensorB, getHost(), lbl_80676A20)) {
        if (MR::isMsgAttackRockReflect(msg)) {
            MR::startAction(reinterpret_cast< LiveActor* >(getHost()->mShell), (const char*)&lbl_807CF5E0);
            MR::tryHitReaction(getHost(), lbl_80676A28);
            return true;
        }
        return fn_800CC250(msg);
    }
    if (MR::isMsgAttackRockReflect(msg)) {
        if (isAttackable()) {
            mHp -= 1;
            if (mHp == 0) {
                MR::stopStageBGM(0);
            }
            MR::emitEffectHitBetweenSensors(getHost(), pSensorA, pSensorB, lbl_807D8630, (const char*)&lbl_807CF5E0);
            fn_800E7A20(mStateDamage, pSensorB);
            fn_800E7AD0(mStateDamage, mHp == 0);
            fn_8022BBC0(getHost(), MR::getSensorPos(pSensorA));
            setNerve(&NrvKoopaBattleStage::KoopaBattleStageNrvDamage::sInstance);
            return true;
        }
    }
    return false;
}
