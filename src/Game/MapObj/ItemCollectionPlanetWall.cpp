#pragma auto_inline off
#include "Game/MapObj/ItemCollectionPlanetWall.hpp"
#include "Game/MapObj/ItemDiceFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include <cmath>

namespace { const char* sDemoName = "パワースター帰還[アイテム惑星]"; }
namespace NrvItemCollectionPlanetWall {
    class ItemCollectionPlanetWallNrvWait : public Nerve {
    public:
        void execute(Spine*) const {}
        static ItemCollectionPlanetWallNrvWait sInstance;
    };
    ItemCollectionPlanetWallNrvWait ItemCollectionPlanetWallNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(ItemCollectionPlanetWallNrvFadeIn, ItemCollectionPlanetWall, FadeIn);
}
ItemCollectionPlanetWall::ItemCollectionPlanetWall(const char* name) : LiveActor(name) {}
void ItemCollectionPlanetWall::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvWait::sInstance);
    MR::startAction(this, "FadeIn");
    MR::setAllAnimFrameAtEnd(this, "FadeIn");
    if (MR::tryRegisterDemoCast(this, sDemoName, iter)) {
        MR::registerDemoActionNerveDirect(this, &NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvFadeIn::sInstance, sDemoName, nullptr);
        MR::registerDemoActionFunctorDirect(this, MR::FunctorV0M<ItemCollectionPlanetWall*, void (ItemCollectionPlanetWall::*)()>(this, &ItemCollectionPlanetWall::listenDemoAction), sDemoName, nullptr);
    }
    makeActorAppeared();
}
void ItemCollectionPlanetWall::listenDemoAction() { MR::startAction(this, "FadeIn"); }
void ItemCollectionPlanetWall::exeFadeIn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FadeIn");
        MR::setAllAnimFrameAndStop(this, "FadeIn", 0.0f);
    }
    if (MR::isDemoLastStep()) setNerve(&NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvWait::sInstance);
}
ItemCollectionPlanetWall::~ItemCollectionPlanetWall() {}

namespace ItemDiceFunction {
// NONMATCHING: direction extraction and sign branches compile differently.
s32 calcFaceIndex(MtxPtr source, const TVec3f* direction) {
    TRot3f matrix;
    matrix.set(source);
    TVec3f x;
    x.set<f32>(matrix.mMtx[0][0], matrix.mMtx[1][0], matrix.mMtx[2][0]);
    TVec3f y;
    matrix.getYDir(y);
    TVec3f z;
    matrix.getZDir(z);
    f32 dx = x.dot(*direction);
    f32 dy = y.dot(*direction);
    f32 dz = z.dot(*direction);
    if (__fabsf(dx) > __fabsf(dy)) {
        if (__fabsf(dx) > __fabsf(dz)) return dx < 0.0f ? 0 : 1;
        return dz < 0.0f ? 4 : 5;
    }
    if (__fabsf(dy) > __fabsf(dz)) return dy < 0.0f ? 2 : 3;
    return dz < 0.0f ? 4 : 5;
}
// NONMATCHING: matrix/vector helpers inline with the current shared headers.
void initItemPose(LiveActor* actor, const TVec3f* position, TRot3f* output, f32 angle) {
    TPos3f matrix;
    MR::setPosition(actor, *position);
    TVec3f gravity;
    MR::calcGravityVector(actor, &gravity, nullptr, 0);
    TVec3f front = *MR::getPlayerPos() - *position;
    MR::normalizeOrZero(&front);
    TVec3f right;
    PSVECCrossProduct(&front, &gravity, &right);
    if (!MR::normalizeOrZero(&right)) {
        TVec3f up;
        PSVECCrossProduct(&front, &right, &up);
        matrix.setXDir(right);
        matrix.setYDir(up);
        matrix.setZDir(front);
    }
    else {
        MR::makeMtxTR(matrix.toMtxPtr(), actor);
        TVec3f up;
        matrix.getYDir(up);
        TQuat4f quat;
        quat.setRotate(up, -gravity);
        TRot3f rotation;
        rotation.setQuat(quat);
        matrix.concat(rotation, matrix);
    }
    TPos3f rotation;
    rotation.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), angle);
    matrix.concat(matrix, rotation);
    matrix.setTrans(*position);
    if (output) output->set(matrix);
    matrix.getEulerXYZ(actor->mRotation);
    actor->mRotation.x *= 57.2957763671875f;
    actor->mRotation.y *= 57.2957763671875f;
    actor->mRotation.z *= 57.2957763671875f;
}
}
