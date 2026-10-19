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


namespace NrvElectricPressureBullet {
    NEW_NERVE(ElectricPressureBulletNrvFly, ElectricPressureBullet, Fly);
};  // namespace NrvElectricPressureBullet

ElectricPressureBullet::ElectricPressureBullet(const char* pName) : LiveActor(pName), mFront(0.0f, 0.0f, 0.0f), mOwner(), mSpeed(0.0f), mTargetPlayer(false) {
}

void ElectricPressureBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "ElectricBullet", false);
    initNerve(GET_NERVE(ElectricPressureBullet, ElectricPressureBulletNrvFly));
    makeActorDead();
}

void ElectricPressureBullet::kill() {
    MR::tryHitReactionBreak(this);
    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "EmElecbublletBreak");
    LiveActor::kill();
}

namespace {
    char sLongBreakReaction[] = "破壊（余韻あり）";
    char sLongBreakEffect[] = "BreakLong";
}

static inline void killLong(LiveActor* pActor) {
    MR::tryHitReaction(pActor, sLongBreakReaction);
    MR::emitEffect(pActor, sLongBreakEffect);
    MR::startActionSound(pActor, "EmElecbublletBreak");
    pActor->LiveActor::kill();
}

void ElectricPressureBullet::shotElectricBullet(LiveActor* actor, const TPos3f& pos, const f32& value, bool flag) {
    mOwner = actor;
    mSpeed = value;
    mTargetPlayer = flag;
    pos.getZDir(mFront);
    MR::normalize(&mFront);
    mVelocity.scale(mSpeed, mFront);
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
                TVec3f target(0.0f, 0.0f, 0.0f);
                if (mTargetPlayer) {
                    TVec3f up(0.0f, 1.0f, 0.0f);
                    MR::getPlayerUpVec(&up);
                    JMAVECScaleAdd(&up, MR::getPlayerPos(), &target, 120.0f);
                } else {
                    target.set(*MR::getPlayerCenterPos());
                }
                TVec3f v3;
                v3.sub(target, mPosition);
                MR::normalize(&v3);
                MR::turnVecToVecDegree(&mFront, mFront, v3, 4.0f, TVec3f(0, 1, 0));
                mVelocity.scale(mSpeed, mFront);
            }
        }
    }

    if (MR::isBinded(this)) {
        if (MR::isBindedGround(this)) {
            MR::sendMsgEnemyAttackElectric(MR::getGroundSensor(this), getSensor(nullptr));
        } else if (MR::isBindedRoof(this)) {
            MR::sendMsgEnemyAttackElectric(MR::getRoofSensor(this), getSensor(nullptr));
        } else if (MR::isBindedWall(this)) {
            MR::sendMsgEnemyAttackElectric(MR::getWallSensor(this), getSensor(nullptr));
        }
        killLong(this);
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
