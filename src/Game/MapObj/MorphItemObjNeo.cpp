#pragma auto_inline off
#include "Game/MapObj/MorphItemObjNeo.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace {
    const char* const cEffectKeeperName = "MorphItemObj";
    const char* const cSoundName = "MorphItemObjNeo";
};  // namespace

static inline TVec3f makeZeroVec() {
    TVec3f v;
    v.setAll< f32 >(0.0f);
    return v;
}

MorphItemObjNeo::MorphItemObjNeo(const char* pName)
    : LiveActor(pName), mAppearMode(0), mAppearFrames(0), mIsUseSwitch(true), mIsFloating(false), _92(false), mCameraMode(0), mRotateMode(0), _95(0),
      mContainerType(0), _97(0), mIsBind(0), mShadowType(0), mAngle(0.0f), mSpawnPosition(makeZeroVec()), mActorCameraInfo(nullptr),
      mContainerModel(nullptr), mCameraTargetMtx(nullptr), mRailMover(nullptr), mRailClipping(0.0f, 0.0f, 0.0f), mFallPos(0.0f, 0.0f, 0.0f) {
}

inline bool MorphItemObjNeo::isPowerUp(int type) {
    return getPowerUp() == type;
}

void MorphItemObjNeo::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    mSpawnPosition = mPosition;

    MR::initActor(this, rIter, getMorphModelName(getPowerUp()), false);

    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x4A /* ATYPE_MORPH_ITEM */, 4, 80.0f, TVec3f(0.0f, 60.0f, 0.0f));

    MR::initStarPointerTargetAtPos(this, 80.0f, &mPosition, TVec3f(0.0f, 60.0f, 0.0f));

    initBinder(70.0f, 60.0f, 0);
    MR::offBind(this);

    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);

    s32 arg1 = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg1);

    s32 arg2 = -1;
    MR::getJMapInfoArg2NoInit(rIter, &arg2);

    s32 arg3 = -1;
    MR::getJMapInfoArg3NoInit(rIter, &arg3);

    s32 arg4 = -1;
    MR::getJMapInfoArg4NoInit(rIter, &arg4);

    s32 arg5 = -1;
    MR::getJMapInfoArg5NoInit(rIter, &arg5);

    s32 arg6 = -1;
    MR::getJMapInfoArg6NoInit(rIter, &arg6);

    s32 arg7 = -1;
    MR::getJMapInfoArg7NoInit(rIter, &arg7);

    _95 = 0;

    if (arg0 == 1) {
        _95 = 1;
    }

    mIsBind = arg1;

    if (mIsBind == 1) {
        MR::onBind(this);
    } else {
        MR::offBind(this);
    }

    mIsFloating = true;

    if (arg2 == 1) {
        mIsFloating = false;
    }

    if (isPowerUp(1)) {
        MR::connectToSceneIndirectMapObjStrongLight(this);
    } else {
        MR::connectToSceneNoSilhouettedMapObjStrongLight(this);
    }

    if (arg3 == 0) {
        mContainerType = 1;

        ModelObj* pModel = new ModelObj("アイテム泡", "ItemBubble", nullptr, -2, -2, -2, false);
        pModel->initWithoutIter();
        MR::setPosition(pModel, mPosition);
        MR::setRotation(pModel, mRotation);

        if (MR::isConnectedWithRail(rIter)) {
            MR::onCalcShadow(pModel, "Body");
        }

        pModel->appear();
        MR::startAction(pModel, "Move");

        mContainerModel = pModel;
    }

    if (MR::isConnectedWithRail(rIter) && mContainerType == 1) {
        initRailRider(rIter);

        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(rIter);
        mRailMover->start();

        MR::moveCoordToNearestPos(this, mPosition);
        MR::initAndSetRailClipping(&mRailClipping, this, 100.0f, 500.0f);
    }

    mShadowType = 0;

    switch (arg4) {
    case -1:
        mShadowType = 1;

        break;
    case 1:
        mShadowType = 2;

        break;
    case 2:
        mShadowType = 3;

        break;
    case 3:
        mShadowType = 4;

        break;
    case 4:
        mShadowType = 5;

        break;
    }

    if (mShadowType != 0) {
        if ((mShadowType & 4) != 0) {
            MR::setClippingTypeSphere(this, 2000.0f);
            mShadowType = 1;
        }

        MR::initShadowVolumeSphere(this, 40.0f);
        MR::setShadowDropLength(this, nullptr, 2000.0f);
        MR::onShadowVolumeCutDropLength(this, nullptr);

        if ((mShadowType & 1) != 0) {
            if ((mShadowType & 2) != 0) {
                MR::onCalcShadowDropPrivateGravity(this, nullptr);
            } else {
                MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);
            }
        } else {
            if ((mShadowType & 2) != 0) {
                MR::onCalcShadow(this, nullptr);
            } else {
                MR::onCalcShadowOneTime(this, nullptr);
            }
        }
    }

    initEffectKeeper(4, ::cEffectKeeperName, false);

    initSound(4, ::cSoundName, nullptr, makeZeroVec());

    mCameraMode = 0;

    if (arg5 > 0) {
        if (arg5 == 1) {
            MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "出現1");
            mCameraMode = 1;
        } else {
            MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "出現1");
            MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "出現2");

            mCameraMode = 2;
        }

        mCameraTargetMtx = new CameraTargetMtx("カメラターゲットダミー");
    }

    mRotateMode = arg6;

    MR::getRotatedAxisZ(&mZRotation, mRotation);

    initNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait));

    MR::validateClipping(this);

    MR::validateHitSensors(this);

    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);

    mIsUseSwitch = MR::useStageSwitchSyncAppear(this, rIter);

    if (isPowerUp(6) && arg7 == 1) {
        makeActorDead();
    } else if (!mIsUseSwitch) {
        makeActorAppeared();
    }

    MR::tryRegisterDemoCast(this, rIter);
    MR::registerMarioFaceShipNpc(this, rIter);
}

