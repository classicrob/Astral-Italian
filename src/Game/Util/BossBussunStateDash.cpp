#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

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

bool BossBussunStateDash::tryStartDash(LiveActor* pActor) {
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
    if (fn_80098B50(mHost)) {
        fn_80098CD0(mHost);
    }
    MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->_D0, mHost->_C4, 2.0f);
    fn_80098B00(mHost);
    if (MR::isGreaterStep(this, 0x41)) {
        setNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvDash));
    }
}

void BossBussunStateDash::exeDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Dash");
    }
    if (fn_80098B50(mHost)) {
        fn_80098CD0(mHost);
        if (PSVECDistance(&mHost->_C4, &mHost->mPosition) > 1800.0f) {
            MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->_D0, mHost->_C4, 0.5f);
        }
    }
    if (mCoolTime <= 0) {
        MR::addVelocityMoveToDirection(mHost, mHost->_D0, 3.5f);
    }
    fn_80098B00(mHost);
    f32 dist = fn_80098420(mHost);
    if (MR::calcDistanceHorizontal(mHost, mHost->_DC, mHost->_E8) > dist) {
        TVec3f sp8(mHost->_DC.x - mHost->mPosition.x, mHost->_DC.y - mHost->mPosition.y, mHost->_DC.z - mHost->mPosition.z);
        if (mHost->_D0.dot(sp8) < 0.0f) {
            setNerve(GET_NERVE(BossBussunStateDash, BossBussunStateDashNrvDashEnd));
        }
    }
}

void BossBussunStateDash::exeDashEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DashEnd");
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

BossBussunStateDash::~BossBussunStateDash() {
}
