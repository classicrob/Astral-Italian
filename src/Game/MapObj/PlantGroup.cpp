#pragma auto_inline off

#include "Game/MapObj/PlantGroup.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cstdio>

namespace {
    const s32 hIsShakableStep = 20;
    const f32 sSize = 160.0f;
    const f32 sCollisionRadius = 50.0f;
    // sWindRadius
    // sMaxAnimRate
    // sMinAnimRate
    const f32 sCheckLineLength = 1000.0f;
    const f32 sCoinVel = 30.0f;
    const f32 sDPDTouchRadius = 100.0f;
    const f32 sScratchVel = 6.0f;
    const s32 hGenerateItemTouch = 1;
    const s32 sHintEffectEmitInterval = 300;
    const f32 hShakeStartFrame = 3.0f;
    const f32 hWeakRate = 0.5f;
    const f32 hMiddleRate = 1.0f;
    const f32 hStrongRate = 1.5f;
    const f32 hStrongDist = 100.0f;
    const f32 hMiddleDist = 300.0f;
    const f32 hWeakDist = 500.0f;
};  // namespace



namespace NrvPlantMember {
    NEW_NERVE(HostTypeNrvWait, PlantMember, Wait);
    NEW_NERVE(HostTypeNrvHint, PlantMember, Hint);
    NEW_NERVE(HostTypeNrvShakeWeak, PlantMember, ShakeWeak);
    NEW_NERVE(HostTypeNrvShakeMiddle, PlantMember, ShakeMiddle);
    NEW_NERVE(HostTypeNrvShakeStrong, PlantMember, ShakeStrong);
};  // namespace NrvPlantMember







PlantGroup::PlantGroup(const char* pName)
    : LiveActor(pName), mMembers(nullptr), mMemberCount(7), mItemCount(0), mPlantType(0), mPlantShape(-1), mItemType(0), mColorFrame(-1),
      mUseGravity(1), mRandomRotation(-1), _B4(0.0f), mBloomTimer(0), mHintPosition(0.0f, 0.0f, 0.0f), mHintRotation(0.0f, 0.0f, 0.0f), mHintTimer(::sHintEffectEmitInterval),
      mHintIndex(0) {
}

void PlantGroup::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x21, -1, -1, -1);
    MR::initDefaultPos(this, rIter);
    initPlantType(&mPlantType, &mPlantShape, rIter);

    s32 count = -1;
    MR::getJMapInfoArg0NoInit(rIter, &count);

    if (count > 0) {
        mMemberCount = count;
    } else if (count == -1) {
        mMemberCount = 7;
    }

    s32 itemCount = 0;
    MR::getJMapInfoArg1NoInit(rIter, &itemCount);

    if (itemCount < 0) {
        itemCount = 0;
    }

    mItemCount = itemCount;
    s32 itemType = -1;
    MR::getJMapInfoArg2NoInit(rIter, &itemType);
    mItemType = itemType;

    if (itemType == 1) {
        MR::declareStarPiece(this, itemCount);
    } else {
        MR::declareCoin(this, itemCount);
    }

    if (mColorFrame == -1) {
        MR::getJMapInfoArg3WithInit(rIter, &mColorFrame);
    }

    bool noGravity = false;
    MR::getJMapInfoArg4NoInit(rIter, &noGravity);

    if (noGravity) {
        mUseGravity = 0;
    }

    bool noRandomRotation = false;
    MR::getJMapInfoArg5NoInit(rIter, &noRandomRotation);

    if (noRandomRotation || (mPlantType == 3 && mPlantShape == 2)) {
        mRandomRotation = 0;
    }

    MR::getJMapInfoArg6NoInit(rIter, &_B4);
    MR::initUseStageSwitchReadAppear(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    initMember(itemCount, rIter);

    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
    }

    initSound(4, "PlantGroup", nullptr, TVec3f(0.0f));
    initEffectKeeper(0, "PlantGroup", false);
    MR::setEffectHostSRT(this, "HintShakeLeaf", &mHintPosition, &mHintRotation, nullptr);
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::tryRegisterDemoCast(this, rIter);
    mHintTimer = MR::getRandom(3L, 10L) * 10;
}

