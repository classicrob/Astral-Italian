#include "Game/Player/MarioTalk.hpp"
#include "Game/Util.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Util/MathUtil.hpp"

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void Mario::startTalk(const LiveActor* pActor) {
    stopWalk();

    mTalk->_14 = pActor;

    changeStatus(mTalk);
}
#endif

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void Mario::endTalk() {
    closeStatus(mTalk);
}
#endif

MarioTalk::MarioTalk(MarioActor* pActor) : MarioState(pActor, MarioStatus_Talk), _14(), mIsUpdate() {
}

bool MarioTalk::update() {
    if (mIsUpdate) {
        TVec3f vec;
        MR::vecKillElement(_14->mPosition - getTrans(), getAirGravityVec(), &vec);

        if (!MR::normalizeOrZero(&vec)) {
            getPlayer()->setFrontVecKeepUp(vec, static_cast< u32 >(16));

            if (!((getPlayer()->getMovementStates().mHighWord >> 8) & 1U)) {
                getPlayer()->setLastNonFixMoveVec(vec);
            } else if (isPlayerModeTeresa()) {
                MR::vecKillElement(vec, getCamDirZ(), &vec);
                if (!MR::normalizeOrZero(&vec)) {
                    getPlayer()->setLastNonFixMoveVec(vec);
                }
            }
        }

        mIsUpdate = false;
    }

    return true;
}

bool MarioTalk::notice() {
    return true;
}

bool MarioTalk::close() {
    if (((getPlayer()->getMovementStates().mHighWord >> 8) & 1U)) {
        getPlayer()->setFrontVecKeepUp(getPlayer()->_334);
    }

    return true;
}

bool MarioTalk::start() {
    bool val = mActor->_B90;
    mActor->_B90 = false;

    if (mActor->_468 == 0) {
        stopAnimationUpper(nullptr);
    }

    changeAnimationNonStop("見る");

    if (val) {
        mActor->_B90 = true;
    }

    mIsUpdate = true;

    return true;
}

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void Mario::setLastNonFixMoveVec(const TVec3f& rVec) {
    _334 = rVec;
}
#endif
