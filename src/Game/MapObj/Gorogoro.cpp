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

extern "C" void fn_8000E760(TVec3f*, LiveActor*, const TVec3f*, f32, f32);
extern "C" void emitEffect__2MRFP9LiveActorPCc_0(LiveActor*, const char*);
extern "C" f32 fn_8000FFD0(f32);
extern "C" void fn_800345E0(LiveActor*, const char*);

namespace NrvGorogoro {
    NEW_NERVE(GorogoroNrvWait, Gorogoro, Wait);
    NEW_NERVE(GorogoroNrvMove, Gorogoro, Move);
    NEW_NERVE(GorogoroNrvDisappear, Gorogoro, Disappear);
};  // namespace NrvGorogoro

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

static TVec3f sCylinderRockBodyShadowOffset(150.0f, -1000.0f, 0.0f);
static TVec3f sCylinderRockUnderShadowOffset(0.0f, -1500.0f, 0.0f);
extern f32 lbl_807E3090;

Gorogoro::Gorogoro(const char* pName) : LiveActor(pName) {
    mRailMover = nullptr;
    mRotator = nullptr;
    mRailClipPos.x = 0.0f;
    mRailClipPos.y = 0.0f;
    mRailClipPos.z = 0.0f;
    mSandModel = nullptr;
    mSensorFollowPos.x = 0.0f;
    mSensorFollowPos.y = 0.0f;
    mSensorFollowPos.z = 0.0f;
    mRollSpeed = 0.0f;
    mSavedPos.x = 0.0f;
    mSavedPos.y = 0.0f;
    mSavedPos.z = 0.0f;
    mBaseTRMtx.identity();
    mCollisionMtx.identity();
}

void Gorogoro::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::setSensorFollowPos(this, "AttackOnGround", &mSensorFollowPos);

    const char* pObjName = nullptr;
    MR::getObjectName(&pObjName, rIter);
    if (MR::isEqualString(pObjName, "GorogoroPair")) {
        mRollSpeed = 500.0f;
    } else {
        mRollSpeed = 250.0f;
    }

    if (MR::isExistRail(this)) {
        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(rIter);
        mRotator = new MapPartsRotator(this);
        mRotator->init(rIter);

        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&mRailClipPos, this, 100.0f, radius);
    }

    initNerve(GET_NERVE(Gorogoro, GorogoroNrvWait), 0);
    MR::calcGravity(this);

    TVec3f front(0.0f);
    MR::calcFrontVec(&front, this);
    TVec3f up = -mGravity;
    MR::makeMtxUpFrontPos(&mBaseTRMtx, up, front, mPosition);

    TVec3f shadowOffset(0.0f, 0.0f, -10.0f);
    MR::setShadowDropPositionMtxPtr(this, "Body", mBaseTRMtx.mMtx, shadowOffset);

    char nameBuf[0x80];
    snprintf(nameBuf, 0x80, "%sAlpha", pObjName);
    ModelObj* pSand = new ModelObj("\x94\xbc\x93\xa7\x96\xbe\x89\xbb\x83\x49\x83\x75\x83\x57\x83\x46", nameBuf, getBaseMtx(), 0x14, 0x2A, -2, false);
    mSandModel = pSand;
    pSand->initWithoutIter();

    mSavedPos = mPosition;
    JMath::gekko_ps_copy12(&mCollisionMtx, getBaseMtx());
    mCollisionParts->_0 = &mCollisionMtx;
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Gorogoro::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    if (mRotator && mRotator->isWorking()) {
        mtx.concat(mRotator->getRotateMtx());
    }
    mtx.setTrans(mPosition);
    MR::setBaseTRMtx(this, mtx);
    JMath::gekko_ps_copy12(&mCollisionMtx, &mtx);
    mCollisionMtx.setTrans(mSavedPos);
    MR::setMtxTrans(mBaseTRMtx.mMtx, mPosition.x, mPosition.y, mPosition.z);
}

void Gorogoro::control() {
    fn_8000E760(&mSensorFollowPos, this, &mPosition, -mRollSpeed, mRollSpeed);
}