void PlantGroup::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    for (s32 i = 0; i < mMemberCount; i++) {
        if (!MR::isValidSwitchAppear(this)) {
            mMembers[i]->makeActorAppeared();
        }
    }
}

void PlantGroup::makeActorDead() {
    LiveActor::makeActorDead();

    for (s32 i = 0; i < mMemberCount; i++) {
        mMembers[i]->makeActorDead();
    }
}

void PlantGroup::makeModelName(char* pDst, u32 size) const {
    const char* pName = nullptr;

    if (mPlantType == 0) {
        pName = "PlantA";
    }

    if (mPlantType == 1) {
        pName = "PlantB";
    }

    if (mPlantType == 2) {
        pName = "PlantC";
    }

    if (mPlantType == 3) {
        pName = "PlantD";
    }

    snprintf(pDst, size, "%s%02d", pName, mPlantShape);
}

bool PlantGroup::isBloomEnd() const {
    if (MR::isDead(this)) {
        return false;
    }

    return mBloomTimer < 0;
}

// NONMATCHING: loop-invariant string and loop register allocation differ
void PlantGroup::initMember(s32 itemCount, const JMapInfoIter&) {
    mMembers = new PlantMember*[mMemberCount];
    char modelName[0x40];
    makeModelName(modelName, sizeof(modelName));
    bool useLight = false;
    bool strongLight = true;

    if (mPlantType == 0 && mPlantShape == 8) {
        useLight = true;
        strongLight = false;
    }

    if (mPlantType == 0 && mPlantShape == 1) {
        useLight = false;
    } else {
        strongLight = true;
    }

    const char* pMemberName = "汎用植物";
    for (s32 i = 0; i < mMemberCount; i++) {
        mMembers[i] = new PlantMember(pMemberName, modelName, useLight, strongLight);
        mMembers[i]->initWithoutIter();
        mMembers[i]->mScale = mScale;
        mMembers[i]->mPlantType = mPlantType;

        if (MR::isExistBtp(mMembers[i], "Color")) {
            f32 frame = mColorFrame == -1 ? 0.0f : mColorFrame;
            MR::startBtp(mMembers[i], "Color");

            if (frame <= MR::getBtpFrameMax(mMembers[i]) - 1.0f) {
                MR::setBtpFrameAndStop(mMembers[i], frame);
            } else {
                MR::setBtpFrameAndStop(mMembers[i], 0.0f);
            }
        }

        MR::invalidateClipping(mMembers[i]);

        if (i < itemCount) {
            mMembers[i]->mHasItem = true;
        } else {
            mMembers[i]->mHasItem = false;
        }
    }

    if (!MR::isValidSwitchAppear(this)) {
        for (s32 i = 0; i < mMemberCount; i++) {
            s32 index = MR::getRandom(0L, i + 1);
            bool hasItem = mMembers[i]->mHasItem;
            mMembers[i]->mHasItem = mMembers[index]->mHasItem;
            mMembers[index]->mHasItem = hasItem;
        }
    }

    initHitSensor(1);
    MR::addHitSensorMapObj(this, "境界球", 16, 100.0f, TVec3f(gZeroVec));
}

