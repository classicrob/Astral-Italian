#include "Game/NPC/GhostAttackGhostFootprint.hpp"
#include "Game/NPC/GhostAttackGhost.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/FootPrint.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"

#ifdef GALAXY_NATIVE
// The Wii .sbss2 object is zero-initialized; its consumer loads one f32.
f32 sGhostFootprintOpacity = 0.0f;
#endif

struct GhostSpaceFlyAnimationNames {
    char end[12];
    char loop[16];
    char shortFly[16];
    char shortStart[20];
    char start[16];
};

GhostSpaceFlyAnimationNames sSpaceFlyAnimationNames = {
    "SpaceFlyEnd", "SpaceFlyLoop", "SpaceFlyShort", "SpaceFlyShortStart", "SpaceFlyStart"
};

namespace NrvGhostAttackGhostFootprint {
    class GhostAttackGhostFootprintNrvStop : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static GhostAttackGhostFootprintNrvStop sInstance;
    };
    GhostAttackGhostFootprintNrvStop GhostAttackGhostFootprintNrvStop::sInstance ATTRIBUTE_WEAK;
    class GhostAttackGhostFootprintNrvFollow : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static GhostAttackGhostFootprintNrvFollow sInstance;
    };
    GhostAttackGhostFootprintNrvFollow GhostAttackGhostFootprintNrvFollow::sInstance ATTRIBUTE_WEAK;
}
#define PRINT_NERVE(name) (&NrvGhostAttackGhostFootprint::GhostAttackGhostFootprintNrv##name::sInstance)

GhostAttackGhostFootprint::GhostAttackGhostFootprint(LiveActor* host) : LiveActor("足跡"), mHost(host), mFootPrint(new FootPrint("足跡", 256)), mPreviousMovement(TVec3f(0.0f)), mDirection(TVec3f(0.0f, 0.0f, 1.0f)) {
    mBaseMtx.identity();
    mJointMtx.identity();
    mFootPrint->setTexture(MR::getTexFromArc("Footprint.bti", mHost));
    initPrintParameters();
    initBinder(70.0f, 60.0f, 0);
    MR::invalidateClipping(this);
    initNerve(PRINT_NERVE(Stop), 0);
    MR::setPosition(this, mHost->mPosition);
    MR::setRotation(this, mHost->mRotation);
    MR::onCalcGravity(this);
    makeActorDead();
}
void GhostAttackGhostFootprint::appear() {
    LiveActor::appear();
    mJointMtx.set(MR::getJointMtx(mHost, "Center"));
    setNerve(PRINT_NERVE(Follow));
}
char sNonGroupAnimationName[] = "dup-non-group";

