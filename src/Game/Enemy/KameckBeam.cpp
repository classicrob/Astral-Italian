#pragma auto_inline off
#include "Game/Enemy/KameckBeam.hpp"
#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/KameckFireBall.hpp"
#include "Game/Enemy/KameckTurtle.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/Util.hpp"

namespace {
    const f32 sFireAngleLebel1[] = {0.0f};
    const f32 sFireAngleLebel2[] = {30.0f, -30.0f};
    const f32 sFireAngleLebel3[] = {0.0f, 120.0f, -120.0f};
    const f32* sFireAngleList[] = {nullptr, sFireAngleLebel1, sFireAngleLebel2, sFireAngleLebel3, nullptr};
    static const s32 sMaxBeamLife = 360;
    static const f32 sBeamRadius = 80.0f;
    // const s32 sMorphTime =
    static const s32 sBurningTime = 40;
    static const f32 sStormRange = 500.0f;
};  // namespace

KameckBeamEventListener::KameckBeamEventListener() {
}

bool KameckBeamCollisionFilter::isInvalidParts(const CollisionParts* pCollisionParts) const {
    u32 sensorType = MR::getCollisionSensorType(pCollisionParts);

    if (sensorType == 87 /* ATYPE_KAMECK_BARRIER */) {
        return true;
    }

    if (sensorType == 83 /* ATYPE_CLIP_FIELD_MAP_PARTS */) {
        return true;
    }

    if (!MR::isExistClipAreaHolder()) {
        return false;
    }

    if (sensorType != 67) return false;
    return !MR::isInClipArea(*mBeamPosition);
}

namespace NrvKameckBeam {
    NEW_NERVE(KameckBeamNrvFollowWand, KameckBeam, FollowWand);
    NEW_NERVE(KameckBeamNrvShoot, KameckBeam, Shoot);
    NEW_NERVE(KameckBeamNrvExplosion, KameckBeam, Explosion);
    class KameckBeamNrvFire : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<KameckBeam*>(spine->mExecutor)->exeProjectileFlight(); }
        static KameckBeamNrvFire sInstance;
    };
    KameckBeamNrvFire KameckBeamNrvFire::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKameckBeam

KameckBeam::KameckBeam(const char* pName, s32 kind, s32 count)
    : LiveActor(pName), mEventListener(), mFollowMtx(), mShootDirection(0, 0, 1), mWandLocalPosition(0, 0, 0), mBeamKind(kind), mProjectileCount(count) {
    for (u32 i = 0; i < ARRAY_SIZE(mProjectiles); i++) {
        mProjectiles[i] = nullptr;
    }
}

namespace MR {
    void setKameckBeamCollisionFilter(LiveActor* pActor) {
        MR::setBinderCollisionPartsFilter(pActor, new KameckBeamCollisionFilter(&pActor->mPosition));
    }
};  // namespace MR

KameckBeamCollisionFilter::KameckBeamCollisionFilter(const TVec3f* pVec) : mBeamPosition(pVec) {
}

void KameckBeamProjectile::resetBeamProjectile() {}
bool KameckBeamProjectile::tryStormProjectile() { return false; }
void KameckBeamProjectile::appearDirection(const TVec3f&, const TVec3f&) {}
bool KameckBeamProjectile::isBeamProjectileDead() const { return false; }

void KameckBeam::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneEnemyDecorationMovementCalcAnim(this);
    initNerve(GET_NERVE(KameckBeam, KameckBeamNrvFollowWand));
    initSound(4, "KameckBeam", nullptr, TVec3f(0.0f));
    initBinder(::sBeamRadius, 0.0f, 0);
    MR::setBinderCollisionPartsFilter(this, new KameckBeamCollisionFilter(&mPosition));
    initEffectKeeper(0, "BossKameckBeam", false);
    MR::setEffectHostSRT(this, "BeamTurtleReady", &mPosition, nullptr, nullptr);
    MR::setEffectHostSRT(this, "BeamFireReady", &mPosition, nullptr, nullptr);
    MR::setEffectHostSRT(this, "BeamEnemyReady", &mPosition, nullptr, nullptr);
    MR::setEffectHostSRT(this, "BeamTurtle", &mPosition, nullptr, nullptr);
    MR::setEffectHostSRT(this, "BeamFire", &mPosition, nullptr, nullptr);
    MR::setEffectHostSRT(this, "BeamEnemy", &mPosition, nullptr, nullptr);
    initHitSensor(1);
    MR::addHitSensorEnemyAttack(this, "attack", 8, ::sBeamRadius, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initShadowVolumeSphere(this, ::sBeamRadius);
    MR::onCalcShadow(this, nullptr);
    switch (mBeamKind) {
    case BeamType_FireBall1:
        for (u32 i = 0; i < mProjectileCount; i++) {
            KameckFireBall* ball = new KameckFireBall("火弾");
            ball->initWithoutIter();
            mProjectiles[i] = ball;
        }
        break;
    case BeamType_Turtle:
        for (u32 i = 0; i < mProjectileCount; i++) {
            KameckTurtle* turtle = new KameckTurtle("カメ弾");
            turtle->initWithoutIter();
            mProjectiles[i] = turtle;
        }
        break;
    case BeamType_KuriboMini:
        for (u32 i = 0; i < mProjectileCount; i++) mProjectiles[i] = new KameckBeamKuriboMini;
        break;
    case BeamType_Meramera:
        for (u32 i = 0; i < mProjectileCount; i++) mProjectiles[i] = new KameckBeamMeramera;
        break;
    }
    MR::invalidateClipping(this);
    makeActorDead();
}