// NONMATCHING: vector temporary stack slots and loop register allocation differ
s32 PlantGroup::placeOnCollisionFormCircle(TVec3f* pCenter, const TVec3f& rGravity, const TVec3f& rAxisX, const TVec3f& rAxisY) {
    pCenter->set<int>(0, 0, 0);
    f32 spacing;

    switch (mPlantType) {
    case 0:
        spacing = 160.0f;
        break;
    case 1:
        spacing = 160.0f;
        break;
    case 2:
        spacing = 250.0f;
        break;
    case 3:
        spacing = 160.0f;
        break;
    default:
        spacing = 160.0f;
        break;
    }

    spacing *= mScale.x;
    f32 angle = 0.0f;
    f32 radius = 0.0f;
    f32 angleStep = 2.0f * PI;
    s32 ring = 0;
    s32 ringCount = 0;
    TVec3f sum(0.0f);

    for (s32 i = 0; i < mMemberCount; i++) {
        TVec3f offset(rAxisX);
        offset.scale(MR::cos(angle));
        offset += rAxisY * MR::sin(angle);
        offset.scale(radius);
        TVec3f pos(mPosition);
        pos += offset;
        sum += pos;
        angle += angleStep;

        if (angle >= 2.0f * PI) {
            ringCount += 6;
            ring++;
            angle = 0.0f;
            angleStep = 2.0f * PI / ringCount;
            radius = ring * spacing;
        }
    }

    sum.scale(1.0f / mMemberCount);
    TVec3f average(sum);
    average -= mPosition;
    angle = 0.0f;
    angleStep = 2.0f * PI;
    radius = 0.0f;
    ring = 0;
    ringCount = 0;
    s32 placedCount = 0;

    for (s32 i = 0; i < mMemberCount; i++) {
        TVec3f offset(rAxisX);
        offset.scale(MR::cos(angle));
        offset += rAxisY * MR::sin(angle);
        offset.scale(radius);
        TVec3f start(mPosition);
        start -= average;
        start += offset;
        TVec3f up(rGravity);
        up.scale(100.0f);
        start -= up;
        TVec3f ray(rGravity);
        ray.scale(::sCheckLineLength);

        bool hit = MR::getFirstPolyOnLineToMap(&mMembers[i]->mPosition, nullptr, start, ray);
        if (hit) {
            TVec3f* pPos = &mMembers[i]->mPosition;
            *pPos += -rGravity * _B4;
            *pCenter += mMembers[i]->mPosition;
            placedCount++;
        } else {
            mMembers[i]->kill();
            continue;
        }
        {
            if (mUseGravity) {
                MR::calcGravity(mMembers[i]);
            } else {
                mMembers[i]->mGravity = mGravity;
            }

            mMembers[i]->initPosture(mRandomRotation != 0);
            angle += angleStep;

            if (angle >= 2.0f * PI) {
                ringCount += 6;
                ring++;
                angle = 0.0f;
                angleStep = 2.0f * PI / ringCount;
                radius = ring * spacing;
            }
        }
    }

    pCenter->scale(1.0f / placedCount);
    return placedCount;
}

f32 PlantGroup::calcBoundingSphereRadius(const TVec3f& rCenter) {
    TVec3f min(rCenter);
    TVec3f max(rCenter);

    for (s32 i = 0; i < mMemberCount; i++) {
        const TVec3f& pos = mMembers[i]->mPosition;

        if (pos.x < min.x) {
            min.x = pos.x;
        } else if (max.x < pos.x) {
            max.x = pos.x;
        }

        if (pos.y < min.y) {
            min.y = pos.y;
        } else if (max.y < pos.y) {
            max.y = pos.y;
        }

        if (pos.z < min.z) {
            min.z = pos.z;
        } else if (max.z < pos.z) {
            max.z = pos.z;
        }

        mMembers[i]->calcAnim();
    }

    MR::setPosition(this, (max + min) / 2.0f);
    TVec3f size(max - min);
    return size.length() * 0.5f;
}

void PlantGroup::initAfterPlacement() {
    TVec3f axisX;
    axisX.set(1, 0, 0);
    TVec3f axisY;
    axisY.set(0, 1, 0);

    if (mUseGravity) {
        MR::calcGravityVector(this, mPosition, &mGravity, nullptr, 0);
    } else {
        TVec3f up(0.0f);
        MR::getRotatedAxisY(&up, mRotation);
        mGravity = -up;
    }

    TPos3f mtx;
    PSMTXIdentity(mtx.toMtxPtr());
    MR::makeMtxRotate(mtx.toMtxPtr(), mRotation);
    TVec3f front;
    MR::extractMtxZDir(mtx.toMtxPtr(), &front);
    MR::makeMtxUpFront(&mtx, mGravity, front);
    MR::extractMtxXDir(mtx.toMtxPtr(), &axisX);
    MR::extractMtxZDir(mtx.toMtxPtr(), &axisY);
    TVec3f center;

    if (MR::isExistRail(this)) {
        placeOnRail(&center);
    } else {
        placeOnCollisionFormCircle(&center, mGravity, axisX, axisY);
    }

    f32 radius = calcBoundingSphereRadius(center);
    f32 scale = mScale.y;
    MR::setSensorRadius(this, "境界球", radius + ::sSize * scale);
    MR::setClippingTypeSphere(this, radius + ::sSize * scale);
}

