#pragma auto_inline off

#include "Game/MapObj/SuperSpinDriver.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/SpinDriverCamera.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/MapObj/SpinDriverShootPath.hpp"
#include "Game/MapObj/SpinDriverUtil.hpp"
#include <cstring>
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"


void SuperSpinDriver_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)MR::epsilon();
    (void)0.0f;
    (void)0.5f;
    (void)2.0f;
    (void)-1.0f;
    (void)MR::pi();
}

namespace {
    static s32 cSpaceFlyStartFrame = 10;
    static s32 cSpaceFlyLoopFrame = 100;
    static s32 cSpaceFlyEndFrame = 50;
};  // namespace

inline void SuperSpinDriver::endShoot() {
    MR::invalidateHitSensor(this, "Body");
}

namespace NrvSuperSpinDriver {
    NEW_NERVE(SuperSpinDriverNrvTryDemo, SuperSpinDriver, TryDemo);
    NEW_NERVE(SuperSpinDriverNrvEmptyNonActive, SuperSpinDriver, EmptyNonActive);
    NEW_NERVE(SuperSpinDriverNrvEmptyAppear, SuperSpinDriver, EmptyAppear);
    NEW_NERVE(SuperSpinDriverNrvEmptyWait, SuperSpinDriver, EmptyWait);
    NEW_NERVE(SuperSpinDriverNrvNonActive, SuperSpinDriver, NonActive);
    NEW_NERVE(SuperSpinDriverNrvAppear, SuperSpinDriver, Appear);
    NEW_NERVE(SuperSpinDriverNrvWait, SuperSpinDriver, Wait);
    NEW_NERVE(SuperSpinDriverNrvCapture, SuperSpinDriver, Capture);
    NEW_NERVE(SuperSpinDriverNrvShootStart, SuperSpinDriver, ShootStart);
    NEW_NERVE_ONEND(SuperSpinDriverNrvShoot, SuperSpinDriver, Shoot, Shoot);
    NEW_NERVE(SuperSpinDriverNrvCoolDown, SuperSpinDriver, CoolDown);
};  // namespace NrvSuperSpinDriver

// Fix JGeometry inlining
void SuperSpinDriver_DUMMY() {
    TQuat4f e;
    e = e;
}

SuperSpinDriver::SuperSpinDriver(const char* pName, s32 color)
    : LiveActor(pName), mBindActor(), mShootPath(), mSpinDriverCamera(), mPathDrawer(), mEmptyModel(), mTalkCtrl(), _A4(0, 0, 0, 1),
      _B4(0, 0, 0, 1), _C4(0, 0, 0), _D0(0, 0, 0), mShootPathDirection(0, 1, 0), _E8(0, 0, 1), _F4(1, 0, 0), _100(0, 1, 0),
      mShootPathPosition(0, 0, 0), _118(0, 0, 0), _124(0, 0, 0), _134(), _138(), _13C(), mFrontAngle(), _144(), _148(), mShadowLength(-1.0f),
      mFlightTime(300), _154(50), _158(230), _15C(280), mDrawPathRangeIdx(-1), mPlayerLandRotation(), _168(), mAlreadyDoneFlagIdx(-1), mColor(color), _SMG2_1A8(0),
      _174(true), _178(), _17C(), mIsPullPlayer(true), mIsDisableJingle(), mIsNoPauseMenuStars() {
    _SMG2A8.identity();
}

