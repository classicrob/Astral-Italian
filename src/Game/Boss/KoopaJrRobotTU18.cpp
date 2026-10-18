#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <cmath>

// PLACEHOLDER(objects): unnamed target symbol fn_800CAED0, declared extern "C" so the reloc name matches; resolve when fn_800CAED0 gets its real name in symbols.txt
extern "C" void fn_800CAED0(TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800CB0E0, declared extern "C" so the reloc name matches; resolve when fn_800CB0E0 gets its real name in symbols.txt
extern "C" void fn_800CB0E0(TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800CB140, declared extern "C" so the reloc name matches; resolve when fn_800CB140 gets its real name in symbols.txt
extern "C" void fn_800CB140(TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800CC280, declared extern "C" so the reloc name matches; resolve when fn_800CC280 gets its real name in symbols.txt
extern "C" bool fn_800CC280(u32);
// PLACEHOLDER(objects): unnamed target symbol fn_800CC290, declared extern "C" so the reloc name matches; resolve when fn_800CC290 gets its real name in symbols.txt
extern "C" bool fn_800CC290(u32);
// PLACEHOLDER(objects): unnamed target symbol fn_800CB710, declared extern "C" so the reloc name matches; resolve when fn_800CB710 gets its real name in symbols.txt
extern "C" void fn_800CB710();
// PLACEHOLDER(objects): unnamed target symbol fn_8003D4A0, declared extern "C" so the reloc name matches; resolve when fn_8003D4A0 gets its real name in symbols.txt
extern "C" void fn_8003D4A0(TQuat4f*, TQuat4f*, f32);

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
    getSensor(nullptr);
    fn_800CB710();
    mShadow = new ShadowVolumeQuakeWave(this);
    makeActorDead();
}

void KoopaJrRobotQuakeWave::control() {
    fn_8003D4A0(&mQuat, &mTargetQuat, _B4);
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
    if (fn_800CC280(msg) || fn_800CC290(msg)) {
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
    fn_800CAED0(&pos);
    MR::resetPosition(this, pos);
    TVec3f front;
    fn_800CB0E0(&front);
    TVec3f up;
    fn_800CB140(&up);
    MR::makeQuatFrontUp(&mTargetQuat, -up, front);
    fn_8003D4A0(&mQuat, &mTargetQuat, _B4);
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
