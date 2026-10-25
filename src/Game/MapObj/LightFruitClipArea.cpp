#pragma auto_inline off
#include "Game/MapObj/LightFruitClipArea.hpp"
#include "Game/MapObj/ClipAreaShape.hpp"
#include "Game/MapObj/FallOutFieldDraw.hpp"
#include "Game/MapObj/StarLightAreaHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace {
    inline f32 calcCylinderOffset(f32 height) {
        return -0.5f * height;
    }


}

namespace NrvLightFruitClipArea {
NEW_NERVE(LightFruitClipAreaNrvAppear, LightFruitClipArea, Appear);
NEW_NERVE(LightFruitClipAreaNrvTimer, LightFruitClipArea, Timer);
NEW_NERVE(LightFruitClipAreaNrvDisappear, LightFruitClipArea, Disappear);
}
LightFruitClipArea::LightFruitClipArea() : ClipArea("光フルーツ用クリップエリア"), mCylinder(0), mVisual(0), mFront(0.0f,0.0f,1.0f) {
    mCylinder = new ClipAreaShapeCylinder;
    mCylinder->mHeight = 2000.0f;
    setShape(mCylinder);
}
void LightFruitClipArea::init(const JMapInfoIter& iter) {
    initBaseMatrix(iter);
    initSound(1, "LightFruitClipArea", 0, TVec3f(0.0f));
    MR::connectToScene(this, 27, -1, -1, 46);
    MR::registerStarLightArea(this);
    mVisual = MR::createPartsModelMapObj2(this, "光壁モデル", "YoshiLightAreaModel", getBaseMtx());
    MR::invalidateClipping(this);
    initNerve(&NrvLightFruitClipArea::LightFruitClipAreaNrvAppear::sInstance);
    makeActorDead();
}
void LightFruitClipArea::killGlow() {
    if (!MR::isDead(this)) {
        mCylinder->mRadius = 1.0f;
        YoshiFunction::deleteGlowEffect();
        MR::resetFallOutFieldDrawEdgeColor();
        kill();
    }
}
// NONMATCHING: SMatrix34C::set expands to the equivalent copy12; target calls it out of line.
void LightFruitClipArea::control() {
    MR::setPosition(this, *MR::getPlayerPos());
    TVec3f playerFront;
    MR::getPlayerFrontVec(&playerFront);
    if (MR::vecBlendSphere(&mFront, mFront, playerFront, 0.1f)) MR::turnRandomVector(&mFront, mFront, 0.01f);
    mScale.x = 0.7f; mScale.y = 1.0f; mScale.z = 1.0f;
    TVec3f gravity;
    if (!MR::calcGravityVectorOrZero(this, mPosition, &gravity, 0, 0)) gravity.set(*MR::getPlayerGravity());
    TPos3f mtx;
    MR::makeMtxUpFrontPos(&mtx, gravity, mFront, mPosition);
    MR::addTransMtxLocalY(mtx.toMtxPtr(), calcCylinderOffset(mCylinder->mHeight));
    MR::addTransMtxLocalZ(mtx.toMtxPtr(), 0.4f * (mCylinder->mRadius - 30.0f));
    mBaseMtx.set(mtx.toMtxPtr());
    TVec3f scale;
    mCylinder->calcScale(&scale, mScale);
    mVisual->mScale.set(scale);
    f32 remaining = 1.0f - MR::normalize(mCylinder->mRadius, 30.0f, 800.0f);
    f32 blue = MR::normalize(remaining, 0.5f, 0.9f);
    f32 green = 1.0f - MR::normalize(remaining, 0.5f, 0.9f);
    f32 red = 1.0f - MR::normalize(remaining, 0.1f, 0.5f);
    GXColor color;
    color.a = 255;
    color.r = u8(255.0f * red); color.g = u8(255.0f * green); color.b = u8(255.0f * blue);
    MR::setFallOutFieldDrawEdgeColor(GXColor(color));
}
// NONMATCHING: the ordered radius sum stays in f2 instead of the target's f1.
namespace {
    inline f32 expandLightClipRadius(f32 increment, f32 radius) {
        return radius + increment;
    }
}

void LightFruitClipArea::exeAppear() {
    if (MR::isFirstStep(this)) { YoshiFunction::emitGlowEffect(); MR::startActionSound(this, "SyLightFruitGet"); }
    bool done = false;
    f32 radius = expandLightClipRadius(15.0f, mCylinder->mRadius);
    if (radius > 800.0f) { radius = 800.0f; done = true; }
    mCylinder->mRadius = radius;
    if (done) setNerve(&NrvLightFruitClipArea::LightFruitClipAreaNrvTimer::sInstance);
}
void LightFruitClipArea::exeTimer() { playTimerSound(); if (MR::isGreaterStep(this, 180)) setNerve(&NrvLightFruitClipArea::LightFruitClipAreaNrvDisappear::sInstance); }
void LightFruitClipArea::exeDisappear() {
    f32 previousRadius = mCylinder->mRadius;
    bool done = false;
    playTimerSound();
    f32 radius = previousRadius - 1.2f;
    if (radius < 30.0f) { radius = 30.0f; done = true; }
    mCylinder->mRadius = radius;
    if (done) { YoshiFunction::deleteGlowEffect(); MR::resetFallOutFieldDrawEdgeColor(); MR::startActionSound(this, "SyLightFruitTimerEnd"); kill(); }
}
void LightFruitClipArea::playTimerSound() {
    if (MR::isPlayerDead() || MR::isDemoActive()) return;
    f32 ratio = (mCylinder->mRadius - 30.0f) / 770.0f;
    MR::startActionSound(this, "SyLvLightFruitTimer", s32(50.0f + 50.0f * ratio), s32(120.0f + 240.0f * (1.0f - ratio)), -1);
}
namespace MR {
void createLightFruitClipArea() { createSceneObj(0x5D); }
void startYoshiLightFruit() {
    LightFruitClipArea* area = getSceneObj<LightFruitClipArea*>(0x5D);
    setPosition(area, *getPlayerPos());
    getPlayerFrontVec(&area->mFront);
    area->setNerve(&NrvLightFruitClipArea::LightFruitClipAreaNrvAppear::sInstance);
    area->makeActorAppeared();
}
void endYoshiLightFruit() { getSceneObj<LightFruitClipArea*>(0x5D)->killGlow(); }
}
LightFruitClipArea::~LightFruitClipArea() {}
