#include "Game/Boss/PoltaStateAttackGround.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Polta.hpp"
#include "Game/Boss/PoltaFunction.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvPoltaStateAttackGround {
    NEW_NERVE(PoltaStateAttackGroundNrvStart, PoltaStateAttackGround, Start);
    NEW_NERVE(PoltaStateAttackGroundNrvAttack, PoltaStateAttackGround, Attack);
    NEW_NERVE(PoltaStateAttackGroundNrvToWait, PoltaStateAttackGround, ToWait);
};  // namespace NrvPoltaStateAttackGround

PoltaStateAttackGround::PoltaStateAttackGround(Polta* pPolta) : ActorStateBase< Polta >("[state]地面叩き攻撃", pPolta), mIsAffectBody(true) {
    initNerve(GET_NERVE(PoltaStateAttackGround, PoltaStateAttackGroundNrvStart));
    mAttackStartLength = MR::getBckFrameMax(getHost(), "AttackFrontStart");
    mAttackLength = MR::getBckFrameMax(getHost(), "AttackFront");
    mAttackToWaitLength = MR::getBckFrameMax(getHost(), "AttackFrontToWait");
};

// Unused remnant also found in PoltaStatePunch?
const char* unusedDamage = "Damage";

void PoltaStateAttackGround::appear() {
    mIsDead = false;
    mIsAffectBody = true;
    setNerve(GET_NERVE(PoltaStateAttackGround, PoltaStateAttackGroundNrvStart));
}

void PoltaStateAttackGround::exeStart() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::requestStartControllArm(getHost());
        PoltaFunction::startAction(getHost(), "AttackFrontStart", mIsAffectBody);
        MR::startActionSound(getHost(), "BvPoltaPrepArmDown");
        MR::startActionSound(getHost(), "BmPoltaArmLiftUp");
    }
    if (MR::isLessStep(this, 60)) {
        getHost()->rotateToPlayer();
    }
    if (!PoltaFunction::isEnableAttackLeftArm(getHost()) && !PoltaFunction::isEnableAttackRightArm(getHost())) {
        kill();
    } else {
        if (MR::isGreaterStep(this, mAttackStartLength)) {
            setNerve(GET_NERVE(PoltaStateAttackGround, PoltaStateAttackGroundNrvAttack));
        }
    }
}

void PoltaStateAttackGround::exeAttack() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::startAction(getHost(), "AttackFront", mIsAffectBody);
        MR::startActionSound(getHost(), "BvPoltaArmDown");
        MR::startActionSound(getHost(), "BmPoltaArmSwingDown");
    }
    if (!PoltaFunction::isEnableAttackLeftArm(getHost()) && !PoltaFunction::isEnableAttackRightArm(getHost())) {
        kill();
    } else {
        if (MR::isGreaterStep(this, mAttackLength)) {
            setNerve(GET_NERVE(PoltaStateAttackGround, PoltaStateAttackGroundNrvToWait));
            MR::tryRumblePadStrong(this, WPAD_CHAN0);
            MR::startActionSound(getHost(), "BmPoltaHitGround");
            MR::shakeCameraNormalStrong();
        }
    }
}

void PoltaStateAttackGround::exeToWait() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::startAction(getHost(), "AttackFrontToWait", mIsAffectBody);
    }
    if (MR::isGreaterStep(this, mAttackToWaitLength)) {
        kill();
    }
}

bool PoltaStateAttackGround::isEnableAttack(const HitSensor* pSensor) const {
    if (PoltaFunction::isBodySensor(getHost(), pSensor)) {
        return false;
    } else {
        return isNerve(GET_NERVE(PoltaStateAttackGround, PoltaStateAttackGroundNrvAttack));
    }
}

PoltaStateAttackGround::~PoltaStateAttackGround() {
}
