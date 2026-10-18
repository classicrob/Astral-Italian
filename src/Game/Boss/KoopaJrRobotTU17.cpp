#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"

#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>

namespace NrvKoopaJrRobotShockWave {
    NEW_NERVE(KoopaJrRobotShockWaveNrvHide, KoopaJrRobotShockWave, Hide);
    NEW_NERVE(KoopaJrRobotShockWaveNrvAttack, KoopaJrRobotShockWave, Attack);
};  // namespace NrvKoopaJrRobotShockWave

KoopaJrRobotShockWave::KoopaJrRobotShockWave(const char* pName) : LiveActor(pName), mShadow(nullptr), _94(0.0f), _98(280) {}

void KoopaJrRobotShockWave::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaQuakeWave", false);
    initNerve(&NrvKoopaJrRobotShockWave::KoopaJrRobotShockWaveNrvHide::sInstance, 0);
    MR::invalidateClipping(this);
    MR::addKoopaBattleReceiverSensor(getSensor(nullptr));
    mShadow = new ShadowVolumeQuakeWave(this);
    makeActorDead();
}

void KoopaJrRobotShockWave::control() {}

void KoopaJrRobotShockWave::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrRobotShockWave::KoopaJrRobotShockWaveNrvAttack::sInstance) && MR::isOnGroundPlayer()) {
        TVec3f dir;
        MR::calcSensorDirectionNormalize(&dir, pSender, pReceiver);
        TVec3f up;
        MR::calcUpVec(&up, this);
        f32 ratio = MR::getSensorRadius(pReceiver) / MR::calcSensorDistance(pSender, pReceiver);
        f32 limit;
        if (ratio >= 1.0f) {
            limit = 180.0f;
        } else if (ratio <= 0.0f) {
            limit = 1.0f;
        } else {
            limit = 57.29578f * JMAAsinRadian(ratio);
        }
        if (__fabs(_94 - 57.29578f * MR::diffAngleAbs(up, dir)) < limit && MR::isSensorPlayer(pReceiver)) {
            TVec3f out;
            TVec3f negUp = -up;
            JMAVECScaleAdd(&dir, &negUp, &out, -dir.dot(negUp));
            MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, out);
        }
    }
}

bool KoopaJrRobotShockWave::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgKoopaBA(msg) || MR::isMsgKoopaBB(msg)) {
        makeActorDead();
        return true;
    }
    return false;
}

void KoopaJrRobotShockWave::start(const TVec3f& rTarget) {
    TVec3f pos;
    MR::getKoopaBattleBasePos(&pos);
    TVec3f dir;
    TVec3f diff = rTarget - pos;
    MR::normalizeOrZero(&dir, diff);
    MR::setQuatUpNoSupport(this, dir);
    MR::resetPosition(this, pos);
    _94 = 15.0f;
    appear();
    setNerve(&NrvKoopaJrRobotShockWave::KoopaJrRobotShockWaveNrvHide::sInstance);
    MR::setBckFrameAndStop(this, 0.0f);
    mShadow->_2C = 100.0f + MR::getKoopaBattleRadius();
    mShadow->_30 = _94;
}

void KoopaJrRobotShockWave::setDuration(s32 value) {
    _98 = value;
}

void KoopaJrRobotShockWave::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(this);
    }
    if (MR::isGreaterStep(this, 5)) {
        setNerve(&NrvKoopaJrRobotShockWave::KoopaJrRobotShockWaveNrvAttack::sInstance);
    }
}

void KoopaJrRobotShockWave::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::showModel(this);
        _94 = 15.0f;
    }
    MR::startActionSound(this, "BmLvKoopaSwaveMove", -1, -1, -1);
    MR::setBckFrameAndStop(this, (_94 / 180.0f) * MR::getBckFrameMax(this));
    mShadow->_30 = _94;
    _94 = MR::calcNerveValue(this, _98, 15.0f, 180.0f);
    if (MR::isGreaterStep(this, _98)) {
        kill();
    }
}

KoopaJrRobotShockWave::~KoopaJrRobotShockWave() {}
