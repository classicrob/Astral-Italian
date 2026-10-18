#include "Game/MapObj/EarthenPipe.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

// PLACEHOLDER(objects): reads pipe BGM map arguments; resolve when core names fn_8001B3E0
extern "C" void fn_8001B3E0(s32*, s32*, const JMapInfoIter&);
// PLACEHOLDER(objects): fades or changes BGM on pipe entry; resolve when core names fn_8001B430
extern "C" void fn_8001B430(s32, s32);
// PLACEHOLDER(objects): restores BGM on pipe exit; resolve when core names fn_8001B510
extern "C" void fn_8001B510(s32, s32);
// PLACEHOLDER(objects): prepares rescued Tico for pipe travel; resolve when core names fn_8005FFC0
extern "C" void fn_8005FFC0();
// PLACEHOLDER(objects): launches rescued Tico at the pipe matrix; resolve when core names fn_8005FFD0
extern "C" void fn_8005FFD0(MtxPtr);
// PLACEHOLDER(objects): sets player post-launch timer; resolve when core names fn_8004FF80
extern "C" void fn_8004FF80(s32);

namespace NrvEarthenPipe {
    NEW_NERVE(EarthenPipeNrvWait, EarthenPipe, Wait);
    NEW_NERVE(EarthenPipeNrvReady, EarthenPipe, Ready);
    NEW_NERVE(EarthenPipeNrvPlayerIn, EarthenPipe, PlayerIn);
    NEW_NERVE(EarthenPipeNrvTargetPipeShowUp, EarthenPipe, TargetPipeShowUp);
    NEW_NERVE(EarthenPipeNrvPlayerOut, EarthenPipe, PlayerOut);
    NEW_NERVE(EarthenPipeNrvLaunch, EarthenPipe, Launch);
    NEW_NERVE(EarthenPipeNrvInvalid, EarthenPipe, Invalid);
    NEW_NERVE(EarthenPipeNrvHide, EarthenPipe, Hide);
    NEW_NERVE(EarthenPipeNrvShow, EarthenPipe, Show);
    NEW_NERVE(EarthenPipeNrvWaitToShowUp, EarthenPipe, WaitToShowUp);
    NEW_NERVE(EarthenPipeNrvShowUp, EarthenPipe, ShowUp);
    NEW_NERVE(EarthenPipeNrvWaitToHideDown, EarthenPipe, WaitToHideDown);
    NEW_NERVE(EarthenPipeNrvHideDown, EarthenPipe, HideDown);
};  // namespace NrvEarthenPipe

EarthenPipe::EarthenPipe(const char* pName) : LiveActor(pName) {
    _8C.x = 0.0f;
    _8C.y = 0.0f;
    _8C.z = 0.0f;
    _98.x = 0.0f;
    _98.y = 1.0f;
    _98.z = 0.0f;
    _A4 = 100.0f;
    mIsIgnoreGravity = false;
    mPipeMode = 0;
    _B0 = 0;
    mTopJointMtx = nullptr;
    mBottomJointMtx = nullptr;
    mHostActor = nullptr;
    _180 = 0.0f;
    mHorizExitForce = 5.0f;
    mVertExitForce = 20.0f;
    mMusicChangeIdx = -1;
    mMusicState = -1;
    mSavedBgmId = nullptr;
    mSavedBgmState = -1;
    _194 = 0;
    mPipeStreamModel = nullptr;
    mIsPipeStreamShown = 0;
    mCameraInfo = nullptr;
    _F0.identity();
    _120.identity();
    _150.identity();
    _BC.identity();
}

