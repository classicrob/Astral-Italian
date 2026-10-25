#include "Game/Util/Functor.hpp"
#include "Game/MapObj/GravityLightRoad.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace NrvGravityLightRoad {
    NEW_NERVE(GravityLightRoadNrvAppear, GravityLightRoad, Appear);
    NEW_NERVE(GravityLightRoadNrvWait, GravityLightRoad, Wait);
    NEW_NERVE(GravityLightRoadNrvPlayerFall, GravityLightRoad, PlayerFall);
    NEW_NERVE(GravityLightRoadNrvDisappear, GravityLightRoad, Disappear);
};  // namespace NrvGravityLightRoad


GravityLightRoad::GravityLightRoad(const char* pName) : LiveActor(pName) {
    mFallVec.x = 0.0f;
    mFallVec.y = 0.0f;
    mFallVec.z = 0.0f;
    mFallSpeed = 0.0f;
    mRadius = 2500.0f;
    mEndPos.x = 0.0f;
    mEndPos.y = 0.0f;
    mEndPos.z = 0.0f;
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void GravityLightRoad::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "GravityLightRoad", false);

    mRadius = 2500.0f * mScale.y;

    f32 boundingRadius;
    MR::calcModelBoundingRadius(&boundingRadius, this);

    boundingRadius += 0.5f * mRadius;

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    upVec.negate();

    mEndPos.scaleAdd(0.5f * mRadius, upVec, mPosition);

    MR::setClippingTypeSphere(this, boundingRadius, &mEndPos);

    if (MR::isValidSwitchA(this)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< GravityLightRoad*, void (GravityLightRoad::*)() >(this, &GravityLightRoad::endPlayerFall));
    }

    initNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvAppear));

    MR::useStageSwitchSyncAppear(this, rIter);
}

void GravityLightRoad::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Appear");
    }

    if (MR::isBrkStopped(this)) {
        setNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvWait));
    }
}

void GravityLightRoad::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Wait");
    }
}

// NONMATCHING: same calls and math; remaining diff is ps-vector stack slots and instruction scheduling around the scale/normalize block
void GravityLightRoad::exePlayerFall() {
    if (MR::isFirstStep(this)) {
        mFallSpeed = 0.0f;
        MR::startActionSound(this, "SyReadRiddleS", -1, -1, -1);
        MR::startBckPlayer("Fall", nullptr);
        MR::startSoundPlayer("SE_PM_GRAV_LIGHT_DRAWN_ST", -1, -1);
    }

    TVec3f upVec;
    TVec3f toPlayer;
    TVec3f fwdVec;
    TVec3f fwdDir;
    MR::calcUpVec(&upVec, this);

    mFallSpeed += 2.0f;
    if (mFallSpeed > 30.0f) {
        mFallSpeed = 30.0f;
    }

    f32 fallSpeed = mFallSpeed;
    TVec3f vec(upVec);
    vec.scale(fallSpeed);
    mFallVec.add(vec);

    TVec3f negUp = -upVec;
    MR::turnQuatYDirRate(&mQuat, mQuat, negUp, 0.1f);

    toPlayer.sub(mPosition, *MR::getPlayerPos());

    f32 dot = upVec.dot(toPlayer);
    fwdVec.scaleAdd(-dot, upVec, toPlayer);

    if (!MR::isNearZero(fwdVec, 2.0f)) {
        MR::normalize(fwdVec, &fwdDir);
        mFallVec.scaleAdd(2.0f, fwdDir, mFallVec);
    }

    if (MR::isStep(this, 0x708)) {
        endPlayerFall();
    }
}

void GravityLightRoad::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Disappear");
    }

    if (MR::isBrkStopped(this)) {
        kill();
    }
}

bool GravityLightRoad::receiveOtherMsg(u32 msg, HitSensor* pReceiver, HitSensor* pSender) {
    if (isNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvDisappear))) {
        return false;
    }

    if (MR::isMsgAutoRushBegin(msg) && MR::isSensorPlayer(pReceiver)) {
        return tryPlayerFall();
    }

    if (MR::isMsgUpdateBaseMtx(msg) && isPlayerFall()) {
        MR::setPlayerQuatAndPos(&mQuat, &mFallVec);
        return true;
    }

    if (MR::isMsgRushForceCancel(msg) || MR::isMsgRushCancel(msg)) {
        setNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvDisappear));
        return true;
    }

    return false;
}

void GravityLightRoad::updateHitSensor(HitSensor* pSensor) {
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    upVec.negate();

    TVec3f vec;
    vec.scaleAdd(mRadius, upVec, mPosition);

    MR::calcPerpendicFootToLineInside(MR::getSensorPos2(pSensor), *MR::getPlayerCenterPos(), mPosition, vec);
}

void GravityLightRoad::endPlayerFall() {
    if (isPlayerFall()) {
        TVec3f upVec;
        MR::calcUpVec(&upVec, this);
        upVec.scale(2.0f);

        MR::endBindAndPlayerForceJump(this, upVec, 0);
        MR::validateClipping(this);
    }

    setNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvDisappear));
}

bool GravityLightRoad::tryPlayerFall() {
    if (isPlayerFall()) {
        return false;
    }

    if (!isPlayerInVolume()) {
        return false;
    }

    MR::startBckPlayer("Fall", nullptr);
    MR::invalidateClipping(this);
    MR::getPlayerQuatAndPos(&mQuat, &mFallVec);
    setNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvPlayerFall));
    return true;
}

bool GravityLightRoad::isPlayerFall() const {
    return isNerve(GET_NERVE(GravityLightRoad, GravityLightRoadNrvPlayerFall));
}

bool GravityLightRoad::isPlayerInVolume() {
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    TVec3f dir;
    dir.sub(*MR::getPlayerPos(), mPosition);
    TVec3f fwdVec = upVec.getOrthogonal(dir);

    TVec3f lengthVec;
    lengthVec.scale(upVec.dot(dir), upVec);

    TBox3f boundingBox;
    MR::calcModelBoundingBox(&boundingBox, this);

    if (fwdVec.length() > boundingBox.f.x) {
        return false;
    }

    return lengthVec.length() > boundingBox.f.y - boundingBox.i.y == false;
}
