#include "Game/Gravity.hpp"

GravityInfo::GravityInfo() {
    mGravityVector.x = 0.0f;
    mGravityVector.y = 0.0f;
    mGravityVector.z = 0.0f;
    mLargestPriority = -1;
    mGravityInstance = nullptr;
}

void GravityInfo::init() {
    mGravityVector.zero();
    mLargestPriority = -1;
    mGravityInstance = nullptr;
}
