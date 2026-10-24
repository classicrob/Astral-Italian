#include "Game/Player/MarioBump.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"

// NONMATCHING: zero scalar relocation label.
MarioBump::MarioBump(MarioActor* pActor) : MarioState(pActor, MarioStatus_Bump) {
    _12 = 0;
    _14 = 0.0f;
    _18.x = _18.y = _18.z = 0.0f;
}

bool MarioBump::start() {
    _14 = getPlayer()->mWalkSpeed;

    MR::clamp(_14, 0.0f, 1.0f);

    s32 blendTimer = 0x14 - static_cast< s32 >(16.0f * _14);

    if (blendTimer < 4) {
        blendTimer = 4;
    }

    mActor->setBlendMtxTimer(static_cast< u16 >(blendTimer));
    _12 = static_cast< u16 >(blendTimer);

    if (isAnimationRun(nullptr)) {
        stopAnimation(nullptr);
    }

    _18 = getFrontVec();
    return true;
}

bool MarioBump::update() {
    if (mActor->isRequestJump()) {
        getPlayer()->tryJump();
        return false;
    } else {
        clearVelocity();
        f32 val = getPlayer()->mWalkSpeed;

        if (val < 0.1f) {
            val = 0.1f;
        }

        getPlayer()->mWalkSpeed = val;
        getPlayer()->updateWalkSpeed();

        if (_12) {
            --_12;
        }
    }

    if (_12 == 0) {
        return false;
    }

    return true;
}

bool MarioBump::close() {
    return true;
}
