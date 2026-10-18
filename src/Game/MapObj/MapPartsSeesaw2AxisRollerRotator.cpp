#pragma auto_inline off
#include "Game/MapObj/MapPartsSeesaw2AxisRollerRotator.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
namespace NrvMapPartsSeesaw2AxisRollerRotator {
    class MapPartsSeesaw2AxisRollerRotatorNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static MapPartsSeesaw2AxisRollerRotatorNrvWait sInstance;
    };
    MapPartsSeesaw2AxisRollerRotatorNrvWait MapPartsSeesaw2AxisRollerRotatorNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(MapPartsSeesaw2AxisRollerRotatorNrvStay, MapPartsSeesaw2AxisRollerRotator, Stay);
    NEW_NERVE(MapPartsSeesaw2AxisRollerRotatorNrvMove, MapPartsSeesaw2AxisRollerRotator, Move);
}
MapPartsSeesaw2AxisRollerRotator::MapPartsSeesaw2AxisRollerRotator(LiveActor* host, const char* sound, f32 soundSpeed)
    : MapPartsRotatorBase(host, "シーソー(回転２軸)"), mInertia(500.0f), mUp(0.0f,1.0f,0.0f),
      mAngularVelocity(0.0f,0.0f,0.0f), mPreviousAxis(0.0f,0.0f,0.0f), mSpeed(0.0f),
      mMoveSound(sound), mMoveSoundSpeed(soundSpeed), mMaxSpeed(0.6f) {
    mRotateMtx.identity();
    mBaseMtx.identity();
}
MapPartsSeesaw2AxisRollerRotator::~MapPartsSeesaw2AxisRollerRotator() {}
void MapPartsSeesaw2AxisRollerRotator::init(const JMapInfoIter& iter) {
    initNerve(&NrvMapPartsSeesaw2AxisRollerRotator::MapPartsSeesaw2AxisRollerRotatorNrvWait::sInstance);
    s32 inertia = 0;
    MR::getMapPartsArgRotateAccelType(&inertia, iter);
    if (inertia > 0) mInertia = inertia;
    f32 angle;
    MR::getMapPartsArgRotateAngle(&angle, iter);
    if (0.0f < angle) mMaxSpeed = angle / 100.0f;
}
void MapPartsSeesaw2AxisRollerRotator::start() {
    MR::calcGravity(mHost);
    mUp = -mHost->mGravity;
    if (MR::isNearZero(mUp, 0.001f)) mUp = TVec3f(0.0f,1.0f,0.0f);
    MR::makeMtxUpNoSupportPos(&mBaseMtx, mUp, mHost->mPosition);
    mBaseMtxInv.invert(mBaseMtx);
    mRotateMtx.set(mHost->getBaseMtx());
    mRotateMtx.mMtx[0][3] = 0.0f;
    mRotateMtx.mMtx[1][3] = 0.0f;
    mRotateMtx.mMtx[2][3] = 0.0f;
    setNerve(&NrvMapPartsSeesaw2AxisRollerRotator::MapPartsSeesaw2AxisRollerRotatorNrvStay::sInstance);
}
void MapPartsSeesaw2AxisRollerRotator::end() {}
bool MapPartsSeesaw2AxisRollerRotator::isWorking() const {
    return !isNerve(&NrvMapPartsSeesaw2AxisRollerRotator::MapPartsSeesaw2AxisRollerRotatorNrvWait::sInstance);
}
bool MapPartsSeesaw2AxisRollerRotator::isMoving() const { return mSpeed > 0.1f; }
// NONMATCHING: final fadds operands are commuted by the compiler.
void MapPartsSeesaw2AxisRollerRotator::rotate() {
    TVec3f position(*MR::getPlayerPos());
    mBaseMtxInv.mult(position, position);
    f32 distance = PSVECMag(&position);
    TVec3f axis;
    axis.cross(mUp, position);
    MR::normalizeOrZero(&axis);
    PSMTXMultVecSR(mBaseMtx, &axis, &axis);
    TPos3f hostMtx;
    hostMtx.set(mHost->getBaseMtx());
    TPos3f inverse;
    inverse.invert(hostMtx);
    PSMTXMultVecSR(inverse, &axis, &axis);
    f32 input = 0.001f * distance;
    f32 inertia = mInertia;
    f32 acceleration = MR::clamp(input / inertia, 0.0f, 1.0f);
    if (!MR::isNearZero(distance, 0.001f) || !MR::isNearZero(axis, 0.001f)) {
        f32 alignment = axis.dot(mPreviousAxis);
        mPreviousAxis = axis;
        mSpeed = mSpeed * MR::clamp(alignment, 0.0f, 1.0f) + 1.5f * acceleration;
        MR::clamp(mSpeed, -mMaxSpeed, mMaxSpeed);
        mAngularVelocity = mPreviousAxis * mSpeed;
        MR::clampLength(&mAngularVelocity, mAngularVelocity, mMaxSpeed);
    }
    calcRotateMtx();
}
void MapPartsSeesaw2AxisRollerRotator::calcRotateMtx() {
    if (!MR::isNearZero(mAngularVelocity, 0.001f)) {
        TPos3f rotation;
        rotation.identity();
        MR::makeMtxRotate(rotation, mAngularVelocity);
        mRotateMtx.concat(mRotateMtx, rotation);
    }
}
void MapPartsSeesaw2AxisRollerRotator::exeMove() {
    if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
        rotate();
    } else {
        mSpeed *= 0.95f;
        mAngularVelocity = mPreviousAxis * mSpeed;
        calcRotateMtx();
    }
    if (mSpeed >= 0.1f) MR::startActionSoundAuto(mHost, "MapPartsSeesaw2AxisRollerRotatorMove", (s32)(1000.0f*mSpeed), -1, -1);
}
const TMtx34f& MapPartsSeesaw2AxisRollerRotator::getRotateMtx() const { return mRotateMtx; }
void MapPartsSeesaw2AxisRollerRotator::exeStay() {
    if (MR::isOnPlayer(MR::getBodySensor(mHost))) setNerve(&NrvMapPartsSeesaw2AxisRollerRotator::MapPartsSeesaw2AxisRollerRotatorNrvMove::sInstance);
}