void SuperSpinDriver::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "SuperSpinDriver", false);
    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");

    initParamFromJMapInfo(rIter);
    initGravityAxis();
    initSensor();
    initEffectKeeper(0, nullptr, false);
    initSound(6, "SuperSpinDriver", nullptr, TVec3f(0.0f));
    initEventCamera(rIter);
    initNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait), 0);
    initShootPath(rIter);
    initShootPathDraw(rIter);

    if (MR::isStageNoPauseMenuStars()) {
        mIsNoPauseMenuStars = true;
    }

    mTalkCtrl = MR::tryCreateTalkCtrlDirectOnRootNodeAutomatic(this, rIter, "SuperSpinDriver", TVec3f(0.0f, 150.0f, 0.0f), nullptr);

    if (mTalkCtrl != nullptr) {
        MR::setDistanceToTalk(mTalkCtrl, 700.0f);
    }

    MR::setClippingFar200m(this);
    MR::makeMtxTR(_SMG2A8, this);
    MR::addBaseMatrixFollowTarget(this, rIter, &_SMG2A8, MR::createBaseMatrixFollowValidateDelegator(this, &SuperSpinDriver::isFollowValid));
    initColor();
    initEmptyModel();

    if (MR::useStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::Functor(this, &SuperSpinDriver::requestActive));
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvNonActive));
    }

    if (MR::useStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnOffB(this, MR::Functor(this, &SuperSpinDriver::requestShow), MR::Functor(this, &SuperSpinDriver::requestHide));
    }

    MR::useStageSwitchSleep(this, rIter);
    initAppearState(rIter);
}

void SuperSpinDriver::initAfterPlacement() {
    SpinDriverUtil::setShadowAndClipping(this, &_124, mShadowLength, 300.0f, &_130);

    if (mEmptyModel != nullptr) {
        SpinDriverUtil::setShadowAndClipping(mEmptyModel, &_124, mShadowLength, 300.0f, &_130);
    }
}

void SuperSpinDriver::initParamFromJMapInfo(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mFlightTime);

    mShadowLength = -1.0f;
    MR::getJMapInfoArg1NoInit(rIter, &mShadowLength);

    s32 isPullPlayer = 0;

    if (MR::getJMapInfoArg2NoInit(rIter, &isPullPlayer)) {
        mIsPullPlayer = isPullPlayer != 0;
    }

    s32 isDisableJingle = 0;

    if (MR::getJMapInfoArg4NoInit(rIter, &isDisableJingle)) {
        mIsDisableJingle = isDisableJingle == 1;
    }
}

void SuperSpinDriver::initGravityAxis() {
    MR::offCalcGravity(this);

    TVec3f gravity;
    MR::calcGravityVectorOrZero(this, &gravity, nullptr, 0);

    if (MR::isNearZero(gravity)) {
        gravity.set< f32 >(0.0f, -1.0f, 0.0f);
    }

    mGravity.set(gravity);
}

void SuperSpinDriver::initSensor() {
    initHitSensor(2);
    MR::addHitSensor(this, "bind", 0x5F /* ATYPE_SPIN_CANNON_BIND */, 16, 300.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorPosRide(this, "Body", 16, 300.0f, &_C4, TVec3f(0.0f, 0.0f, 0.0f));
    MR::invalidateHitSensor(this, "Body");
}

void SuperSpinDriver::initEmptyModel() {
    if (!isNeedEmptyModel()) {
        return;
    }

    mEmptyModel = MR::createModelObjMapObjStrongLight("無効モデル", "SuperSpinDriverEmpty", getBaseMtx());
    MR::startBtp(mEmptyModel, "SuperSpinDriverEmpty");
    MR::setBtpFrameAndStop(mEmptyModel, mColor);
    MR::initShadowVolumeFlatModel(mEmptyModel, "SuperSpinDriverShadow", MR::getJointMtx(this, "Outside"));
    mEmptyModel->mGravity.set(mGravity);
    onUse();
}

void SuperSpinDriver::initEventCamera(const JMapInfoIter& rIter) {
    mSpinDriverCamera = new SpinDriverCamera();
    mSpinDriverCamera->init(rIter, this);
}

void SuperSpinDriver::initShootPath(const JMapInfoIter& rIter) {
    mPlayerLandRotation = 0.0f;
    MR::getJMapInfoArg5NoInit(rIter, &mPlayerLandRotation);

    mShootPath = new SpinDriverShootPath();
    mShootPath->initUsingParabolic(rIter, mPosition);
    mShootPath->calcInitPose(&_100, &_E8, &_F4, mPlayerLandRotation);
}

void SuperSpinDriver::initShootPathDraw(const JMapInfoIter& rIter) {
    s32 drawPathRangeIdx = -1;
    f32 coord = 0.0f;

    MR::getJMapInfoArg7NoInit(rIter, &drawPathRangeIdx);
    mDrawPathRangeIdx = MR::registerStorageSpinDriverPathDrawRange(this, rIter, drawPathRangeIdx, &coord);

    mPathDrawer = new SpinDriverPathDrawer(mShootPath);
    mPathDrawer->setCoord(1.0f);
    mPathDrawer->initWithoutIter();
}

void SuperSpinDriver::initColor() {
    switch (mColor) {
    case 0:
        MR::startBtp(this, "SuperSpinDriver");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::startBrk(this, "Yellow");
        mPathDrawer->setColorNormal();
        break;
    case 2:
        MR::startBtp(this, "SuperSpinDriver");
        MR::setBtpFrameAndStop(this, 2.0f);
        MR::startBrk(this, "Pink");
        mPathDrawer->setColorPink();
        break;
    }
}

bool SuperSpinDriver::isFollowValid(s32) const {
    return isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShootStart)) || isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShoot));
}

