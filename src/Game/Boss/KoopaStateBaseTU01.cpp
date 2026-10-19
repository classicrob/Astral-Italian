#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateAttackQuake {
    NEW_NERVE(KoopaStateAttackQuakeNrvMoveStart, KoopaStateAttackQuake, MoveStart);
    NEW_NERVE(KoopaStateAttackQuakeNrvMoveLoop, KoopaStateAttackQuake, MoveLoop);
    NEW_NERVE(KoopaStateAttackQuakeNrvStraightPunchStart, KoopaStateAttackQuake, StraightPunchStart);
    NEW_NERVE(KoopaStateAttackQuakeNrvStraightPunch, KoopaStateAttackQuake, StraightPunch);
    NEW_NERVE(KoopaStateAttackQuakeNrvStraightPunchLoop, KoopaStateAttackQuake, StraightPunchLoop);
    NEW_NERVE(KoopaStateAttackQuakeNrvStraightPunchEnd, KoopaStateAttackQuake, StraightPunchEnd);
};  // namespace NrvKoopaStateAttackQuake

namespace {
    const s32 sWaveDurations[5] = {180, 240, 300, 360, 420};
};

KoopaStateAttackQuake::KoopaStateAttackQuake(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ地震攻撃", pHost), mVolumeDrawer(nullptr), mRock(nullptr), mMode(1), mTurnSpeed(45.0f), mDuration(240) {
    for (s32 i = 0; i < 2; i++) {
        mShockWaves[i] = new KoopaJrRobotShockWave("地震波");
        mShockWaves[i]->setDuration(sWaveDurations[i]);
        mShockWaves[i]->initWithoutIter();
    }
    for (s32 i = 0; i < 5; i++) {
        mQuakeWaves[i] = new KoopaJrRobotQuakeWave("地震波(縦方向)");
        mQuakeWaves[i]->setParam(sWaveDurations[i]);
        mQuakeWaves[i]->initWithoutIter();
    }
    mVolumeDrawer = MR::createVolumeDrawer("波範囲ボリューム描画", "VolumeSphere", nullptr);
    mVolumeDrawer->setColor(Color8(0x78, 0, 0, 0x64));
    mVolumeDrawer->setDrawPasses(5);
    mVolumeDrawer->setScale(TVec3f(0.0f, 0.05f, 0.0f));
    mVolumeDrawer->setScaleDownRate(-1);
    MR::setScale(mVolumeDrawer, TVec3f(10.0f, 10.0f, 10.0f));
    mRock = new KoopaJrRobotRock("パンチ岩");
    mRock->initWithoutIter();
    initNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvStraightPunchStart::sInstance, 0);
}

void KoopaStateAttackQuake::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvMoveStart::sInstance);
    TVec3f basePos;
    MR::getKoopaBattleBasePos(&basePos);
    MR::resetPosition(mVolumeDrawer, basePos);
}

void KoopaStateAttackQuake::kill() {
    mIsDead = true;
    MR::zeroVelocity(mHost);
    MR::endKoopaBattleFaceCtrl(mHost, 0);
    MR::setKoopaBattleVolumeScaleDownRate(mHost, 0);
    mRock->startDisappear();
}

void KoopaStateAttackQuake::setMode(s32 mode) {
    mMode = mode;
    switch (mode) {
    case 1:
        mTurnSpeed = 45.0f;
        mDuration = 240;
        break;
    case 2:
        mTurnSpeed = 60.0f;
        mDuration = 180;
        break;
    case 3:
        mTurnSpeed = 70.0f;
        mDuration = 120;
        break;
    case 4:
        mTurnSpeed = 70.0f;
        mDuration = 100;
        break;
    }
}

void KoopaStateAttackQuake::exeMoveStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "MoveStart");
        MR::endKoopaBattleFaceCtrl(mHost, -1);
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvMoveLoop::sInstance);
    }
}

void KoopaStateAttackQuake::exeMoveLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "MoveLoop");
        MR::startKoopaBattleFaceCtrl(mHost, 30);
        MR::setKoopaBattleVolumeScaleUpRate(mHost, 40);
    }
    KoopaBattle* pHost = mHost;
    if (MR::turnKoopaBattleToPlayer(pHost, MR::calcNerveEaseInOutRate(this, 30), mTurnSpeed)) {
        setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvStraightPunchStart::sInstance);
    }
}

void KoopaStateAttackQuake::exeStraightPunchStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "StraightPunchStart");
        MR::endKoopaBattleFaceCtrl(mHost, 5);
    }
    KoopaBattle* pHost = mHost;
    MR::turnKoopaBattleToPlayer(pHost, 1.0f - MR::calcNerveEaseInOutRate(this, 20), 180.0f);
    if (MR::isActionEnd(mHost)) {
        MR::zeroVelocity(mHost);
        setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvStraightPunch::sInstance);
    }
}

void KoopaStateAttackQuake::exeStraightPunch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "StraightPunch");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvStraightPunchLoop::sInstance);
    }
}

// NONMATCHING: the target calls KoopaJrRobotQuakeWave::start(f32, const TVec3f&) with a second (unused) arg = &mHost->mPosition; our header's start(f32) has one parameter (4 insns)
void KoopaStateAttackQuake::exeStraightPunchLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "StraightPunchLoop");
        MR::tryHitReaction(mHost, "StraightPunchHit");
        MR::setKoopaBattleVolumeScaleDownRate(mHost, 20);
        if (mMode == 2) {
            mShockWaves[0]->start(mHost->mPosition);
        }
        if (mMode == 3) {
            for (s32 i = 0; i < 3; i++) {
                mQuakeWaves[i]->start(60.0f * i);
            }
        }
        if (mMode == 4) {
            for (s32 i = 0; i < 4; i++) {
                mQuakeWaves[i]->start(45.0f * i);
            }
        }
        mVolumeDrawer->setScaleUpRate(6);
        mRock->appearAtArenaEdge();
    }
    if (MR::isStep(this, 6)) {
        MR::sendMsgKoopaReleaseAttackRock();
        mVolumeDrawer->setScaleDownRate(40);
    }
    if (MR::isGreaterStep(this, mDuration)) {
        setNerve(&NrvKoopaStateAttackQuake::KoopaStateAttackQuakeNrvStraightPunchEnd::sInstance);
    }
}

void KoopaStateAttackQuake::exeStraightPunchEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "StraightPunchEnd");
        mRock->startDisappear();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

KoopaStateAttackQuake::~KoopaStateAttackQuake() {
}
