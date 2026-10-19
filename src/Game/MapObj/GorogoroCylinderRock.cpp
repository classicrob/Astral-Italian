#include "Game/MapObj/Gorogoro.hpp"
#include <cstdio>
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Map/CollisionParts.hpp"



namespace NrvGorogoroCylinderRock {
    NEW_NERVE(GorogoroCylinderRockNrvHide, GorogoroCylinderRock, Hide);
    NEW_NERVE(GorogoroCylinderRockNrvAppear, GorogoroCylinderRock, Appear);
    NEW_NERVE(GorogoroCylinderRockNrvFall, GorogoroCylinderRock, Fall);
};  // namespace NrvGorogoroCylinderRock

namespace {
    class GorogoroCylinderRockRollNrv : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GorogoroCylinderRock* actor = reinterpret_cast< GorogoroCylinderRock* >(pSpine->mExecutor);
            actor->updateRoll();
        }
    };
    class GorogoroCylinderRockFallNrv : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GorogoroCylinderRock* actor = reinterpret_cast< GorogoroCylinderRock* >(pSpine->mExecutor);
            actor->updateFallOnRail();
        }
    };
    class GorogoroCylinderRockRotateNrv : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GorogoroCylinderRock* actor = reinterpret_cast< GorogoroCylinderRock* >(pSpine->mExecutor);
            actor->updateRotateOnRail();
        }
    };
    class GorogoroCylinderRockRotateNoGravityNrv : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GorogoroCylinderRock* actor = reinterpret_cast< GorogoroCylinderRock* >(pSpine->mExecutor);
            actor->updateRotateNoGravity();
        }
    };

    GorogoroCylinderRockRollNrv sCylinderRockRollNrv;
    GorogoroCylinderRockFallNrv sCylinderRockFallNrv;
    GorogoroCylinderRockRotateNrv sCylinderRockRotateNrv;
    GorogoroCylinderRockRotateNoGravityNrv sCylinderRockRotateNoGravityNrv;
}  // namespace

// zero-valued .sbss2 constant (0x807E3090)
static const volatile f32 sCylinderRockMinRollSpeed = 0.0f;
static TVec3f sCylinderRockBodyShadowOffset(150.0f, -1000.0f, 0.0f);
static TVec3f sCylinderRockUnderShadowOffset(0.0f, -1500.0f, 0.0f);

GorogoroCylinderRock::GorogoroCylinderRock(const char* pName) : LiveActor(pName) {
    mClipVec.x = 0.0f;
    mClipVec.y = 0.0f;
    mClipVec.z = 0.0f;
    mRailDirection.x = 0.0f;
    mRailDirection.y = 0.0f;
    mRailDirection.z = 0.0f;
    mGroundPos.x = 0.0f;
    mGroundPos.y = 0.0f;
    mGroundPos.z = 0.0f;
    mRotateSpeed = 0.0f;
    mRailPointNo = -1;
    mRotateCoef = 0.0573f;
    mRailSpeed = 10.0f;
    mAppearStep = 0;
    mBaseTRMtx.identity();
    mShadowMtx.identity();
}

void GorogoroCylinderRock::init(const JMapInfoIter& rIter) {
    char objName[0x100];
    MR::getMapPartsObjectNameIfExistShapeID(objName, 0x100, rIter);
    MR::initActor(this, rIter, objName, false);
    MR::makeMtxTR(mBaseTRMtx.mMtx, this);

    TVec3f front(0.0f);
    MR::calcFrontVec(&front, this);
    TVec3f up = -mGravity;
    MR::makeMtxUpFrontPos(&mShadowMtx, up, front, mPosition);

    MR::initMapPartsClipping(this, rIter, &mClipVec, false);
    HitSensor* pSensor = getSensor(nullptr);
    MR::initCollisionParts(this, objName, pSensor, nullptr);
    MR::calcSideVec(&mRailDirection, this);

    MR::getJMapInfoArg0NoInit(rIter, &mRailSpeed);
    mRotateCoef = 0.0573f * mRailSpeed;
    MR::getJMapInfoArg1NoInit(rIter, &mAppearStep);
    MR::useStageSwitchSyncAppear(this, rIter);

    if (mAppearStep > 0) {
        initNerve(GET_NERVE(GorogoroCylinderRock, GorogoroCylinderRockNrvHide), 0);
    } else {
        initNerve(GET_NERVE(GorogoroCylinderRock, GorogoroCylinderRockNrvAppear), 0);
    }

    if (MR::isValidSwitchAppear(this)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }

    MR::setShadowDropPositionMtxPtr(this, "Body", mShadowMtx.mMtx, sCylinderRockBodyShadowOffset);
    MR::setShadowDropPositionMtxPtr(this, "Under", mShadowMtx.mMtx, sCylinderRockUnderShadowOffset);
}

void GorogoroCylinderRock::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    TVec3f negGravity = -mGravity;
    TVec3f cross;
    PSVECCrossProduct(&mRailDirection, &negGravity, &cross);
    MR::isNearZero(cross, 0.001f);
    mBaseTRMtx.setTrans(mPosition);
    mShadowMtx.setTrans(mPosition);
    mtx.setInline(mBaseTRMtx);
    MR::rotateMtxLocalZ(mtx.mMtx, 3.1415927f * mRotation.z / 180.0f);
    MR::setBaseTRMtx(this, mtx);
}

void GorogoroCylinderRock::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }

    if (MR::isGreaterEqualStep(this, mAppearStep)) {
        MR::showModel(this);
        MR::moveCoordAndTransToNearestRailPos(this);
        mRailPointNo = MR::getCurrentRailPointNo(this);
        tryLandOnGround();
        MR::calcRailDirection(&mRailDirection, this);
        tryUpdateNerve();
    }
}

