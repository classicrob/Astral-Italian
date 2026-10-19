#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE
#include "Game/MapObj/WingBlock.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvWingBlock {
    NEW_NERVE(WingBlockNrvFly, WingBlock, Fly);
    NEW_NERVE(WingBlockNrvHit, WingBlock, Hit);
};  // namespace NrvWingBlock

WingBlock::WingBlock(const char* pName) : LiveActor(pName), mQuat(0.0f, 0.0f, 0.0f, 1.0f), mRailPos(0.0f, 0.0f, 0.0f), mFlySpeed(4.0f), mItemType(-1) {
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
    switch (mItemType) {
    case 0: {
        TVec3f dir(mGravity);
        dir *= -10.0f;
        MR::appearCoinPopToDirection(this, mPosition, dir, 1);
        break;
    }
    case 1: {
        TVec3f dir;
        dir.negate(mGravity);
        MR::appearStarPieceToDirection(this, mPosition, dir, 8, 10.0f, 40.0f, false);
        break;
    }
    }
}

void WingBlock::control() {
    TVec3f camZ = MR::getCamZdir();
    TVec3f zDir;
    mQuat.getZDir(zDir);

    if (zDir.dot(camZ) <= 0.0f) {
        camZ.negate();
    }

    MR::turnQuatZDirRate(&mQuat, mQuat, camZ, 0.05f);
    TVec3f up;
    up.negate(mGravity);
    MR::turnQuatYDirRate(&mQuat, mQuat, up, 0.95f);
    mQuat.normalize();
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

    if (static_cast< s32 >(MR::getBckFrame(this)) % 20 == 1) {
        MR::startActionSound(this, "FlyTrg", -1, -1, -1);
    }

    if (MR::isExistRail(this)) {
        f32 rate = MR::normalize(MR::calcDistanceToNearestRailEnd(this), 0.0f, 50.0f);
        MR::moveCoordAndFollowTrans(this, MR::getInterpolateValue(rate, 0.1f * mFlySpeed, mFlySpeed));

        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
        }
    }

    MR::attachSupportTicoToTarget(this);
}

void WingBlock::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
    }

    if (MR::isActionEnd(this)) {
        MR::emitEffect(this, "Break");
        appearItem();
        kill();
    }
}

WingBlock::~WingBlock() {
}
