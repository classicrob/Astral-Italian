#pragma auto_inline off
#include "Game/MapObj/StarPieceTracking.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Util/Color.hpp"
#endif
#include "JSystem/JMath/JMath.hpp"
#include "JSystem/JMath/JMATrigonometric.hpp"

namespace NrvStarPieceTrackingPiece {
    NEW_NERVE(StarPieceTrackingPieceNrvFly, StarPieceTrackingPiece, Fly);
    NEW_NERVE(StarPieceTrackingPieceNrvFlyWorldMap, StarPieceTrackingPiece, FlyWorldMap);
    NEW_NERVE(StarPieceTrackingPieceNrvTracking, StarPieceTrackingPiece, Tracking);
    NEW_NERVE(StarPieceTrackingPieceNrvTrackingWorldMap, StarPieceTrackingPiece, TrackingWorldMap);
}

StarPieceTrackingPiece::StarPieceTrackingPiece(const char* name)
    : LiveActor(name), mColor((GXColor){0, 0, 0, 255}), mDirection(0.0f, 0.0f, 0.0f), mUp(0.0f, 0.0f, 0.0f), mSpeed(1.4f),
      mTrackingVelocity(0.0f, 0.0f, 0.0f), mTrackingPosition(0.0f, 0.0f, 0.0f), mTrackingStartPosition(0.0f, 0.0f, 0.0f) {}

void StarPieceTrackingPiece::init(const JMapInfoIter& iter) {
    u32 color = StarPieceDirector::getRandomColor();
#ifdef GALAXY_NATIVE
    mColor = static_cast<GXColor>(Color8(color));
#else
    mColor = *reinterpret_cast<const GXColor*>(&color);
#endif
    MR::initActor(this, iter, "StarPiece", true);
    MR::initDLMakerMatColor0(this, "StarPiece1_v", &mColor);
    MR::newDifferedDLBuffer(this);
    MR::connectToSceneNoSilhouettedMapObj(this);
    initEffectKeeper(0, "StarPiece", false);
    initSound(4, "StarPieceHoming", NULL, TVec3f(0.0f));
    f32 radius = 60.0f;
    initHitSensor(2);
    MR::addHitSensorEye(this, "attack", 16, radius, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensor(this, "Body", 0x46, 16, radius, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initStarPointerTarget(this, 28.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::onCalcGravity(this);
    MR::startBtk(this, "Gift");
    MR::setBtkFrameAndStop(this, 5.0f);
    initNerve(&NrvStarPieceTrackingPiece::StarPieceTrackingPieceNrvFly::sInstance, 0);
    makeActorDead();
}

void StarPieceTrackingPiece::appearFromActor(const LiveActor* actor, const TVec3f& direction, const TVec3f& up, f32 speed, f32 height, f32 distance) {
    appear(actor->mPosition, direction, up, speed, height, distance);
}

void StarPieceTrackingPiece::appear(const TVec3f& position, const TVec3f& direction, const TVec3f& up, f32 speed, f32 height, f32 distance) {
    mPosition = position + up * height;
    mDirection = direction;
    mUp = up;
    mSpeed = speed;
    mDistance = distance;
    MR::invalidateClipping(this);
    static_cast<LiveActor*>(this)->appear();
}

// NONMATCHING: vector temporaries occupy different stack slots.
void StarPieceTrackingPiece::appearScattered(const TVec3f& position, const TVec3f& up, f32 speed, f32 height) {
    TVec3f random(0.0f);
    MR::getRandomVector(&random, speed);
    random = random * 50.0f;
    MR::vecKillElement(random, up, &random);
    MR::normalizeOrZero(&mUp, up);
    mPosition = position;
    TVec3f launch(mUp);
    launch *= height;
    mVelocity.set(launch + random);
    MR::setScale(this, 10.0f);
    MR::emitEffect(this, "WorldMapStarPieceLight");
    MR::startBtkAndSetFrameAndStop(this, "WorldMap", 0.0f);
    MR::invalidateClipping(this);
    setNerve(&NrvStarPieceTrackingPiece::StarPieceTrackingPieceNrvFlyWorldMap::sInstance);
    static_cast<LiveActor*>(this)->appear();
}

// NONMATCHING: distance multiplication uses swapped commutative operands.
void StarPieceTrackingPiece::updateTrackingVelocity(bool) {
    TVec3f direction(0.0f);
    TVec3f playerPosition(*MR::getPlayerPos());
    direction = playerPosition - mPosition;
    f32 distance = PSVECMag(&direction);
    MR::normalizeOrZero(&direction, direction);
    f32 acceleration = 0.01f * (distance * 0.3f);
    mTrackingVelocity *= 0.98f;
    mTrackingVelocity += direction * acceleration;
}

void StarPieceTrackingPiece::exeFly() {
    if (MR::isFirstStep(this)) {
        calcLaunchVelocity();
        MR::emitEffect(this, "StarPieceFlyingBlur");
    } else {
        mVelocity -= mUp * mSpeed;
    }
    if (MR::isGreaterStep(this, 30)) setNerve(&NrvStarPieceTrackingPiece::StarPieceTrackingPieceNrvTracking::sInstance);
}

void StarPieceTrackingPiece::exeFlyWorldMap() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::emitEffect(this, "StarPieceFlyingBlur");
        mTrackingVelocity = mVelocity;
    }
    updateTrackingVelocity(false);
    f32 dot = mUp.dot(mTrackingVelocity);
    mVelocity = mTrackingVelocity;
    if (dot < 0.0f) {
        setNerve(&NrvStarPieceTrackingPiece::StarPieceTrackingPieceNrvTrackingWorldMap::sInstance);
        MR::validateHitSensors(this);
        mTrackingStartPosition = mPosition;
        mTrackingPosition = mTrackingStartPosition;
        MR::zeroVelocity(this);
    }
}

// NONMATCHING: vector temporaries occupy different stack slots.
void StarPieceTrackingPiece::exeTracking() {
    TVec3f direction(0.0f);
    const TVec3f* playerVelocity = MR::getPlayerVelocity();
    TVec3f predictedPosition = *MR::getPlayerPos() + *playerVelocity;
    direction = predictedPosition - mPosition;
    MR::normalizeOrZero(&direction, direction);
    mVelocity *= 0.9f;
    mVelocity += direction * 14.0f;
}

void StarPieceTrackingPiece::exeTrackingWorldMap() {
    updateTrackingVelocity(true);
    mTrackingPosition += mTrackingVelocity;
    f32 rate = MR::calcNerveRate(this, 60);
    TVec3f playerPosition(*MR::getPlayerPos());
    mPosition = mTrackingPosition * (1.0f - rate) + playerPosition * rate;
}

void StarPieceTrackingPiece::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!isNerve(&NrvStarPieceTrackingPiece::StarPieceTrackingPieceNrvFly::sInstance) && MR::isSensorPlayerOrRide(receiver)) {
        if (MR::isEqualSensor(sender, this, "attack")) {
            MR::gotByPlayer();
            MR::tryRumblePadMiddle(this, 0);
            MR::forceDeleteEffect(this, "StarPieceFlyingBlur");
            MR::emitEffect(this, "StarPieceGet");
            kill();
        }
    }
}