void MorphItemObjNeo::initAfterPlacement() {
    if (mRailMover == nullptr) {
        MR::setClippingRangeIncludeShadow(this, &mRailClipping, 150.0f);
    }
}

void MorphItemObjNeo::makeActorAppeared() {
    if (mContainerModel != nullptr) {
        mContainerModel->makeActorAppeared();
    }

    MR::showModelIfHidden(this);
    LiveActor::makeActorAppeared();
}

void MorphItemObjNeo::makeActorDead() {
    if (mContainerModel != nullptr) {
        mContainerModel->makeActorDead();
    }

    LiveActor::makeActorDead();
}

void MorphItemObjNeo::appear() {
    LiveActor::appear();

    if (mCameraMode != 0) {
        _92 = true;
    }

    mAppearMode = 0;

    if (_95 == 1) {
        mAppearMode = 60;
    }

    bool b1 = false;

    if (mCameraMode != 0 || _95 != 0) {
        b1 = true;

        setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvSwitchAppear));

        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
    } else {
        setNerve(mContainerModel != nullptr ? static_cast< const Nerve* >(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait)) :
                              static_cast< const Nerve* >(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2)));
    }

    _95 = 0;
    mIsUseSwitch = false;

    if (mIsBind != 0) {
        MR::onBind(this);
    }

    if (!b1) {
        emitAppearEffects();
    }

    MR::startAction(this, "AppearFirst");
}

void MorphItemObjNeo::emitAppearEffects() {
}

void MorphItemObjNeo::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    LiveActor::kill();
}

void MorphItemObjNeo::control() {
    updateRailMover();

    if (!isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvFly))) {
        tryStartAppearDemo();
        updateRotateY();
    }
}

void MorphItemObjNeo::calcContainerBaseMtx() {
    ModelObj* pModel = mContainerModel;

    if (pModel != nullptr) {
        MtxPtr pSrc = pModel->getBaseMtx();
        PSMTXCopy(pSrc, getBaseMtx());

        TVec3f yDir;
        reinterpret_cast< TPos3f* >(getBaseMtx())->getYDir(yDir);

        TVec3f offset(yDir);
        offset.scale(-50.0f);
        MR::addTransMtx(getBaseMtx(), offset);

        MtxPtr pMtx = MR::tmpMtxScale(0.8f, 0.8f, 0.8f);
        PSMTXConcat(getBaseMtx(), pMtx, getBaseMtx());
    }
}