// NONMATCHING: integer-to-float conversion temporaries reuse different stack slots
s32 PlantGroup::placeOnRail(TVec3f* pCenter) {
    f32 length = MR::getRailTotalLength(this);

    if (MR::isLoopRail(this)) {
        length /= mMemberCount;
    } else {
        length /= mMemberCount - 1;
    }

    MR::moveCoordToStartPos(this);
    MR::setRailCoordSpeed(this, length);
    s32 placedCount = 0;

    for (s32 i = 0; i < mMemberCount; i++) {
        TVec3f pos(MR::getRailPos(this));
        TVec3f gravity(0.0f);

        if (mUseGravity) {
            MR::calcGravityVector(this, pos, &gravity, nullptr, 0);
        } else {
            gravity = mGravity;
        }

        TVec3f up(gravity);
        up.scale(100.0f);
        pos -= up;
        TVec3f ray(gravity);
        ray.scale(::sCheckLineLength);

        if (MR::getFirstPolyOnLineToMap(&mMembers[i]->mPosition, nullptr, pos, ray)) {
            *pCenter += mMembers[i]->mPosition;
            placedCount++;
        } else {
            mMembers[i]->kill();
            continue;
        }
        {
            if (mUseGravity) {
                MR::calcGravity(mMembers[i]);
            } else {
                mMembers[i]->mGravity = mGravity;
            }

            mMembers[i]->initPosture(mRandomRotation != 0);
            MR::moveRailRider(this);
        }
    }

    pCenter->scale(1.0f / placedCount);
    return placedCount;
}

void PlantGroup::control() {
    updateBloom();

    for (s32 i = 0; i < mMemberCount; i++) {
        if (!MR::isDead(mMembers[i])) {
            mMembers[i]->animControl(this);
            mMembers[i]->movement();
        }
    }

    emitHintEffect();
}

void PlantGroup::initPlantType(s32* pType, s32* pShape, const JMapInfoIter& rIter) {
    const char* pName = nullptr;
    MR::getObjectName(&pName, rIter);

    if (MR::isEqualString(pName, "CutBushGroup")) {
        *pType = 0;
        *pShape = 8;
    } else if (MR::isEqualString(pName, "FlowerBlueGroup")) {
        *pType = 0;
        *pShape = 7;
    } else if (MR::isEqualString(pName, "FlowerGroup")) {
        *pType = 0;
        *pShape = 7;
        mColorFrame = 1;
    } else {
        if (MR::isEqualString(pName, "PlantA")) {
            *pType = 0;
        } else if (MR::isEqualString(pName, "PlantB")) {
            *pType = 1;
        } else if (MR::isEqualString(pName, "PlantC")) {
            *pType = 2;
        } else if (MR::isEqualString(pName, "PlantD")) {
            *pType = 3;
        }

        s32 shape = -1;
        MR::getJMapInfoShapeIdWithInit(rIter, &shape);

        if (shape == -1) {
            shape = 0;
        }

        *pShape = shape;
    }
}

void PlantGroup::updateBloom() {
    if (mBloomTimer >= 0) {
        mBloomTimer++;

        if (mBloomTimer > 4) {
            bool found = false;

            for (s32 i = 0; i < mMemberCount; i++) {
                if (MR::isDead(mMembers[i])) {
                    mMembers[i]->appear();
                    MR::emitEffectHit(this, mMembers[i]->getBaseMtx(), "Bloom");
                    found = true;
                    break;
                }
            }

            if (found) {
                mBloomTimer = 0;
            } else {
                mBloomTimer = -1;
            }
        }
    }
}

