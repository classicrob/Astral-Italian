#include "Game/MapObj/HipDropSwitch.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SpringValue.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

namespace {
    static const s32 sOnSomethingAcc = 0;
};  // namespace

namespace NrvHipDropSwitch {
    NEW_NERVE(HipDropSwitchNrvOff, HipDropSwitch, Off);
    NEW_NERVE(HipDropSwitchNrvSwitchDown, HipDropSwitch, SwitchDown);
    NEW_NERVE(HipDropSwitchNrvOn, HipDropSwitch, On);
};  // namespace NrvHipDropSwitch

HipDropSwitch::HipDropSwitch(const char* pName) : LiveActor(pName), _90(), mConnector(), mSpringModel(), mIsHipDropped(), mWasTouched(), mIsTouched() {
    mConnector = new MapObjConnector(this);
    mSpringValue = new SpringValue();
    mSpringValue->setParam(0.0f, 0.0f, 0.2f, 0.90f, 0.0f);
    _A0.identity();
}

void SpringValue::reset() {
    mSpringValue = mRestValue;
    mVelocity = 0.0f;
}

void HipDropSwitch::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    CollisionParts* pParts = mSpringModel;
    pParts->_CC = true;
    pParts->_CD = true;
}

void HipDropSwitch::makeActorDead() {
    LiveActor::makeActorDead();
    mSpringModel->_CC = false;
}

void HipDropSwitch::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "HipDropSwitch", false);
    MR::connectToSceneMapObjDecorationStrongLight(this);
    MR::initLightCtrl(this);
    initSound(4, "HipDropSwitch", nullptr, TVec3f(0.0f));
    initHitSensor(2);
    MR::addHitSensorMapObj(this, "Body", 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorMapObj(this, "hit", 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initCollisionParts(this, "HipDropSwitch", getSensor("Body"), nullptr);
    mSpringModel = MR::createCollisionPartsFromLiveActor(this, "Move", getSensor("hit"), MR::CollisionScaleType_Unk2);
    MR::validateCollisionParts(mSpringModel);
    initNerve(GET_NERVE(HipDropSwitch, HipDropSwitchNrvOff));
    MR::needStageSwitchWriteA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void HipDropSwitch::initAfterPlacement() {
    mConnector->attachToUnder();
}

void HipDropSwitch::control() {
    mIsHipDropped = false;
    mWasTouched = mIsTouched;
    mIsTouched = false;
}

void HipDropSwitch::calcAnim() {
    LiveActor::calcAnim();
    TPos3f mtx;
    MtxPtr jointMtx = MR::getJointMtx(this, "Move");
    mtx.set(jointMtx);
    f32 springValue = mSpringValue->mSpringValue;

    if (!MR::isNearZero(springValue)) {
        TVec3f trans, upVec;
        mtx.getTrans(trans);
        MR::calcUpVec(&upVec, this);
        trans += upVec * springValue;
        mtx.setTrans(trans);
        PSMTXCopy(mtx, jointMtx);
    }

    mSpringModel->setMtx(mtx);
}

void HipDropSwitch::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mConnector->connect();
}

bool HipDropSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceAttack(msg)) {
        mIsTouched = true;
        return true;
    } else if (MR::isMsgPlayerHipDropFloor(msg) && mSpringModel->mHitSensor == pReceiver && isPlayerOnTop() && !mIsHipDropped) {
        mIsHipDropped = true;
        return true;
    }

    return false;
}

bool HipDropSwitch::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg) && mSpringModel->mHitSensor == pReceiver && isPlayerOnTop()) {
        mIsTouched = true;
        return mWasTouched == false;
    }

    return false;
}

bool HipDropSwitch::trySwitchDown() {
    if (mIsHipDropped) {
        setNerve(GET_NERVE(HipDropSwitch, HipDropSwitchNrvSwitchDown));
        return true;
    }

    return false;
}

bool HipDropSwitch::tryOn() {
    if (MR::isGreaterStep(this, ::sOnSomethingAcc) && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(HipDropSwitch, HipDropSwitchNrvOn));
        return true;
    }

    return false;
}

bool HipDropSwitch::isPlayerOnTop() {
    if (!MR::isOnPlayer(getSensor("hit"))) {
        return false;
    }

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    return upVec.dot(*MR::getPlayerGroundNormal()) > 0.9f;
}

void HipDropSwitch::exeOff() {
    if (MR::isFirstStep(this)) {
        mIsHipDropped = false;
        MR::validateClipping(this);
        MR::offSwitchA(this);
        MR::startAction(this, "Wait");
        MR::startBrk(this, "Off");
        mSpringValue->reset();
    }

    if (!mWasTouched && mIsTouched) {
        mSpringValue->mVelocity += -10.0f;
        MR::startActionSound(this, "OjPncKinokoBound");
    }

    mSpringValue->update();

    if (trySwitchDown()) {
        mSpringValue->reset();
    }
}

void HipDropSwitch::exeSwitchDown() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }

    if (MR::isStep(this, ::sOnSomethingAcc)) {
        MR::startAction(this, "On");
        MR::startBrk(this, "On");
    }

    if (tryOn()) {
        return;
    }
}

void HipDropSwitch::exeOn() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::onSwitchA(this);
        MR::startActionSound(this, "OjHipdropSwitchOn");
        MR::shakeCameraNormal();
    }
}
