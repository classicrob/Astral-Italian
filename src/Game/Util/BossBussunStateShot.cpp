#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateShot {
    NEW_NERVE(BossBussunStateShotNrvShotSign, BossBussunStateShot, ShotSign);
    NEW_NERVE(BossBussunStateShotNrvShot, BossBussunStateShot, Shot);
};  // namespace NrvBossBussunStateShot

BossBussunStateShot::BossBussunStateShot(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスン、ショット！", pHost), mShotGroup(nullptr), mFixedPos(nullptr), mShotCount(3), mShotCountMax(3) {
    mFixedPos = new FixedPosition(mHost, "ShotPos", nullptr);
    mShotGroup = new DeriveActorGroup< BossBussunShot >("弾リスト", 0x20);
    mShotGroup->initWithoutIter();
    for (s32 i = 0; i < 0x20; i++) {
        BossBussunShot* pShot = new BossBussunShot("ボスブッスン弾");
        pShot->mPosition.set(mHost->mPosition);
        pShot->initWithoutIter();
        mShotGroup->registerActor(pShot);
    }
    initNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShotSign), 0);
}

void BossBussunStateShot::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShotSign));
    mShotCount = mShotCountMax;
}

// NONMATCHING: regalloc
void BossBussunStateShot::exeShotSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ShotSign");
    }
    if (mHost->isPlayerAttackable()) {
        mHost->mTargetPos.set(*MR::getPlayerPos());
        f32 dist = MR::calcDistanceToPlayerH(mHost);
        TVec3f* pPlayerMove = MR::getPlayerLastMove();
        TVec3f killed;
        JMAVECScaleAdd(&mHost->mGravity, pPlayerMove, &killed, -mHost->mGravity.dot(*pPlayerMove));
        TVec3f lead = killed * (dist / 3.0f);
        mHost->mTargetPos.add(lead);
    }
    TVec3f target(mHost->mTargetPos);
    MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, target, 60.0f);
    mHost->updateVelocityOnGround();
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShot));
    }
}

void BossBussunStateShot::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Shot");
        BossBussunShot* pShot = mShotGroup->getDeadMember();
        if (pShot != nullptr) {
            mFixedPos->calc();
            TVec3f pos;
            pos.set(mFixedPos->mMtx.mMtx[0][3], mFixedPos->mMtx.mMtx[1][3], mFixedPos->mMtx.mMtx[2][3]);
            TVec3f target(pos);
            TVec3f forward = mHost->mFrontDir * 500.0f;
            target.add(forward);
            pShot->mGravity.set(mHost->mGravity);
            pShot->shoot(pos, target);
        }
    }
    if (MR::isActionEnd(mHost)) {
        if (--mShotCount <= 0) {
            kill();
        } else {
            setNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShotSign));
        }
    }
}

void BossBussunStateShot::setShotCountMax(s32 count) {
    mShotCountMax = count;
}

void BossBussunStateShot::killAll() {
    mShotGroup->makeAllDead();
}

BossBussunStateShot::~BossBussunStateShot() {
}