void Gorogoro::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        bool isHit = false;
        if (MR::isEqualSensor(pSender, this, "AttackRight") || MR::isEqualSensor(pSender, this, "AttackLeft") ||
            MR::isEqualSensor(pSender, this, "AttackOnGround")) {
            isHit = true;
        }

        if (isHit) {
            TVec3f side;
            MR::calcSideVec(&side, this);
            if (MR::isEqualSensor(pSender, this, "AttackRight")) {
                TVec3f negSide = -side;
                side = negSide;
            }

            TVec3f toPlayer = *MR::getPlayerPos() - *MR::getSensorPos(pSender);
            f32 dot = toPlayer.dot(side);

            if (MR::isEqualSensor(pSender, this, "AttackOnGround")) {
                if (!(toPlayer.dot(mGravity) > 0.0f) && !(__fabsf(dot) > 10.0f)) {
                    MR::sendMsgEnemyAttackFlip(pReceiver, pSender);
                }
            } else {
                if (dot < 0.0f && dot < 30.0f) {
                    TVec3f reflectBase(*MR::getSensorPos(this, "ReflectSensorBase"));
                    TVec3f scaledSide(side);
                    scaledSide.x *= 10.0f;
                    scaledSide.y *= 10.0f;
                    scaledSide.z *= 10.0f;
                    TVec3f newPos(*MR::getSensorPos(pReceiver));
                    newPos += scaledSide;
                    MR::setSensorPos(this, "ReflectSensorBase", newPos);
                    MR::sendMsgGorogoroReflect(pReceiver, getSensor("ReflectSensorBase"));
                    MR::setSensorPos(this, "ReflectSensorBase", reflectBase);
                }
            }
        }
    }
}

bool Gorogoro::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xDC) {
        kill();
        return true;
    }
    return false;
}

void Gorogoro::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mSandModel, "Wait");
        setNerve(GET_NERVE(Gorogoro, GorogoroNrvMove));
        mRailMover->start();
        mRotator->start();
    }
}

void Gorogoro::exeMove() {
    if (MR::isFirstStep(this)) {
        emitEffect__2MRFP9LiveActorPCc_0(this, "GorogoroSand");
    }

    doRailMove();

    TVec3f camPos = MR::getCamPos();
    TVec3f diff = mPosition - camPos;
    TVec3f camZdir = MR::getCamZdir();
    if (camZdir.dot(diff) < 800.0f) {
        mSandModel->appear();
        MR::invalidateClipping(this);
        MR::invalidateClipping(mSandModel);
        setNerve(GET_NERVE(Gorogoro, GorogoroNrvDisappear));
        MR::tryDeleteEffect(this, "GorogoroSand");
    }
}

void Gorogoro::exeDisappear() {
    doRailMove();

    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(this);
        MR::startAction(mSandModel, "Disappear");
    }

    if (MR::isActionEnd(mSandModel)) {
        mSandModel->kill();
        kill();
    }
}

void Gorogoro::doRailMove() {
    if (mRailMover) {
        mRailMover->movement();
        if (mRailMover->isWorking()) {
            MR::setPosition(this, mSavedPos);
            mSavedPos = mRailMover->_28;
            mRailMover->tryResetPositionRepeat();
            MR::startActionSound(this, "Move", -1, -1, -1);
        }
    }

    if (mRotator) {
        mRotator->movement();
    }
}

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
    mtx = mBaseTRMtx;
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
        MR::moveCoordAndFollowTransXZ(this);
    }

    mRailPointNo = MR::getCurrentRailPointNo(this);
    MR::calcRailDirection(&mRailDirection, this);
    TVec3f offset = mGravity * 10.0f;
    mPosition += offset;
    if (tryLandOnGround()) {
        MR::zeroVelocity(this);
        mRotateSpeed = mRotateCoef;
    }

    mRotation.z = fn_8000FFD0(mRotation.z + mRotateSpeed);
    if (mRailPointNo != oldPointNo) {
        tryUpdateNerve();
    }
    emitEffect__2MRFP9LiveActorPCc_0(this, "Roll");
    MR::startActionSound(this, "Rotate", -1, -1, -1);
}

void GorogoroCylinderRock::updateFallOnRail() {
    TVec3f gravityScaled = mGravity * 1010.0f;
    if (tryLandOnGround()) {
        f32 speed = PSVECMag(&mVelocity);
        MR::zeroVelocity(this);
        MR::moveRailToNextPoint(this);
        tryUpdateNerve();
        if (!(speed >= lbl_807E3090)) {
            fn_800345E0(this, "\x92\x85\x92\x6e");
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

    mRotation.z = fn_8000FFD0(mRotation.z + mRotateSpeed);
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

    mRotation.z = fn_8000FFD0(mRotation.z + mRotateSpeed);
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
