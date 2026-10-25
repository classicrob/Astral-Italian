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


namespace NrvGorogoro {
    NEW_NERVE(GorogoroNrvWait, Gorogoro, Wait);
    NEW_NERVE(GorogoroNrvMove, Gorogoro, Move);
    NEW_NERVE(GorogoroNrvDisappear, Gorogoro, Disappear);
};  // namespace NrvGorogoro

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
    TVec3f shadowOffset;
    MR::calcFrontVec(&front, this);
    TVec3f up = -mGravity;
    MR::makeMtxUpFrontPos(&mBaseTRMtx, up, front, mPosition);

    shadowOffset.x = 0.0f;
    shadowOffset.y = 0.0f;
    shadowOffset.z = -10.0f;
    MR::setShadowDropPositionMtxPtr(this, "Body", mBaseTRMtx.mMtx, shadowOffset);

    char nameBuf[0x80];
    snprintf(nameBuf, 0x80, "%sAlpha", pObjName);
    // Shift-JIS "半透明化オブジェ" (translucent object) model name
    ModelObj* pSand = new ModelObj("\x94\xbc\x93\xa7\x96\xbe\x89\xbb\x83\x49\x83\x75\x83\x57\x83\x46", nameBuf, getBaseMtx(), 0x14, 0x2A, -2, false);
    mSandModel = pSand;
    pSand->initWithoutIter();

    mSavedPos = mPosition;
    JMath::gekko_ps_copy12(&mCollisionMtx, getBaseMtx());
    mCollisionParts->mFollowMtx = &mCollisionMtx;
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
    MR::calcNearestPointToPlayerOnSideLine(&mSensorFollowPos, this, &mPosition, -mRollSpeed, mRollSpeed);
}

// NONMATCHING: target inlines TVec3f::add on the reflect position as paired-single ops; ours calls the out-of-line add
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
                side = -side;
            }

            TVec3f toPlayer = *MR::getPlayerPos() - *MR::getSensorPos(pSender);
            f32 dot = toPlayer.dot(side);

            if (MR::isEqualSensor(pSender, this, "AttackOnGround")) {
                if (!(toPlayer.dot(mGravity) > 0.0f) && !(__fabsf(dot) > 10.0f)) {
                    MR::sendMsgEnemyAttackFlip(pReceiver, pSender);
                }
            } else {
                if (dot >= 0.0f && dot < 30.0f) {
                    TVec3f reflectBase(*MR::getSensorPos(this, "ReflectSensorBase"));
                    MR::setSensorPos(this, "ReflectSensorBase", *MR::getSensorPos(pReceiver) + side.multInLine(10.0f));
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

namespace {
    inline TVec3f calcPositionFromCamera(const TVec3f& position) {
        TVec3f cameraPosition = MR::getCamPos();
        return position - cameraPosition;
    }
}

void Gorogoro::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::tryEmitEffect(this, "GorogoroSand");
    }

    doRailMove();

    TVec3f diff = calcPositionFromCamera(mPosition);
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
            mSavedPos = mRailMover->mPosition;
            mRailMover->tryResetPositionRepeat();
            MR::startActionSound(this, "Move", -1, -1, -1);
        }
    }

    if (mRotator) {
        mRotator->movement();
    }
}

