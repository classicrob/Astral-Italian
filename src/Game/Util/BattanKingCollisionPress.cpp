#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE
#define JGEOMETRY_VEC3_ASSIGN_SUB_NOINLINE
#pragma auto_inline off
#include "Game/Util/BattanKingCollisionPress.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "JSystem/J2DGraph/J2DPicture.hpp"

namespace {
#ifndef GALAXY_NATIVE
    typedef char BattanKingCollisionSizeCheck[sizeof(BattanKingCollision) == 0x98 ? 1 : -1];
    typedef char BattanKingCollisionPressSizeCheck[sizeof(BattanKingCollisionPress) == 0x98 ? 1 : -1];
    typedef char BattanKingTalkStateSizeCheck[sizeof(BattanKingTalkState) == 0x1C ? 1 : -1];
#endif

}

namespace BattanKingCollisionVec {
    TVec3f sDemoSize(4000.0f, 500.0f, 3800.0f);
    TVec3f sCollisionOffset(2600, 3000, 800);
    TVec3f sCollisionOrigin(0, 800, 0);
}

BattanKingCollisionPress::BattanKingCollisionPress(LiveActor* host, const char* name, const char* parts, HitSensor* sensor, MtxPtr mtx, s32 type)
    : LiveActor(name), mHost(host), mBaseMtx(0) {
    if (type < -1) type = 29;
    MR::connectToScene(this, type, -1, -1, -1);
    if (!sensor) {
        initHitSensor(1);
        MR::addBodyMessageSensorMapObjPress(this);
        sensor = getSensor(0);
    }
    mBaseMtx = mtx;
    if (!mtx) mBaseMtx = host->getBaseMtx();
    MR::invalidateClipping(this);
    MR::initCollisionPartsFromResourceHolder(this, parts, sensor, MR::getResourceHolder(host), mtx);
}
void BattanKingCollisionPress::init(const JMapInfoIter&) { makeActorAppeared(); }
void BattanKingCollisionPress::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::validateCollisionParts(this);
}
void BattanKingCollisionPress::makeActorDead() {
    LiveActor::makeActorDead();
    MR::invalidateCollisionParts(this);
}
void BattanKingCollisionPress::movement() {
    LiveActor::movement();
    calcAnim();
}
bool BattanKingCollisionPress::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    return mHost->receiveMessage(msg, sender, receiver);
}
BattanKingCollision::BattanKingCollision(LiveActor* host, const char* name, const char* parts, HitSensor* sensor, MtxPtr mtx, s32 type)
    : LiveActor(name), mHost(host), mBaseMtx(0) {
    if (type < -1) type = 29;
    MR::connectToScene(this, type, -1, -1, -1);
    if (!sensor) {
        initHitSensor(1);
        MR::addBodyMessageSensorMapObj(this);
        sensor = getSensor(0);
    }
    mBaseMtx = mtx;
    if (!mtx) mBaseMtx = host->getBaseMtx();
    MR::invalidateClipping(this);
    MR::initCollisionPartsFromResourceHolder(this, parts, sensor, MR::getResourceHolder(host), mtx);
}
void BattanKingCollision::init(const JMapInfoIter&) { makeActorAppeared(); }
void BattanKingCollision::appear() {
    LiveActor::appear();
    MR::resetPosition(this);
}
void BattanKingCollision::movement() {
    LiveActor::movement();
    calcAnim();
}
bool BattanKingCollision::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    return mHost->receiveMessage(msg, sender, receiver);
}
MtxPtr BattanKingCollision::getBaseMtx() const { return mBaseMtx; }
MtxPtr BattanKingCollisionPress::getBaseMtx() const { return mBaseMtx; }
BattanKingCollisionPress::~BattanKingCollisionPress() {}
BattanKingCollision::~BattanKingCollision() {}

bool MR::isPlayerInCollisionCylinder(MtxPtr mtx, const TVec3f* offset, f32 radius, f32 depth) {
    TVec3f translation(0.0f), x(0.0f), y(0.0f), z(0.0f);
    MR::extractMtxTrans(mtx, &translation);
    MR::extractMtxXYZDir(mtx, &x, &y, &z);
    TVec3f center(translation);
    center += x * offset->x;
    center += y * offset->y;
    center += z * offset->z;
    TVec3f delta(*MR::getPlayerCenterPos());
    delta -= center;
    f32 distance = z.dot(delta);
    JMAVECScaleAdd(z, delta, delta, -z.dot(delta));
    bool result = false;
    if (PSVECMag(&delta) < radius && 0.0f >= distance && distance > -depth) result = true;
    return result;
}
bool BattanKingCollisionUtil::isPointInBox(const TPos3f* mtx, const TVec3f* size, const TVec3f* point) {
    TPos3f inverse;
    TVec3f local;
    PSMTXInverse(mtx->mMtx, inverse.mMtx);
    PSMTXMultVec(inverse.mMtx, point, &local);
    TVec3f half(*size);
    half.scale(0.5f);
    if (-half.x < local.x && local.x < half.x && -half.y < local.y && local.y < half.y && -half.z < local.z && local.z < half.z) return true;
    return false;
}
bool MR::isPlayerInCollisionBox(const TPos3f* mtx, const TVec3f* size) {
    TVec3f offset(*size);
    offset.x *= 0.0f; offset.y *= 0.5f; offset.z *= 0.5f;
    TPos3f translated;
    PSMTXCopy(mtx->mMtx, translated.mMtx);
    MR::addTransMtxLocal(translated.mMtx, offset);
    return BattanKingCollisionUtil::isPointInBox(&translated, size, MR::getPlayerCenterPos());
}
void BattanKingCollisionUtil::calcDemoBounds(const NameObj* host, TVec3f* min, TVec3f* max, TVec3f* origin) {
    TPos3f mtx;
    MR::findLinkNamePos(host, "バッタンキング基準位置", mtx.mMtx);
    if (origin) mtx.getTrans(*origin);
    MR::addTransMtxLocal(mtx.mMtx, -BattanKingCollisionVec::sDemoSize * 0.5f);
    mtx.getTrans(*min);
    MR::addTransMtxLocal(mtx.mMtx, BattanKingCollisionVec::sDemoSize);
    mtx.getTrans(*max);
    if (min->x > max->x) J2DPicture::swap(min->x, max->x);
    if (min->y > max->y) J2DPicture::swap(min->y, max->y);
    if (min->z > max->z) J2DPicture::swap(min->z, max->z);
}
bool BattanKingCollisionUtil::isPointInDemoBounds(const NameObj* host, const TVec3f* point) {
    TVec3f min, max, origin;
    calcDemoBounds(host, &min, &max, &origin);
    bool result = false;
    if (min.x < point->x && point->x < max.x && min.y < point->y && point->y < max.y && min.z < point->z && point->z < max.z) result = true;
    return result;
}
// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool BattanKingCollisionUtil::isFacingOutsideDemoBounds(const BattanKing* host) {
    if (!isPointInDemoBounds(host, &host->mPosition)) {
        TVec3f min, max, origin;
        calcDemoBounds(host, &min, &max, &origin);
        const TVec3f* position = &host->mPosition;
        TVec3f direction(origin);
        direction.operator-=(*position);
        MR::normalizeOrZero(&direction);
        f32 threshold = MR::cosDegree(85.0f);
        if (direction.dot(host->mFront) < threshold) return true;
    }
    return false;
}
