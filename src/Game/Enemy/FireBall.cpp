#include "Game/Enemy/FireBall.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/FireBubble.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace {
    const f32 cSensorRadius = 40.0f;
    const f32 cBinderRadius = 60.0f;
    const f32 cForceKillDistance = 7000.0f;
    const f32 cStarWandRadius3d = 150.0f;
    const f32 cThrowOffsetY = 120.0f;
    const f32 cReflectSpeed = 100.0f;
    const f32 cReflectReduceRate = 0.96f;
    const s32 cReflectFrame = 30;
    const s32 cReflectInvalidFrame = 60;
    const f32 cReflectCursorSpeed = 30.0f;
    const f32 cReflectRandomVelocityY = 1.0f;
    static const GXColor color = {255, 192, 0, 255};
};  // namespace

namespace NrvFireBall {
    NEW_NERVE(FireBallNrvThrow, FireBall, Throw);
};  // namespace NrvFireBall

inline void FireBall::exeFreeze() {
    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(FireBall, FireBallNrvThrow))) {
        mVelocity = mReflectDir;
    }
}
namespace NrvFireBall {
    NEW_NERVE(FireBallNrvFreeze, FireBall, Freeze);
    NEW_NERVE(FireBallNrvReflect, FireBall, Reflect);
}

FireBall::FireBall(const char* pName) : LiveActor(pName), mHost(), mScaleController(0), mBindStarPointer(0), mReflectDir(0.0f, 1.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mUnusedB4(1.0f) {
}

inline void FireBall::initHitSensors(const char* const hitSensor) {
    initHitSensor(1);
    MR::addHitSensorEnemy(this, hitSensor, 8, ::cSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));
}

void FireBall::init(const JMapInfoIter&) {
    MR::initActor(this, "FireBall", false);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(FireBall, FireBallNrvThrow), 1);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(FireBall, FireBallNrvFreeze), "SupportFreeze");
    makeActorDead();
}

void FireBall::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(FireBall, FireBallNrvThrow));
}

void FireBall::kill() {
    MR::forceDeleteEffect(this, "FireBall");

    MR::emitEffect(this, "FireBallBreak");

    MR::startActionSound(this, "OjFireBallBreak");

    LiveActor::kill();
}

void FireBall::appearAndThrow(const TVec3f& rAppearPos, f32 speed, f32 angle) {
    mPosition.set(rAppearPos);

    MR::calcGravity(this);

    mUp.negate(mGravity);

    setVelocityToPlayer(speed);

    TPos3f mtx;
    mtx.makeRotate(mUp, MR::toRadian(angle));
    mtx.mult33(mVelocity, mVelocity);

    appear();
}

// NONMATCHING: Color8 argument construction and temporary copies lower differently.
void FireBall::control() {
    GXColor light = color;
    MR::requestPointLight(this, mPosition, Color8(light.r, light.g, light.b, light.a), 1.0f, -1);
}

void FireBall::calcAndSetBaseMtx() {
    if (!MR::isNearZero(mVelocity)) {
        TVec3f frontVec;
        TPos3f baseMtx;
        MR::normalize(&frontVec, mVelocity);
        if (MR::isSameDirection(frontVec, mUp, 0.01f)) {
            MR::makeMtxFrontNoSupportPos(&baseMtx, frontVec, mPosition);
        } else {
            MR::makeMtxFrontUpPos(&baseMtx, frontVec, mUp, mPosition);
        }
        MR::setBaseTRMtx(this, baseMtx);
    }
}

void FireBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
        kill();
        return;
    }

    if (MR::isSensorEnemy(pReceiver) && MR::getSensorHost(pReceiver) != mHost && MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
        kill();
    }
}

bool FireBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgJetTurtleAttack(msg)) {
        kill();
        return false;
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (isNerve(GET_NERVE(FireBall, FireBallNrvThrow)) || isNerve(GET_NERVE(FireBall, FireBallNrvFreeze))) {
            randomizeReflectVelocity();
            setNerve(GET_NERVE(FireBall, FireBallNrvReflect));
            return true;
        }
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        kill();
        return true;
    }

    return false;
}

HitSensor* FireBall::isBindedAny() const {
    if (MR::isBindedGround(this)) {
        return MR::getGroundSensor(this);
    }

    if (MR::isBindedWall(this)) {
        return MR::getWallSensor(this);
    }

    if (MR::isBindedRoof(this)) {
        return MR::getRoofSensor(this);
    }

    return nullptr;
}

bool FireBall::tryToKill() {
    HitSensor* bindedSensor = isBindedAny();

    if (bindedSensor != nullptr) {
        MR::sendMsgEnemyAttack(bindedSensor, getSensor("Body"));

        kill();

        return true;
    }

    if (!MR::isNearPlayer(this, ::cForceKillDistance)) {
        kill();
        return true;
    }

    return false;
}

// NONMATCHING: equivalent vector addition remains an out-of-line JMath helper.
void FireBall::setVelocityToPlayer(f32 magnitude) {
    TVec3f target;
    target.scale(::cThrowOffsetY, mUp);
    JMathInlineVEC::PSVECAdd(&target, MR::getPlayerPos(), &target);

    TVec3f dir;
    dir.sub(target, mPosition);
    MR::normalize(&dir);
    mVelocity.scale(magnitude, dir);
}

void FireBall::calcReflectVelocity() {
    MR::getStarPointerWorldVelocityDirection(&mVelocity, *MR::getStarPointerLastPointedPort(this));

    if (MR::isNearZero(mVelocity)) {
        mVelocity.set(mUp);
    } else {
        MR::vecKillElement(mVelocity, mUp, &mVelocity);

        TVec3f accel;
        accel.scale(MR::getRandom(0.0f, ::cReflectRandomVelocityY), mUp);
        mVelocity.add(accel);

        if (MR::isNearZero(mVelocity)) {
            mVelocity.set(mUp);
        } else {
            MR::normalize(&mVelocity);
        }
    }

    mVelocity.mult(::cReflectSpeed);
}

void FireBall::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
    }

    if (MR::changeShowModelFlagSyncNearClipping(this, 200.0f)) {
        MR::emitEffect(this, "FireBall");
    } else {
        MR::deleteEffect(this, "FireBall");
    }

    MR::startActionSound(this, "Fly");
    if (MR::isGreaterStep(this, ::cReflectFrame) && mBindStarPointer->tryStartPointBind()) {
        mReflectDir = mVelocity;
        setNerve(GET_NERVE(FireBall, FireBallNrvFreeze));
        return;
    }

    if (tryToKill()) {
        return;
    }
}

void FireBall::exeReflect() {
    if (MR::isFirstStep(this)) {
        MR::start2PAttackAssistSound();
        MR::startActionSound(this, "EmFireBubbleReflect");
    }

    mVelocity.mult(::cReflectReduceRate);

    if (!tryToKill() && MR::isStep(this, ::cReflectInvalidFrame)) {
        kill();
    }
}

void FireBall::launchFireBallWithVelocity(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set<f32>(pos);
    MR::calcGravity(this);
    mUp.negate(mGravity);
    mVelocity.set<f32>(velocity);
    appear();
}

void FireBall::randomizeReflectVelocity() {
    if (MR::isNearZero(mReflectDir)) {
        mReflectDir.set<f32>(mUp);
    }
    MR::turnVecRandomAroundAxis(&mVelocity, mUp, mReflectDir, -20.0f, 20.0f);
    mVelocity *= 100.0f;
}