void PlantGroup::emitHintEffect() {
    mHintTimer--;

    if (mHintTimer <= 0) {
        mHintTimer = ::sHintEffectEmitInterval;
        s32 index = (mMemberCount + mHintIndex) % mMemberCount;
        do {
            if (mMembers[index]->tryEmitHint()) {
                mHintIndex = (index + mMemberCount + 1) % mMemberCount;
                mHintPosition.set(mMembers[index]->mPosition);
                mHintRotation.set(mMembers[index]->mRotation);
                MR::emitEffect(this, "HintShakeLeaf");
                break;
            }

            index = (index + mMemberCount + 1) % mMemberCount;
        } while (index != mHintIndex);
    }
}

bool PlantGroup::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor*) {
    if (MR::isMsgSupportTicoSpin(msg)) {
        for (s32 i = 0; i < mMemberCount; i++) {
            mMembers[i]->tryShake(pSender);
        }
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        for (s32 i = 0; i < mMemberCount; i++) {
            mMembers[i]->tryPush(*MR::getSensorPos(pSender), MR::getSensorRadius(pSender), 1);
        }
    }

    return false;
}

bool PlantGroup::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor*) {
    if (MR::isMsgSpinStormRange(msg)) {
        for (s32 i = 0; i < mMemberCount; i++) {
            mMembers[i]->tryShake(pSender);
        }
    }

    return false;
}

void PlantGroup::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) || MR::isSensorEnemy(pReceiver)) {
        TVec3f velocity(MR::getSensorHost(pReceiver)->mVelocity);
        MR::vecKillElement(velocity, MR::getSensorHost(pReceiver)->mGravity, &velocity);

        if (velocity.squared() < 2.0f) {
            return;
        }

        s32 touchType = 0;

        if (MR::isSensorPlayerOrRide(pReceiver) || MR::isSensorNpc(pReceiver)) {
            touchType = 1;
        } else if (MR::sendArbitraryMsg(0xF5 /* ACTMES_PLANT_GROUP_EMIT_ITEM */, pReceiver, pSender)) {
            touchType = 1;
        }

        for (s32 i = 0; i < mMemberCount; i++) {
            mMembers[i]->tryPush(*MR::getSensorPos(pReceiver), MR::getSensorRadius(pReceiver), touchType);
        }
    }
}

void PlantGroup::startClipped() {
    LiveActor::startClipped();

    for (s32 i = 0; i < mMemberCount; i++) {
        if (!MR::isDead(mMembers[i])) {
            mMembers[i]->startClipped();
        }
    }
}

void PlantGroup::endClipped() {
    LiveActor::endClipped();

    for (s32 i = 0; i < mMemberCount; i++) {
        if (!MR::isDead(mMembers[i])) {
            mMembers[i]->endClipped();
        }
    }
}

PlantMember::PlantMember(const char* pName, const char* pModelName, bool useLight, bool strongLight) : LiveActor(pName), _98(0.0f) {
    MR::initActor(this, pModelName, false);

    if (strongLight) {
        MR::connectToSceneNoSilhouettedMapObjStrongLightCalcAnim(this);
    } else {
        MR::connectToSceneNoSilhouettedMapObjWeakLightCalcAnim(this);
    }

    if (useLight) {
        MR::initLightCtrl(this);
    }
}

void PlantMember::init(const JMapInfoIter&) {
    _95 = false;
    mHasItem = false;
    MR::onCalcAnim(this);
    initSound(1, "PlantMember", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(PlantMember, HostTypeNrvWait), 0);
    makeActorAppeared();
}

void PlantMember::makeActorAppeared() {
    if (MR::getLightCtrl(this)) {
        MR::updateLightCtrlDirect(this);
    }

    LiveActor::makeActorAppeared();
    _98 = -1.0f;
    MR::setScale(this, 1.0f, 1.0f, 1.0f);
    _95 = false;
}

