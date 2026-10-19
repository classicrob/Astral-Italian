#pragma auto_inline off
#include "Game/MapObj/StarPieceClusterRock.hpp"
#include "Game/MapObj/StarPieceTracking.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
namespace NrvStarPieceClusterRock {
NEW_NERVE(StarPieceClusterRockNrvWait, StarPieceClusterRock, Wait);
NEW_NERVE(StarPieceClusterRockNrvBreak, StarPieceClusterRock, Break);
}
StarPieceClusterRock::StarPieceClusterRock(const char* name) : LiveActor(name), mPieceCount(4), mPieces(0), mSpeed(1.0f), mItemType(0) {}
// NONMATCHING: tracking-piece name address hoisting and register allocation differ.
void StarPieceClusterRock::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvStarPieceClusterRock::StarPieceClusterRockNrvWait::sInstance);
    s32 itemType = -1;
    MR::getJMapInfoArg0NoInit(iter, &itemType);
    if (itemType != -1) mItemType = itemType;
    MR::getJMapInfoArg1NoInit(iter, &mPieceCount);
    if (mItemType == 0) {
        mPieces = new StarPieceTracking("スターピース", mPieceCount);
        StarPieceTracking* group = mPieces;
        for (s32 i=0; i<group->mObjNumMax; ++i) {
            StarPieceTrackingPiece* piece = new StarPieceTrackingPiece("追尾スターピース");
            piece->initWithoutIter();
            group->registerActor(piece);
        }
        mPieces->makeAllDead();
    } else MR::declareCoin(this, mPieceCount);
    makeActorAppeared();
}
void StarPieceClusterRock::attackSensor(HitSensor* sender, HitSensor* receiver) { if (isNerve(&NrvStarPieceClusterRock::StarPieceClusterRockNrvWait::sInstance) && MR::isSensorPlayerOrRide(receiver)) MR::sendMsgPush(receiver, sender); }
bool StarPieceClusterRock::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (!isNerve(&NrvStarPieceClusterRock::StarPieceClusterRockNrvWait::sInstance)) return false;
    if (MR::isMsgRockAttack(msg) || MR::isMsgSupportTicoSpin(msg)) {
        if (mItemType == 0) calcStarPieceDirection();
        else if (mItemType == 1) MR::appearCoinPop(this, mPosition, mPieceCount);
        setNerve(&NrvStarPieceClusterRock::StarPieceClusterRockNrvBreak::sInstance);
        return true;
    }
    return false;
}

// NONMATCHING: negation temporary uses r4/r3 in the opposite order.
void StarPieceClusterRock::calcStarPieceDirection() {
    TVec3f up(0.0f), side(0.0f), front(0.0f);
    front.set(*MR::getPlayerVelocity());
    if (MR::normalizeOrZero(&front)) { MR::calcFrontVec(&front, this); front = -front; }
    MR::calcUpVec(&up, this);
    PSVECCrossProduct(&up, &front, &side);
    if (MR::normalizeOrZero(&side)) MR::calcSideVec(&side, this);
    TVec3f oldFront(front);
    PSVECCrossProduct(&side, &up, &front);
    if (MR::normalizeOrZero(&side)) front.set(oldFront);
    TVec3f direction(front);
    if (mPieceCount != 1) {
        mPieces->appear(*MR::getSensorPos(this, "Body"), front, up, side, 180.0f, mSpeed, 100.0f, 2000.0f);
    } else {
        for (s32 i = 0; i < mPieceCount; ++i) {
            static_cast<StarPieceTrackingPiece*>(mPieces->getActor(i))->appearFromActor(this, direction, up, mSpeed, 200.0f, 2000.0f);
        }
    }
}

void StarPieceClusterRock::exeWait() { MR::attachSupportTicoToTarget(this); }
void StarPieceClusterRock::exeBreak() { if (MR::isFirstStep(this)) { MR::tryHitReaction(this, "壊れ"); kill(); } }
StarPieceClusterRock::~StarPieceClusterRock() {}