void EarthenPipe::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg2NoInit(rIter, &mPipeMode);
    if (isLaunchMode()) {
        mHorizExitForce = 4.0f;
        mVertExitForce = 25.0f;
    }
    MR::getJMapInfoArg0NoInit(rIter, &mHorizExitForce);
    MR::getJMapInfoArg1NoInit(rIter, &mVertExitForce);
    fn_8001B3E0(&mMusicChangeIdx, &mMusicState, rIter);
    MR::getJMapInfoArg6NoInit(rIter, &mIsIgnoreGravity);
    s32 arg7 = 0;
    MR::getJMapInfoArg7NoInit(rIter, &arg7);
    bool isWaterPipe = false;

    if (MR::isObjectName(rIter, "EarthenPipeInWater")) {
        isWaterPipe = true;
    }

    if (mPipeMode == 4) MR::declareCoin(this, 3);
    if (mPipeMode == 5) MR::declareStarPiece(this, 8);
    if (isLaunchMode()) {
        _B0 = this;
    } else {
        MR::createSceneObj(SceneObj_EarthenPipeMediator);
        MR::getSceneObj<EarthenPipeMediator>(SceneObj_EarthenPipeMediator)->entry(this, rIter);
    }
    _8C.set(mPosition);
    MR::calcGravity(this);
    MR::initActor(this, rIter, "EarthenPipe", false);
    mTopJointMtx = MR::getJointMtx(this, "Top");
    mBottomJointMtx = MR::getJointMtx(this, "Bottom");
    MR::startBrk(this, "EarthenPipe");
    MR::setBrkFrameAndStop(this, arg7);
    MR::connectToSceneMapObjNoCalcAnimStrongLight(this);
    TPos3f v25;
    v25.identity();
    MR::makeMtxTR(v25, this);
    TVec3f v23;
    v25.getZDir(v23);

    if (mIsIgnoreGravity) {
        v25.getYDir(_98);
    } else {
        _98.set(mGravity);
        f32 _x = _98.x;
        f32 mult = -1.0f;
        f32 x = _x * mult;
        f32 y = _98.y * mult;
        f32 z = _98.z * mult;

        _98.x = x;
        _98.y = y;
        _98.z = z;
    }

    TPos3f v24;
    v24.identity();
    MR::makeMtxUpFrontPos(&v24, _98, v23, _8C);
    PSMTXCopy(v24, mTopJointMtx);
    _BC.setInline(v24);
    TVec3f v22(_8C);
    TVec3f v21(_98);
    v21.scale(50.0f);
    v22.sub(v21);
    v24.setTrans(v22);
    PSMTXCopy(v24, mBottomJointMtx);
    MR::setBaseTRMtx(this, mTopJointMtx);

    if (isWaterPipe) {
        initHitSensor(3);
    } else {
        initHitSensor(1);
    }

    MR::addHitSensorBinder(this, "binder", 8, 70.0f, TVec3f(0.0f, -50.0f, 0.0f));

    if (isWaterPipe) {
        MR::addHitSensorBinder(this, "BinderWaterUpper", 8, 200.0f, TVec3f(0.0f, 180.0f, 0.0f));
        MR::addHitSensorBinder(this, "BinderWaterLower", 8, 80.0f, TVec3f(0.0f, 50.0f, 0.0f));
    }

    MR::initCollisionParts(this, "EarthenPipe", getSensor("binder"), mTopJointMtx);
    if (MR::isEqualStageName("WhiteSnowGalaxy") && mScale.y == 7.0f) {
        MR::setClippingTypeSphere(this, 1000.0f);
    }

    f32 v14 = (100.0f * mScale.y);
    mScale.y = 1.0f;
    _A4 = v14;
    calcTrans(1.0f);
    initEffectKeeper(0, nullptr, false);
    MR::setEffectHostMtx(this, "LavaAppear", _BC.mMtx);
    MR::setEffectHostMtx(this, "LavaVanish", _BC.mMtx);
    initSound(4, "EarthenPipe", nullptr, TVec3f(0.0f));
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "出現");
    initNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWait));

    if ((mPipeMode - 1) <= 1u) {
        MR::invalidateCollisionParts(this);
        calcTrans(0.0f);
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHide));
    }

    if (isWaterPipe) {
        mPipeStreamModel = MR::createPartsModelMapObj(this, "土管水流", "EarthenPipeStream", nullptr);
        MR::setClippingTypeSphere(mPipeStreamModel, 500.0f);
        MR::tryStartAllAnim(mPipeStreamModel, "EarthenPipeStream");
        mPipeStreamModel->appear();
        mIsPipeStreamShown = 1;
    } else {
        mIsPipeStreamShown = 0;
    }

    if (MR::isValidSwitchA(this)) MR::invalidateHitSensors(this);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        if (mPipeStreamModel != nullptr) {
            MR::registerDemoSimpleCastAll(mPipeStreamModel);
        }
    } else if (MR::isEqualStageName("HeavensDoorGalaxy")) {
        MR::registerDemoSimpleCastAll(this);
        MR::registerMarioFaceShipNpc(this, rIter);
    }

    makeActorAppeared();
}

