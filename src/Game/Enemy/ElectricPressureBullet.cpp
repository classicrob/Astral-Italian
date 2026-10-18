#include "Game/Enemy/ElectricPressureBullet.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JMath/JMath.hpp>

extern "C" void fn_800348C0(LiveActor*);

namespace NrvElectricPressureBullet {
    NEW_NERVE(ElectricPressureBulletNrvFly, ElectricPressureBullet, Fly);
};  // namespace NrvElectricPressureBullet

ElectricPressureBullet::ElectricPressureBullet(const char* pName) : LiveActor(pName), _8C(0.0f, 0.0f, 0.0f), _98(), _9C(0.0f), _A4(false) {
}

void ElectricPressureBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "ElectricBullet", false);
    initNerve(GET_NERVE(ElectricPressureBullet, ElectricPressureBulletNrvFly));
    makeActorDead();
}

void ElectricPressureBullet::kill() {
    fn_800348C0(this);
    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "EmElecbublletBreak");
    LiveActor::kill();
}

void ElectricPressureBullet::shotElectricBullet(LiveActor* actor, const TPos3f& pos, const f32& value, bool flag) {
    _98 = actor;
    _9C = value;
    _A4 = flag;
    pos.getZDir(_8C);
    mVelocity.scale(_9C, _8C);
    pos.getTrans(mPosition);
    mRotation.zero();
    makeActorAppeared();
    MR::validateHitSensors(this);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(ElectricPressureBullet, ElectricPressureBulletNrvFly));
}

void ElectricPressureBullet::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::startBtk(this, "ElectricBullet");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Move");
    }
    MR::startActionSound(this, "EmLvElecbublletFly");
    if (MR::isGreaterEqualStep(this, 10)) {
        if (MR::isLessStep(this, 150)) {
            s32 v2 = getNerveStep();
            if (!(v2 % 5)) {
                TVec3f v3;
                v3.sub(*MR::getPlayerCenterPos(), mPosition);
                MR::normalize(&v3);
                MR::turnVecToVecDegree(&_8C, _8C, v3, 4.0f, TVec3f(0, 1, 0));
                mVelocity.scale(_9C, _8C);
            }
        }
    }

    if (MR::isBinded(this)) {
        kill();
    } else if (MR::isStep(this, 360)) {
        kill();
    }
}

void ElectricPressureBullet::control() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        kill();
    }
}

void ElectricPressureBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
        kill();
    }
}

bool ElectricPressureBullet::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStartPowerStarGet(msg)) {
        kill();
        return true;
    }

    return false;
}
