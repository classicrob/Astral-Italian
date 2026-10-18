#include "Game/MapObj/MarinePlant.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/mtx.h>
#include <revolution/types.h>
#include <cstdio>

namespace NrvMarinePlantMember {
    NEW_NERVE(MarinePlantMemberNrvWait, MarinePlantMember, Wait);
    NEW_NERVE(MarinePlantMemberNrvReact, MarinePlantMember, React);
};  // namespace NrvMarinePlantMember

MarinePlant::MarinePlant(const char* pName) : LiveActor(pName), mMembers(nullptr), mNumMembers(7), mRadius(200.0f) {
}

void MarinePlant::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x21, -1, -1, -1);
    MR::initDefaultPos(this, rIter);
    MR::validateClipping(this);

    s32 numMembers = -1;
    MR::getJMapInfoArg0NoInit(rIter, &numMembers);
    if (numMembers > 0) {
        mNumMembers = numMembers;
    } else if (numMembers == -1) {
        mNumMembers = 7;
    }

    s32 frame = 0;
    MR::getJMapInfoArg1NoInit(rIter, &frame);
    MR::getJMapInfoArg2NoInit(rIter, &mRadius);

    initMember(rIter);

    for (s32 i = 0; i < mNumMembers; i++) {
        MR::startBtpAndSetFrameAndStop(mMembers[i], "Color", (f32)frame);
    }

    initHitSensor(1);
    MR::addHitSensorMapObj(this, "境界球", 16, 100.0f, gZeroVec);
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void MarinePlant::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->makeActorAppeared();
    }
}

void MarinePlant::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->makeActorDead();
    }
}

void MarinePlant::initAfterPlacement() {
    TVec3f gravity(0.0f);
    TVec3f side(1.0f, 0.0f, 0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);

    TVec3f rotated(0.0f);
    MR::getRotatedAxisY(&rotated, mRotation);
    TVec3f negRot(-rotated);
    mGravity = negRot;
    gravity = mGravity;
    MR::makeAxisCrossPlane(&side, &up, gravity);

    TVec3f center(0.0f);
    calcMemberSpread(&center, gravity, side, up);
    f32 extent = calcMemberBounds(center);

    f32 radius = extent * mScale.y + mRadius;
    MR::setSensorRadius(this, "境界球", radius);
    MR::setClippingTypeSphere(this, extent * mScale.y + mRadius);
}

void MarinePlant::control() {
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->movement();
    }
}

bool MarinePlant::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSpinStormRange(msg)) {
        for (s32 i = 0; i < mNumMembers; i++) {
            mMembers[i]->tryReactToSensor(pReceiver);
        }
    }
    return false;
}

void MarinePlant::initMember(const JMapInfoIter& rIter) {
    mMembers = new MarinePlantMember*[mNumMembers];

    char name[0x40];
    makeMemberName(name, 0x40, rIter);
    for (s32 i = 0; i < mNumMembers; i++) {
        MarinePlantMember* pMember = new MarinePlantMember("海草", name);
        mMembers[i] = pMember;
        pMember->initWithoutIter();
        MR::invalidateClipping(pMember);
    }
}

// NONMATCHING: spreads the members around the plant on a circle; the asm uses a sin/cos table and per-member collision probes, approximated here
s32 MarinePlant::calcMemberSpread(TVec3f* pOut, const TVec3f& pGravity, const TVec3f& pSide, const TVec3f& pUp) {
    pOut->set<int>(0, 0, 0);

    s32 count = 0;
    for (s32 i = 0; i < mNumMembers; i++) {
        f32 angle = ((f32)i / (f32)mNumMembers) * 6.2831855f;
        TVec3f dir(pSide);
        dir.scale(MR::sin(angle));
        TVec3f offset(pUp);
        offset.scale(MR::cos(angle));
        dir.add(offset);

        MarinePlantMember* pMember = mMembers[i];
        TVec3f pos(*pOut);
        pos.add(dir);
        pos.scale(mRadius);
        MR::setPosition(pMember, pos);
        pMember->mGravity = pGravity;
        count++;
    }
    return count;
}