void EarthenPipe::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHide))) {
        MR::invalidateCollisionParts(this);
    }
}

bool EarthenPipe::tryShowUp() {
    MR::invalidateClipping(this);
    MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "出現", -1);
    if (mPipeMode != 1 && mPipeMode != 2) {
        return false;
    }

    setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWaitToShowUp));
    return true;
}

bool EarthenPipe::tryHideDown() {
    MR::validateClipping(this);
    MR::endMultiActorCamera(this, mCameraInfo, "出現", true, -1);

    if (!mPipeMode) {
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid));
        return false;
    } else {
        if (mPipeMode == 2) {
            mPipeMode = 0;
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid));
            return false;
        } else {
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWaitToHideDown));
            return true;
        }
    }
}

bool EarthenPipe::isNerveShowUp() const {
    bool ret;

    if (isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWaitToShowUp)) || isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvShowUp))) {
        ret = true;
    } else {
        ret = false;
    }

    return ret;
}

void EarthenPipe::exeWait() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            if (!MR::isValidHitSensor(this, "binder")) {
                MR::validateHitSensors(this);
                if (mPipeStreamModel != nullptr) {
                    mPipeStreamModel->appear();
                    MR::startAction(mPipeStreamModel, "Appear");
                    mIsPipeStreamShown = 1;
                }
            }
        } else if (MR::isValidHitSensor(this, "binder")) {
            MR::invalidateHitSensors(this);
            if (mPipeStreamModel != nullptr) {
                mPipeStreamModel->kill();
            }

            mIsPipeStreamShown = 0;
        }
    }
}

void EarthenPipe::exeReady() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeReady", nullptr);
    }

    MR::blendMtx(_120.mMtx, _150.mMtx, MR::calcNerveRate(this, MR::getBckFrameMaxPlayer()), _F0.mMtx);
    if (MR::isBckStoppedPlayer()) {
        if (!MR::isPlayerDead()) {
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvPlayerIn));
        }
    }
}

void EarthenPipe::exePlayerIn() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeIn", nullptr);
        MR::startActionSound(this, "DmDokanWarp", -1, -1, -1);
        if (MR::getPlayingStageBgmId()) {
            mMusicChangeIdx = 10000;
            mSavedBgmId = reinterpret_cast<const char*>(MR::getPlayingStageBgmId());
            mSavedBgmState = MR::getStageBgmState();
        }
        if (_B0->mSavedBgmId) {
            if (MR::getPlayingStageBgmId() && !MR::isEqualString(reinterpret_cast<const char*>(MR::getPlayingStageBgmId()), _B0->mSavedBgmId))
                MR::stopStageBGM(60);
        } else if (_B0->mMusicChangeIdx != 10000) {
            fn_8001B430(_B0->mMusicChangeIdx, _B0->mMusicState);
        }
    }
    if (MR::isBckStoppedPlayer() && !MR::isPlayerDead()) {
        _F0.setInline(_B0->getBaseMtx());
        fn_8005FFC0();
        if (isLaunchMode()) {
            if (mPipeMode == 4) MR::appearCoinAgainstGravity(this, mPosition, 3);
            if (mPipeMode == 5) MR::appearStarPiece(this, mPosition, 8, 10.0f, 40.0f, false);
            mPipeMode = 3;
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvLaunch));
        } else if (_B0->tryShowUp()) {
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvTargetPipeShowUp));
        } else {
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvPlayerOut));
        }
    }
}

void EarthenPipe::exeTargetPipeShowUp() {
    if (MR::isFirstStep(this)) {
        MR::hidePlayer();
    }

    if (!_B0->isNerveShowUp()) {
        _F0.setInline(_B0->getBaseMtx());
        fn_8005FFC0();
        MR::showPlayer();
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvPlayerOut));
    }
}

