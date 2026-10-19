#include "Game/MapObj/MarinePlant.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
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
    TVec3f side;
    side.set<int>(1, 0, 0);
    TVec3f up;
    up.set<int>(0, 1, 0);

    TVec3f rotated(0.0f);
    MR::getRotatedAxisY(&rotated, mRotation);
    mGravity = -rotated;
    gravity = mGravity;
    MR::makeAxisCrossPlane(&side, &up, gravity);

    TVec3f center;
    calcMemberSpread(&center, gravity, side, up);
    f32 extent = calcMemberBounds(center);

    f32 scale = mScale.y;
    f32 radius = extent * scale + mRadius;
    MR::setSensorRadius(this, "境界球", radius);
    MR::setClippingTypeSphere(this, extent * scale + mRadius);
}

void MarinePlant::control() {
    for (s32 i = 0; i < mNumMembers; i++) {
        mMembers[i]->movement();
    }
}

bool MarinePlant::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSpinStormRange(msg)) {
        for (s32 i = 0; i < mNumMembers; i++) {
            mMembers[i]->tryReactToSensor(pSender);
        }
    }
    return false;
}

void MarinePlant::initMember(const JMapInfoIter& rIter) {
    mMembers = new MarinePlantMember*[mNumMembers];

    char name[0x40];
    makeMemberName(name, 0x40, rIter);
    const char* memberName = "海草";
    for (s32 i = 0; i < mNumMembers; i++) {
        MarinePlantMember* pMember = new MarinePlantMember(memberName, name);
        mMembers[i] = pMember;
        mMembers[i]->initWithoutIter();
        MR::invalidateClipping(mMembers[i]);
    }
}

// NONMATCHING: saved registers, stack slots, and independent setup scheduling differ.
s32 MarinePlant::calcMemberSpread(TVec3f* pOut, const TVec3f& gravity, const TVec3f& side, const TVec3f& up) {
    pOut->set<int>(0, 0, 0);
    s32 ring = 0;
    s32 ringCount = 0;
    s32 count = 0;
    f32 angle = 0.0f;
    f32 angleStep = 6.2831855f;
    f32 spacing = mRadius;
    f32 radius = spacing * (f32)ring;
    for (s32 i = 0; i < mNumMembers; ++i) {
        TVec3f dir(side);
        dir.scale(JMath::sSinCosTable.cosRadian(angle));
        f32 sine = JMath::sSinCosTable.sinRadian(angle);
        TVec3f across(up);
        across.scale(sine);
        dir += across;
        dir.scale(radius);
        TVec3f pos(mPosition);
        pos += dir;
        pos += -gravity * 100.0f;
        Triangle triangle;
        if (MR::getFirstPolyOnLineToMap(&mMembers[i]->mPosition, &triangle, pos, gravity * 1000.0f)) {
            *pOut += mMembers[i]->mPosition;
            count++;
        } else {
            mMembers[i]->kill();
            pos += gravity * 1000.0f;
            continue;
        }
        mGravity = gravity;
        TVec3f* memberGravity = &mMembers[i]->mGravity;
        *memberGravity = -*triangle.getNormal(0);
        mMembers[i]->initAxisAndScale();
        f32 scale = mScale.x;
        scale *= 1.0f + 0.5f * (MR::getRandom() - 0.5f);
        MR::setScale(mMembers[i], scale);
        MR::setBckFrameAtRandom(mMembers[i]);
        angle += angleStep;
        if (angle >= 6.2831855f) {
            ringCount += 6;
            ring++;
            angle = 0.0f;
            angleStep = 6.2831855f / (f32)ringCount;
            radius = (f32)ring * spacing;
        }
    }
    pOut->scale(1.0f / (f32)count);
    return count;
}

// NONMATCHING: vector temporary stack slots differ; ordered arithmetic matches.
f32 MarinePlant::calcMemberBounds(const TVec3f& rCenter) {
    TVec3f min(rCenter);
    TVec3f max(rCenter);

    for (s32 i = 0; i < mNumMembers; i++) {
        if (MR::isDead(mMembers[i])) {
            continue;
        }
        MarinePlantMember* pMember = mMembers[i];
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

    TVec3f half((max + min) * 0.5f);
    MR::setPosition(this, half);
    TVec3f extent(max);
    extent -= min;
    if (extent.x < extent.y) {
        if (extent.y < extent.z) {
            f32 radius = extent.z;
            radius *= 0.5f;
            return radius;
        }
        f32 radius = extent.y;
        radius *= 0.5f;
        return radius;
    }
    if (extent.x < extent.z) {
        f32 radius = extent.z;
        radius *= 0.5f;
        return radius;
    }
    f32 radius = extent.x;
    radius *= 0.5f;
    return radius;
}

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

// NONMATCHING: final radians-to-degrees multiply/store register scheduling differs.
void MarinePlantMember::initAxisAndScale() {
    TVec3f ref;
    ref.set<int>(1, 0, 0);
    if (MR::isSameDirection(mGravity, ref, 0.01f)) {
        ref.set<int>(0, 1, 0);
    }
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, ref);
    f32 angle = MR::getRandom(-3.1415927f, 3.1415927f);
    TPos3f rotation;
    rotation.makeRotate(mGravity, angle);
    mtx.concat(rotation, mtx);
    TVec3f angles;
    mtx.getEuler(angles);
    mRotation.x = angles.x * 57.29578f;
    mRotation.y = angles.y * 57.29578f;
    mRotation.z = angles.z * 57.29578f;
    calcAnim();
}

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