// NONMATCHING: unsigned conversion loads and multiply scheduling differ
// NONMATCHING: target reloads mAppearMode (lhz) for the RotAxis angle and orders fmuls operands differently (scheduling/CSE)
void MorphItemObjNeo::calcAndSetBaseMtx() {
    if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait))) {
        if (mContainerType == 1) {
            calcContainerBaseMtx();

            if (mRailMover != nullptr) {
                MtxPtr pMtx = getBaseMtx();
                f32 z = pMtx[2][3];
                f32 y = pMtx[1][3];
                f32 x = pMtx[0][3];
                mPosition.set< f32 >(x, y, z);
            }
        } else {
            LiveActor::calcAndSetBaseMtx();

            PSMTXConcat(getBaseMtx(), MR::tmpMtxRotYDeg(mAngle), getBaseMtx());
        }
    } else {
        LiveActor::calcAndSetBaseMtx();

        PSMTXConcat(getBaseMtx(), MR::tmpMtxRotYDeg(mAngle), getBaseMtx());

        if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvFall))) {
            MtxPtr pMtx = MR::tmpMtxScale(0.8f, 0.8f, 0.8f);
            PSMTXConcat(getBaseMtx(), pMtx, getBaseMtx());
        } else if (mAppearMode != 0) {
            TPos3f rotMtx;
            PSMTXRotAxisRad(rotMtx, mVelocity, mAppearMode * 0.20943952f);

            TVec3f rotVec;
            PSMTXMultVec(rotMtx, mZRotation, rotVec);

            rotVec.setLength(30.0f + 2.0f * mAppearMode);
            MR::addTransMtx(getBaseMtx(), rotVec);
        }
    }
}

bool MorphItemObjNeo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isHiddenModel(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvAppear))) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if ((MR::isMsgPlayerHitAll(msg) || MR::isMsgPlayerHipDrop(msg)) && isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait))) {
        if (mContainerModel != nullptr) {
            if (MR::isMsgFireBallAttack(msg) && mContainerType == 1) {
                setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvFall));
                return false;
            }

            mContainerModel->kill();

            MR::startActionSound(this, "OjItemBubbleBreak");

            if (mIsBind != 0) {
                MR::onBind(this);
            }

            setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2));
            return true;
        }

        return false;
    }

    return false;
}

bool MorphItemObjNeo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x8F) {
        recover();
        return true;
    }

    if (MR::isMsgItemGet(msg)) {
        return tryGetItem(pSender, pReceiver);
    }

    return false;
}

