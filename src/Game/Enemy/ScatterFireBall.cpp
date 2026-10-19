#define COLOR8_GX_CTOR_INLINE
#include "Game/Enemy/ScatterFireBall.hpp"
#include "Game/Enemy/ScatterFireBallLava.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Map/HitInfo.hpp"

#define ADDRESS_NERVE(name, actorType, method) \
    class name : public Nerve { \
    public: \
        virtual void execute(Spine* spine) const { static_cast<actorType*>(spine->mExecutor)->method(); } \
        static name sInstance; \
    }; \
    name name::sInstance ATTRIBUTE_WEAK;

namespace NrvScatterFireBall {
    ADDRESS_NERVE(ScatterFireBallNrvThrow, ScatterFireBall, exeScatterThrow);
    NEW_NERVE(ScatterFireBallNrvFreeze, ScatterFireBall, Freeze);
    ADDRESS_NERVE(ScatterFireBallNrvReflect, ScatterFireBall, exeScatterReflect);
}

ScatterFireBall::ScatterFireBall(const char* name) : LiveActor(name), mHost(nullptr), mScaleCtrl(nullptr), mBindStarPointer(nullptr),
    mUpVec(0.0f, 1.0f, 0.0f), mStoredVelocity(0.0f, 1.0f, 0.0f), mUnusedScale(1.0f), mNoHitTimer(0), mIsFalling(false) {}

void ScatterFireBall::init(const JMapInfoIter&) {
    MR::initActor(this, "FireBall", "Spread", false);
    MR::invalidateClipping(this);
    mLava = new ScatterFireBallLava(this, "拡散溶岩");
    mLava->initWithoutIter();
    initNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow), 1);
    mScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(ScatterFireBall, ScatterFireBallNrvFreeze), "SupportFreeze");
    makeActorDead();
}

void ScatterFireBall::appear() {
    LiveActor::appear();
    mNoHitTimer = 60;
    setNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow));
}

void ScatterFireBall::kill() {
    mIsFalling = false;
    MR::forceDeleteEffect(this, "FireBallSpread");
    MR::emitEffect(this, "FireBallSpreadBreak");
    MR::startActionSound(this, "OjFireBallBreak", -1, -1, -1);
    LiveActor::kill();
}

void ScatterFireBall::shootWithSpreadAngle(const TVec3f& pos, f32 speed, f32 angle) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUpVec.negate(mGravity);
    setVelocityTowardPlayer(speed);
    TPos3f mtx;
    mtx.makeRotate(mUpVec, MR::toRadian(angle));
    mtx.mult33(mVelocity, mVelocity);
    appear();
}

void ScatterFireBall::shootWithVelocity(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUpVec.negate(mGravity);
    mVelocity.set(velocity);
    appear();
}

void ScatterFireBall::shootFalling(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUpVec.negate(mGravity);
    mVelocity.set(velocity);
    mIsFalling = true;
    appear();
}

bool ScatterFireBall::isFullyDead() { return MR::isDead(this) && MR::isDead(mLava); }

bool ScatterFireBall::tryKillLava() {
    if (!MR::isDead(mLava)) {
        mLava->kill();
        return true;
    }
    return false;
}

void ScatterFireBall::control() {
    if (mNoHitTimer > 0) mNoHitTimer--;
    static const GXColor light = {255, 192, 0, 255};
    MR::requestPointLight(this, mPosition, light, 1.0f, -1, -1.0f);
}

void ScatterFireBall::calcAndSetBaseMtx() {
    if (!MR::isNearZero(mVelocity, 0.001f)) {
        TVec3f front;
        TPos3f mtx;
        MR::normalize(&front, mVelocity);
        if (MR::isSameDirection(front, mUpVec, 0.01f)) {
            MR::makeMtxFrontNoSupportPos(&mtx, front, mPosition);
        } else {
            MR::makeMtxFrontUpPos(&mtx, front, mUpVec, mPosition);
        }
        MR::setBaseTRMtx(this, mtx);
    }
}