void SuperSpinDriver::makeActorDead() {
    LiveActor::makeActorDead();

    if (mPathDrawer != nullptr) {
        mPathDrawer->makeActorDead();
    }
}

void SuperSpinDriver::initAppearState(const JMapInfoIter& rIter) {
    bool isAppear = true;

    if (MR::useStageSwitchSyncAppear(this, rIter)) {
        mSpinDriverCamera->initAppearCamera(rIter, this);
        isAppear = false;
    }

    if (MR::tryRegisterDemoCast(this, rIter) && MR::isRegisteredDemoActionAppear(this)) {
        isAppear = false;
    }

    if (MR::joinToGroupArray(this, rIter, "惑星出現", 32)) {
        mSpinDriverCamera->initAppearCamera(rIter, this);
        isAppear = false;
    }

    if (isAppear) {
        makeActorAppeared();
        return;
    }

    s32 respawnType = 0;
    MR::getJMapInfoArg6NoInit(rIter, &respawnType);

    if (respawnType == 1) {
        _168 = MR::setupAlreadyDoneFlag(&mAlreadyDoneFlagIdx, "ドライバ出現済み", rIter, 1);
    } else {
        _168 = false;
        mAlreadyDoneFlagIdx = -1;
    }

    if (_168 != 0) {
        makeActorAppeared();
    } else {
        makeActorDead();
    }
}

void SuperSpinDriver::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    if (mPathDrawer != nullptr && mPathDrawer->_B0 > 0.0f) {
        startPathDraw();
    }
}

void SuperSpinDriver::appear() {
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        return;
    }

    if (!MR::isDead(this)) {
        return;
    }

    LiveActor::appear();
    onUse();
    requestAppear();

    if (mAlreadyDoneFlagIdx >= 0) {
        MR::updateAlreadyDoneFlag(mAlreadyDoneFlagIdx);
    }
}

void SuperSpinDriver::control() {
    if (!_174 && isFarPlayer()) {
        _174 = true;
    }

    if (_SMG2_1A8 > 0) {
        _SMG2_1A8--;
    }

    if (_178 > 0) {
        _178--;
    }

    _17C = _178 > 0;

    mFrontAngle += _144;
    mFrontAngle = MR::repeat(mFrontAngle, -PI, TWO_PI);
    _144 *= 0.985f;
}

void SuperSpinDriver::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxUpFrontPos(&mtx, _100, _E8, mPosition);

    TPos3f mtxRotate;
    mtxRotate.identity();
    mtxRotate.setEulerY(mFrontAngle);

    mtx.concat(mtx, mtxRotate);
    MR::setBaseTRMtx(this, mtx);
}

void SuperSpinDriver::startClipped() {
    LiveActor::startClipped();
    MR::deleteEffectAll(this);
}

void SuperSpinDriver::endClipped() {
    LiveActor::endClipped();
    endPathDraw();
}