// NONMATCHING: sensor/vector temporary register allocation differs
// NONMATCHING: regalloc (pSender/pReceiver saved-register order) and the resulting psq load order in velocity -= senderPos
bool MorphItemObjNeo::tryGetItem(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isHiddenModel(this)) {
        return false;
    }

    if (MR::isDead(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvAppear))) {
        return false;
    }

    if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait))) {
        if (mContainerType == 1) {
            MR::startActionSound(this, "OjItemBubbleBreak");

            if (mContainerModel != nullptr) {
                mContainerModel->kill();
            }

            setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2));

            if (mIsBind != 0) {
                MR::onBind(this);
            }
        }

        if (mContainerType != 0) {
            return false;
        }
    }

    if (MR::isPlayerElementModeInvincible()) {
        if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2)) ||
            (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait)) && mContainerType == 0)) {
            setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvFly));

            const TVec3f* senderPos = MR::getSensorPos(pSender);
            TVec3f velocity(*MR::getSensorPos(pReceiver));
            velocity -= *senderPos;
            mVelocity = velocity;
            mVelocity.setLength(50.0f);

            MR::calcGravity(this);

            mVelocity += -mGravity * 30.0f;
        }

        return false;
    }

    int powerUp = getPowerUp();
    bool isInvalid = false;

    switch (powerUp) {
    case 4:
        MR::startActionSound(this, "PowerUp2");
        break;
    case 5:
        MR::startActionSound(this, "PowerUp2");
        break;
    case 6:
        MR::startActionSound(this, "PowerUp2");
        break;
    case 7:
        if (!MR::isOnGameEventFlagCloudMarioAtFirst()) {
            MR::startActionSound(this, "PowerUp2");
        } else {
            MR::startActionSound(this, "PowerUp");
        }
        break;
    case 1:
        if (MR::isPlayerElementModeHopper()) {
            isInvalid = true;
        } else if (!MR::isOnGameEventFlagHopperMarioAtFirst()) {
            MR::startActionSound(this, "Morph2");
        } else {
            MR::startActionSound(this, "Morph");
        }
        break;
    case 2:
        if (MR::isPlayerElementModeBee()) {
            isInvalid = true;
        } else if (!MR::isOnGameEventFlagBeeMarioAtFirst()) {
            MR::startActionSound(this, "Morph2");
        } else {
            MR::startActionSound(this, "Morph");
        }
        break;
    case 3:
        if (MR::isPlayerElementModeTeresa()) {
            isInvalid = true;
        } else if (!MR::isOnGameEventFlagTeresaMarioAtFirst()) {
            MR::startActionSound(this, "Morph2");
        } else {
            MR::startActionSound(this, "Morph");
        }
        break;
    case 8:
        if (MR::isPlayerElementModeRock()) {
            isInvalid = true;
        } else if (!MR::isOnGameEventFlagRockMarioAtFirst()) {
            MR::startActionSound(this, "Morph2");
        } else {
            MR::startActionSound(this, "Morph");
        }
        break;
    case 0:
    default:
        MR::startActionSound(this, "PowerUp");
        break;
    }

    if (isInvalid) {
        MR::startActionSound(this, "OjMorphItemInvalid");
    }

    switch (getPowerUp()) {
    case 1:
        MR::changePlayerModeHopper();
        break;
    case 2:
        MR::changePlayerModeBee();
        break;
    case 3:
        MR::changePlayerModeTeresa();
        break;
    case 4:
        MR::changePlayerModeIce();
        break;
    case 5:
        MR::changePlayerModeFire();
        break;
    case 6:
        MR::changePlayerModeFoo();
        break;
    case 8:
        MR::changePlayerModeRock();
        break;
    }

    mAngle = 0.0f;

    calcAndSetBaseMtx();

    vanishByGet();

    return true;
}

void MorphItemObjNeo::resetToSpawn() {
    MR::setPosition(this, mSpawnPosition);
    MR::zeroVelocity(this);

    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    _97 = 0;
}

void MorphItemObjNeo::vanishByGet() {
    MR::deleteEffectAll(this);
    MR::hideModel(this);

    if (mContainerModel != nullptr) {
        mContainerModel->kill();
    }

    MR::invalidateHitSensors(this);

    setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvHide));

    appearTransformation();
}

void MorphItemObjNeo::appearTransformation() {
}

void MorphItemObjNeo::recover() {
    resetToSpawn();

    MR::startAction(this, "Appear");

    if (mIsBind != 1) {
        MR::offBind(this);
    }

    setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait));

    killTransformation();
}

void MorphItemObjNeo::killTransformation() {
}

void MorphItemObjNeo::updateRailMover() {
    if (mRailMover != nullptr) {
        if (!isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvFall))) {
            mRailMover->movement();
        }

        if (mRailMover->isWorking() && isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait))) {
            if (mContainerModel != nullptr) {
                MR::setPosition(mContainerModel, mRailMover->_28);
            }

            mRailMover->tryResetPositionRepeat();

            MR::setSensorPos(this, "Body", mRailMover->_28);

            if (mContainerModel != nullptr) {
                MR::setShadowDropPosition(mContainerModel, "Body", mRailMover->_28);
            }
        }
    }
}

void MorphItemObjNeo::updateRotateY() {
    if (!isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvAppear)) && !isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvSwitchAppear))) {
        if (isNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait))) {
            if (mContainerType == 0 && !mIsUseSwitch) {
                if ((mRotateMode & 1) != 0) {
                    mAngle += 2.0f;
                }
            } else if ((mRotateMode & 2) != 0) {
                mAngle += 2.0f;
            }
        } else if ((mRotateMode & 1) != 0) {
            mAngle += 2.0f;
        }
    }

    mAngle = MR::repeatDegree(mAngle);
}