void GorogoroCylinderRock::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        mRailPointNo = MR::getCurrentRailPointNo(this);
        tryLandOnGround();
        MR::calcRailDirection(&mRailDirection, this);
        tryUpdateNerve();
    }
}

void GorogoroCylinderRock::exeFall() {
    TVec3f dir = mGravity * 1010.0f;
    if (MR::isExistMapCollision(mPosition, dir)) {
        MR::zeroVelocity(this);
        setNerve(&sCylinderRockRollNrv);
    } else {
        MR::addVelocityToGravity(this, 0.2f);
    }
    MR::startActionSound(this, "Fall", -1, -1, -1);
}

void GorogoroCylinderRock::updateRoll() {
    s32 oldPointNo = mRailPointNo;
    if (MR::isRailReachedGoal(this)) {
        mRotateSpeed = 0.0f;
        MR::moveCoordAndTransToStartPos(this);
        MR::resetPosition(this);
    } else {
        MR::moveCoordAndFollowTransXZ(this, mRailSpeed);
    }

    mRailPointNo = MR::getCurrentRailPointNo(this);
    MR::calcRailDirection(&mRailDirection, this);
    TVec3f offset = mGravity * 10.0f;
    mPosition += offset;
    if (tryLandOnGround()) {
        MR::zeroVelocity(this);
        mRotateSpeed = mRotateCoef;
    }

    mRotation.z = MR::normalizeAngleDegree(mRotation.z + mRotateSpeed);
    if (mRailPointNo != oldPointNo) {
        tryUpdateNerve();
    }
    MR::tryEmitEffect(this, "Roll");
    MR::startActionSound(this, "Rotate", -1, -1, -1);
}

void GorogoroCylinderRock::updateFallOnRail() {
    TVec3f gravityScaled = mGravity * 1010.0f;
    if (tryLandOnGround()) {
        f32 speed = PSVECMag(&mVelocity);
        MR::zeroVelocity(this);
        MR::moveRailToNextPoint(this);
        tryUpdateNerve();
        if (!(speed >= sCylinderRockMinRollSpeed)) {
            // Shift-JIS "着地" (landing) hit reaction
            MR::tryHitReaction(this, "\x92\x85\x92\x6e");
        }
    } else {
        TVec3f offset = mGravity * 12.0f;
        mPosition += offset;
    }
    MR::startActionSound(this, "Fall", -1, -1, -1);
}

void GorogoroCylinderRock::updateRotateOnRail() {
    s32 oldPointNo = mRailPointNo;
    if (MR::isFirstStep(this)) {
        MR::calcRailDirection(&mRailDirection, this);
    }

    MR::moveCoord(this, mRailSpeed);
    mRailPointNo = MR::getCurrentRailPointNo(this);

    TVec3f offset = mRailDirection * mRailSpeed;
    MR::vecKillElement(offset, mGravity, &offset);
    mPosition += offset;
    MR::addVelocityToGravity(this, 0.2f);

    if (mRailPointNo != oldPointNo) {
        if (tryUpdateNerve()) {
            MR::zeroVelocity(this);
        }
    }

    mRotation.z = MR::normalizeAngleDegree(mRotation.z + mRotateSpeed);
}

void GorogoroCylinderRock::updateRotateNoGravity() {
    s32 oldPointNo = mRailPointNo;
    if (MR::isFirstStep(this)) {
        MR::calcRailDirection(&mRailDirection, this);
    }

    MR::moveCoord(this, mRailSpeed);
    mRailPointNo = MR::getCurrentRailPointNo(this);

    TVec3f offset = mRailDirection * mRailSpeed;
    mPosition += offset;
    if (mRailPointNo != oldPointNo) {
        tryUpdateNerve();
    }

    mRotation.z = MR::normalizeAngleDegree(mRotation.z + mRotateSpeed);
    MR::startActionSound(this, "RotateNoGlavity", -1, -1, -1);
}

bool GorogoroCylinderRock::tryLandOnGround() {
    TVec3f gravityScaled = mGravity * 1010.0f;
    TVec3f gravityOffset = mGravity * 5.0f;
    TVec3f start = mPosition - gravityOffset;
    TVec3f hitPos(0.0f);
    mGroundPos = hitPos;

    Triangle tri;
    if (MR::getFirstPolyOnLineToMapExceptActor(&hitPos, &tri, start, gravityScaled, this)) {
        TVec3f newPos = hitPos - gravityScaled;
        mPosition = newPos;
        mGroundPos = hitPos;
        return true;
    }
    return false;
}

bool GorogoroCylinderRock::tryUpdateNerve() {
    s32 railArg0 = -1;
    MR::getCurrentRailPointArg0NoInit(this, &railArg0);
    bool railArg1 = false;
    MR::getCurrentRailPointArg1NoInit(this, &railArg1);
    if (railArg1) {
        MR::invalidateShadow(this, "Body");
    } else {
        MR::validateShadow(this, "Body");
    }

    const Nerve* pOldNerve = mSpine->getCurrentNerve();
    if (railArg0 == 0) {
        setNerve(&sCylinderRockFallNrv);
    } else if (railArg0 == 1) {
        setNerve(&sCylinderRockRotateNrv);
    } else if (railArg0 == 2) {
        setNerve(&sCylinderRockRotateNoGravityNrv);
    } else {
        setNerve(&sCylinderRockRollNrv);
    }
    return !isNerve(pOldNerve);
}