void PlantMember::appear() {
    LiveActor::appear();
    MR::setScale(this, 0.01f, 0.01f, 0.01f);
    _98 = 0.0f;
    MR::startActionSound(this, "Appear");

    if (mPlantType == 0) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle));
    } else {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak));
    }
}

// PLACEHOLDER(objects): JMAHermiteInterpolation<f32>(t, x0, y0, d0, x1, y1, d1); resolve when core names fn_802DB510
extern "C" f32 fn_802DB510(f32, f32, f32, f32, f32, f32, f32);

#define PLANT_APPEAR_SCALE(dst, t)                                                                                                         \
    if ((t) <= 0.75f) {                                                                                                                       \
        dst = fn_802DB510(t, 0.0f, 0.0f, 3.0f, 0.75f, 1.0f, -0.9f);                                                               \
    } else if ((t) <= 0.92f) {                                                                                                                \
        dst = fn_802DB510(t, 0.75f, 1.0f, -0.9f, 0.92f, 1.0f, 0.55f);                                                             \
    } else {                                                                                                                                  \
        dst = fn_802DB510(t, 0.92f, 1.0f, 0.55f, 1.0f, 1.0f, 0.0f);                                                               \
    }

// NONMATCHING: target hoists the 0.92 Hermite threshold into f31 (5 saved FPRs); ours reloads it (3 saved FPRs)
void PlantMember::control() {
    if (isAppearing()) {
        f32 rate = 1.0f;
        f32 maxFrame = 20.0f;

        if (mPlantType == 1 || mPlantType == 3) {
            maxFrame = 27.0f;
        }

        if (_98 < maxFrame) {
            _98 += 1.0f;
            rate = _98 / maxFrame;
        } else {
            _98 = -1.0f;
        }

        TVec3f& rScale = mScale;
        if (rate >= 1.0f) {
            rScale.set<f32>(1.0f, 1.0f, 1.0f);
        } else {
            f32 x = MR::clamp(MR::normalize(rate, 0.0f, 0.7f), 0.01f, 1.0f);
            f32 y = MR::clamp(MR::normalize(rate, 0.1f, 0.9f), 0.01f, 1.0f);
            f32 z = MR::clamp(MR::normalize(rate, 0.3f, 1.0f), 0.01f, 1.0f);
            f32 sx, sy, sz;
            PLANT_APPEAR_SCALE(sx, x);
            PLANT_APPEAR_SCALE(sy, y);
            PLANT_APPEAR_SCALE(sz, z);
            rScale.x = sx;
            rScale.y = sy;
            rScale.z = sz;
        }
    }
}

bool PlantMember::tryEmitHint() {
    if (!mHasItem) {
        return false;
    }

    if (isNerve(GET_NERVE(PlantMember, HostTypeNrvWait))) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvHint));
        return true;
    }

    return false;
}

void PlantMember::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckIfExist(this, "Wait");
    }
}

void PlantMember::exeHint() {
    if (MR::isFirstStep(this)) {
        MR::startBckIfExist(this, "HintShake");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvWait));
    }
}

void PlantMember::exeShakeWeak() {
    if (MR::isFirstStep(this)) {
        if (MR::startBckIfExist(this, "Shake")) {
            MR::setBckFrame(this, ::hShakeStartFrame);
        }

        MR::startActionSound(this, "OjLeavesSwing");
        MR::setBckRate(this, ::hWeakRate);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvWait));
    }
}

void PlantMember::exeShakeMiddle() {
    if (MR::isFirstStep(this)) {
        if (MR::startBckIfExist(this, "Shake")) {
            MR::setBckFrame(this, ::hShakeStartFrame);
        }

        MR::startActionSound(this, "OjLeavesSwing");
        MR::setBckRate(this, ::hMiddleRate);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvWait));
    }
}

void PlantMember::exeShakeStrong() {
    if (MR::isFirstStep(this)) {
        if (MR::startBckIfExist(this, "Shake")) {
            MR::setBckFrame(this, ::hShakeStartFrame);
        }

        MR::startActionSound(this, "OjLeavesSwing");
        MR::setBckRate(this, ::hStrongRate);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvWait));
    }
}

