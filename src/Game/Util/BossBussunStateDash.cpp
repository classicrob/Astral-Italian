#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    inline const TVec3f& getDashPosition(const BossBussun* host) {
        return host->mPosition;
    }
    inline const TVec3f& getDashTarget(const BossBussun* host) {
        return host->mTargetPos;
    }
    inline TVec3f getDashHomeOffset(const BossBussun* host) {
        TVec3f diff(host->mHomePos);
        diff.sub(host->mPosition);
        return diff;
    }
    inline const TVec3f& getDashUp(const BossBussun* host) {
        return host->mUpVec;
    }
    inline const TVec3f& getDashHome(const BossBussun* host) {
        return host->mHomePos;
    }
}

namespace NrvBossBussunStateDash {
    NEW_NERVE(BossBussunStateDashNrvStart, BossBussunStateDash, Start);
    NEW_NERVE(BossBussunStateDashNrvDash, BossBussunStateDash, Dash);
    NEW_NERVE(BossBussunStateDashNrvDashEnd, BossBussunStateDash, DashEnd);
};  // namespace NrvBossBussunStateDash

BossBussunStateDash::BossBussunStateDash(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスンダッシュ状態", pHost), mCoolTime(0) {
    initNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvStart), 0);
}

void BossBussunStateDash::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvStart));
    mCoolTime = 0;
}

void BossBussunStateDash::control() {
    if (mCoolTime > 0) {
        mCoolTime--;
    }
}

bool BossBussunStateDash::tryStartDash() {
    if (mCoolTime <= 0) {
        mCoolTime = 10;
        MR::zeroVelocity(mHost);
        return true;
    }
    return false;
}

void BossBussunStateDash::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DashSign");
    }
    if (mHost->isPlayerAttackable()) {
        mHost->storePlayerPos();
    }
    MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mTargetPos, 2.0f);
    mHost->updateVelocityOnGround();
    if (MR::isGreaterStep(this, 0x41)) {
        setNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvDash));
    }
}

void BossBussunStateDash::exeDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Dash");
    }
    if (mHost->isPlayerAttackable()) {
        mHost->storePlayerPos();
        if (PSVECDistance(&getDashTarget(mHost), &getDashPosition(mHost)) > 1800.0f) {
            MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, getDashTarget(mHost), 0.5f);
        }
    }
    if (mCoolTime <= 0) {
        MR::addVelocityMoveToDirection(mHost, mHost->mFrontDir, 3.5f);
    }
    mHost->updateVelocityOnGround();
    BossBussun* pHost = mHost;
    const TVec3f& rUp = getDashUp(pHost);
    const TVec3f& rHome = getDashHome(pHost);
    f32 limit = pHost->getDashDistanceLimit();
    if (MR::calcDistanceHorizontal(pHost, rHome, rUp) > limit) {
        TVec3f diff = getDashHomeOffset(mHost);
        if (mHost->mFrontDir.dot(diff) < 0.0f) {
            setNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvDashEnd));
        }
    }
}

void BossBussunStateDash::exeDashEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DashEnd");
    }
    mHost->updateVelocityOnGround();
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

BossBussunStateDash::~BossBussunStateDash() {
}
