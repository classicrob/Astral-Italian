#include "Game/MapObj/FirePressureBullet.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"

namespace NrvFirePressureBullet {
    NEW_NERVE(FirePressureBulletNrvFly, FirePressureBullet, Fly);
    NEW_NERVE(FirePressureBulletNrvBindStarPointer, FirePressureBullet, BindStarPointer);
};  // namespace NrvFirePressureBullet

FirePressureBullet::FirePressureBullet(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
    _8C.x = 0.0f;
    _8C.y = 0.0f;
    _8C.z = 0.0f;
    mFirePressure = nullptr;
    _9C = 0.0f;
    _A0 = false;
    _A1 = false;
}

void FirePressureBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "FireBullet", false);
    initNerve(&NrvFirePressureBullet::FirePressureBulletNrvFly::sInstance, 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStarPointer, &NrvFirePressureBullet::FirePressureBulletNrvBindStarPointer::sInstance, nullptr);
    makeActorDead();
}

void FirePressureBullet::kill() {
    MR::startActionSound(this, "OjFPressureFireBurst");
    LiveActor::kill();
}

void FirePressureBullet::control() {
    mAnimScaleCtrl->updateNerve();
}

void FirePressureBullet::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, _8C);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void FirePressureBullet::shotFireBullet(LiveActor* pActor, const TPos3f& rPos, const f32& a3, bool a4, bool a5) {
    mFirePressure = pActor;
    _9C = a3;
    _A0 = a4;
    _A1 = a5;

    rPos.getZDir(_8C);
    mVelocity.scale(_9C, _8C);
    rPos.getTrans(mPosition);
    mRotation.zero();
    MR::validateHitSensors(this);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(FirePressureBullet, FirePressureBulletNrvFly));
    makeActorAppeared();
}

void FirePressureBullet::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::startBtk(this, "FireBullet");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Move");
    }

    MR::startActionSound(this, "OjLvFPressureBullet");

    if (_A0 && !(getNerveStep() % 5)) {
        MR::turnDirectionToTargetDegree(this, &_8C, *MR::getPlayerPos(), 3.0f);
        mVelocity.scale(_9C, _8C);
    }

    if (mBindStarPointer->tryStartPointBind()) {
        setNerve(&NrvFirePressureBullet::FirePressureBulletNrvBindStarPointer::sInstance);
    } else if (isCrash()) {
        kill();
    } else if (MR::isStep(this, 300)) {
        kill();
    }
}

void FirePressureBullet::exeBindStarPointer() {
    if (MR::updateActorStateAndNextNerve(this, &NrvFirePressureBullet::FirePressureBulletNrvFly::sInstance)) {
        mVelocity.scale(_9C, _8C);
    }
}

void FirePressureBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
        if (MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
            kill();
        }
    } else if (MR::isSensorPlayerOrRide(pReceiver) && MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
        kill();
    }
}

bool FirePressureBullet::isCrash() const {
    if (_A1) {
        return false;
    }

    return MR::isBinded(this);
}

FirePressureBullet::~FirePressureBullet() {
}