void SuperSpinDriver::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mBindActor != nullptr && !MR::isSensorAutoRush(pSender) && MR::tryGetItem(pSender, pReceiver)) {
        return;
    }
}

bool SuperSpinDriver::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgIsRushTakeOver(msg)) {
        _178 = 60;
        return canBind(pSender, pReceiver);
    }

    if (MR::isMsgAutoRushBegin(msg)) {
        _178 = 60;

        if (tryBind(pSender, pReceiver)) {
            return true;
        }
    }

    if (MR::isMsgRushCancel(msg)) {
        if (isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShootStart)) ||
            isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShoot)) && MR::isLessStep(this, 40)) {
            return false;
        }

        if (mBindActor != nullptr) {
            mBindActor = nullptr;
            mSpinDriverCamera->cancel();
            return true;
        }
    }

    if (MR::isMsgRushForceCancel(msg) && mBindActor != nullptr) {
        mBindActor = nullptr;
        mSpinDriverCamera->cancel();
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg) && mBindActor != nullptr) {
        updateBindActorMatrix();
        return true;
    }

    return false;
}

bool SuperSpinDriver::canBind(HitSensor* pSender, HitSensor* pReceiver) const {
    if (!isNerveEnableBind()) {
        return false;
    }

    if (isSwingOr2PTrigger()) {
        return true;
    }

    if (mIsPullPlayer && _174 && MR::isNear(pSender, pReceiver, 240.0f)) {
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryBind(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerveEnableBind()) {
        return false;
    }

    if (isSwingOr2PTrigger()) {
        mBindActor = MR::getSensorHost(pSender);
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShootStart));
    } else if (mIsPullPlayer && _174 && MR::isNear(pSender, pReceiver, 240.0f)) {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvCapture));
    } else {
        return false;
    }

    _C4 = (mBindActor = MR::getSensorHost(pSender))->mPosition;
    _D0 = *MR::getPlayerLastMove();

    f32 length = _D0.length();

    if (length > 40.0f) {
        _D0 *= 40.0f / length;
    }

    _134 = 0.0f;
    _138 = 0.0f;
    _148 = 0.0f;
    TPos3f bindActorBaseMtx(mBindActor->getBaseMtx());
    bindActorBaseMtx.getQuat(_A4);
    _B4 = _A4;

    ::cSpaceFlyStartFrame = MR::getBckFrameMax(mBindActor, "SpaceFlyStart");
    ::cSpaceFlyLoopFrame = MR::getBckFrameMax(mBindActor, "SpaceFlyLoop");
    ::cSpaceFlyEndFrame = MR::getBckFrameMax(mBindActor, "SpaceFlyEnd");

    MR::invalidateClipping(this);

    mSpinDriverCamera->start(_100, _E8, mPosition);
    return true;
}

bool SuperSpinDriver::tryEndCapture() {
    if (MR::isGreaterStep(this, 60) && _C4.distance(mPosition) < 15.0f) {
        cancelBind();
        _174 = false;
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait));
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryForceCancel() {
    if (mBindActor == nullptr) {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvCoolDown));
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryShootStart() {
    bool isSwingOrPointed = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOrPointed) {
        MR::deleteEffect(this, "SuperSpinDriverLight");
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShootStart));
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryShoot() {
    if (MR::isGreaterStep(this, 45)) {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvShoot));
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryEndShoot() {
    if (MR::isGreaterEqualStep(this, mFlightTime)) {
        endBind();
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvCoolDown));
        return true;
    }

    return false;
}

bool SuperSpinDriver::tryEndCoolDown() {
    if (MR::isGreaterStep(this, 60) && _178 == 0) {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait));
        return true;
    }

    return false;
}

bool SuperSpinDriver::trySwitchOff() {
    if (MR::isValidSwitchAppear(this) && !MR::isOnSwitchAppear(this)) {
        kill();
        return true;
    }

    return false;
}

void SuperSpinDriver::requestAppear() {
    MR::invalidateClipping(this);

    if (mSpinDriverCamera->isUseAppearCamera(this)) {
        MR::requestStartDemo(this, "出現", GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvAppear),
                             GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvTryDemo));
    } else {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvAppear));
    }
}

