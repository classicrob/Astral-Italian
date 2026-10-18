#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util.hpp"

// Nerves are declared first so that the small exe functions can be defined inline (the target expands them inside the nerve execute).
namespace NrvKoopaStateLv2Sequence {
    class KoopaStateLv2SequenceNrvDemoOpening : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv2SequenceNrvDemoOpening sInstance;
    };
    KoopaStateLv2SequenceNrvDemoOpening KoopaStateLv2SequenceNrvDemoOpening::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv2SequenceNrvBattleLv1 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv2SequenceNrvBattleLv1 sInstance;
    };
    KoopaStateLv2SequenceNrvBattleLv1 KoopaStateLv2SequenceNrvBattleLv1::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv2SequenceNrvDemoPowerUp : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv2SequenceNrvDemoPowerUp sInstance;
    };
    KoopaStateLv2SequenceNrvDemoPowerUp KoopaStateLv2SequenceNrvDemoPowerUp::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv2SequenceNrvBattleLv2 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv2SequenceNrvBattleLv2 sInstance;
    };
    KoopaStateLv2SequenceNrvBattleLv2 KoopaStateLv2SequenceNrvBattleLv2::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv2SequenceNrvDemoDown : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv2SequenceNrvDemoDown sInstance;
    };
    KoopaStateLv2SequenceNrvDemoDown KoopaStateLv2SequenceNrvDemoDown::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaStateLv2Sequence

#define SINST(name) NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrv##name::sInstance

inline void KoopaStateLv2Sequence::exeDemoOpening() {
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv1))) {
        mBattleLv1->start(true);
    }
}

inline void KoopaStateLv2Sequence::exeBattleLv1() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoPowerUp));
}

inline void KoopaStateLv2Sequence::exeBattleLv2() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07B", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoDown));
}

inline void KoopaStateLv2Sequence::exeDemoDown() {
    if (MR::updateActorState(this)) {
        kill();
    }
}

KoopaStateLv2Sequence::KoopaStateLv2Sequence(KoopaBattle* pHost, const JMapInfoIter& rIter)
    : KoopaStateBase("クッパVs2シーケンス", pHost), mBattleLv1(nullptr), mBattleLv2(nullptr), mPowerUpDemo(nullptr), mDownDemo(nullptr), mSky(nullptr) {
    const char* pArcName = "TicoBabyAnim";
    initNerve(&SINST(DemoOpening), 8);
    mBattleLv1 = new KoopaStateBattle(pHost, 2, 0);
    MR::initActorState(this, mBattleLv1, &SINST(BattleLv1), "BattleLv1");
    mBattleLv2 = new KoopaStateBattle(pHost, 2, 1);
    MR::initActorState(this, mBattleLv2, &SINST(DemoPowerUp), "BattleLv2");
    KoopaStateOpeningDemo* pOpening = new KoopaStateOpeningDemo(mHost, rIter, "DemoKoopaVs2Opening", pArcName, false);
    MR::initActorState(this, pOpening, &SINST(DemoOpening), "DemoOpening");
    mPowerUpDemo = new KoopaStatePowerUpDemo(mHost);
    MR::initActorState(this, mPowerUpDemo, &SINST(BattleLv2), "DemoPowerUp");
    mDownDemo = new KoopaStateDownDemo(mHost, rIter, "DemoKoopaVs2Down", pArcName);
    MR::initActorState(this, mDownDemo, &SINST(DemoDown), "DemoDown");
    mSky = MR::createKoopaBattleSky(mHost);
}

void KoopaStateLv2Sequence::appear() {
    mIsDead = false;
    setNerve(&SINST(DemoOpening));
    mSky->makeActorAppeared();
}

void KoopaStateLv2Sequence::exeDemoPowerUp() {
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv2))) {
        mBattleLv2->start(mPowerUpDemo->mIsCameraInterpolating);
    }
}

void KoopaStateLv2Sequence::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        mBattleLv1->attackSensor(pSender, pReceiver);
    } else if (isNerve(&SINST(BattleLv2))) {
        mBattleLv2->attackSensor(pSender, pReceiver);
    }
}

bool KoopaStateLv2Sequence::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaStateLv2Sequence::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

KoopaStateLv2Sequence::~KoopaStateLv2Sequence() {
}

void NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrvDemoOpening::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv2Sequence* >(pSpine->mExecutor)->exeDemoOpening();
}

void NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrvBattleLv1::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv2Sequence* >(pSpine->mExecutor)->exeBattleLv1();
}

void NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrvDemoPowerUp::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv2Sequence* >(pSpine->mExecutor)->exeDemoPowerUp();
}

void NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrvBattleLv2::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv2Sequence* >(pSpine->mExecutor)->exeBattleLv2();
}

void NrvKoopaStateLv2Sequence::KoopaStateLv2SequenceNrvDemoDown::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv2Sequence* >(pSpine->mExecutor)->exeDemoDown();
}
