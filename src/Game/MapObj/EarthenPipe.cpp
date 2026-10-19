#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/MapObj/EarthenPipe.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"


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
    mInitialPos.x = 0.0f;
    mInitialPos.y = 0.0f;
    mInitialPos.z = 0.0f;
    mUp.x = 0.0f;
    mUp.y = 1.0f;
    mUp.z = 0.0f;
    mMoveDistance = 100.0f;
    mIsIgnoreGravity = false;
    mPipeMode = 0;
    mTargetPipe = 0;
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
    mPlayerMtx.identity();
    mPlayerStartMtx.identity();
    mPlayerEndMtx.identity();
    mEffectMtx.identity();
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
    MR::getStageBgmJMapArgs(&mMusicChangeIdx, &mMusicState, rIter);
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
        mTargetPipe = this;
    } else {
        MR::createSceneObj(SceneObj_EarthenPipeMediator);
        MR::getSceneObj<EarthenPipeMediator>(SceneObj_EarthenPipeMediator)->entry(this, rIter);
    }
    mInitialPos.set(mPosition);
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
        v25.getYDir(mUp);
    } else {
        mUp.set(mGravity);
        f32 _x = mUp.x;
        f32 mult = -1.0f;
        f32 x = _x * mult;
        f32 y = mUp.y * mult;
        f32 z = mUp.z * mult;

        mUp.x = x;
        mUp.y = y;
        mUp.z = z;
    }

    TPos3f v24;
    v24.identity();
    MR::makeMtxUpFrontPos(&v24, mUp, v23, mInitialPos);
    PSMTXCopy(v24, mTopJointMtx);
    mEffectMtx.setInline(v24);
    TVec3f v22(mInitialPos);
    TVec3f v21(mUp);
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
    mMoveDistance = v14;
    calcTrans(1.0f);
    initEffectKeeper(0, nullptr, false);
    MR::setEffectHostMtx(this, "LavaAppear", mEffectMtx.mMtx);
    MR::setEffectHostMtx(this, "LavaVanish", mEffectMtx.mMtx);
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
    }
    MR::registerMarioFaceShipNpc(this, rIter);

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
            if (MR::tryValidateHitSensor(this, "binder")) {
                MR::validateHitSensors(this);
                if (mPipeStreamModel != nullptr) {
                    mPipeStreamModel->appear();
                    MR::startAction(mPipeStreamModel, "Appear");
                    mIsPipeStreamShown = 1;
                }
            }
        } else if (MR::tryInvalidateHitSensor(this, "binder")) {
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

    MR::blendMtx(mPlayerStartMtx.mMtx, mPlayerEndMtx.mMtx, MR::calcNerveRate(this, MR::getBckFrameMaxPlayer()), mPlayerMtx.mMtx);
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
            mSavedBgmId = MR::getPlayingStageBgmId();
            mSavedBgmState = MR::getStageBgmState();
        }
        if (mTargetPipe->mSavedBgmId) {
            if (MR::getPlayingStageBgmId() && !MR::isEqualString(MR::getPlayingStageBgmId(), mTargetPipe->mSavedBgmId))
                MR::stopStageBGM(60);
        } else if (mTargetPipe->mMusicChangeIdx != 10000) {
            MR::fadeOutStageBGMForIndex(mTargetPipe->mMusicChangeIdx, mTargetPipe->mMusicState);
        }
    }
    if (MR::isBckStoppedPlayer() && !MR::isPlayerDead()) {
        mPlayerMtx.setInline(mTargetPipe->getBaseMtx());
        MR::pauseRescuedStrayTicoFollow();
        if (isLaunchMode()) {
            if (mPipeMode == 4) MR::appearCoinAgainstGravity(this, mPosition, 3);
            if (mPipeMode == 5) MR::appearStarPiece(this, mPosition, 8, 10.0f, 40.0f, false);
            mPipeMode = 3;
            setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvLaunch));
        } else if (mTargetPipe->tryShowUp()) {
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

    if (!mTargetPipe->isNerveShowUp()) {
        mPlayerMtx.setInline(mTargetPipe->getBaseMtx());
        MR::pauseRescuedStrayTicoFollow();
        MR::showPlayer();
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvPlayerOut));
    }
}