void ScatterFireBall::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver) && MR::sendMsgEnemyAttackFire(receiver, sender)) {
        kill();
        return;
    }
    if (MR::isSensorEnemy(receiver) && MR::getSensorHost(receiver) != mHost && MR::sendMsgEnemyAttackFire(receiver, sender)) kill();
}

bool ScatterFireBall::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgJetTurtleAttack(msg)) { kill(); return false; }
    if (MR::isMsgSupportTicoSpin(msg)) {
        if (isNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow)) || isNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvFreeze))) {
            scatterVelocity();
            setNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvReflect));
            return true;
        }
        return false;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
    if (MR::isMsgStarPieceAttack(msg)) { kill(); return true; }
    return false;
}

bool ScatterFireBall::receiveMsgEnemyAttack(u32, HitSensor* sender, HitSensor*) {
    if (!mHost) return false;
    if (MR::getSensorHost(sender) != mHost) return false;
    if (mNoHitTimer > 0) return false;
    kill();
    return true;
}

HitSensor* ScatterFireBall::getContactSensor() const {
    if (MR::isBindedGround(this)) return MR::getGroundSensor(this);
    if (MR::isBindedWall(this)) return MR::getWallSensor(this);
    if (MR::isBindedRoof(this)) return MR::getRoofSensor(this);
    return nullptr;
}

#pragma dont_inline on
bool ScatterFireBall::tryExplodeOnContact() {
    HitSensor* sensor = getContactSensor();
    if (sensor) {
        MR::sendMsgEnemyAttack(sensor, getSensor("Body"));
        MR::calcGravity(this);
        TVec3f pos(mPosition);
        Triangle triangle;
        TVec3f direction(mGravity);
        direction.scale(300.0f);
        MR::getFirstPolyOnLineToMapWithSensorType(&pos, &triangle, mPosition, direction, 24);
        mLava->landAt(pos);
        kill();
        return true;
    }
    if (!MR::isNearPlayer(this, 7000.0f)) { kill(); return true; }
    return false;
}

#pragma dont_inline reset
void ScatterFireBall::setVelocityTowardPlayer(f32 speed) {
    TVec3f target;
    target.scale(120.0f, mUpVec);
    target.add(*MR::getPlayerPos());
    TVec3f dir;
    dir.sub(target, mPosition);
    MR::normalize(&dir);
    mVelocity.scale(speed, dir);
}

void ScatterFireBall::scatterVelocity() {
    if (MR::isNearZero(mStoredVelocity, 0.001f)) mStoredVelocity.set(mUpVec);
    MR::turnVecRandomAroundAxis(&mVelocity, mUpVec, mStoredVelocity, -20.0f, 20.0f);
    mVelocity.x *= 100.0f;
    mVelocity.y *= 100.0f;
    mVelocity.z *= 100.0f;
}

void ScatterFireBall::exeScatterThrow() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Spin");
    if (MR::changeShowModelFlagSyncNearClipping(this, 200.0f)) MR::emitEffect(this, "FireBallSpread");
    else MR::deleteEffect(this, "FireBallSpread");
    if (mIsFalling) MR::startActionSound(this, "Fall", -1, -1, -1);
    else MR::startActionSound(this, "Fly", -1, -1, -1);
    if (MR::isGreaterStep(this, 30) && mBindStarPointer->tryStartPointBind()) {
        mStoredVelocity = mVelocity;
        setNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvFreeze));
        return;
    }
    if (tryExplodeOnContact()) return;
}

void ScatterFireBall::exeScatterReflect() {
    if (MR::isFirstStep(this)) {
        MR::start2PAttackAssistSound();
        MR::startActionSound(this, "EmFireBubbleReflect", -1, -1, -1);
    }
    mVelocity.x *= 0.96f;
    mVelocity.y *= 0.96f;
    mVelocity.z *= 0.96f;
    if (!tryExplodeOnContact() && MR::isStep(this, 60)) kill();
}

ScatterFireBall::~ScatterFireBall() {}

void ScatterFireBall::exeFreeze() {
    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow))) mVelocity = mStoredVelocity;
}
