#include "Game/MapObj/BlackHole.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBlackHole {
    NEW_NERVE(BlackHoleNrvWait, BlackHole, Wait);
    NEW_NERVE(BlackHoleNrvDemo, BlackHole, Demo);
    NEW_NERVE(BlackHoleNrvDisappear, BlackHole, Disappear);
};  // namespace NrvBlackHole

BlackHole::BlackHole(const char* pName) : LiveActor(pName), mBlackHoleModel(), _94(gZeroVec), mModelScale(1.0f), mSensorRadius(500.0f), mCubeBounds(), mCameraInfo() {
    mCubeMtx.identity();
    mEffectMtx.identity();
}

void BlackHole::init(const JMapInfoIter& rIter) {
    initMapToolInfo(rIter);
    initModel();
    MR::connectToSceneMapObj(this);
    initHitSensor(1);
    MR::addHitSensorEye(this, "Body", 16, mSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));
    initEffectKeeper(0, nullptr, false);
    MR::setEffectHostMtx(this, "BlackHoleSuction", mEffectMtx.toMtxPtr());
    f32 radius = mSensorRadius;
    f32 val = 500.0f * mModelScale;

    if (radius >= val) {
        radius = radius;
    } else {
        radius = val;
    }

    f32 clippingRadius = 500.0f + radius;
    MR::setClippingTypeSphere(this, clippingRadius);
    MR::setClippingTypeSphere(mBlackHoleModel, clippingRadius);
    MR::setClippingFarMax(this);
    MR::setClippingFarMax(mBlackHoleModel);
    TVec3f soundOffset;
    soundOffset.setAll<f32>(0.0f);
    initSound(4, "BlackHole", nullptr, soundOffset);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionNerve(this, GET_NERVE(BlackHole, BlackHoleNrvDisappear), 0);
    }

    bool isCreated = MR::createActorCameraInfoIfExist(rIter, &mCameraInfo);

    if (isCreated) {
        MR::initActorCamera(this, rIter, &mCameraInfo);
    }

    initNerve(GET_NERVE(BlackHole, BlackHoleNrvWait));
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
}

void BlackHole::makeActorAppeared() {
    bool isOnSwitch = MR::isValidSwitchA(this) && MR::isOnSwitchA(this);

    if (!isOnSwitch) {
        LiveActor::makeActorAppeared();
        mBlackHoleModel->makeActorAppeared();
    }
}

void BlackHole::makeActorDead() {
    LiveActor::makeActorDead();
    if (!MR::isDead(mBlackHoleModel)) {
        mBlackHoleModel->makeActorDead();
    }
}

void BlackHole::kill() {
    mBlackHoleModel->kill();
    LiveActor::kill();
}

bool BlackHole::tryStartDemoCamera() {
    if (mCameraInfo != nullptr) {
        MR::startActorCameraTargetSelf(this, mCameraInfo);
        return true;
    }

    return false;
}

void BlackHole::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TVec3f dirToCam;
    dirToCam.sub(MR::getCamPos(), mPosition);
    TVec3f camYDir;
    camYDir.set(MR::getCamYdir());

    if (MR::normalizeOrZero(&dirToCam)) {
        return;
    }

    if (MR::isSameDirection(dirToCam, camYDir)) {
        return;
    }

    MR::makeMtxFrontUpPos(&mEffectMtx, dirToCam, camYDir, mPosition);

    mEffectMtx.scaleXYZ(mScale.x);
}

void BlackHole::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(BlackHole, BlackHoleNrvWait))) {
        return;
    }

    if (mCubeBounds != nullptr && !isInCubeBox(*MR::getSensorPos(pReceiver))) {
        return;
    }

    if (!MR::sendArbitraryMsg(ACTMES_INHALE_BLACK_HOLE, pReceiver, pSender)) {
        return;
    }

    if (!MR::isSensorPlayer(pReceiver)) {
        return;
    }

    setNerve(GET_NERVE(BlackHole, BlackHoleNrvDemo));
}

void BlackHole::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);

    if (MR::isEqualObjectName(rIter, "BlackHoleCube")) {
        initCubeBox();
    }

    if (mCubeBounds == nullptr) {
        mSensorRadius = 500.0f * mScale.x;
    } else {
        TVec3f size(mScale);
        size.scale(500.0f);

        mSensorRadius = size.length();
    }

    f32 arg0;

    if (MR::getJMapInfoArg0NoInit(rIter, &arg0)) {
        mModelScale = arg0 / 1000.0f;
    } else if (mCubeBounds == nullptr) {
        mModelScale = mScale.x;
    } else {
        mModelScale = 1.0f;
    }
}

void BlackHole::initModel() {
    MR::initActor(this, "BlackHoleRange", false);
    mBlackHoleModel = MR::createModelObjMapObj("コアモデル", "BlackHole", getBaseMtx());
    mBlackHoleModel->makeActorDead();
    updateModelScale(mModelScale, mModelScale);
}

void BlackHole::initCubeBox() {
    MR::makeMtxRotate(mCubeMtx, mRotation.x, mRotation.y, mRotation.z);
    mCubeMtx.setTrans(mPosition);

    mCubeBounds = new TBox3f();
    TVec3f vecStart(0.5f * (1000.0f * -mScale.x), 0.5f * (1000.0f * -mScale.y), 0.5f * (1000.0f * -mScale.z));
    TVec3f vecEnd(0.5f * (1000.0f * mScale.x), 0.5f * (1000.0f * mScale.y), 0.5f * (1000.0f * mScale.z));
    mCubeBounds->set(vecStart, vecEnd);
}

bool BlackHole::isInCubeBox(const TVec3f& rVec) const {
    TVec3f pos;
    mCubeMtx.multTranspose(rVec, pos);
    return mCubeBounds->intersectsPoint(pos);
}

void BlackHole::updateModelScale(f32 a1, f32 a2) {
    mScale.setAll< f32 >(a1);
    mBlackHoleModel->mScale.setAll< f32 >(0.5f * a2);
}

void BlackHole::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlackHoleRange");
        MR::startBtk(this, "BlackHoleRange");
        MR::startBtk(mBlackHoleModel, "BlackHole");
    }

    MR::startActionSound(this, "OjLvBlackHole");
    bool isOnSwitch = false;

    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        isOnSwitch = true;
    }

    if (isOnSwitch) {
        kill();
    }
}

void BlackHole::exeDemo() {
    MR::startActionSound(this, "OjLvBlackHole");
}

void BlackHole::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjBlackHoleDisappear");
    }

    f32 nerveEaseIn = MR::calcNerveEaseInValue(this, 90, mModelScale, 0.0f);
    f32 blackHoleEase = MR::calcNerveEaseInValue(this, 30, 90, mModelScale, 0.0f);
    mScale.setAll< f32 >(nerveEaseIn);
    mBlackHoleModel->mScale.setAll< f32 >(0.5f * blackHoleEase);

    if (MR::isStep(this, 90)) {
        kill();
    }
}

BlackHole::~BlackHole() {
}