void SuperSpinDriver::requestEmptyAppear() {
    MR::invalidateClipping(this);

    if (mSpinDriverCamera->isUseAppearCamera(this)) {
        MR::requestStartDemo(this, "出現", GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvEmptyAppear),
                             GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvTryDemo));
    } else {
        setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvEmptyAppear));
    }
}

void SuperSpinDriver::requestActive() {
    if (isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvNonActive))) {
        requestAppear();
    } else if (isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvEmptyNonActive))) {
        requestEmptyAppear();
    }
}

void SuperSpinDriver::requestHide() {
    if (!MR::isDead(this)) {
        if (mBindActor != nullptr) {
            endBind();
        }

        makeActorDead();
    }
}

void SuperSpinDriver::requestShow() {
    if (MR::isDead(this)) {
        makeActorAppeared();
    }
}

void SuperSpinDriver::exeTryDemo() {
}

void SuperSpinDriver::exeEmptyNonActive() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }

    onUse();
    setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvNonActive));
}

void SuperSpinDriver::exeEmptyAppear() {
    if (MR::isFirstStep(this)) {
        mSpinDriverCamera->startAppearCamera(this, _100, _E8, mPosition);

        if (!mIsDisableJingle) {
            MR::startActionSound(this, "SySpinDriverAppear");
            MR::startActionSound(this, "OjSSpinDrvAppear");
        }
    }

    if (MR::isActionEnd(this)) {
        s32 frames = mSpinDriverCamera->getAppearCameraFrames();

        if (MR::isGreaterStep(this, frames)) {
            setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvEmptyWait));

            if (mSpinDriverCamera->isUseAppearCamera(this)) {
                mSpinDriverCamera->endAppearCamera(this);
                MR::endDemoWaitCameraInterpolating(this, "出現");
            }
        }
    }
}

void SuperSpinDriver::exeEmptyWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }

    onUse();
    setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait));
}

void SuperSpinDriver::exeNonActive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "NonActive");
        MR::validateClipping(this);
    }

    if (mTalkCtrl != nullptr) {
        MR::tryTalkNearPlayer(mTalkCtrl);
    }

    addSwingSignRotateY();
}

void SuperSpinDriver::exeAppear() {
    if (MR::isFirstStep(this)) {
        mSpinDriverCamera->startAppearCamera(this, _100, _E8, mPosition);

        if (!mIsDisableJingle) {
            MR::startActionSound(this, "SySpinDriverAppear");
            MR::startActionSound(this, "OjSSpinDrvAppear");
        }

        MR::startAction(this, "Appear");
        _144 = 0.0f;
        _SMG2_1A8 = 45;
    }

    if (MR::isActionEnd(this)) {
        s32 frames = mSpinDriverCamera->getAppearCameraFrames();

        if (MR::isGreaterStep(this, frames)) {
            setNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait));

            if (mSpinDriverCamera->isUseAppearCamera(this)) {
                mSpinDriverCamera->endAppearCamera(this);
                MR::endDemoWaitCameraInterpolating(this, "出現");
            }
        }
    }
}

void SuperSpinDriver::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }

    if (_SMG2_1A8 <= 0) {
        addSwingSignRotateY();
    }

    if (_178 > 0) {
        MR::startActionSound(this, "OjLvSSpinDrvShine");

        if (!_17C) {
            MR::emitEffect(this, "SuperSpinDriverLight");
            MR::startCSSound("CS_SPIN_BIND", nullptr, 0, 0, 1.0f);
        }
    } else {
        MR::deleteEffect(this, "SuperSpinDriverLight");
    }

    if (mTalkCtrl != nullptr) {
        MR::tryTalkNearPlayer(mTalkCtrl);
    }

    trySwitchOff();
}