void StarPieceTrackingPiece::calcLaunchVelocity() {
    s32 launchTime = 30;
    f32 frontSpeed = mDistance / static_cast<f32>(launchTime);
    f32 upSpeed = mSpeed * static_cast<f32>(launchTime);
    mVelocity = mUp * upSpeed + mDirection * frontSpeed;
}

// NONMATCHING: loop register allocation and conversion scheduling differ.
void StarPieceTracking::appear(const TVec3f& position, const TVec3f& up, const TVec3f& side, const TVec3f& front, f32 angle, f32 speed, f32 height, f32 distance) {
    s32 count = mObjNum;
    TVec3f upVector(up);
    f32 interval;
    if (MR::isNearZero(360.0f - angle, 0.001f)) interval = angle / static_cast<f32>(count);
    else interval = angle / (static_cast<f32>(count) - 1.0f);
    for (s32 i = 0; i < count; ++i) {
        f32 step = interval * static_cast<f32>(i);
        f32 cosine = JMACosDegree(step);
        f32 sine = JMASinDegree(step);
        TVec3f direction = side * sine + front * cosine;
        MR::normalizeOrZero(&direction, direction);
        static_cast<StarPieceTrackingPiece*>(getActor(i))->appear(position, upVector, direction, speed, height, distance);
    }
}

StarPieceTrackingPiece::~StarPieceTrackingPiece() {}