bool MorphItemObjNeo::tryStartAppearDemo() {
    if (!_92) {
        return false;
    }

    if (!MR::tryStartDemoWithoutCinemaFrame(this, "出現")) {
        return false;
    }

    _92 = false;

    TPos3f* pMtx = &mCameraTargetMtx->mMatrix;
    pMtx->set(getBaseMtx());
    MR::startMultiActorCameraTargetSelf(this, mActorCameraInfo, "出現1", -1);
    mAppearFrames = MR::getMultiActorCameraFrames(this, mActorCameraInfo, "出現1");

    if (mAppearFrames != 0) {
        mAppearFrames--;
    }

    MR::requestMovementOn(this);

    MR::pauseOffCameraDirector();

    return true;
}

void MorphItemObjNeo::exeWait() {
    if (mContainerType == 1) {
        MR::setSensorRadius(this, "Body", 110.0f);
        MR::setSensorOffset(this, "Body", TVec3f(0.0f, 0.0f, 0.0f));
    } else {
        glowEffect(this, getPowerUp());
    }

    if (!isDemo()) {
        MR::zeroVelocity(this);

        if (MR::isInvalidClipping(this)) {
            MR::validateClipping(this);
        }
    }
}

bool MorphItemObjNeo::isDemo() const {
    bool ret = false;

    if (mAppearMode != 0 || mAppearFrames != 0) {
        ret = true;
    }

    return ret;
}

void MorphItemObjNeo::exeWait2() {
    MR::setSensorRadius(this, "Body", 80.0f);
    MR::setSensorOffset(this, "Body", TVec3f(0.0f, 60.0f, 0.0f));

    glowEffect(this, getPowerUp());

    if (!isDemo() && mIsFloating && _97 <= 3) {
        if (MR::isBindedGround(this) && MR::calcGravitySpeed(this) > 0.0f) {
            if (MR::calcVelocityLength(this) < 2.0f) {
                MR::zeroVelocity(this);
                return;
            }

            if (_97 < 3) {
                mVelocity = mVelocity * -0.5f;

                MR::tryStartActionNotPlaying(this, "Land");
            } else {
                MR::zeroVelocity(this);
            }

            _97++;
        } else {
            MR::calcGravity(this);

            mVelocity += mGravity * 0.75f;
        }
    }
}

void MorphItemObjNeo::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::calcGravity(this);
        MR::addVelocityJump(this, 25.0f);

        if (mIsBind != 0) {
            MR::onBind(this);
        }
    }

    setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2));
}

void MorphItemObjNeo::exeSwitchAppear() {
    if (MR::isFirstStep(this)) {
        MR::calcGravity(this);

        if (mAppearMode != 0) {
            MR::startAction(this, "AppearFirst");

            if (isPowerUp(3)) {
                MR::emitEffect(this, "AppearTeresa");
            } else if (isPowerUp(1)) {
                MR::emitEffect(this, "AppearHopper");
            } else {
                MR::emitEffect(this, "AppearBee");
            }

            if (!isPowerUp(4) && !isPowerUp(5)) {
                MR::startActionSound(this, "OjMorphItemAppear");
            }
        } else {
            MR::hideModel(this);
        }

        MR::invalidateHitSensors(this);

        mVelocity = mGravity * -10.0f;
    }

    if (MR::isStep(this, 60)) {
        MR::showModel(this);

        if (isPowerUp(4)) {
            MR::startAction(this, "AppearFirst");
            MR::emitEffect(this, "AppearIceFlower");
            MR::startActionSound(this, "OjMorphFlowerAppear");
        }

        if (isPowerUp(5)) {
            MR::startAction(this, "AppearFirst");
            MR::emitEffect(this, "AppearFireFlower");
            MR::startActionSound(this, "OjMorphFlowerAppear");
        }
    }

    if (mCameraMode != 0 && mAppearFrames != 0 && --mAppearFrames == 0) {
        MR::endMultiActorCamera(this, mActorCameraInfo, "出現1", false, -1);
        MR::endMultiActorCamera(this, mActorCameraInfo, "出現2", false, -1);

        mCameraMode--;

        if (mCameraMode == 0) {
            MR::endDemo(this, "出現");
            MR::validateClipping(this);
        } else {
            TPos3f* pMtx = &mCameraTargetMtx->mMatrix;
            pMtx->set(getBaseMtx());

            MR::startMultiActorCameraTargetOther(this, mActorCameraInfo, "出現2", CameraTargetArg(nullptr, mCameraTargetMtx, nullptr, nullptr), -1);

            mAppearFrames = MR::getMultiActorCameraFrames(this, mActorCameraInfo, "出現2");
        }
    }

    if (mAppearMode != 0) {
        mAppearMode--;
    }

    if (mAppearMode == 0) {
        MR::validateHitSensors(this);
        MR::zeroVelocity(this);
    }

    if (mCameraMode == 0 && mAppearMode == 0) {
        setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait2));
    }
}