void SuperSpinDriver::exeCapture() {
    if (tryForceCancel()) {
        MR::deleteEffect(this, "SuperSpinDriverLight");
    } else {
        if (MR::isFirstStep(this)) {
            MR::emitEffect(this, "SuperSpinDriverLight");
            MR::startBckPlayer("SpinDriverWait", "SuperSpinDriverCapture");
            _144 = 0.0f;
        }

        MR::startActionSound(this, "OjLvSSpinDrvShine");
        MR::startActionSound(this, "OjLvSpinDrvCapture");
        moveBindPosToCenter();
        f32 rate = MR::calcNerveRate(this, 60);
        _134 = rate;
        updateBindActorPoseToShoot((f64)rate);
        _144 += 0.0040f;
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        _178 = 60;

        if (!tryEndCapture()) {
            if (!tryShootStart()) {
                return;
            }
        }
    }
}

void SuperSpinDriver::exeCoolDown() {
    // BUG, is supposed to be a conditional to call tryEndCoolDown

    if (MR::isFirstStep(this)) {
    }

    if (!tryEndCoolDown()) {
        trySwitchOff();
    }
}

void SuperSpinDriver::exeShootStart() {
    if (tryForceCancel()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        MR::startLevelSoundPlayer("SpinAttack", -1, -1);
        MR::startCSSound("CS_SPIN_DRIVE_LONG", "SE_SY_CS_S_SPIN_DRV_START", 0, 0, 1.0f);
        MR::startActionSound(this, "OjSSpinDrvPrepJump");

        if (MR::isInAreaObj("Water", mPosition)) {
            MR::startActionSound(this, "PmSpinDrvInWater1");
        }

        MR::deleteEffectAll(this);
        MR::emitEffect(this, "SuperSpinDriverStart");
        MR::startAction(this, "Start");
        MR::startBckPlayer("SuperSpinDriverStart", "SuperSpinDriverShoot");
        _118 = _C4;
        updateBindActorPoseToShoot(1.0f);
    }

    f32 rate = MR::calcNerveRate(this, 15);

    _C4 = mPosition * rate + _118 * (1.0f - rate);
    _144 *= 0.8f;

    MR::tryRumblePadMiddle(this, WPAD_CHAN0);

    if (tryShoot()) {
        return;
    }
}

void SuperSpinDriver::exeShoot() {
    if (tryForceCancel()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        calcShootMotionTime();
        MR::validateHitSensor(this, "Body");
        MR::shakeCameraStrong();
        MR::tryRumblePadVeryStrong(this, WPAD_CHAN0);
        mShootPathDirection.set(_100);
        startPathDraw();
    }

    f32 rate = MR::calcNerveRate(this, mFlightTime);
    updatePathDraw(rate);
    updateBindPosition(rate);

    if (!MR::isNearZero(mShootPathDirection)) {
        TVec3f stack_8;
        MR::normalize(&stack_8, mShootPathDirection);
        turnBindHead(stack_8, 0.4f);
    }

    updateShootMotion();
    updateEaseRotate();
    mSpinDriverCamera->update(mShootPathDirection, mShootPathPosition);

    if (tryEndShoot()) {
        return;
    }
}

void SuperSpinDriver::updateEaseRotate() {
    if (_154 <= getNerveStep() && getNerveStep() <= _158) {
        f32 v = MR::normalize(getNerveStep(), _154, _158);
        _138 = _13C * MR::getEaseOutValue(v, 0.0f, 1.0f, 1.0f);
    }

    _148 = PI * MR::normalize(getNerveStep(), _158, _15C);
}