void EarthenPipe::exePlayerOut() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeOut", nullptr);
        MR::startActionSound(this, "DmDokanWarp", -1, -1, -1);
        if (mTargetPipe->mSavedBgmId) {
            if (!MR::getPlayingStageBgmId() || !MR::isEqualString(MR::getPlayingStageBgmId(), mTargetPipe->mSavedBgmId)) {
                MR::startStageBGM(mTargetPipe->mSavedBgmId, false);
                if (mTargetPipe->mSavedBgmState >= 0) MR::setStageBGMState(mTargetPipe->mSavedBgmState, 0);
            }
        } else if (mTargetPipe->mMusicChangeIdx != 10000) {
            MR::startStageBGMForIndex(mTargetPipe->mMusicChangeIdx, mTargetPipe->mMusicState);
        }
    }
    if (MR::isBckStoppedPlayer()) launchPlayer(false);
}

void EarthenPipe::exeLaunch() {
    if (MR::isStep(this, 22)) {
        MR::startActionSound(this, "DmDokanWarp", -1, -1, -1);
        MR::startStageBGMForIndex(mTargetPipe->mMusicChangeIdx, mTargetPipe->mMusicState);
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
        if (!MR::isPlayerSwimming() && delta.length() > 50.0f && diff.dot(mUp) < -5.0f) {
            return false;
        }
        mHostActor = MR::getSensorHost(pSender);
        MR::invalidateClipping(this);
        mPlayerStartMtx.set(MR::getPlayerBaseMtx());
        mPlayerMtx.set(mPlayerStartMtx);
        TVec3f camPos = MR::getCamPos();
        camPos.sub(mPosition);
        MR::normalize(&camPos);
        MR::makeMtxUpFrontPos(&mPlayerEndMtx, mUp, camPos, mPosition);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvReady));
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mPlayerMtx);
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
        mTargetPipe->tryHideDown();
        setNerve(GET_NERVE(EarthenPipe, EarthenPipeNrvInvalid));
        return true;
    }
    MR::isMsgRushCancel(msg);
    return false;
}

void EarthenPipe::calcTrans(f32 a1) {
    mPosition.set(mUp);
    mPosition.scale(a1 * mMoveDistance);
    mPosition.add(mInitialPos);

    (mTopJointMtx)[0][3] = mPosition.x;
    (mTopJointMtx)[1][3] = mPosition.y;
    (mTopJointMtx)[2][3] = mPosition.z;
    calcAndSetBaseMtx();
    if (MR::isValidCollisionParts(this)) {
        MR::setCollisionMtx(this);
    }
}

void EarthenPipe::launchPlayer(bool quick) {
    MtxPtr baseMtx = mTargetPipe->getBaseMtx();
    MR::launchRescuedStrayTicoFromMtx(baseMtx);
    TVec3f front(baseMtx[0][2], baseMtx[1][2], baseMtx[2][2]);
    TVec3f horizontal(front);
    TVec3f vertical(baseMtx[0][1], baseMtx[1][1], baseMtx[2][1]);
    horizontal.scale(mTargetPipe->mHorizExitForce);
    vertical.scale(mTargetPipe->mVertExitForce);
    TVec3f velocity(vertical);
    if (!MR::isInWater(mTargetPipe->mPosition)) velocity.add(horizontal);
    MR::startLevelSoundPlayer("JumpMM", -1, -1);
    MR::startLevelSoundPlayer("JumpObj", -1, -1);
    if (quick) MR::startBckPlayerJ("土管クイックジャンプ");
    else MR::startBckPlayer("EarthenPipeJump", nullptr);
    MR::setPlayerFrontTargetVec(front, 1);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, velocity);
    MR::setPlayerFootState(5);
    mHostActor = nullptr;
    mTargetPipe->tryHideDown();
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
            entry->_0->mTargetPipe = pPipe;
            entry->_4->mTargetPipe = entry->_0;
            return;
        }
    }

    EarthenPipeMediator::Entry* entry = &mPipeEntries[mNumEntries];
    entry->_0 = pPipe;
    entry->mPipeID = pipeID;
    mNumEntries++;
}