void MorphItemObjNeo::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }

    MR::attenuateVelocity(this, 0.999f);
    MR::calcGravity(this);
    MR::addVelocityToGravity(this, 0.2f);

    if (MR::isGreaterEqualStep(this, 300)) {
        MR::validateClipping(this);
        recover();
    }
}

void MorphItemObjNeo::exeFall() {
    if (MR::isFirstStep(this)) {
        if (mRailMover == nullptr) {
            mFallPos.x = mPosition.x;
            mFallPos.y = mPosition.y;
            mFallPos.z = mPosition.z;
            MR::zeroVelocity(this);
        }

        MR::invalidateClipping(this);
        MR::onBind(this);
        MR::addVelocityJump(this, 20.0f);
    }

    MR::attenuateVelocity(this, 0.999f);
    MR::calcGravity(this);
    MR::addVelocityToGravity(this, 0.4f);

    if (mContainerModel != nullptr) {
        TPos3f mtx;
        mtx.set(getBaseMtx());

        TVec3f yDir;
        mtx.getYDir(yDir);

        TVec3f offset(yDir);
        offset.scale(50.0f);

        TVec3f pos(mPosition);
        pos += offset;
        MR::setPosition(mContainerModel, pos);
    }

    if (mRailMover == nullptr || !MR::isGreaterEqualStep(this, 98)) {
        if (0.0f < mGravity.dot(mVelocity) && MR::isNear(this, mFallPos, 10.0f)) {
        } else if (!MR::isBinded(this)) {
            return;
        }
    }

    if (mRailMover == nullptr) {
        MR::setPosition(this, mFallPos);
        MR::setPosition(mContainerModel, mFallPos);
    }

    MR::offBind(this);
    MR::validateClipping(this);
    setNerve(GET_NERVE(MorphItemObjNeo, MorphItemObjNeoNrvWait));
}

void MorphItemObjNeo::endHide() {
    MR::showModel(this);
    MR::validateHitSensors(this);

    if (!MR::isDead(this) && mContainerModel != nullptr) {
        mContainerModel->appear();
    }
}

const char* MorphItemObjNeo::getMorphModelName(int type) {
    const char* names[] = {"-----", "PowerUpHopper", "PowerUpBee", "PowerUpTeresa", "PowerUpIce", "PowerUpFire", "PowerUpFoo", "PowerUpCloud", "PowerUpRock", "END"};
    return names[type];
}

void MorphItemObjNeo::glowEffect(LiveActor* pActor, int type) {
    if (!MR::isHiddenModel(pActor)) {
        switch (type) {
        case 1:
            MR::emitEffect(pActor, "GlowHopper");
            break;
        case 2:
            MR::emitEffect(pActor, "GlowBee");
            break;
        case 3:
            MR::emitEffect(pActor, "GlowTeresa");
            break;
        case 4:
            MR::emitEffect(pActor, "GlowIce");
            break;
        case 5:
            MR::emitEffect(pActor, "GlowFire");
            break;
        case 6:
            MR::emitEffect(pActor, "GlowFoo");
            break;
        case 8:
            MR::emitEffect(pActor, "GlowRock");
            break;
        }
    }
}

void MorphItemNeoHopper::emitAppearEffects() {
    MR::emitEffect(this, "AppearHopper");
    MR::startActionSound(this, "OjMorphItemAppear");
}

void MorphItemNeoFire::emitAppearEffects() {
    MR::emitEffect(this, "AppearFireFlower");
    MR::startActionSound(this, "OjMorphFlowerAppear");
}