void SuperSpinDriver::updateShootMotion() {
    if (mIsNoPauseMenuStars) {
        if (MR::isFirstStep(this)) {
            MR::startBckPlayer("CannonFlyStart", nullptr);
            MR::startActionSound(this, "SySSpinDrvMeAlt");
            MR::startActionSound(this, "OjSSpinDrvJump");
        }

        if (MR::isStep(this, 3)) {
            MR::startSoundPlayer("SE_PM_SPINDRIVER_PANIC", -1, -1);
        }

        MR::startLevelSoundPlayer("SE_PM_LV_S_SPIN_DRV_FLY", -1);
        return;
    }

    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("SpaceFlyStart", "SuperSpinDriverFlyStart");

        if (MR::hasME()) {
            MR::startSystemME("ME_MAGIC_L");
        } else {
            MR::startActionSound(this, "SySSpinDrvMeAlt");
        }

        MR::startActionSound(this, "OjSSpinDrvJump");

        if (MR::isPlayerElementModeYoshi()) {
            MR::startActionSoundPlayer("StarRingStart", -1, -1);
        } else if (mFlightTime < 180) {
            MR::startLevelSoundPlayer("JumpObj", -1, -1);
        } else {
            MR::startLevelSoundPlayer("StarRingStart", -1, -1);
        }

        if (MR::isSupportPlayActive()) {
            MR::startLevelSoundPlayer("SupportTicoSpinDriverShoot", -1, -1);
        }

        if (MR::isInAreaObj("Water", mPosition)) {
            MR::startActionSound(this, "PmSpinDivInWater2");
        }
    }

    if (MR::isStep(this, _154)) {
        MR::startBckPlayer("SpaceFlyLoop", "SuperSpinDriverFlyLoop");
    }

    if (MR::isLessStep(this, _158)) {
        MR::startLevelSoundPlayer("SE_PM_LV_S_SPIN_DRV_FLY", -1);
    }

    if (MR::isStep(this, _158)) {
        MR::startBckPlayer("SpaceFlyEnd", "SuperSpinDriverFlyEnd");
        MR::startLevelSoundPlayer("SpinDrvCoolDown", -1, -1);

        if (mFlightTime >= 135) {
            if (MR::isPlayerElementModeYoshi()) {
                MR::startActionSoundPlayer("PreLandTurn", -1, -1);
            } else {
                MR::startLevelSoundPlayer("JumpSV", -1, -1);
            }
        }
    }

    if (MR::isStep(this, _15C)) {
        MR::startBckPlayer("Fall", "SuperSpinDriverFall");
    }
}

void SuperSpinDriver::cancelBind() {
    if (mBindActor != nullptr) {
        MR::endBindAndPlayerJump(this, _D0, 0);
        mBindActor = nullptr;
    }

    mSpinDriverCamera->cancel();
}

void SuperSpinDriver::endBind() {
    MR::endBindAndSpinDriverJump(this, _D0);
    mBindActor = nullptr;
    mSpinDriverCamera->end();
}

void SuperSpinDriver::updateBindActorMatrix() {
    TPos3f mtx;
    mtx.identity();
    mtx.setEulerY(_138);
    mtx.setTrans(0.0f, 0.0f, 0.0f);

    TPos3f mtx2;
    mtx2.identity();
    mtx2.setEulerX(_148);

    TPos3f mtx3;
    mtx3.setQT(_B4, _C4);

    mtx3.concat(mtx3, mtx2);
    mtx3.concat(mtx3, mtx);

    LiveActor* pBindActor = mBindActor;

    if (strcmp(pBindActor->mName, "マリオアクター") == 0) {
        MR::setPlayerBaseMtx(mtx3);
    } else {
        MR::setBaseTRMtx(pBindActor, mtx3);
    }

    PSMTXCopy(mtx3, _SMG2A8);
}

void SuperSpinDriver::updateBindActorPoseToShoot(f32 a1) {
    TPos3f stack_18;
    stack_18.identity();

    MR::makeMtxUpFront(&stack_18, _100, _E8);

    TQuat4f stack_8;
    stack_18.getQuat(stack_8);

    _B4.set(_A4);
    _B4.slerp(stack_8, a1);
}

void SuperSpinDriver::turnBindHead(const TVec3f& rVec, f32 a2) {
    TVec3f quatYDir;
    _B4.getYDir(quatYDir);

    TQuat4f quat;
    quat.setRotate(quatYDir, rVec, a2);

    _B4.mult(quat);
    _B4.normalize();
}

void SuperSpinDriver::moveBindPosToCenter() {
    TVec3f pos(mPosition);
    _C4 += _D0;

    TVec3f vec(pos - _C4);
    f32 scalar;
    MR::separateScalarAndDirection(&scalar, &vec, vec);

    _D0 += vec * 1.5f * (scalar / 120.0f);

    _D0.mult(0.8f);
}

