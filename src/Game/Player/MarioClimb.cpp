#include "Game/Player/MarioClimb.hpp"
#include "Game/Util.hpp"
#include "Game/Player/MarioActor.hpp"

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void Mario::connectToClimb() {
    _790 = -getShadowNorm();
    changeStatus(mClimb);
}
#endif

MarioClimb::MarioClimb(MarioActor* pActor) : MarioState(pActor, MarioStatus_Climb), mTimer() {
}

bool MarioClimb::update() {
    if (mTimer < 15 && checkTrgA()) {
        getPlayer()->tryJump();
        return false;
    }

    if (mTimer != 0) {
        mTimer--;
    }

    addVelocity(getFrontVec(), 6.0f);

    if (mTimer == 0) {
        return false;
    }

    return true;
}

bool MarioClimb::start() {
    changeAnimation("匍匐前進", "匍匐前進");

    if (mActor->_468 != 0) {
        changeAnimationUpper("ひろいウエイト");
    }

    mTimer = 15;
    return true;
}

bool MarioClimb::close() {
    getPlayer()->mWalkSpeed = getStickP();

    if (((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
        stopAnimation("匍匐前進", "基本");
    } else {
        stopAnimation("匍匐前進", "落下");
        getPlayer()->set3BC(8);
    }

    return true;
}
