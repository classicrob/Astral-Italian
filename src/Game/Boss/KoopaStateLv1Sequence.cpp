#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaJrRobotTU14.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util.hpp"

// Nerves are declared first so that the small exe functions can be defined inline (the target expands them inside the nerve execute).
namespace NrvKoopaStateLv1Sequence {
    class KoopaStateLv1SequenceNrvDemoOpening : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv1SequenceNrvDemoOpening sInstance;
    };
    KoopaStateLv1SequenceNrvDemoOpening KoopaStateLv1SequenceNrvDemoOpening::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv1SequenceNrvBattleLv1 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv1SequenceNrvBattleLv1 sInstance;
    };
    KoopaStateLv1SequenceNrvBattleLv1 KoopaStateLv1SequenceNrvBattleLv1::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv1SequenceNrvDemoPowerUp : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv1SequenceNrvDemoPowerUp sInstance;
    };
    KoopaStateLv1SequenceNrvDemoPowerUp KoopaStateLv1SequenceNrvDemoPowerUp::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv1SequenceNrvBattleLv2 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv1SequenceNrvBattleLv2 sInstance;
    };
    KoopaStateLv1SequenceNrvBattleLv2 KoopaStateLv1SequenceNrvBattleLv2::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLv1SequenceNrvDemoDown : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLv1SequenceNrvDemoDown sInstance;
    };
    KoopaStateLv1SequenceNrvDemoDown KoopaStateLv1SequenceNrvDemoDown::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaStateLv1Sequence

#define SINST(name) NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrv##name::sInstance

inline void KoopaStateLv1Sequence::exeDemoOpening() {
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv1))) {
        mBattleLv1->start(true);
    }
}

inline void KoopaStateLv1Sequence::exeBattleLv1() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoPowerUp));
}

inline void KoopaStateLv1Sequence::exeBattleLv2() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG2_BOSS_07B", false);
    }
    MR::updateActorStateAndNextNerve(this, &SINST(DemoDown));
}

inline void KoopaStateLv1Sequence::exeDemoDown() {
    if (MR::updateActorState(this)) {
        kill();
    }
}

KoopaStateLv1Sequence::KoopaStateLv1Sequence(KoopaBattle* pHost, const JMapInfoIter& rIter)
    : KoopaStateBase("クッパVs1シーケンス", pHost), mBattleLv1(nullptr), mBattleLv2(nullptr), mPowerUpDemo(nullptr), mSky(nullptr) {
    const char* pArcName = "TicoBabyAnimVsKoopaLv1Galaxy";
    initNerve(&SINST(DemoOpening), 8);
    mBattleLv1 = new KoopaBattleStage(pHost, 1, 0);
    MR::initActorState(this, mBattleLv1, &SINST(BattleLv1), "BattleLv1");
    mBattleLv2 = new KoopaBattleStage(pHost, 1, 1);
    MR::initActorState(this, mBattleLv2, &SINST(BattleLv2), "BattleLv2");
    KoopaStateOpeningDemo* pOpening = new KoopaStateOpeningDemo(mHost, rIter, "DemoKoopaVs1Opening", pArcName, false);
    MR::initActorState(this, pOpening, &SINST(DemoOpening), "DemoOpening");
    mPowerUpDemo = new KoopaStatePowerUpDemo(mHost);
    MR::initActorState(this, mPowerUpDemo, &SINST(DemoPowerUp), "DemoPowerUp");
    KoopaStateDownDemo* pDownDemo = new KoopaStateDownDemo(mHost, rIter, "DemoKoopaVs1Down", pArcName);
    MR::initActorState(this, pDownDemo, &SINST(DemoDown), "DemoDown");
    mSky = MR::createKoopaBattleSky(mHost);
}

void KoopaStateLv1Sequence::appear() {
    mIsDead = false;
    setNerve(&SINST(DemoOpening));
    mSky->makeActorAppeared();
}

void KoopaStateLv1Sequence::exeDemoPowerUp() {
    if (MR::updateActorStateAndNextNerve(this, &SINST(BattleLv2))) {
        mBattleLv2->start(mPowerUpDemo->mIsCameraInterpolating);
    }
}

void KoopaStateLv1Sequence::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        mBattleLv1->attackSensor(pSender, pReceiver);
    } else if (isNerve(&SINST(BattleLv2))) {
        mBattleLv2->attackSensor(pSender, pReceiver);
    }
}

bool KoopaStateLv1Sequence::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaStateLv1Sequence::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&SINST(BattleLv1))) {
        return mBattleLv1->receiveOtherMsg(msg, pSender, pReceiver);
    }
    if (isNerve(&SINST(BattleLv2))) {
        return mBattleLv2->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

KoopaStateLv1Sequence::~KoopaStateLv1Sequence() {
}

void NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrvDemoOpening::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv1Sequence* >(pSpine->mExecutor)->exeDemoOpening();
}

void NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrvBattleLv1::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv1Sequence* >(pSpine->mExecutor)->exeBattleLv1();
}

void NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrvDemoPowerUp::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv1Sequence* >(pSpine->mExecutor)->exeDemoPowerUp();
}

void NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrvBattleLv2::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv1Sequence* >(pSpine->mExecutor)->exeBattleLv2();
}

void NrvKoopaStateLv1Sequence::KoopaStateLv1SequenceNrvDemoDown::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLv1Sequence* >(pSpine->mExecutor)->exeDemoDown();
}
