#define JGEOMETRY_VEC3_MULASSIGN_NOINLINE
#include "Game/MapObj/SphereRailDash.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"

namespace NrvSphereRailDash {
    NEW_NERVE(SphereRailDashNrvWait, SphereRailDash, Wait);
    NEW_NERVE(SphereRailDashNrvRailMove, SphereRailDash, RailMove);

    class SphereRailDashNrvAfterRailMove : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SphereRailDashNrvAfterRailMove sInstance;
    };
    SphereRailDashNrvAfterRailMove SphereRailDashNrvAfterRailMove::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSphereRailDash

SphereRailDash::~SphereRailDash() {
}

SphereRailDash::SphereRailDash(const char* pName)
    : LiveActor(pName), mSensor(nullptr), mOffset(0.0f, 0.0f, 0.0f), mSensorRadius(200.0f), mEndSpeed(50.0f), mBlendStep(180), _AC(0.0f, 0.0f, 0.0f), mSideVelocity(0.0f, 0.0f, 0.0f),
      mAccelCubic(0.0f), mAccelSquare(0.0f), mStartSpeed(0.0f), mDuration(0) {
}

void SphereRailDash::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE(SphereRailDash, SphereRailDashNrvWait));
    initRailRider(rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mSensorRadius);
    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x75 /* ATYPE_SPHERE_DASH */, 5, mSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::getJMapInfoArg0NoInit(rIter, &mSensorRadius);
    MR::getJMapInfoArg1NoInit(rIter, &mEndSpeed);
    MR::getJMapInfoArg2NoInit(rIter, &mBlendStep);
    appear();
}

bool SphereRailDash::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xA4 /* ACTMES_SPHERE_PLAYER_BINDED */ && isNerve(GET_NERVE(SphereRailDash, SphereRailDashNrvWait))) {
        TVec3f hostVel(MR::getSensorHost(pSender)->mVelocity);
        const TVec3f* pRecvPos = MR::getSensorPos(pReceiver);
        TVec3f diff = *MR::getSensorPos(pSender) - *pRecvPos;
        if (diff.dot(hostVel) > 0.0f) {
            return false;
        }
        if (mGravity.dot(diff) > 0.0f) {
            MR::normalizeOrZero(&diff);
            diff.dot(hostVel);
            MR::zeroVelocity(MR::getSensorHost(pSender));
            return false;
        }
        mSensor = pSender;
        return true;
    }

    return false;
}

void SphereRailDash::exeWait() {
    MR::moveCoordToNearestPos(this, *MR::getPlayerPos());
    MR::moveTransToCurrentRailPos(this);

    if (mSensor != nullptr) {
        MR::moveCoordToStartPos(this);
        TVec3f up(0.0f, 120.0f, 0.0f);
        LiveActor* pHost = MR::getSensorHost(mSensor);
        mOffset = pHost->mPosition - (MR::getRailPos(this) + up);
        TVec3f direction(MR::getRailDirection(this));
        TVec3f velocity(MR::getSensorHost(mSensor)->mVelocity);
        f32 speed = direction.dot(velocity);

        if (speed < 0.0f) {
            speed = 0.0f;
            mSideVelocity = velocity;
        } else {
            mSideVelocity = velocity - direction * speed;
        }

        f32 endSpeed = mEndSpeed;
        f32 duration = 3.0f * MR::getRailTotalLength(this) / (speed + 2.0f * endSpeed);
        mStartSpeed = speed;
        f32 acceleration = 2.0f * (endSpeed - speed) / duration;
        mAccelCubic = -acceleration / (2.0f * duration) / 3.0f;
        mAccelSquare = acceleration / 2.0f;
        mDuration = duration + (duration - static_cast< s32 >(duration) > 0.0001f ? 1.0f : 0.0f);

        if (mDuration < 0) {
            mDuration = 0;
        }

        setNerve(GET_NERVE(SphereRailDash, SphereRailDashNrvRailMove));
    }
}

void SphereRailDash::exeRailMove() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(MR::getSensorHost(mSensor));
    }

    if (getNerveStep() + 1 >= mDuration) {
        MR::moveCoordToEndPos(this);
    } else {
        f32 time = getNerveStep() + 1;
        MR::setRailCoord(this, time * (mStartSpeed + time * (mAccelSquare + mAccelCubic * time)));
    }

    f32 progress = MR::getRailCoord(this) / MR::getRailTotalLength(this);
    mOffset += mSideVelocity;
    mSideVelocity.mult(0.85f);
    TVec3f previousPosition(MR::getSensorHost(mSensor)->mPosition);
    f32 rate = mBlendStep == 0 ? 1.0f : MR::calcNerveRate(this, mBlendStep);
    TVec3f up(0.0f, 120.0f, 0.0f);
    TVec3f target = (MR::getRailPos(this) + mOffset.multInLine2(1.0f - rate)) + up;
    LiveActor* pHost = MR::getSensorHost(mSensor);
    pHost->mVelocity.set(target - previousPosition);

    if (MR::isRailReachedGoal(this)) {
        MR::setPosition(MR::getSensorHost(mSensor), target);
        LiveActor* pActor = MR::getSensorHost(mSensor);
        f32 endSpeed = mEndSpeed;
        pActor->mVelocity.set(MR::getRailDirection(this) * endSpeed);
        MR::sendArbitraryMsg(0xA8 /* ACTMES_END_RAIL_DASH */, mSensor, getSensor("Body"));
        mSensor = nullptr;
        setNerve(GET_NERVE(SphereRailDash, SphereRailDashNrvAfterRailMove));
    }
}