void GhostAttackGhostFootprint::stop() {
    if (isNerve(PRINT_NERVE(Follow))) setNerve(PRINT_NERVE(Stop));
}
// NONMATCHING: zero constant placement and store scheduling.
void GhostAttackGhostFootprint::initPrintParameters() {
    mFootPrint->_2C = sGhostFootprintOpacity;
    mFootPrint->_30 = sGhostPrintWidth;
    mFootPrint->_34 = sGhostPrintHeight;
    mFootPrint->mMinPrintDistance = sGhostPrintDistance;
}
// NONMATCHING: Euler conversion register and load ordering.
void GhostAttackGhostFootprint::control() {
    if (!isNerve(PRINT_NERVE(Stop))) {
        mBaseMtx.set(mHost->getBaseMtx());
        mBaseMtx.getTrans(mPosition);
        TVec3f rotation;
        mBaseMtx.getEuler(rotation);
        mRotation.set<f32>(rotation.x * 57.29578f, rotation.y * 57.29578f, rotation.z * 57.29578f);
        TVec3f movement(static_cast<GhostAttackGhost*>(mHost)->mMovementDelta);
        mDirection.set((movement + mPreviousMovement) * 0.5f);
        if (MR::normalizeOrZero(&mDirection)) {
            MR::extractMtxZDir(mBaseMtx, &mDirection);
            return;
        }
        mPreviousMovement.set(static_cast<GhostAttackGhost*>(mHost)->mMovementDelta);
    }
}
void GhostAttackGhostFootprint::updatePrint() {
    if (!tryAddPrintOnSlide() && !tryAddPrintInWater()) addPrintFromGround();
}
void GhostAttackGhostFootprint::addPrintFromRay() {
    TVec3f hit(0.0f);
    TVec3f origin;
    mBaseMtx.getTrans(origin);
    TVec3f ray(mGravity * 500.0f);
    Triangle triangle;
    if (MR::getFirstPolyOnLineToMap(&hit, &triangle, origin, ray)) {
        TVec3f direction;
        MR::extractMtxZDir(mBaseMtx, &direction);
        MR::vecKillElement(direction, -mGravity, &direction);
        if (!MR::normalizeOrZero(&direction)) {
            TVec3f position(hit);
            TVec3f normal(*triangle.getNormal(0));
            position.add(normal * 4.0f);
            mFootPrint->addPrint(position, direction, normal, false);
        }
    }
}
void GhostAttackGhostFootprint::addPrintFromGround() {
    if (!MR::isBindedGround(mHost)) {
        addPrintFromRay();
        return;
    }
    TVec3f direction;
    MR::extractMtxZDir(mBaseMtx, &direction);
    MR::vecKillElement(direction, -mGravity, &direction);
    if (!MR::normalizeOrZero(&direction)) {
        TVec3f position;
        mBaseMtx.getTrans(position);
        TVec3f normal(MR::getGroundNormal(mHost));
        position.add(normal * 4.0f);
        mFootPrint->addPrint(position, direction, normal, false);
    }
}
// NONMATCHING: host pointer registers and matrix component load ordering.
bool GhostAttackGhostFootprint::tryAddPrintOnSlide() {
    if (static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName() == nullptr) return false;
    if (!MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "チューブスライダー着地") && !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "チューブスライダー")) return false;
    TVec3f direction(static_cast<GhostAttackGhost*>(mHost)->mMovementDelta);
    if (!MR::isNearZero(direction, 0.001f)) MR::normalize(&direction);
    else MR::extractMtxZDir(mBaseMtx, &direction);
    TVec3f position(0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    mBaseMtx.getTrans(position);
    up.set<f32>(mBaseMtx[0][1], mBaseMtx[1][1], mBaseMtx[2][1]);
    position.add(up * 4.0f);
    mFootPrint->addPrint(position, direction, up, false);
    return true;
}
// NONMATCHING: host pointer registers and string pooling.
bool GhostAttackGhostFootprint::tryAddPrintInWater() {
    if (static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName() == nullptr) return false;
    if (!MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳一掻き") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳ジェット開始") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳ジェット") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳スピン") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳スピン移動") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳潜り") &&
        !MR::isEqualString(static_cast<GhostAttackGhost*>(mHost)->getRecordedAnimationName(), "水泳亀投げ")) return false;
    TVec3f direction(static_cast<GhostAttackGhost*>(mHost)->mMovementDelta);
    if (!MR::isNearZero(direction, 0.001f)) MR::normalize(&direction);
    else MR::extractMtxZDir(mBaseMtx, &direction);
    TVec3f hit(0.0f);
    TVec3f origin(0.0f);
    mBaseMtx.getTrans(origin);
    TVec3f ray(mGravity * 2000.0f);
    Triangle triangle;
    if (!MR::getFirstPolyOnLineToMap(&hit, &triangle, origin, ray)) return true;
    TVec3f normal(*triangle.getNormal(0));
    MR::vecKillElement(direction, normal, &direction);
    if (MR::isNearZero(direction, 0.001f)) return true;
    MR::normalize(&direction);
    hit.add(normal * 4.0f);
    mFootPrint->addPrint(hit, direction, normal, false);
    return true;
}
MtxPtr GhostAttackGhostFootprint::getBaseMtx() const { return (MtxPtr)&mBaseMtx; }
GhostAttackGhostFootprint::~GhostAttackGhostFootprint() {}

const f32 sGhostPrintWidth = 50.0f;
const f32 sGhostPrintHeight = 50.0f;
const f32 sGhostPrintDistance = 480.0f;
