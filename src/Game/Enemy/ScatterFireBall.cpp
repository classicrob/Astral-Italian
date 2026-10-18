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
    ADDRESS_NERVE(ScatterFireBallNrvThrow, ScatterFireBall, fn_8017EE80);
    NEW_NERVE(ScatterFireBallNrvFreeze, ScatterFireBall, Freeze);
    ADDRESS_NERVE(ScatterFireBallNrvReflect, ScatterFireBall, fn_8017EF90);
}

ScatterFireBall::ScatterFireBall(const char* name) : LiveActor(name), mHost(nullptr), _94(nullptr), _98(nullptr),
    _9C(0.0f, 1.0f, 0.0f), _A8(0.0f, 1.0f, 0.0f), _B4(1.0f), _BC(0), _C0(false) {}

void ScatterFireBall::init(const JMapInfoIter&) {
    MR::initActor(this, "FireBall", "Spread", false);
    MR::invalidateClipping(this);
    _B8 = new ScatterFireBallLava(this, "拡散溶岩");
    _B8->initWithoutIter();
    initNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow), 1);
    _94 = new AnimScaleController(nullptr);
    _98 = new WalkerStateBindStarPointer(this, _94);
    MR::initActorState(this, _98, GET_NERVE(ScatterFireBall, ScatterFireBallNrvFreeze), "SupportFreeze");
    makeActorDead();
}

void ScatterFireBall::appear() {
    LiveActor::appear();
    _BC = 60;
    setNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow));
}

void ScatterFireBall::kill() {
    _C0 = false;
    MR::forceDeleteEffect(this, "FireBallSpread");
    MR::emitEffect(this, "FireBallSpreadBreak");
    MR::startActionSound(this, "OjFireBallBreak", -1, -1, -1);
    LiveActor::kill();
}

void ScatterFireBall::fn_8017E490(const TVec3f& pos, f32 speed, f32 angle) {
    mPosition.set(pos);
    MR::calcGravity(this);
    _9C.negate(mGravity);
    fn_8017ED40(speed);
    TPos3f mtx;
    mtx.makeRotate(_9C, MR::toRadian(angle));
    mtx.mult33(mVelocity, mVelocity);
    appear();
}

void ScatterFireBall::fn_8017E660(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    _9C.negate(mGravity);
    mVelocity.set(velocity);
    appear();
}

void ScatterFireBall::fn_8017E6D0(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    _9C.negate(mGravity);
    mVelocity.set(velocity);
    _C0 = true;
    appear();
}

bool ScatterFireBall::fn_8017E750() { return MR::isDead(this) && MR::isDead(_B8); }

bool ScatterFireBall::fn_8017E7B0() {
    if (!MR::isDead(_B8)) {
        _B8->kill();
        return true;
    }
    return false;
}

// NONMATCHING: color argument copy temporaries, stack slots and saved-register allocation differ.
void ScatterFireBall::control() {
    if (_BC > 0) _BC--;
    const GXColor light = {255, 192, 0, 255};
    MR::requestPointLight(this, mPosition, Color8(light.r, light.g, light.b, light.a), 1.0f, -1, -1.0f);
}

void ScatterFireBall::calcAndSetBaseMtx() {
    if (!MR::isNearZero(mVelocity, 0.001f)) {
        TVec3f front;
        TPos3f mtx;
        MR::normalize(&front, mVelocity);
        if (MR::isSameDirection(front, _9C, 0.01f)) {
            MR::makeMtxFrontNoSupportPos(&mtx, front, mPosition);
        } else {
            MR::makeMtxFrontUpPos(&mtx, front, _9C, mPosition);
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
            fn_8017EDF0();
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
    if (_BC > 0) return false;
    kill();
    return true;
}

HitSensor* ScatterFireBall::fn_8017EBB0() const {
    if (MR::isBindedGround(this)) return MR::getGroundSensor(this);
    if (MR::isBindedWall(this)) return MR::getWallSensor(this);
    if (MR::isBindedRoof(this)) return MR::getRoofSensor(this);
    return nullptr;
}

#pragma dont_inline on
bool ScatterFireBall::fn_8017EC30() {
    HitSensor* sensor = fn_8017EBB0();
    if (sensor) {
        MR::sendMsgEnemyAttack(sensor, getSensor("Body"));
        MR::calcGravity(this);
        TVec3f pos(mPosition);
        Triangle triangle;
        TVec3f direction(mGravity);
        direction.scale(300.0f);
        MR::fn_80038020(&pos, &triangle, mPosition, direction, 24);
        _B8->fn_8017F450(pos);
        kill();
        return true;
    }
    if (!MR::isNearPlayer(this, 7000.0f)) { kill(); return true; }
    return false;
}

#pragma dont_inline reset
void ScatterFireBall::fn_8017ED40(f32 speed) {
    TVec3f target;
    target.scale(120.0f, _9C);
    target.add(*MR::getPlayerPos());
    TVec3f dir;
    dir.sub(target, mPosition);
    MR::normalize(&dir);
    mVelocity.scale(speed, dir);
}

void ScatterFireBall::fn_8017EDF0() {
    if (MR::isNearZero(_A8, 0.001f)) _A8.set(_9C);
    MR::turnVecRandomAroundAxis(&mVelocity, _9C, _A8, -20.0f, 20.0f);
    mVelocity.x *= 100.0f;
    mVelocity.y *= 100.0f;
    mVelocity.z *= 100.0f;
}

void ScatterFireBall::fn_8017EE80() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Spin");
    if (MR::changeShowModelFlagSyncNearClipping(this, 200.0f)) MR::emitEffect(this, "FireBallSpread");
    else MR::deleteEffect(this, "FireBallSpread");
    if (_C0) MR::startActionSound(this, "Fall", -1, -1, -1);
    else MR::startActionSound(this, "Fly", -1, -1, -1);
    if (MR::isGreaterStep(this, 30) && _98->tryStartPointBind()) {
        _A8 = mVelocity;
        setNerve(GET_NERVE(ScatterFireBall, ScatterFireBallNrvFreeze));
        return;
    }
    if (fn_8017EC30()) return;
}

void ScatterFireBall::fn_8017EF90() {
    if (MR::isFirstStep(this)) {
        MR::start2PAttackAssistSound();
        MR::startActionSound(this, "EmFireBubbleReflect", -1, -1, -1);
    }
    mVelocity.x *= 0.96f;
    mVelocity.y *= 0.96f;
    mVelocity.z *= 0.96f;
    if (!fn_8017EC30() && MR::isStep(this, 60)) kill();
}

ScatterFireBall::~ScatterFireBall() {}

void ScatterFireBall::exeFreeze() {
    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(ScatterFireBall, ScatterFireBallNrvThrow))) mVelocity = _A8;
}
