#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Enemy/Tongari.hpp"
#include "Game/Enemy/Togezo.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include <revolution/mtx.h>

namespace NrvTongari {
    NEW_NERVE(TongariNrvWait, Tongari, Wait);
    NEW_NERVE(TongariNrvGuard, Tongari, Guard);
    NEW_NERVE(TongariNrvTurn, Tongari, Turn);
    NEW_NERVE(TongariNrvFreeze, Tongari, Freeze);
};  // namespace NrvTongari

Tongari::Tongari(const char* pName) : LiveActor(pName) {
    mFront.set(0.0f, 0.0f, 1.0f);
    mUp.set(0.0f, 1.0f, 0.0f);
    mSpeed = 2.0f;
    mTurnFlag = false;
    mIs2D = false;
    mCollisionParts = nullptr;
    mBindStarPointer = nullptr;
    mScaleCtrl = nullptr;
    mMtx.identity();
}

Tongari::~Tongari() {}

void Tongari::init(const JMapInfoIter& rIter) {
    const char* pObjName = nullptr;
    MR::getObjectName(&pObjName, rIter);
    mIs2D = MR::isEqualString(pObjName, "Tongari2D");

    if (mIs2D) {
        MR::initActor(this, rIter, "Tongari", "2D", false);
    } else {
        MR::initActor(this, rIter, false);
    }

    if (mIs2D) {
        bool hasCollision = false;
        MR::getJMapInfoArg1NoInit(rIter, &hasCollision);
        if (hasCollision) {
            mCollisionParts = MR::createCollisionPartsFromLiveActor(this, "Tongari", getSensor("Collision"), mMtx, MR::CollisionScaleType_Default);
            MR::validateCollisionParts(mCollisionParts);
        }
    }

    MR::getJMapInfoArg0NoInit(rIter, &mSpeed);
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    MR::moveCoordAndTransToNearestRailPos(this);

    TVec3f railDir;
    MR::calcRailDirection(&railDir, this);
    if (!MR::isRailGoingToEnd(this)) {
        railDir = -railDir;
    }
    mFront = railDir;

    initNerve(&NrvTongari::TongariNrvWait::sInstance, 1);

    mScaleCtrl = new AnimScaleController(nullptr);
    mScaleCtrl->setParamTight();
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, &NrvTongari::TongariNrvFreeze::sInstance, "SupportFreeze");

    makeActorAppeared();
}

void Tongari::initAfterPlacement() {
    NameObj::initAfterPlacement();
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f dropDir = -up;
    MR::setShadowDropDirection(this, "Body", dropDir);
}

void Tongari::control() {
    mScaleCtrl->updateNerve();

    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f dropDir = -up;
    MR::setShadowDropDirection(this, "Body", dropDir);

    if (!isNerve(&NrvTongari::TongariNrvGuard::sInstance) && !isNerve(&NrvTongari::TongariNrvFreeze::sInstance) &&
        mBindStarPointer->tryStartPointBind()) {
        setNerve(&NrvTongari::TongariNrvFreeze::sInstance);
    }
}

void Tongari::calcAndSetBaseMtx() {
    MR::calcAndSetBaseMtxFromFrontAndUp(this, &mFront, &mUp);
    if (mCollisionParts != nullptr) {
        mMtx.setInline(getBaseMtx());
        MR::preScaleMtx(mMtx, 0.95f);
        mCollisionParts->setMtx();
    }
    MR::updateBaseScale(this, mScaleCtrl);
}

void Tongari::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mIs2D) {
        bool damaging = MR::isPlayerDamaging();
        if (mCollisionParts != nullptr && !MR::isPlayerElementModeTeresa() && damaging) {
            return;
        }
    }
    {
        if (MR::isSensorPlayer(pReceiver) && !MR::isPlayerElementModeTeresa()) {
            bool attacked = MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender);
            if (!isNerve(&NrvTongari::TongariNrvFreeze::sInstance) && attacked) {
                setNerve(&NrvTongari::TongariNrvGuard::sInstance);
            }
        }
    }
}