void SuperSpinDriver::startPathDraw() {
    if (mPathDrawer != nullptr) {
        if (MR::isDead(mPathDrawer)) {
            mPathDrawer->appear();
            MR::emitEffect(this, "EndGlow");
        }
    }
}

void SuperSpinDriver::endPathDraw() {
    if (mPathDrawer != nullptr) {
        if (!MR::isDead(mPathDrawer)) {
            MR::emitEffect(this, "EndGlow");
        }
    }
}

void SuperSpinDriver::updatePathDraw(f32 coord) {
    if (mPathDrawer == nullptr) {
        return;
    }

    mPathDrawer->setCoord(coord);

    if (mDrawPathRangeIdx >= 0) {
        MR::updateStorageSpinDriverPathDrawRange(mDrawPathRangeIdx, mPathDrawer->_B0);
    }
}

void SuperSpinDriver::updateBindPosition(f32 coord) {
    mShootPath->calcPosition(&mShootPathPosition, coord);

    TVec3f direction;
    mShootPath->calcDirection(&direction, coord, 0.0099999998f);

    if (!MR::isNearZero(direction)) {
        mShootPathDirection = direction;
    }

    TVec3f stack_14 = _C4;
    _C4.set(mShootPathPosition);
    _D0 = _C4 - stack_14;
}

void SuperSpinDriver::calcShootMotionTime() {
    if (mFlightTime < 20) {
        _154 = -1;
        _158 = -1;
        _15C = 0;
        return;
    }

    if (mFlightTime < 20 + ::cSpaceFlyEndFrame) {
        _154 = -1;
        _158 = 0;
        _15C = mFlightTime - 20;
        return;
    }

    _158 = mFlightTime - (20 + ::cSpaceFlyEndFrame);
    _15C = mFlightTime - 20;

    s32 temp = 0.2f * mFlightTime;

    if (temp > 90) {
        temp = 90;
    }

    _154 = (temp / ::cSpaceFlyStartFrame) * ::cSpaceFlyStartFrame;

    if (_158 <= _154) {
        _154 = 0;
    }

    s32 temp2 = ((_158 - _154) * 0.05f) / TWO_PI;
    _138 = 0.0f;
    _13C = TWO_PI * temp2;
}

void SuperSpinDriver::addSwingSignRotateY() {
    if (isSwingOr2PTrigger()) {
        _144 += 0.1f;

        if (_144 > 0.23f) {
            _144 = 0.23f;
        }
    }
}

void SuperSpinDriver::onUse() {
    if (mEmptyModel != nullptr) {
        mEmptyModel->kill();
    }

    MR::showModel(this);
}

void SuperSpinDriver::offUse() {
    if (mEmptyModel != nullptr) {
        mEmptyModel->appear();
    }

    MR::hideModelAndOnCalcAnim(this);
}

bool SuperSpinDriver::isNeedEmptyModel() const {
    switch (mColor) {
    case 1:
        return true;
    default:
        return false;
    }
}

bool SuperSpinDriver::isRightToUse() const {
    switch (mColor) {
    case 1:
        return MR::isOnGameEventFlagGreenDriver();
    default:
        return true;
    }
}

bool SuperSpinDriver::isNerveEnableBind() const {
    return isNerve(GET_NERVE(SuperSpinDriver, SuperSpinDriverNrvWait)) && _SMG2_1A8 == 0;
}

bool SuperSpinDriver::isSwingOr2PTrigger() const {
    return MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();
}

bool SuperSpinDriver::isFarPlayer() const {
    return !MR::isNearPlayerAnyTime(this, 350.0f);
}

namespace MR {
    NameObj* createSuperSpinDriverYellow(const char* pName) {
        return new SuperSpinDriver(pName, 0);
    }

    NameObj* createSuperSpinDriverPink(const char* pName) {
        return new SuperSpinDriver(pName, 2);
    }
};  // namespace MR
