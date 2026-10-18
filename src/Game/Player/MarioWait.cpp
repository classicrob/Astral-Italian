#include "Game/Player/MarioWait.hpp"
#include "Game/Util.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/MathUtil.hpp"

bool MarioWait::checkStart() {
    if (mActor->_EA4) {
        _16 = 0;
        return false;
    }

    if (getPlayer()->isStatusActive(MarioStatus_Wait)) {
        return false;
    }

    bool damaged = mActor->is481or482On();
    if (damaged) {
        _16 = 0;
        return false;
    }

    if (checkCancel()) {
        _16 = 0;
        return false;
    }
    _16++;
    if (_16 < 1800) return false;
    if (MR::getAreaObj("NonSleepCube", getTrans()) != nullptr) {
        _12 = 1;
        return true;
    }
    _12 = 0;
    return true;
}

bool MarioWait::checkCancel() {
    switch (getPlayer()->_960) { case 27: case 28: return true; }
    if (getStickP() != 0.0f || checkLvlA() || checkLvlZ() || mActor->isRequestRush()) return true;
    if (getPlayer()->mMovementStates.jumping && getPlayerMode() != PlayerMode_Teresa) return true;
    if (getPlayer()->isStatusActive(6)) return true;
    if (getPlayer()->isStatusActive(5)) return true;
    if (getPlayer()->isStatusActive(0x1A)) return true;
    if (getPlayer()->isStatusActive(0x1B)) return true;
    if (getPlayerMode() == PlayerMode_Hopper) return true;
    if (isPlayerModeYoshi()) return true;
    if (isStatusActiveID(0x18)) return true;
    if (getPlayer()->isStatusActive(0x12)) return true;
    if (getPlayer()->isStatusActive(0x11)) return true;
    if (getPlayer()->isStatusActive(0x22)) return true;
    if (mActor->mBeeWallWalk) return true;
    if (getPlayer()->_1C._5) return true;
    if (getPlayer()->mMovementStates._23) return true;
    if (getPlayer()->mMovementStates._A) return true;
    if (getPlayer()->mDrawStates.mIsUnderwater) return true;
    if (!MR::isNearZero(getPlayer()->_184, 0.001f)) return true;
    if (mActor->_480) return true;
    HitSensor* sensor;
    if (mActor->_468 == 0) sensor = nullptr;
    else sensor = mActor->_428[0];
    if (sensor) return true;
    return !MR::isNearZero(getPlayer()->mWalkSpeed, 0.001f);
}

bool MarioWait::start() {
    switch (_12) {
    case 0:
        changeAnimation("特殊ウエイト1A");
        break;
    case 1:
        changeAnimation("戦闘ウエイト");
        break;
    }

    _16 = 0;
    _14 = 0;
    return true;
}

bool MarioWait::update() {
    if (getStickP() != 0.0f || checkLvlA() || checkLvlZ() || mActor->isRequestJump2P()) {
        stopAnimation(nullptr);
        getPlayer()->mainMove();
        return false;
    }

    bool cancel = false;
    if (mActor->isRequestRush()) {
        cancel = true;
    }

    if (getPlayer()->isStatusActive(MarioStatus_FpView)) {
        cancel = true;
    }

    if (getPlayer()->_1C._5) {
        cancel = true;
    }

    if (getPlayerMode() != PlayerMode_Teresa && !getPlayer()->mMovementStates._1) {
        cancel = true;
    }

    if (!MR::isNearZero(getPlayer()->_184)) {
        cancel = true;
    }

    if (cancel) {
        stopAnimation(nullptr);
        return false;
    }

    _16++;
    switch (_12) {
    case 0:
        switch (_14) {
        case 0:
            if (!mActor->_468) {
                u32 duration = 1800;
                if (isPlayerModeTeresa()) {
                    duration = 210;
                }

                if (_16 == duration) {
                    _16 = 0;
                    _14++;
                    changeAnimation("特殊ウエイト1B");
                }
            }

            break;
        case 1:
            break;
        }

        break;
    case 1:
    case 2:
        break;
    }

    return true;
}

bool MarioWait::close() {
    stopSound("声あくび", 0);
    stopSound("声いびき１", 0);
    stopSound("声いびき２", 0);
    _16 = 0;
    return true;
}

bool Mario::isBlendWaitGround() const {
    return calcAngleD(_368) > 8.0f;
}

void Mario::checkSpecialWaitAnimation() {
    if (mWait->checkStart()) {
        changeStatus(mWait);
    }
}

void Mario::resetSleepTimer() {
    mWait->_16 = 0;
    if (isStatusActive(MarioStatus_Wait)) {
        closeStatus(mWait);
    }
}

MarioWait::MarioWait(MarioActor* pActor) : MarioState(pActor, MarioStatus_Wait) {
    _16 = 0;
    _12 = 0;
    _14 = 0;
}

void MarioState::hitPoly(const TVec3f&, HitSensor*) {
}

bool MarioState::keep() {
    return true;
}

bool MarioState::notice() {
    return false;
}

void MarioState::init() {
}