// bounding box over the live members (fn_802CD0D0): sets this->position to the box centre, returns half the largest extent
f32 MarinePlant::calcMemberBounds(const TVec3f& rCenter) {
    TVec3f min(rCenter);
    TVec3f max(rCenter);

    for (s32 i = 0; i < mNumMembers; i++) {
        MarinePlantMember* pMember = mMembers[i];
        if (MR::isDead(pMember)) {
            continue;
        }

        if (pMember->mPosition.x < min.x) {
            min.x = pMember->mPosition.x;
        } else if (max.x < pMember->mPosition.x) {
            max.x = pMember->mPosition.x;
        }
        if (pMember->mPosition.y < min.y) {
            min.y = pMember->mPosition.y;
        } else if (max.y < pMember->mPosition.y) {
            max.y = pMember->mPosition.y;
        }
        if (pMember->mPosition.z < min.z) {
            min.z = pMember->mPosition.z;
        } else if (max.z < pMember->mPosition.z) {
            max.z = pMember->mPosition.z;
        }
    }

    TVec3f center(max);
    center += min;
    TVec3f half(center);
    half.scale(0.5f);
    MR::setPosition(this, half);

    TVec3f extent(max);
    extent -= min;
    f32 largest;
    if (extent.x < extent.y) {
        largest = (extent.y >= extent.z) ? extent.y : extent.z;
    } else {
        largest = (extent.x >= extent.z) ? extent.x : extent.z;
    }
    return largest * 0.5f;
}

// NONMATCHING: name builder ("%s%02d" from the object name and shape id); arguments follow the asm, the wide-char path is not verified
void MarinePlant::makeMemberName(char* pBuf, s32 size, const JMapInfoIter& rIter) {
    const char* objName = nullptr;
    MR::getObjectName(&objName, rIter);
    s32 shapeId = -1;
    MR::getJMapInfoShapeIdWithInit(rIter, &shapeId);
    snprintf(pBuf, size, "%s%02d", objName, shapeId);
}

void MarinePlant::attackSensor(HitSensor*, HitSensor*) {
}

void MarinePlant::startClipped() {
    LiveActor::startClipped();
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->startClipped();
    }
}

void MarinePlant::endClipped() {
    LiveActor::endClipped();
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->endClipped();
    }
}

MarinePlantMember::MarinePlantMember(const char* pName, const char* pSubName) : LiveActor(pName) {
    mSpinAxis.x = 0.0f;
    mSpinAxis.y = 0.0f;
    mSpinAxis.z = 0.0f;
    MR::initActor(this, pSubName, false);
    MR::connectToSceneNoSilhouettedMapObjWeakLightCalcAnim(this);
}

void MarinePlantMember::init(const JMapInfoIter&) {
    initNerve(GET_NERVE(MarinePlantMember, MarinePlantMemberNrvWait), 0);
    appear();
}

// NONMATCHING: spin-axis setup from gravity; the asm builds an orientation matrix and converts angles, approximated here
void MarinePlantMember::initAxisAndScale() {
    TVec3f ref(1.0f, 0.0f, 0.0f);
    if (MR::isSameDirection(mGravity, ref, 0.001f)) {
        ref.set<int>(0, 1, 0);
    }

    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, ref);

    f32 r = MR::getRandom(0.0f, 1.0f);
    f32 angle = MR::getRandom(-3.1415927f, 3.1415927f);
    mSpinAxis = mGravity;
    MR::normalize(&mSpinAxis);
    mRotation.x = sin(angle) * r * 57.29578f;
    mRotation.y = 0.0f;
    mRotation.z = cos(angle) * r * 57.29578f;

    calcAnim();
}

// NONMATCHING: reaction distance test against the sensor host; the asm computes the vector from the host to the member minus the gravity component
bool MarinePlantMember::tryReactToSensor(HitSensor* pSensor) {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(GET_NERVE(MarinePlantMember, MarinePlantMemberNrvReact)) && MR::isLessStep(this, 40)) {
        return false;
    }

    TVec3f diff(MR::getSensorHostPos(pSensor));
    diff -= mPosition;
    MR::vecKillElement(diff, mGravity, &diff);

    f32 dist = PSVECMag(&diff);
    if (dist < mScale.x * 300.0f) {
        setNerve(GET_NERVE(MarinePlantMember, MarinePlantMemberNrvReact));
    }
    return true;
}

MarinePlant::~MarinePlant() {
}

MarinePlantMember::~MarinePlantMember() {
}

void MarinePlantMember::exeWait() {
}

void MarinePlantMember::exeReact() {
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(MarinePlantMember, MarinePlantMemberNrvWait));
    }
}
