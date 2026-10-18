#include "Game/MapObj/WingBlock.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvWingBlock {
    NEW_NERVE(WingBlockNrvFly, WingBlock, Fly);
    NEW_NERVE(WingBlockNrvHit, WingBlock, Hit);
};  // namespace NrvWingBlock

WingBlock::WingBlock(const char* pName) : LiveActor(pName) {
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mRailPos.zero();
    mFlySpeed = 0.0f;
    mItemType = -1;
}

void WingBlock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "WingBlock", false);
    MR::makeQuatFromRotate(&mQuat, this);
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f gravity;
    gravity.negate(up);
    mGravity.set(gravity);
    MR::calcGravity(this);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    initItem(rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mFlySpeed);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        MR::initAndSetRailClipping(&mRailPos, this, 1.0f, 1000.0f);
    }
    initNerve(&NrvWingBlock::WingBlockNrvFly::sInstance, 0);
    makeActorAppeared();
}

void WingBlock::initItem(const JMapInfoIter& rIter) {
    if (MR::isObjectName(rIter, "WingBlockCoin")) {
        MR::declareCoin(this, 1);
        mItemType = 0;
    } else if (MR::isObjectName(rIter, "WingBlockStarPiece")) {
        MR::declareStarPiece(this, 8);
        mItemType = 1;
    }
}

void WingBlock::appearItem() {
    if (mItemType == 0) {
        TVec3f dir(mGravity);
        dir *= mFlySpeed;
        MR::appearCoinPopToDirection(this, mPosition, dir, 1);
    } else if (mItemType == 1) {
        TVec3f dir;
        dir.negate(mGravity);
        MR::appearStarPieceToDirection(this, mPosition, dir, 8, 1.0f, 2.0f, false);
    }
}

// NONMATCHING: control() approximated (camera-facing quat turn), ~36%; rates and sign handling not recovered.
void WingBlock::control() {
    TVec3f camZ = MR::getCamZdir();
    TVec3f back;
    back.negate(camZ);
    MR::turnQuatZDirRate(&mQuat, mQuat, back, 0.0f);
    TVec3f up;
    up.negate(mGravity);
    MR::turnQuatYDirRate(&mQuat, mQuat, up, 0.0f);
}

void WingBlock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

bool WingBlock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerUpperPunch(msg)) {
        if (isNerve(&NrvWingBlock::WingBlockNrvFly::sInstance)) {
            setNerve(&NrvWingBlock::WingBlockNrvHit::sInstance);
            MR::sendMsgReflectV(pReceiver, pSender);
            return true;
        }
    } else if (MR::isMsgSupportTicoSpin(msg)) {
        if (isNerve(&NrvWingBlock::WingBlockNrvFly::sInstance)) {
            setNerve(&NrvWingBlock::WingBlockNrvHit::sInstance);
            return true;
        }
    }
    return false;
}

void WingBlock::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    if (MR::getBckFrame(this) >= 0.0f && ((s32)MR::getBckFrame(this) % 20) == 1) {
        MR::startActionSound(this, "FlyTrg", -1, -1, -1);
    }
    if (MR::isExistRail(this)) {
        MR::moveCoordAndFollowTrans(this, mFlySpeed);
        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
            MR::attachSupportTicoToTarget(this);
        }
    }
}

void WingBlock::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
    }
    if (MR::isActionEnd(this)) {
        appearItem();
        kill();
    }
}

WingBlock::~WingBlock() {
}
