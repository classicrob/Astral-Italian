#include "Game/Player/MarioStep.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <revolution/mtx.h>

MarioStep::MarioStep(MarioActor* pActor) : MarioState(pActor, MarioStatus_Step) {
    _14 = 0.0f;
    _18 = 0.0f;
}

bool MarioStep::start() {
    _18 = getPlayer()->mWalkSpeed;
    MR::clamp(_18, 0.0f, 1.0f);

    s32 v2 = 10 - static_cast< s32 >(6.0f * _18);

    if (v2 < 4) {
        v2 = 4;
    }

    mActor->setBlendMtxTimer(v2);
    _14 = static_cast< u16 >(v2);

    if (isAnimationRun(nullptr) && !mActor->_3E5) {
        stopAnimation(nullptr);
    }

    return true;
}

bool MarioStep::update() {
    if (mActor->isRequestJump()) {
        getPlayer()->tryJump();
        return false;
    } else if (((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return false;
    }

    getPlayer()->fn_803A0BE0();
    clearVelocity();

    f32 _278 = getPlayer()->mWalkSpeed;
    if (_278 < 0.1f) {
        _278 = 0.1f;
    }

    getPlayer()->mWalkSpeed = _278;
    getPlayer()->updateWalkSpeed();

    if (_278 < 0.5f) {
        _278 = 0.5f;
    }

    f32 v7 = _14 - _278;
    _14 -= _278;

    if (v7 < 0.0f) {
        return false;
    } else {
        mActor->setBlendMtxTimer(v7);
        return true;
    }
}

bool MarioStep::close() {
    return true;
}
