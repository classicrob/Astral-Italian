#include "Game/Gravity/BarrelGravity.hpp"

BarrelGravity::BarrelGravity() {
    mLocalPosition.set(0.0f, 0.0f, 0.0f);
    mPosition.set(0.0f, 0.0f, 0.0f);
    mLocalUpDirection.set(0.0f, 1.0f, 0.0f);
    mUpDirection.set(0.0f, 1.0f, 0.0f);
    mRadius = 1000.0f;
    mScaledRadius = 1000.0f;
    mUnusedArg0 = 500.0f;
    mInwardsThreshold = 1.0f;
}

void BarrelGravity::setLocalPosition(const TVec3f& rPos) {
    mLocalPosition.set(rPos);
}

void BarrelGravity::setUpDirection(const TVec3f& rDir) {
    mLocalUpDirection.set(rDir);
}

void BarrelGravity::setRadius(f32 radius) {
    mRadius = radius;
}

void BarrelGravity::setUnusedArg0(f32 arg) {
    mUnusedArg0 = arg;
}

void BarrelGravity::setInwardsThreshold(f32 threshold) {
    mInwardsThreshold = threshold;
}