void KameckBeam::control() {
}

void KameckBeam::kill() {
    LiveActor::kill();
    resetBeam();
}

void KameckBeam::calcAnim() {
    if (mFollowMtx != nullptr) {
        PSMTXMultVec(mFollowMtx, mWandLocalPosition, mPosition);
    }
}

void KameckBeam::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if ((MR::isSensorPlayer(pReceiver) || MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) && sendBeamAttack(pSender, pReceiver)) {
        if (mEventListener != nullptr) mEventListener->hitBeam();
        kill();
    }
}

bool KameckBeam::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSpinStormRange(msg)) {
        return requestStorm(pSender, pReceiver);
    }

    if (MR::isMsgStartPowerStarGet(msg)) {
        makeActorDead();

        return true;
    }

    return false;
}

void KameckBeam::setWandLocalPosition(const TVec3f& rLocalPos) {
    mWandLocalPosition.set(rLocalPos);
}

void KameckBeam::setEventListener(KameckBeamEventListener* pEventListener) {
    mEventListener = pEventListener;
}

void KameckBeam::resetBeam() {
    for (u32 i = 0; i < mProjectileCount; i++) mProjectiles[i]->resetBeamProjectile();
}

bool KameckBeam::requestFollowWand(MtxPtr pMtx, f32 scale) {
    MR::setScale(this, scale);
    MR::setShadowVolumeSphereRadius(this, nullptr, scale * ::sBeamRadius);
    MR::setBinderRadius(this, scale * ::sBeamRadius);
    MR::setSensorRadius(this, "attack", scale * ::sBeamRadius);
    emitBeamReadyEffect();
    mFollowMtx = pMtx;
    makeActorAppeared();
    setNerve(GET_NERVE(KameckBeam, KameckBeamNrvFollowWand));
    MR::offBind(this);
    MR::invalidateHitSensors(this);
    MR::invalidateShadow(this, nullptr);

    return true;
}

void KameckBeam::requestShootToPlayerCenter(f32 speed) {
    TVec3f shootDir;
    MR::getPlayerGroundPos(&shootDir);
    shootDir += mGravity * 80.0f;
    shootDir -= mPosition;
    MR::normalizeOrZero(&shootDir);
    if (MR::isNearZero(shootDir)) {
        MR::calcGravity(this);
        shootDir.set(mGravity);
    }
    requestShoot(shootDir, speed);
}

void KameckBeam::requestShoot(const TVec3f& rDir, f32 speed) {
    emitBeamEffect();
    MR::onBind(this);
    MR::validateHitSensors(this);
    MR::validateShadow(this, nullptr);
    mFollowMtx = nullptr;
    setNerve(GET_NERVE(KameckBeam, KameckBeamNrvShoot));
    mShootDirection.set(rDir);
    mVelocity.set(rDir * speed);
}

bool KameckBeam::requestStorm(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::getSensorPos(pSender)->distance(*MR::getSensorPos(pReceiver)) >= 500.0f) return false;
    if (!isNerve(GET_NERVE(KameckBeam, KameckBeamNrvShoot))) return false;
    for (u32 i = 0; i < mProjectileCount; i++) {
        if (mProjectiles[i]->tryStormProjectile()) {
            kill();
            return true;
        }
    }
    return false;
}

bool KameckBeam::tryShootEnd() {
    if (MR::isBinded(this)) {
        MR::sendMsgEnemyAttackExplosionToBindedSensor(this, getSensor("attack"));
        MR::invalidateHitSensors(this);
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(KameckBeam, KameckBeamNrvExplosion));

        return true;
    }

    if (MR::isGreaterStep(this, ::sMaxBeamLife)) {
        kill();

        return true;
    }

    return false;
}