bool PlantMember::generateItem(PlantGroup* pGroup) {
    if (mHasItem && _95) {
        TVec3f velocity(-mGravity);
        velocity.scale(::sCoinVel);

        if (pGroup->mItemType == 1) {
            MR::appearStarPiece(pGroup, mPosition, 1, 10.0f, 40.0f, false);
        } else {
            MR::appearCoinPop(pGroup, mPosition, 1);
        }

        mHasItem = false;
        return true;
    }

    return false;
}

void PlantMember::initPosture(bool randomRotation) {
    TVec3f axis;
    axis.set(1, 0, 0);

    if (MR::isSameDirection(mGravity, axis, 0.01f)) {
        axis.set(0, 1, 0);
    }

    TPos3f posture;
    MR::calcMtxFromGravityAndZAxis(&posture, this, mGravity, axis);
    TVec3f euler;
    TPos3f rotation;
    f32 angle = 0.0f;

    if (randomRotation) {
        angle = MR::getRandom(-PI, PI);
    }

    rotation.makeRotate(mGravity, angle);
    posture.concat(rotation, posture);
    posture.getEuler(euler);
    f32 x = euler.x;
    f32 y = euler.y;
    f32 z = euler.z;
    x = _180_PI * x;
    y = _180_PI * y;
    z = _180_PI * z;
    mRotation.set(x, y, z);
    calcAnim();
}

bool PlantMember::tryShake(HitSensor* pSensor) {
    if (MR::isDead(this)) {
        return false;
    }

    if ((isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak)) || isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle)) ||
         isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong))) &&
        MR::isLessStep(this, ::hIsShakableStep)) {
        return false;
    }

    TVec3f diff(*MR::getSensorPos(pSensor));
    diff -= mPosition;

    f32 height = diff.dot(mGravity);
    if (__fabsf(height) > 600.0f) {
        return false;
    }

    MR::vecKillElement(diff, mGravity, &diff);
    f32 distance = diff.length();

    if (distance < ::hStrongDist) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong));
    } else if (distance < ::hMiddleDist) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle));
    } else if (distance < ::hWeakDist) {
        setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak));
    } else {
        return false;
    }

    return true;
}

bool PlantMember::tryPush(const TVec3f& rPosition, f32 radius, s32 touchType) {
    if (MR::isDead(this) || isAppearing()) {
        return false;
    }

    if ((isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak)) || isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle)) ||
         isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong))) &&
        MR::isLessStep(this, ::hIsShakableStep)) {
        return false;
    }

    if ((isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak)) || isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle)) ||
         isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong))) &&
        (u32)(mPlantType - 1) <= 2) {
        return false;
    }

    TVec3f diff(rPosition);
    diff -= mPosition;
    f32 size = 50.0f;

    switch (mPlantType) {
    case 0:
        size = 50.0f;
        break;
    case 1:
        size = 220.0f;
        break;
    case 2:
        size = 100.0f;
        break;
    case 3:
        size = 220.0f;
        break;
    }

    f32 height = diff.dot(mGravity);
    MR::vecKillElement(diff, mGravity, &diff);
    f32 limit = radius + size * mScale.y;

    if (diff.length() < limit && __fabs(height) < radius + size * mScale.y) {
        if (isNerve(GET_NERVE(PlantMember, HostTypeNrvWait)) || isNerve(GET_NERVE(PlantMember, HostTypeNrvHint))) {
            setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak));
        } else if (isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak))) {
            setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle));
        } else if (isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle))) {
            setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong));
        } else if (isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong))) {
            setNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong));
        }

        _95 = touchType;
        return true;
    }

    return false;
}

void PlantMember::animControl(PlantGroup* pGroup) {
    if (!isAppearing()) {
        if (isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeWeak)) || isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeMiddle)) ||
            isNerve(GET_NERVE(PlantMember, HostTypeNrvShakeStrong))) {
            generateItem(pGroup);
        }
    }
}

PlantGroup::~PlantGroup() {
}

PlantMember::~PlantMember() {
}
