#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"

#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <cmath>

namespace NrvKoopaJrRobotQuakeWave {
    NEW_NERVE(KoopaJrRobotQuakeWaveNrvHide, KoopaJrRobotQuakeWave, Hide);
    NEW_NERVE(KoopaJrRobotQuakeWaveNrvSign, KoopaJrRobotQuakeWave, Sign);
    NEW_NERVE(KoopaJrRobotQuakeWaveNrvEnd, KoopaJrRobotQuakeWave, End);
};  // namespace NrvKoopaJrRobotQuakeWave

KoopaJrRobotQuakeWave::KoopaJrRobotQuakeWave(const char* pName)
    : LiveActor(pName), mShadow(nullptr), mQuat(0.0f, 0.0f, 0.0f, 1.0f), mTargetQuat(0.0f, 0.0f, 0.0f, 1.0f), _B4(0.0f), _B8(0.0f), _BC(0.0f) {}

void KoopaJrRobotQuakeWave::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaQuakeWaveVertical", false);
    initNerve(&NrvKoopaJrRobotQuakeWave::KoopaJrRobotQuakeWaveNrvHide::sInstance, 0);
    MR::invalidateClipping(this);
    MR::addKoopaBattleReceiverSensor(getSensor(nullptr));
    mShadow = new ShadowVolumeQuakeWave(this);
    makeActorDead();
}

void KoopaJrRobotQuakeWave::control() {
    MR::rotateQuatZAxisDegree(&mQuat, mTargetQuat, _B4);
}

void KoopaJrRobotQuakeWave::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void KoopaJrRobotQuakeWave::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isOnGroundPlayer()) {
        TVec3f dir;
        MR::calcSensorDirection(&dir, pSender, pReceiver);
        TVec3f up;
        MR::calcUpVec(&up, this);
        f32 d = dir.dot(up);
        if (!(MR::abs(d) >= 60.0f)) {
            TVec3f front;
            MR::calcFrontVec(&front, this);
            f32 ang = MR::diffAngleAbs(dir, -front) / 3.1415927f;
            if (!(MR::abs(d) >= 60.0f) && 0.25f + _BC < ang && ang < _B8 && MR::isSensorPlayer(pReceiver)) {
                MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, d > 0.0f ? up : -up);
            }
        }
    }
}

bool KoopaJrRobotQuakeWave::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgKoopaBA(msg) || MR::isMsgKoopaBB(msg)) {
        makeActorDead();
        return true;
    }
    return false;
}

void KoopaJrRobotQuakeWave::start(f32 param) {
    _B8 = 0.0f;
    _BC = 0.0f;
    _B4 = param;
    TVec3f pos;
    MR::getKoopaBattleBasePos(&pos);
    MR::resetPosition(this, pos);
    TVec3f front;
    MR::getKoopaBattleUp(&front);
    TVec3f up;
    MR::calcKoopaBattleDirToActor(&up);
    MR::makeQuatFrontUp(&mTargetQuat, -up, front);
    MR::rotateQuatZAxisDegree(&mQuat, mTargetQuat, _B4);
    setNerve(&NrvKoopaJrRobotQuakeWave::KoopaJrRobotQuakeWaveNrvHide::sInstance);
    appear();
}

void KoopaJrRobotQuakeWave::setParam(s32 value) {
    _C0 = value;
}

void KoopaJrRobotQuakeWave::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(this);
    }
    if (MR::isGreaterStep(this, 5)) {
        setNerve(&NrvKoopaJrRobotQuakeWave::KoopaJrRobotQuakeWaveNrvSign::sInstance);
    }
}

void KoopaJrRobotQuakeWave::exeSign() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "Sign");
        _BC = 0.0f;
    }
    _B8 = MR::calcNerveValue(this, 150, 0.3f, 1.0f);
    MR::setBckFrameAndStop(this, _B8 * MR::getBckFrameMax(this));
    MR::setBtkFrameAndStop(this, _B8 * MR::getBtkFrameMax(this));
    if (MR::isGreaterEqualStep(this, 150)) {
        setNerve(&NrvKoopaJrRobotQuakeWave::KoopaJrRobotQuakeWaveNrvEnd::sInstance);
    }
}

void KoopaJrRobotQuakeWave::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "End");
        _B8 = 1.0f;
    }
    _BC = MR::calcNerveValue(this, 150, 0.0f, 1.0f);
    MR::setBckFrameAndStop(this, _BC * MR::getBckFrameMax(this));
    MR::setBtkFrameAndStop(this, _BC * MR::getBtkFrameMax(this));
    if (MR::isGreaterEqualStep(this, 150)) {
        kill();
    }
}

KoopaJrRobotQuakeWave::~KoopaJrRobotQuakeWave() {}
