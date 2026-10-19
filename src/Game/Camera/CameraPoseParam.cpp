#include "Game/Camera/CameraPoseParam.hpp"

namespace {
    extern const f32 sUnit;
    extern const f32 sZero;
    extern const f32 sDefaultFovy;
}

CameraPoseParam::CameraPoseParam() {
    mWatchUpVec.set(sZero, sUnit, sZero);
    mWatchPos.set(sZero, sZero, sUnit);
    mUpVec.set(sZero, sUnit, sZero);
    mPos.set(sZero, sZero, sZero);
    mFovy = sDefaultFovy;
    mGlobalOffset.set(sZero, sZero, sZero);
    mLocalOffset.set(sZero, sZero, sZero);
    mFrontOffset = sZero;
    mUpperOffset = sZero;
    mRoll = sZero;
}

namespace {
    const f32 sUnit = 1.0f;
    const f32 sZero = 0.0f;
    const f32 sDefaultFovy = 40.0f;
}