bool Tongari::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool Tongari::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTongari::TongariNrvGuard::sInstance) != false) {
        return false;
    }
    if (isNerve(&NrvTongari::TongariNrvFreeze::sInstance) != false) {
        return false;
    }
    if (!MR::isPlayerElementModeTeresa()) {
        return false;
    }
    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }
    if (msg != 0x4D) {
        return false;
    }
    setNerve(&NrvTongari::TongariNrvGuard::sInstance);
    return false;
}

void Tongari::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }

    if (MR::checkPassBckFrame(this, 0.0f) || MR::checkPassBckFrame(this, 72.0f) || MR::checkPassBckFrame(this, 137.0f)) {
        MR::startActionSound(this, "WaitA", -1, -1, -1);
    } else if (MR::checkPassBckFrame(this, 30.0f) || MR::checkPassBckFrame(this, 102.0f) || MR::checkPassBckFrame(this, 162.0f)) {
        MR::startActionSound(this, "WaitB", -1, -1, -1);
    }

    MR::moveCoordAndFollowTrans(this, mSpeed);
    if (!MR::isLoopRail(this) && MR::isRailReachedGoal(this)) {
        setNerve(&NrvTongari::TongariNrvTurn::sInstance);
        return;
    }

    rotateOnRail(0.5f);
    if (mIs2D) {
        calcDiagonalRailDirectionFacingCamera(&mFront, this);
    }
}

void Tongari::exeTurn() {
    if (MR::isFirstStep(this) && !mTurnFlag) {
        mTurnFlag = true;
        MR::reverseRailDirection(this);
    }

    if (MR::checkPassBckFrame(this, 0.0f) || MR::checkPassBckFrame(this, 72.0f) || MR::checkPassBckFrame(this, 137.0f)) {
        MR::startActionSound(this, "TurnA", -1, -1, -1);
    } else if (MR::checkPassBckFrame(this, 30.0f) || MR::checkPassBckFrame(this, 102.0f) || MR::checkPassBckFrame(this, 162.0f)) {
        MR::startActionSound(this, "TurnB", -1, -1, -1);
    }

    TVec3f dir;
    MR::calcRailDirection(&dir, this);
    if (!MR::isRailGoingToEnd(this)) {
        dir = -dir;
    }
    if (mIs2D) {
        calcDiagonalRailDirectionFacingCamera(&dir, this);
    }
    MR::turnVecToVecCosOnPlane(&mFront, dir, mUp, MR::cosDegree(1.0f));

    if (mFront.dot(dir) > 0.999f) {
        mTurnFlag = false;
        setNerve(&NrvTongari::TongariNrvWait::sInstance);
    }
}

void Tongari::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
    }
    if (MR::isActionEnd(this)) {
        if (mTurnFlag) {
            setNerve(&NrvTongari::TongariNrvTurn::sInstance);
            return;
        }
        setNerve(&NrvTongari::TongariNrvWait::sInstance);
    }
}

void Tongari::exeFreeze() {
    if (mTurnFlag) {
        MR::updateActorStateAndNextNerve(this, &NrvTongari::TongariNrvTurn::sInstance);
    } else {
        MR::updateActorStateAndNextNerve(this, &NrvTongari::TongariNrvWait::sInstance);
    }
}

void Tongari::rotateOnRail(f32 angle) {
    TVec3f dir;
    MR::calcRailDirection(&dir, this);
    if (!MR::isRailGoingToEnd(this)) {
        dir = -dir;
    }

    TVec3f cross(0.0f);
    TVec3f side(1.0f, 0.0f, 0.0f);
    MR::calcSideVec(&side, this);
    PSVECCrossProduct(&side, &dir, &cross);
    if (cross.dot(mUp) < 0.0f) {
        cross = -cross;
    }
    MR::turnVecToVecCosOnPlane(&mUp, cross, side, MR::cosDegree(angle));
    MR::turnVecToVecCosOnPlane(&mFront, dir, mUp, MR::cosDegree(angle));
}