void EarthenPipe::exePlayerOut() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeOut", nullptr);
        MR::startActionSound(this, "DmDokanWarp", -1, -1, -1);
        if (_B0->mSavedBgmId) {
            if (!MR::getPlayingStageBgmId() || !MR::isEqualString(reinterpret_cast<const char*>(MR::getPlayingStageBgmId()), _B0->mSavedBgmId)) {
                MR::startStageBGM(_B0->mSavedBgmId, false);
                if (_B0->mSavedBgmState >= 0) MR::setStageBGMState(_B0->mSavedBgmState, 0);
            }
        } else if (_B0->mMusicChangeIdx != 10000) {
            fn_8001B510(_B0->mMusicChangeIdx, _B0->mMusicState);
        }
    }
    if (MR::isBckStoppedPlayer()) launchPlayer(false);
}

void EarthenPipe::exeLaunch() {
    if (MR::isStep(this, 22)) {
        MR::startActionSound(this, "DmDokanWarp", -1, -1, -1);
        fn_8001B510(_B0->mMusicChangeIdx, _B0->mMusicState);
        launchPlayer(true);
    }
}

inline bool isNear(const LiveActor* actor) {
    return !(MR::isNearPlayer(actor, 100.0f));
}

void EarthenPipe::exeInvalid() {
    if ((MR::isGreaterStep(this, 30) && MR::isOnGroundPlayer()) || (MR::isPlayerSwimming() && isNear(this))) {
        MR::validateClipping(this);
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWait));
    }
}

void EarthenPipe::exeHide() {
}

void EarthenPipe::exeShow() {
}

void EarthenPipe::exeWaitToHideDown() {
    if (MR::isStep(this, 40)) {
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHideDown));
    }
}

void EarthenPipe::exeWaitToShowUp() {
    if (MR::isStep(this, 20)) {
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvShowUp));
    }
}

void EarthenPipe::exeShowUp() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::validateCollisionParts(this);
        MR::emitEffect(this, "LavaAppear");
        MR::startActionSound(this, "SyItemAppear", -1, -1, -1);
    }

    f32 easeIn = MR::getEaseInValue(getNerveStep() / 30.0f, 0.0f, 1.0f, 1.0f);
    calcTrans(easeIn);

    if (MR::isLessStep(this, 30)) {
        MR::startActionSound(this, "OjLvPipeAppearMagma", -1, -1, -1);
    }

    if (MR::isStep(this, 30)) {
        MR::deleteEffect(this, "LavaAppear");
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvShow));
    }
}

void EarthenPipe::exeHideDown() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "LavaVanish");
    }

    f32 easeIn = MR::getEaseInValue((15 - getNerveStep()) / 15.0f, 0.0f, 1.0f, 1.0f);
    calcTrans(easeIn);
    if (MR::isLessStep(this, 15)) {
        MR::startActionSound(this, "OjLvPipeVanishMagma", -1, -1, -1);
    }

    if (MR::isStep(this, 15)) {
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
        MR::validateClipping(this);
        MR::deleteEffect(this, "LavaVanish");
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHide));
    }
}

void EarthenPipe::control() {
    MR::updateMaterial(this);
    if (mIsPipeStreamShown) {
        MR::startActionSound(this, "OjLvPipeInWater", -1, -1, -1);
    }
}

