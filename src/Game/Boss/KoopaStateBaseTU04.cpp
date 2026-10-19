#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateDefense {
    NEW_NERVE(KoopaStateDefenseNrvStart, KoopaStateDefense, Start);
    NEW_NERVE(KoopaStateDefenseNrvWait, KoopaStateDefense, Wait);
    NEW_NERVE(KoopaStateDefenseNrvClose, KoopaStateDefense, Close);
    NEW_NERVE(KoopaStateDefenseNrvEnd, KoopaStateDefense, End);
};  // namespace NrvKoopaStateDefense

KoopaStateDefense::KoopaStateDefense(KoopaBattle* pHost, bool flag)
    : ActorStateBase< KoopaBattle >("クッパ防御姿勢状態", pHost), mStartActionName("DefenseStart"), mEndActionName("DefenseEnd"), mFlag(flag) {
    initNerve(&NrvKoopaStateDefense::KoopaStateDefenseNrvStart::sInstance, 0);
}

void KoopaStateDefense::setStartActionName(const char* pName) {
    mStartActionName = pName;
}

void KoopaStateDefense::appear() {
    mIsDead = false;
    MR::killKoopaBattlePlanet();
    if (mFlag) {
        if (!mHost->mIsDefenseForm) {
            setNerve(&NrvKoopaStateDefense::KoopaStateDefenseNrvStart::sInstance);
        } else {
            kill();
        }
    } else if (mHost->mIsDefenseForm) {
        setNerve(&NrvKoopaStateDefense::KoopaStateDefenseNrvEnd::sInstance);
    } else {
        kill();
    }
}

void KoopaStateDefense::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, mStartActionName);
        MR::endKoopaBattleFaceCtrl(mHost, 5);
    }
    if (MR::isActionEnd(mHost)) {
        MR::invalidateKoopaBattleCollision(mHost);
        mHost->getDefenseForm()->appear();
        setNerve(&NrvKoopaStateDefense::KoopaStateDefenseNrvWait::sInstance);
    }
}

void KoopaStateDefense::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(mHost);
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void KoopaStateDefense::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost->getDefenseForm(), "Close");
        MR::hideModelAndOnCalcAnim(mHost);
    }
    if (MR::isGreaterStep(this, 30)) {
        kill();
    }
}

void KoopaStateDefense::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::showModel(mHost);
    }
    if (MR::isStep(this, 2)) {
        MR::validateKoopaBattleCollision(mHost);
        mHost->getDefenseForm()->kill();
        kill();
    }
}

KoopaStateDefense::~KoopaStateDefense() {
}
