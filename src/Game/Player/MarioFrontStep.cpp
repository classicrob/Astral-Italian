#include "Game/Player/MarioFrontStep.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioModule.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

bool Mario::doFrontStep() {
    if (getCurrentStatus() == MarioStatus_FrontStep) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }

    if (checkWallCode("NotSideStep", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    if (mActor->_468 != 0) {
        return false;
    }

    if (((mFlags20.mWord >> 26) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }

    if (_960 == 2) {
        return false;
    }

    if (calcAngleD(_368) >= 30.0f) {
        return false;
    }

    if (mAirGravityVec.dot(_374) >= 0.25f) {
        return false;
    }

    Triangle triangle = Triangle();
    MarioActor* pMario = mActor;
    TVec3f* vec = &pMario->_2A0;
    if (!MR::getFirstPolyOnLineToMap(nullptr, &triangle, *vec, mFrontVec * 200.0f)) {
        return false;
    }

    setFrontVecKeepUp(-*MR::getNormal(&triangle));

    stopWalk();
    forceStopTornado();

    changeStatus(mFrontStep);

    return true;
}

bool MarioFrontStep::start() {
    changeAnimation("前壁ウエイト");

    mActor->setBlendMtxTimer(10);

    getPlayer()->lockGroundCheck(this, true);

    return true;
}

/*
 */

MarioFrontStep::MarioFrontStep(MarioActor* pActor) : MarioState(pActor, MarioStatus_FrontStep) {
}

// NONMATCHING: vector temporary stack slots and scalar literal labels.
bool MarioFrontStep::update() {
    getPlayer()->stopWalk();

    if (((getPlayer()->mFlags20.mWord >> 26) & 1U)) {
        return false;
    }

    MarioActor* pMario = mActor;
    TVec3f collisionOffset(getFrontVec());
    collisionOffset.x *= 200.0f;
    collisionOffset.y *= 200.0f;
    collisionOffset.z *= 200.0f;
    if (!getPlayer()->fn_803B2880(pMario->_240, collisionOffset)) {
        return false;
    }

    if (!(getStickP() < 0.1f)) {
        TVec3f worldDir(getWorldPadDir());
        MR::vecKillElement(worldDir, getPlayer()->getAirGravityVec(), &worldDir);

        MR::normalizeOrZero(&worldDir);

        TVec3f vec;
        f32 val = MR::vecKillElement(worldDir, -getPlayer()->mFrontVec, &vec);
        if (!(val < -0.866f) && val > 0.0f) {
            return false;
        }
    }

    if (checkTrgA()) {
        getPlayer()->resetInline();

        getPlayer()->setFrontVecKeepUp(-getPlayer()->getWallNorm());

        getPlayer()->tryJump();

        return false;
    }

    if (mActor->isRequestRush()) {
        if (isPlayerModeFire()) {
            mActor->updateFireSpin();
        } else {
            getPlayer()->tryWallPunch();
        }
        return false;
    }

    if (!((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
        getPlayer()->tryDrop();
        return false;
    }

    return true;
}

bool MarioFrontStep::close() {
    stopAnimation("前壁ウエイト");

    getPlayer()->lockGroundCheck(this, false);

    return true;
}

bool MarioFrontStep::postureCtrl(MtxPtr pMtx) {
    TVec3f blendVec;
    if (MR::vecBlendSphere(-getPlayer()->getAirGravityVec(), -getPlayer()->mFrontVec, &blendVec, 0.25f)) {
        MR::normalizeOrZero(&blendVec);
    } else {
        blendVec = -getPlayer()->getAirGravityVec();
    }

    MR::makeMtxUpFront(reinterpret_cast< TPos3f* >(pMtx), blendVec, getPlayer()->mFrontVec);

    return true;
}