bool KameckBeam::tryFireProjectiles() {
    TVec3f direction;
    direction.killElement(mShootDirection, mGravity);
    if (MR::isNearZero(direction)) MR::makeAxisVerticalZX(&direction, mGravity);
    else MR::normalize(&direction);
    for (u32 i = 0; i < mProjectileCount; i++) {
        mProjectiles[i]->setProjectileEventListener(mEventListener);
        TVec3f rotated;
        MR::rotateVecDegree(&rotated, direction, mGravity, sFireAngleList[mProjectileCount][i]);
        mProjectiles[i]->appearDirection(mPosition, rotated);
    }
    MR::offBind(this);
    MR::invalidateShadow(this, nullptr);
    MR::invalidateHitSensors(this);
    MR::forceDeleteEffectAll(this);
    setNerve(GET_NERVE(KameckBeam, KameckBeamNrvFire));
    return true;
}

void KameckBeam::exeShoot() {
    if (MR::isFirstStep(this)) MR::startActionSound(this, "BmKameckFireShoot");
    if (MR::isInWater(mPosition) || MR::isBindedGroundSinkDeath(this)) {
        emitBeamBreakEffect();
        kill();
        return;
    }
    if (!tryShootEnd()) {
        startBeamLevelSound();
        MR::startActionSound(this, "BmLvKameckMagicComm");
    }
}

void KameckBeam::exeExplosion() {
    if (tryFireProjectiles()) {
        MR::shakeCameraNormalWeak();
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        emitBeamBreakEffect();
        return;
    }
    kill();
}

void KameckBeam::exeProjectileFlight() {
    bool dead = true;
    for (u32 i = 0; i < mProjectileCount; i++) {
        if (!mProjectiles[i]->isBeamProjectileDead()) { dead = false; break; }
    }
    if (dead) kill();
}

bool KameckBeam::sendBeamAttack(HitSensor* sender, HitSensor* receiver) {
    switch (mBeamKind) {
    case BeamType_Turtle: return MR::sendMsgEnemyAttackStrong(receiver, sender);
    case BeamType_FireBall1: return MR::sendMsgEnemyAttackFire(receiver, sender);
    default: return MR::sendMsgEnemyAttack(receiver, sender);
    }
}

void KameckBeam::startBeamLevelSound() {
    switch (mBeamKind) {
    case BeamType_Turtle: MR::startActionSound(this, "BmLvKameckMagicTurt"); break;
    case BeamType_FireBall1: MR::startActionSound(this, "BmLvKameckMagicFire"); break;
    default: MR::startActionSound(this, "BmLvKameckMagicTurt"); break;
    }
}

void KameckBeam::emitBeamReadyEffect() {
    switch (mBeamKind) {
    case BeamType_Turtle: MR::emitEffect(this, "BeamTurtleReady"); break;
    case BeamType_FireBall1: MR::emitEffect(this, "BeamFireReady"); break;
    default: MR::emitEffect(this, "BeamEnemyReady"); break;
    }
}

void KameckBeam::emitBeamEffect() {
    switch (mBeamKind) {
    case BeamType_Turtle: MR::emitEffect(this, "BeamTurtle"); break;
    case BeamType_FireBall1: MR::emitEffect(this, "BeamFire"); break;
    default: MR::emitEffect(this, "BeamEnemy"); break;
    }
}

void KameckBeam::emitBeamBreakEffect() {
    switch (mBeamKind) {
    case BeamType_Turtle: MR::emitEffect(this, "BeamTurtleBreak"); return;
    case BeamType_FireBall1:
        MR::emitEffect(this, "BeamFireBreak");
        MR::startActionSound(this, "BmKameckFireExplosion");
        return;
    default: MR::emitEffect(this, "BeamEnemyBreak"); return;
    }
}

void KameckBeam::setBeamKind(s32 kind) {
    mBeamKind = kind;
}

void KameckBeam::requestShootToPlayerGround(f32 speed) {
    TVec3f shootDir;
    MR::getPlayerGroundPos(&shootDir);

    shootDir -= mGravity * ::sBeamRadius;
    shootDir -= mPosition;
    MR::normalizeOrZero(&shootDir);

    if (MR::isNearZero(shootDir)) {
        MR::calcGravity(this);
        shootDir.set(mGravity);
    }

    requestShoot(shootDir, speed);
}

void KameckBeam::exeFollowWand() {
    startBeamLevelSound();
}

