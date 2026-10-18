#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaJrRobotTU14.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util.hpp"

// Nerves are declared first so that the small exe functions can be defined inline (the target expands them inside the nerve execute).
namespace NrvKoopaStateLv3Sequence {
    class KoopaStateLv3SequenceNrvDemoOpening : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvDemoOpening sInstance;
    };
    KoopaStateLv3SequenceNrvDemoOpening KoopaStateLv3SequenceNrvDemoOpening::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvBattleLv1 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvBattleLv1 sInstance;
    };
    KoopaStateLv3SequenceNrvBattleLv1 KoopaStateLv3SequenceNrvBattleLv1::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvDemoPowerUp : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvDemoPowerUp sInstance;
    };
    KoopaStateLv3SequenceNrvDemoPowerUp KoopaStateLv3SequenceNrvDemoPowerUp::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvBattleLv2 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvBattleLv2 sInstance;
    };
    KoopaStateLv3SequenceNrvBattleLv2 KoopaStateLv3SequenceNrvBattleLv2::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvLastBattleStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvLastBattleStart sInstance;
    };
    KoopaStateLv3SequenceNrvLastBattleStart KoopaStateLv3SequenceNrvLastBattleStart::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvLastBattle : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvLastBattle sInstance;
    };
    KoopaStateLv3SequenceNrvLastBattle KoopaStateLv3SequenceNrvLastBattle::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv3SequenceNrvDemoLastBattleEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv3SequenceNrvDemoLastBattleEnd sInstance;
    };
    KoopaStateLv3SequenceNrvDemoLastBattleEnd KoopaStateLv3SequenceNrvDemoLastBattleEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaStateLv3Sequence

#define SINST(name) NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrv##name::sInstance

inline void KoopaStateLv3Sequence::exeBattleLv1() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoPowerUp));
}

inline void KoopaStateLv3Sequence::exeBattleLv2() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07B", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(LastBattleStart));
}

inline void KoopaStateLv3Sequence::exeLastBattleStart() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(10);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(LastBattle));
}

KoopaStateLv3Sequence::KoopaStateLv3Sequence(KoopaBattle* pHost, const JMapInfoIter& rIter)
    : KoopaStateBase("クッパVs3シーケンス", pHost), mBattleLv1(nullptr), mBattleLv2(nullptr), mPowerUpDemo(nullptr), mLastBattle(nullptr), mSky(nullptr),
      mLastBattleSky(nullptr) {
    mSky = MR::createKoopaBattleSky(mHost);
    mLastBattleSky = MR::createKoopaLastBattleSky(mHost);
    KoopaStatePreFinalDemo* pPreFinal = new KoopaStatePreFinalDemo(mHost, rIter);
    pPreFinal->setCastActor(mLastBattleSky);
    pPreFinal->initDemo(rIter);
    KoopaStateEndDemo* pEnd = new KoopaStateEndDemo(mHost);
    pEnd->setCastActor(mLastBattleSky);
    pEnd->initDemo(rIter);
    initNerve(&SINST(DemoOpening), 8);
    mBattleLv1 = new KoopaBattleStage(pHost, 3, 0);
    MR::initActorState(this, mBattleLv1, &SINST(BattleLv1), "BattleLv1");
    mBattleLv2 = new KoopaBattleStage(pHost, 3, 1);
    MR::initActorState(this, mBattleLv2, &SINST(BattleLv2), "BattleLv2");
    mLastBattle = new KoopaStateLastBattle(pHost);
    MR::initActorState(this, mLastBattle, &SINST(LastBattle), "LastBattle");
    KoopaStateOpeningDemo* pOpening = new KoopaStateOpeningDemo(mHost, rIter, "DemoKoopaVs3Opening", "TicoBabyAnimVsKoopaLv3Galaxy", true);
    MR::initActorState(this, pOpening, &SINST(DemoOpening), "DemoOpening");
    mPowerUpDemo = new KoopaStatePowerUpDemo(mHost);
    MR::initActorState(this, mPowerUpDemo, &SINST(DemoPowerUp), "DemoPowerUp");
    MR::initActorState(this, pPreFinal, &SINST(LastBattleStart), "LastBattleStart");
    MR::initActorState(this, pEnd, &SINST(DemoLastBattleEnd), "DemoLastBattleEnd");
}

void KoopaStateLv3Sequence::appear() {
    mIsDead = false;
    setNerve(&SINST(DemoOpening));
}

void KoopaStateLv3Sequence::exeDemoOpening() {
    if (MR::isFirstStep(this)) {
        mSky->makeActorAppeared();
        mLastBattleSky->makeActorDead();
    }
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv1))) {
        mBattleLv1->start(true);
    }
}

void KoopaStateLv3Sequence::exeDemoPowerUp() {
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv2))) {
        mBattleLv2->start(mPowerUpDemo->mIsCameraInterpolating);
    }
}

void KoopaStateLv3Sequence::exeLastBattle() {
    if (MR::isFirstStep(this)) {
        mSky->makeActorDead();
        mLastBattleSky->makeActorAppeared();
        MR::startAction(mLastBattleSky, "KoopaLastBattleSky");
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_10", false);
        MR::setStageBGMState(2, 30);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoLastBattleEnd));
}

void KoopaStateLv3Sequence::exeDemoLastBattleEnd() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(10);
    }
    if (MR::updateActorState(this)) {
        kill();
    }
}

void KoopaStateLv3Sequence::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        mBattleLv1->attackSensor(pSender, pReceiver);
    } else if (isNerve(&SINST(BattleLv2))) {
        mBattleLv2->attackSensor(pSender, pReceiver);
    } else if (isNerve(&SINST(LastBattle))) {
        mLastBattle->attackSensor(pSender, pReceiver);
    }
}

bool KoopaStateLv3Sequence::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(LastBattle))) {
        return mLastBattle->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaStateLv3Sequence::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(LastBattle))) {
        return mLastBattle->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

KoopaStateLv3Sequence::~KoopaStateLv3Sequence() {
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvDemoOpening::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeDemoOpening();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvBattleLv1::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeBattleLv1();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvDemoPowerUp::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeDemoPowerUp();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvBattleLv2::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeBattleLv2();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvLastBattleStart::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeLastBattleStart();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvLastBattle::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeLastBattle();
}

void NrvKoopaStateLv3Sequence::KoopaStateLv3SequenceNrvDemoLastBattleEnd::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv3Sequence* >(pSpine->mExecutor)->exeDemoLastBattleEnd();
}
