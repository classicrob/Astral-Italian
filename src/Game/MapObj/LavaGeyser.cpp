#include "Game/MapObj/LavaGeyser.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"

namespace NrvLavaGeyser {
    NEW_NERVE(HostTypeNrvWaitSwitch, LavaGeyser, WaitSwitch);
    NEW_NERVE(HostTypeNrvWait, LavaGeyser, Wait);
    NEW_NERVE(HostTypeNrvSign, LavaGeyser, Sign);
    NEW_NERVE(HostTypeNrvShootUp, LavaGeyser, ShootUp);
    NEW_NERVE(HostTypeNrvShootKeep, LavaGeyser, ShootKeep);
    NEW_NERVE(HostTypeNrvShootDown, LavaGeyser, ShootDown);
};  // namespace NrvLavaGeyser

LavaGeyser::LavaGeyser(const char* pName) : LiveActor(pName) {
    mTicoStop = nullptr;
    mWaitTime = 180;
    mShootKeepTime = 180;
    mTopPos.set(0.0f, 0.0f, 0.0f);
    mUpDir.set(0.0f, 1.0f, 0.0f);
    _B0 = 0.0f;
    _AC = 0.0f;
    _B8 = 0.0f;
    _B4 = 0.0f;
}

void LavaGeyser::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "LavaGeyser", false);
    MR::setBtkFrameAtRandom(this);
    MR::hideModel(this);
    TQuat4f quat1;
    quat1.setEulerDegree(mRotation.x, mRotation.y, mRotation.z);
    quat1.getYDir(mUpDir);
    MR::getJMapInfoArg0NoInit(iter, &mWaitTime);
    MR::getJMapInfoArg1NoInit(iter, &mShootKeepTime);
    if (MR::isValidSwitchA(this)) {
        initNerve(GET_NERVE(LavaGeyser, HostTypeNrvWaitSwitch));
    } else {
        initNerve(GET_NERVE(LavaGeyser, HostTypeNrvWait));
    }
    mTicoStop = new SupportTicoStop(this, nullptr);
    MR::startBckAndSetFrameAndStop(this, "LavaGeyserAppear", 0.0f, nullptr);
    makeActorAppeared();
}

void LavaGeyser::startClipped() {
    LiveActor::startClipped();
    MR::forceDeleteEffectAll(this);
    if (!isNerve(GET_NERVE(LavaGeyser, HostTypeNrvWaitSwitch))) {
        setNerve(GET_NERVE(LavaGeyser, HostTypeNrvWait));
    }
}

void LavaGeyser::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        MR::forceDeleteEffectAll(this);
        MR::emitEffect(this, "ForceDeath");
    }
}

void LavaGeyser::control() {
    mTicoStop->updateNerve();
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        kill();
    }
}

// NONMATCHING: paired-single vector-add operand registers are reversed.
void LavaGeyser::updateHitSensor(HitSensor* pSensor) {
    TVec3f cPos = *MR::getPlayerCenterPos();
    cPos -= mPosition;
    f32 playerHeight = mUpDir.dot(cPos);
    TVec3f topOffset = mTopPos;
    topOffset -= mPosition;
    f32 topHeight = mUpDir.dot(topOffset) + -100.f;
    if (topHeight < 0.f) {
        MR::setSensorPos(pSensor, mTopPos);
        return;
    }

    f32 clampedHeight = MR::clamp(playerHeight, 0.0f, topHeight);

    MR::setSensorPos(pSensor, mPosition);
    MR::getSensorPos2(pSensor)->add(mUpDir * clampedHeight);
}

void LavaGeyser::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "attack")) {
        MR::sendMsgEnemyAttackFireStrong(pReceiver, pSender);
    }
    if (MR::isEqualSensor(pSender, this, "upper") && (isNerve(GET_NERVE(LavaGeyser, HostTypeNrvShootUp)) || isNerve(GET_NERVE(LavaGeyser, HostTypeNrvShootKeep)))) {
        MR::sendArbitraryMsg(0x54, pReceiver, pSender);
    }
}

void LavaGeyser::exeWaitSwitch() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
    }
    if (MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(LavaGeyser, HostTypeNrvSign));
    }
}

void LavaGeyser::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LavaGeyserDisappear");
        MR::setBckFrameAndStop(this, MR::getBckFrameMax(this));
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        mTopPos.set<f32>(mPosition);
    }
    if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(LavaGeyser, HostTypeNrvWaitSwitch));
    } else if (MR::isStep(this, mWaitTime)) {
        setNerve(GET_NERVE(LavaGeyser, HostTypeNrvSign));
    }
}

void LavaGeyser::exeSign() {
    if (MR::isFirstStep(this)) {
        const TVec3f* pos = &mPosition;
        mTopPos.set<f32>(*pos);
        MR::emitEffect(this, "Sign");
    }
    MR::copyJointPos(this, "Top", &mTopPos);
    MR::startActionSound(this, "OjLvLavaGeyserSign");
    if (MR::isStep(this, 90)) {
        MR::deleteEffect(this, "Sign");
        setNerve(GET_NERVE(LavaGeyser, HostTypeNrvShootUp));
    }
}

void LavaGeyser::exeShootUp() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::validateHitSensors(this);
        MR::startActionSound(this, "OjLavaGeyserShoot");
        MR::startAction(this, "LavaGeyserAppear");
    }
    MR::copyJointPos(this, "Top", &mTopPos);
    if (!mTicoStop->tryAttachTouch()) {
        MR::startActionSound(this, "OjLvLavaGeyserSign");
        MR::startActionSound(this, "OjLvLavaGeyserKeep");
        if (MR::isActionEnd(this)) {
            setNerve(GET_NERVE(LavaGeyser, HostTypeNrvShootKeep));
        }
    }
}

void LavaGeyser::exeShootKeep() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LavaGeyserWait");
    }
    MR::copyJointPos(this, "Top", &mTopPos);
    if (!mTicoStop->tryAttachTouch()) {
        MR::startActionSound(this, "OjLvLavaGeyserSign");
        MR::startActionSound(this, "OjLvLavaGeyserKeep");
        if (MR::isGreaterStep(this, mShootKeepTime)) {
            setNerve(GET_NERVE(LavaGeyser, HostTypeNrvShootDown));
        }
    }
}

void LavaGeyser::exeShootDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LavaGeyserDisappear");
    }
    MR::copyJointPos(this, "Top", &mTopPos);
    if (!mTicoStop->tryAttachTouch()) {
        MR::startActionSound(this, "OjLvLavaGeyserKeep");
        if (MR::isActionEnd(this)) {
            MR::hideModel(this);
            setNerve(GET_NERVE(LavaGeyser, HostTypeNrvWait));
        }
    }
}
