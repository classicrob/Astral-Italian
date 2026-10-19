#include "Game/System/GrandGalaxyMapGalaxy.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Util.hpp"
#include <cstdio>

GrandGalaxyMapGalaxy::GrandGalaxyMapGalaxy(const char* pName, const char* pModelName)
    : LiveActor(pName), mLayoutOffset(0.0f, 0.0f), mBaseScale(0.3f), mBoundingRadius(1000.0f), mIsRotating(true) {
    char path[0x80];
    snprintf(path, sizeof(path), "/ObjectData/%s.arc", pModelName);
    if (!MR::isFileExist(path, true)) {
        pModelName = "MiniFlowerHighlandGalaxy";
    }

    if (!MR::initActor(this, pModelName, "MiniIcon", true)) {
        initEffectKeeper(0, nullptr, false);
        MR::connectToSceneWorldMapIcon(this);
        MR::initStarPointerTarget(this, 1000.0f, TVec3f(0.0f, 0.0f, 0.0f));
    }

    MR::setAllEffectDrawOrder(this, 9);
    MR::setClippingFarMax(this);
    MR::invalidateClipping(this);
    WorldMapFunction::initDrawFog(this);
    MR::calcModelBoundingRadius(&mBoundingRadius, this);
    MR::tryStartAllAnim(this, pModelName);
    setFitRadius(4000.0f);
    mScale.set(TVec3f(mBaseScale));
    makeActorDead();
}

void GrandGalaxyMapGalaxy::emitEffect(bool isNew) {
    MR::tryEmitEffect(this, MR::getModelResName(this));
    WorldMapFunction::setFogDrawOff(this);
}

void GrandGalaxyMapGalaxy::control() {
    if (mIsRotating) {
        mRotation.y += 0.2f;
        MR::repeatDegree(&mRotation.y);
    }
}

void GrandGalaxyMapGalaxy::stopRotate(const TVec3f& rRotate) {
    mIsRotating = false;
    mRotation = rRotate;
}

void GrandGalaxyMapGalaxy::setFitRadius(f32 radius) {
    if (mBoundingRadius > radius) {
        mBaseScale = 0.3f * radius / mBoundingRadius;
    } else {
        mBaseScale = 0.3f;
    }
    mScale.set(TVec3f(mBaseScale));
}

GrandGalaxyMapGalaxy::~GrandGalaxyMapGalaxy() {}