bool EarthenPipe::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (!(!isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid)) && !isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHide)) &&
              !isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWaitToShowUp)) && !isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvShowUp)) &&
              !isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvWaitToHideDown)) && !isNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvHideDown)) &&
              !MR::isPlayerDead())) {
            return false;
        }

        TVec3f diff = *MR::getPlayerPos() - TVec3f(mTopJointMtx[0][3], mTopJointMtx[1][3], mTopJointMtx[2][3]);
        TVec3f delta = diff;
        MR::vecKillElement(delta, mGravity, &delta);
        if (!MR::isPlayerSwimming() && delta.length() > 50.0f && diff.dot(_98) < -5.0f) {
            return false;
        }
        mHostActor = MR::getSensorHost(pSender);
        MR::invalidateClipping(this);
        _120.set(MR::getPlayerBaseMtx());
        _F0.set(_120);
        TVec3f camPos = MR::getCamPos();
        camPos.sub(mPosition);
        MR::normalize(&camPos);
        MR::makeMtxUpFrontPos(&_150, _98, camPos, mPosition);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvReady));
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(_F0);
        return true;
    }
    if (msg == 0xF2 /* ACTMES_HEAVENSDOOR_RUNAWAY_RABBIT_WAIT */) {
        _194 = true;
    }
    if (msg == 0xF3 /* ACTMES_HEAVENSDOOR_RUNAWAY_RABBIT_START */) {
        _194 = false;
        return true;
    }
    if (MR::isMsgRushForceCancel(msg)) {
        mHostActor = nullptr;
        _B0->tryHideDown();
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid));
        return true;
    }
    MR::isMsgRushCancel(msg);
    return false;
}

void EarthenPipe::calcTrans(f32 a1) {
    mPosition.set(_98);
    mPosition.scale(a1 * _A4);
    mPosition.add(_8C);

    (mTopJointMtx)[0][3] = mPosition.x;
    (mTopJointMtx)[1][3] = mPosition.y;
    (mTopJointMtx)[2][3] = mPosition.z;
    calcAndSetBaseMtx();
    if (MR::isValidCollisionParts(this)) {
        MR::setCollisionMtx(this);
    }
}

void EarthenPipe::launchPlayer(bool quick) {
    MtxPtr baseMtx = _B0->getBaseMtx();
    fn_8005FFD0(baseMtx);
    TVec3f front(baseMtx[0][2], baseMtx[1][2], baseMtx[2][2]);
    TVec3f horizontal(front);
    TVec3f vertical(baseMtx[0][1], baseMtx[1][1], baseMtx[2][1]);
    horizontal.scale(_B0->mHorizExitForce);
    vertical.scale(_B0->mVertExitForce);
    TVec3f velocity(vertical);
    if (!MR::isInWater(_B0->mPosition)) velocity.add(horizontal);
    MR::startLevelSoundPlayer("JumpMM", -1, -1);
    MR::startLevelSoundPlayer("JumpObj", -1, -1);
    if (quick) MR::startBckPlayerJ("土管クイックジャンプ");
    else MR::startBckPlayer("EarthenPipeJump", nullptr);
    MR::setPlayerFrontTargetVec(front, 1);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, velocity);
    fn_8004FF80(5);
    mHostActor = nullptr;
    _B0->tryHideDown();
    if (!_194 && MR::isValidSwitchB(this)) MR::onSwitchB(this);
    setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid));
}

#pragma dont_inline on
bool EarthenPipe::isLaunchMode() const {
    return (u32)(mPipeMode - 3) <= 2;
}
#pragma dont_inline reset

EarthenPipeMediator::EarthenPipeMediator() : NameObj("土管仲介者") {
    mNumEntries = 0;
    mPipeEntries = nullptr;
    mPipeEntries = new Entry[0x20];

    for (s32 i = 0; i < 32; i++) {
        mPipeEntries[i]._0 = nullptr;
        mPipeEntries[i]._4 = nullptr;
        mPipeEntries[i].mPipeID = -1;
    }
}

void EarthenPipeMediator::entry(EarthenPipe* pPipe, const JMapInfoIter& rIter) {
    s32 pipeID = -1;
    MR::getJMapInfoArg3NoInit(rIter, &pipeID);

    for (s32 i = 0; i < mNumEntries; i++) {
        EarthenPipeMediator::Entry* entry = &mPipeEntries[i];

        if (pipeID == entry->mPipeID) {
            entry->_4 = pPipe;
            entry->_0->_B0 = pPipe;
            entry->_4->_B0 = entry->_0;
            return;
        }
    }

    EarthenPipeMediator::Entry* entry = &mPipeEntries[mNumEntries];
    entry->_0 = pPipe;
    entry->mPipeID = pipeID;
    mNumEntries++;
}
