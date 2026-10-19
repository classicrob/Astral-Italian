#include "Game/MapObj/PowerStarColor.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/Util.hpp"
#include <cstdio>
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PowerStarAppearPoint.hpp"
#include "Game/MapObj/PowerStarHolder.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/NativeActorObserve.hpp"
#ifdef GALAXY_NATIVE
#include <petari/milestone.hpp>
#include <petari/progress_hook.hpp>
#endif
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

const GXColor lightColor[] = {
    {0x96, 0x96, 0x32, 0}, {0x96, 0x96, 0x32, 0}, {0x32, 0x96, 0x32, 0}, {0x96, 0x32, 0x32, 0}, {0x64, 0x64, 0x64, 0},
};

namespace {
    const char* const cAppearDemoName = "パワースター出現";
    const char* const cStageClearAnimNameKoopaVs3 = "GrandStarGetKoopaVs3";

    void setupColor(LiveActor* pActor, bool isBronze, int frame) {
        MR::startBtp(pActor, "PowerStarColor");
        MR::startBrk(pActor, "PowerStarColor");
        MR::startBtk(pActor, "PowerStarColor");
        MR::setBtpFrameAndStop(pActor, frame);
        MR::setBrkFrameAndStop(pActor, frame);
        MR::setBtkFrameAndStop(pActor, frame);
        MR::startBva(pActor, "PowerStarColor");
        MR::setBvaFrameAndStop(pActor, isBronze ? 1.0f : 0.0f);
    }

    void setupColorGrandStar(LiveActor* pActor, bool isBronze, int frame) {
        MR::hideMaterial(pActor, "FooMat");
        MR::hideMaterial(pActor, "GrandStarBronze");
        MR::hideMaterial(pActor, "GrandStarEmpty");

        if (isBronze) {
            MR::showMaterial(pActor, "GrandStarEmpty");
        } else if (frame == 1) {
            MR::showMaterial(pActor, "GrandStarBronze");
        } else {
            MR::showMaterial(pActor, "FooMat");
        }
    }

    void requestPointLight(const LiveActor* pActor, int a2) {
        TVec3f jointPos;

        MR::copyJointPos(pActor, "PowerStar", &jointPos);
        MR::requestPointLight(pActor, TVec3f(jointPos), lightColor[a2], 1.0f, -1.0f, -1);
    }

    DemoObj* createDemoObjCommon(const char* pName, const char* pModelName, const char* pFuncName, const char* pAnimArcName, MtxPtr pMtx) {
        DemoObj* pObj;

        if (pAnimArcName != nullptr) {
            if (pFuncName != nullptr) {
                pObj = MR::createDemoObjWithAnimArc(pName, pModelName, pAnimArcName, pFuncName, pMtx);
            } else {
                pObj = MR::createDemoObjWithAnimArc(pName, pModelName, pAnimArcName, pMtx);
            }
        } else {
            if (pFuncName != nullptr) {
                pObj = MR::createDemoObj(pName, pModelName, pFuncName, pMtx);
            } else {
                pObj = MR::createDemoObj(pName, pModelName, pMtx);
            }
        }

        MR::invalidateClipping(pObj);
        pObj->makeActorDead();
        return pObj;
    }

    const char* makeBronzeName(char* pBuf, u32 size, const char* pBase, s32 color, bool isGrandStar) {
        if (color == 1 && !isGrandStar) {
            snprintf(pBuf, size, "%sBronze", pBase);
            return pBuf;
        }

        return pBase;
    }

    const char* makeColorName(char* pBuf, u32 size, const char* pBase, s32 color, bool isGrandStar) {
        if (color == 2) {
            snprintf(pBuf, size, "%sGreen", pBase);
            return pBuf;
        }

        return makeBronzeName(pBuf, size, pBase, color, isGrandStar);
    }

    inline void appearAndStartAction(LiveActor* pActor, const char* pName) {
        pActor->makeActorAppeared();
        MR::startAction(pActor, pName);
    }
};  // namespace

namespace NrvPowerStar {
    NEW_NERVE(PowerStarNrvWaitStartAppear, PowerStar, WaitStartAppear);
    NEW_NERVE(PowerStarNrvAppearDemoRise, PowerStar, AppearDemoRise);
    NEW_NERVE(PowerStarNrvAppearDemoMove, PowerStar, AppearDemoMove);
    NEW_NERVE(PowerStarNrvAppearDemoBossFormer, PowerStar, AppearDemoBossFormer);
    NEW_NERVE(PowerStarNrvAppearDemoBossLatter, PowerStar, AppearDemoBossLatter);
    NEW_NERVE(PowerStarNrvWait, PowerStar, Wait);
    NEW_NERVE(PowerStarNrvStageClearDemo, PowerStar, StageClearDemo);
};  // namespace NrvPowerStar

PowerStar::PowerStar(const char* pName)
    : LiveActor(pName), mPowerStarId(-1), mIsInDemo(false), mAppearPosition(gZeroVec), mInitRotation(gZeroVec), mInitPosition(gZeroVec),
      mPowerStarModelObj(nullptr), mPeachModel(nullptr), mTicoBabyModel(nullptr), mSupportTicoModel(nullptr), _12C(false), _12D(false), mColor(0),
      mIsGrandStar(false), _136(false), _137(false), mIsBaseFromAppearPoint(false), mCameraInfo(nullptr), mAppearPoint(nullptr), mIsUseAwakeSwitch(false) {
    mPostureMtx.identity();
    mBaseMtx.identity();
    _144.identity();
}

void PowerStar::init(const JMapInfoIter& rIter) {
    mIsGrandStar = MR::isEqualObjectName(rIter, "GrandStar");
    mIsGreenStar = MR::isEqualObjectName(rIter, "GreenStar");
    initNerve(GET_NERVE(PowerStar, PowerStarNrvWait), 0);
    initMapToolInfo(rIter);
    initModel(rIter);

    if (mIsGrandStar) {
        initSensorGrandStar();
    } else {
        initHitSensor(1);
        MR::addHitSensor(this, "Body", ATYPE_POWER_STAR_BIND, 8, 75.0f, TVec3f(0.0f, 0.0f, 0.0f));
    }

    initShadow(rIter);
    MR::initActorCamera(this, rIter, &mCameraInfo);
    MR::initAnimCamera(mPowerStarModelObj, mCameraInfo, getStarGetAnimCameraName());
    ModelObj* pPowerStarModel = mPowerStarModelObj;
    ActorCameraInfo* pCameraInfo = mCameraInfo;
    MR::initAnimCamera(pPowerStarModel, pCameraInfo, mIsGrandStar ? "YoshiGrandStarGet" : "YoshiPowerStarGet");

    if (MR::isStageScoreAttack()) {
        MR::registerDemoSimpleCastAll(this);
    }

    MR::tryRegisterNamePosLinkObj(this, rIter, "スターゲットデモ座標");

    if (mPowerStarId == -1) {
        if (mIsInDemo) {
            makeActorDead();
        } else if (mIsUseAwakeSwitch) {
            makeActorDead();
        } else {
            makeActorAppeared();
        }
    } else {
        makeActorAppeared();
    }
}

void PowerStar::initAfterPlacement() {
}

void PowerStar::appear() {
    LiveActor::appear();
    MR::invalidateClipping(this);
    requestAppearOrWait();
    MR::stopGalaxyCometEvent();
}

void PowerStar::makeActorAppeared() {
    mInitPosition.set(mPosition);
    initPosture();
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE(PowerStar, PowerStarNrvWait));
}

void PowerStar::requestAppear() {
    MR::stopGalaxyCometEvent();
    makeActorAppeared();
    MR::invalidateClipping(this);
    MR::hideModel(this);
    MR::invalidateHitSensors(this);
    setNerve(GET_NERVE(PowerStar, PowerStarNrvWaitStartAppear));
    const Nerve* pNerve = GET_NERVE(PowerStar, PowerStarNrvAppearDemoRise);

    if (_12D) {
        pNerve = GET_NERVE(PowerStar, PowerStarNrvAppearDemoBossFormer);
    }

    MR::requestStartDemoWithoutCinemaFrame(this, ::cAppearDemoName, pNerve, nullptr);
}

void PowerStar::setDemoAppearPos(const TVec3f& rVec) {
    bool isGroupExist = MR::getGroupFromArray(this);

    if (isGroupExist) {
        mAppearPoint = getNearestAppearPoint(rVec);
        mAppearPosition.set(mAppearPoint->mPosition);
    }

    mPosition.set(rVec);
}

bool PowerStar::isEndAppearDemo() const {
    return isNerve(GET_NERVE(PowerStar, PowerStarNrvWait)) || isNerve(GET_NERVE(PowerStar, PowerStarNrvStageClearDemo));
}


void PowerStar::offAppearDemo() {
    mIsInDemo = false;
}

void PowerStar::setAppearFromBelow() {
    _12D = true;
    TVec3f pos;
    TVec3f offs(mGravity);
    offs.scale(-350.0f);
    pos.sub(mAppearPosition, offs);
    setDemoAppearPos(pos);
}

s32 PowerStar::setupColor(LiveActor* pActor, const NameObj* a2, s32 a3) {
    bool isBronze = false;
    s32 color = 0;

    if (MR::isProductGameScene()) {
        color = PowerStarColor::calcCurrentStage(a2, a3, &isBronze);
    }

    if (MR::isGrandStarInCurrentStageWithDeclarer(a2->mName, a3)) {
        ::setupColorGrandStar(pActor, isBronze, color);
    } else {
        ::setupColor(pActor, isBronze, color);
    }

    return color;
}

s32 PowerStar::setupColorAtResultSequence(LiveActor* pPowerStar, bool a2) {
    bool isBronze = false;
    s32 color = PowerStarColor::calcResult(&isBronze);

    if (a2) {
        ::setupColorGrandStar(pPowerStar, isBronze, color);
    } else {
        ::setupColor(pPowerStar, isBronze, color);
    }

    return color;
}

void PowerStar::requestPointLight(const LiveActor* pActor, const NameObj* a2, s32 a3) {
    s32 colorIdx = 0;

    if (MR::isProductGameScene()) {
        colorIdx = PowerStarColor::calcCurrentStage(a2, a3, nullptr);
    }

    ::requestPointLight(pActor, colorIdx);
}

void PowerStar::requestPointLightAtResultSequence(const LiveActor* pActor) {
    ::requestPointLight(pActor, PowerStarColor::calcResult(nullptr));
}

f32 PowerStar::getPowerStarWaitRotateSpeed() {
    return 3.0f;
}

s32 PowerStar::getBtpFrameCurrentStage(s32 a1) {
    if (MR::isPowerStarGreenInCurrentStage(a1)) {
        return 2;
    }

    return MR::isPowerStarBronzeInCurrentStage(a1);
}

bool PowerStar::isCurrentStageKoopaVs3() {
    return MR::isStageKoopaVs3();
}

void PowerStar::initShadowPowerStar(LiveActor* pActor, bool isCylinder, f32 dropLength) {
    MR::invalidateShadowAll(pActor);
    MR::validateShadowGroup(pActor, isCylinder ? "円柱" : "通常");

    if (0.0f < dropLength) {
        MR::setShadowDropLength(pActor, isCylinder ? "本体(楕円柱)" : "本体", dropLength);
    }
}

void PowerStar::emitColorEffect(LiveActor* pActor, const char* pName, s32 color, bool isGrandStar) {
    char buf[0x40];
    MR::emitEffect(pActor, ::makeBronzeName(buf, sizeof(buf), pName, color, isGrandStar));
}

void PowerStar::control() {
#ifdef GALAXY_NATIVE
    {
        s32 state = 0;
        if (isNerve(GET_NERVE(PowerStar, PowerStarNrvWait))) {
            state = 1;
        } else if (isNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoRise)) || isNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoMove)) ||
                   isNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoBossFormer)) ||
                   isNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoBossLatter))) {
            state = 2;
        } else if (isNerve(GET_NERVE(PowerStar, PowerStarNrvStageClearDemo))) {
            state = 3;
        }
        MR::Native::publishActor("PowerStar", mPosition, -mGravity, state, state == 1 ? PETARI_ACTOR_READY : 0u);
    }
#endif
    if (!isNerve(GET_NERVE(PowerStar, PowerStarNrvWaitStartAppear))) {
        TVec3f jointPos;

        if (isNerve(GET_NERVE(PowerStar, PowerStarNrvStageClearDemo))) {
            MR::copyJointPos(mPowerStarModelObj, "PowerStar", &jointPos);
        } else {
            MR::copyJointPos(this, "PowerStar", &jointPos);
        }

        MR::requestPointLight(this, TVec3f(jointPos), lightColor[mColor], 1.0f, -1.0f, -1);
    }
}

void PowerStar::calcAndSetBaseMtx() {
    TPos3f mtx2;
    TPos3f mtx;
    // FIXME
    mtx.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), MR::toRadian(mRotation.y));

    mtx2.concat(mPostureMtx, mtx);
    mtx2.setTrans(mPosition);
    MR::setBaseTRMtx(this, mtx2);
}

bool PowerStar::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    return MR::isMsgJetTurtleAttack(msg);
}

bool PowerStar::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgIsRushTakeOver(msg)) {
        return true;
    }

    if (MR::isMsgAutoRushBegin(msg) && isNerve(GET_NERVE(PowerStar, PowerStarNrvWait)) && MR::getPlayerLife() > 0) {
        MR::stopAllSe(false);
        MR::stopSoundPlayer("SE_PV_BURN_RUN", 0);
        MR::stopSoundPlayer("SE_PV_NEEDLE_DAMAGE_RUN", 0);
        MR::startActionSound(this, "SyStarGet", -1, -1, -1);

        if (mIsGrandStar) {
            MR::requestGrandStarGetDemo();
        } else {
            MR::requestPowerStarGetDemo();
        }

        MR::endGalaxyCometEvent();

        if (mPowerStarId > 0) {
            GameSequenceFunction::setClearedPowerStarNum(mPowerStarId);
        }

        MR::makeMtxTR((MtxPtr)&mBaseMtx, *MR::getPlayerPos(), *MR::getPlayerRotate());
        setNerve(GET_NERVE(PowerStar, PowerStarNrvStageClearDemo));
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx((MtxPtr)&mBaseMtx);
        return true;
    }

    return false;
}

void PowerStar::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mPowerStarId);
    MR::getJMapInfoArg1NoInit(rIter, &_136);
    MR::getJMapInfoArg2NoInit(rIter, &_137);
    MR::getJMapInfoArg4NoInit(rIter, &mIsBaseFromAppearPoint);

    if (!mIsGreenStar) {
        mIsUseAwakeSwitch = MR::useStageSwitchAwake(this, rIter);
    }

    MR::registerPowerStar(this, mPowerStarId);
    MR::joinToGroupArray(this, rIter, "パワースター出現ポイントグループ", 0x10);

    if (mPowerStarId > 0 && MR::isProductGameScene()) {
        if (!MR::hasPowerStarInCurrentStage(mPowerStarId)) {
            if (GameSequenceFunction::isSuperDreamer()) {
                mColor = 1;
            } else {
                s32 color = 0;

                if (mIsGreenStar) {
                    color = 2;
                }

                mColor = color;
            }
        } else if (MR::isPowerStarBronzeInCurrentStage(mPowerStarId) && !GameSequenceFunction::isSuperDreamer()) {
            mColor = 0;
        } else {
            mColor = 4;
        }
    }
}

void PowerStar::initModel(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, isGrandStar() ? "GrandStar" : "PowerStar", false);
    mAppearPosition.set(mPosition);
    mInitRotation.set(mRotation);

    if (mIsGrandStar) {
        ::setupColorGrandStar(this, static_cast<const volatile s32&>(mColor) == 4, static_cast<const volatile s32&>(mColor));
    } else {
        ::setupColor(this, static_cast<const volatile s32&>(mColor) == 4, static_cast<const volatile s32&>(mColor));
    }

    if (mIsGrandStar) {
        MtxPtr pMtx = (MtxPtr)&mBaseMtx;
        const char* pName = "グランドスターデモモデル";

        mPowerStarModelObj = MR::createModelObjIndirectMapObj(pName, mIsGrandStar ? "GrandStar" : "PowerStar", pMtx);
        ::setupColorGrandStar(mPowerStarModelObj, static_cast<const volatile s32&>(mColor) == 4, static_cast<const volatile s32&>(mColor));
    } else {
        MtxPtr pMtx = (MtxPtr)&mBaseMtx;
        const char* pName = "パワースターデモモデル";

        mPowerStarModelObj = MR::createModelObjNoSilhouettedMapObj(pName, mIsGrandStar ? "GrandStar" : "PowerStar", pMtx);
        ::setupColor(mPowerStarModelObj, static_cast<const volatile s32&>(mColor) == 4, static_cast<const volatile s32&>(mColor));
    }

    MR::invalidateClipping(mPowerStarModelObj);
    mPowerStarModelObj->makeActorDead();

    mSupportTicoModel = ::createDemoObjCommon("デモ用サポートチコ", "SupportTico", nullptr, nullptr, (MtxPtr)&mBaseMtx);

    bool isKoopaVs3 = false;
    if (mIsGrandStar && MR::isStageKoopaVs3()) {
        isKoopaVs3 = true;
    }

    if (isKoopaVs3) {
        mPeachModel = ::createDemoObjCommon("デモ用ピーチ", "Peach", "Demo", "PeachAnim", (MtxPtr)&mBaseMtx);
        mTicoBabyModel = ::createDemoObjCommon("デモ用ベビチコ", "TicoBaby", nullptr, "TicoBabyAnimVsKoopaLv3Galaxy", (MtxPtr)&mBaseMtx);
    }

    MR::tryCreateMirrorActor(this, isGrandStar() ? "GrandStar" : "PowerStar");
}

void PowerStar::initSensorGrandStar() {
    initHitSensor(6);
    TVec3f body1Offs = TVec3f(0.0f, 100.0f, 0.0f);
    const char* body1Name = "body1";
    MR::addHitSensorMtx(this, body1Name, ATYPE_POWER_STAR_BIND, 8, 60.0f, getBaseMtx(), body1Offs);
    TVec3f body2Offs = TVec3f(-95.0f, 35.0f, 0.0f);
    const char* body2Name = "body2";
    MR::addHitSensorMtx(this, body2Name, ATYPE_POWER_STAR_BIND, 8, 60.0f, getBaseMtx(), body2Offs);
    TVec3f body3Offs = TVec3f(-55.0f, -75.0f, 0.0f);
    const char* body3Name = "body3";
    MR::addHitSensorMtx(this, body3Name, ATYPE_POWER_STAR_BIND, 8, 60.0f, getBaseMtx(), body3Offs);
    TVec3f body4Offs = TVec3f(55.0f, -75.0f, 0.0f);
    const char* body4Name = "body4";
    MR::addHitSensorMtx(this, body4Name, ATYPE_POWER_STAR_BIND, 8, 60.0f, getBaseMtx(), body4Offs);
    TVec3f body5Offs = TVec3f(95.0f, 35.0f, 0.0f);
    const char* body5Name = "body5";
    MR::addHitSensorMtx(this, body5Name, ATYPE_POWER_STAR_BIND, 8, 60.0f, getBaseMtx(), body5Offs);
    TVec3f body6Offs = TVec3f(0.0f, 0.0f, 0.0f);
    const char* body6Name = "body6";
    MR::addHitSensorMtx(this, body6Name, ATYPE_POWER_STAR_BIND, 8, 80.0f, getBaseMtx(), body6Offs);
}

void PowerStar::initShadow(const JMapInfoIter& rIter) {
    if (!mIsGrandStar) {
        bool isCylinder = false;
        MR::getJMapInfoArg3NoInit(rIter, &isCylinder);
        f32 dropLength = -1.0f;
        MR::getJMapInfoArg5NoInit(rIter, &dropLength);
        initShadowPowerStar(this, isCylinder, dropLength);
    } else {
        MR::setShadowDropDirectionPtrAll(this, &mGravity);
    }
}

void PowerStar::initPosture() {
    MR::calcGravity(this, mAppearPosition);
    TPos3f mtx;
    MR::makeMtxRotate(mtx, mInitRotation);

    if (!_137) {
        TVec3f zDir;
        mtx.getZDir(zDir);

        TVec3f up = -mGravity;
        if (!MR::isSameDirection(up, zDir)) {
            MR::makeMtxUpFront(&mPostureMtx, up, zDir);
        } else {
            MR::makeMtxUpNoSupport(&mPostureMtx, up);
        }
    } else {
        mPostureMtx.set(mtx);
    }

    mPostureMtx.zeroTrans();
    mRotation.y = 0.0f;
}

// NONMATCHING: literal-pool load order for mColor vs "Light" pool base (lwz/lis scheduling); same class of diff in exeWait and exeStageClearDemo
void PowerStar::startAppearDemoCommon() {
    MR::showModelIfHidden(this);

    if (MR::isStagePlayStarChance(mPowerStarId)) {
        MR::startStarChanceBGM2(false);
    }

    MR::moveVolumeStageBGM(0.0f, 5);
    MR::moveVolumeSubBGM(0.0f, 5);
    MR::startActorCameraTargetOther(getAppearCameraActor(), getAppearCameraInfo(), CameraTargetArg(this), 0);
    mPosition.set(mInitPosition);
    char buf[0x40];
    MR::emitEffect(this, ::makeBronzeName(buf, sizeof(buf), "Light", static_cast<const volatile s32&>(mColor), mIsGrandStar));
}

void PowerStar::endAppearDemo() {
    MR::resetCameraLocalOffset();
    MR::endActorCamera(getAppearCameraActor(), getAppearCameraInfo(), false, 0);

    if (!_12C) {
        MR::endDemo(this, ::cAppearDemoName);
    }

    MR::moveVolumeStageBGM(1.0f, 60);
    MR::moveVolumeSubBGM(1.0f, 60);
    setNerve(GET_NERVE(PowerStar, PowerStarNrvWait));
}

PowerStarAppearPoint* PowerStar::getNearestAppearPoint(const TVec3f& rPos) const {
    LiveActorGroup* pGroup = MR::getGroupFromArray(this);
    f32 minDist = FLOAT_MAX;
    PowerStarAppearPoint* pNearestAppearPoint = nullptr;

    for (int i = 0; i < pGroup->getObjNum(); i++) {
        LiveActor* pActor = pGroup->getActor(i);

        if (pActor != this) {
            f32 dist = rPos.distance(pActor->mPosition);

            if (dist < minDist) {
                pNearestAppearPoint = static_cast< PowerStarAppearPoint* >(pActor);
                minDist = dist;
            }
        }
    }

    return pNearestAppearPoint;
}

LiveActor* PowerStar::getAppearCameraActor() {
    if (mAppearPoint == nullptr) {
        return this;
    }

    return mAppearPoint;
}

ActorCameraInfo* PowerStar::getAppearCameraInfo() const {
    if (mAppearPoint == nullptr) {
        return mCameraInfo;
    }

    return mAppearPoint->mCameraInfo;
}

void PowerStar::requestAppearOrWait() {
    if (mIsInDemo) {
        if (_12C) {
            MR::invalidateClipping(this);
            MR::requestMovementOn(this);

            if (_12D) {
                setNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoBossFormer));
            } else {
                setNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoRise));
            }
        } else {
            requestAppear();
        }
    } else {
        if (MR::isStagePlayStarChance(mPowerStarId)) {
            MR::startStarChanceBGM2(true);
        }

        setNerve(GET_NERVE(PowerStar, PowerStarNrvWait));
    }
}

const char* PowerStar::getStarGetAnimCameraName() const {
    if (mIsGrandStar && MR::isStageKoopaVs3()) {
        return ::cStageClearAnimNameKoopaVs3;
    }

    return mIsGrandStar ? "GrandStarGet" : "PowerStarGet";
}

void PowerStar::calcAppearDemoRiseTrans(TVec3f* pOutTrans, f32 a2) const {
    if (_136) {
        MR::calcGravityVector(this, mInitPosition, pOutTrans, 0, 0);
        pOutTrans->scale(-a2);
    } else {
        TVec3f vec;
        vec.x = 0.0f;
        vec.y = a2;
        vec.z = 0.0f;
        mPostureMtx.mult(vec, *pOutTrans);
    }

    pOutTrans->add(mInitPosition);
}

void PowerStar::applyAppearDemoRiseTrans(const TVec3f& rRiseTrans, f32 minHeight, f32 step) {
    mPosition.lerp(rRiseTrans, mAppearPosition, step);

    TVec3f vec2;
    MR::vecKillElement(rRiseTrans, mGravity, &vec2);

    TVec3f vec3;
    MR::vecKillElement(mAppearPosition, mGravity, &vec3);

    f32 val = vec2.distance(vec3) * 0.5f;
    f32 val2 = MR::clamp(val, minHeight, 5000.0f);

    TVec3f zVec(0.0f, 0.0f, 0.0f);
    zVec.y = MR::sin(step * PI) * val2;
    mPostureMtx.mult(zVec, zVec);
    mPosition.add(zVec);
}

void PowerStar::endAppearDemoMove() {
    mPosition.set(mAppearPosition);

    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::startActionSound(this, "OjPowStarMoveEndW", -1, -1, -1);
    } else {
        MR::startActionSound(this, "OjPowStarMoveEnd", -1, -1, -1);
    }
}

void PowerStar::processWait(f32 val) {
    mRotation.y = MR::repeat(mRotation.y + val, 0.0f, 360.0f);

    if (mIsGrandStar) {
        if (MR::changeShowModelFlagSyncNearClipping(this, 250.0f)) {
            MR::onDrawEffect(this);
        } else {
            MR::offDrawEffect(this);
        }
    }

    if (!MR::isPowerStarGetDemoActive()) {
        if (mIsGreenStar) {
            MR::startActionSound(this, "ExistGreen", -1, -1, -1);
        } else {
            startExistLevelSound();
        }
    }
}

void PowerStar::calcStageClearDemoMtx() {
    if (!MR::tryFindLinkNamePos(this, "スターゲットデモ座標", (MtxPtr)&mBaseMtx)) {
        if (mIsBaseFromAppearPoint) {
            MR::makeMtxTR((MtxPtr)&mBaseMtx, mAppearPoint);
            mBaseMtx.zeroTrans();
        } else {
            mBaseMtx.set(mPostureMtx);
        }

        mBaseMtx.setTrans(mPosition);
    }
}

void PowerStar::startExistLevelSound() {
    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::startActionSound(this, "OjLvPowStarExistW", -1, -1, -1);
    } else {
        MR::startActionSound(this, "OjLvPowStarExist", -1, -1, -1);
    }
}

void PowerStar::exeWaitStartAppear() {
}

void PowerStar::exeAppearDemoRise() {
    if (MR::isFirstStep(this)) {
        startAppearDemoCommon();
    }

    startExistLevelSound();

    f32 easeOut = MR::calcNerveEaseOutValue(this, 60, 0.0f, 300.0f);
    calcAppearDemoRiseTrans(&mPosition, easeOut);
    mRotation.y = MR::repeat(mRotation.y + 10.0f, 0.0f, 360.0f);

    if (MR::isStep(this, 80)) {
        setNerve(GET_NERVE(PowerStar, PowerStarNrvAppearDemoMove));
    }
}

void PowerStar::exeAppearDemoMove() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "SyPowStarAppear", -1, -1, -1);
    }

    f32 rotSpeed = 20.0f;

    if (MR::isLessStep(this, 120)) {
        f32 step = getNerveStep() / 120.0f;
        TVec3f vec;
        calcAppearDemoRiseTrans(&vec, 300.0f);
        applyAppearDemoRiseTrans(vec, 300.0f, step);
    } else if (MR::isStep(this, 120)) {
        endAppearDemoMove();
    } else {
        startExistLevelSound();
        rotSpeed = MR::getEaseOutValue((f32)(getNerveStep() - 120) / 90.0f, 20.0f, mIsGrandStar ? 2.0f : 3.0f, 1.0f);
    }

    mRotation.y = MR::repeat(mRotation.y + rotSpeed, 0.0f, 360.0f);

    if (MR::isStep(this, 210)) {
        endAppearDemo();
    }
}

void PowerStar::exeAppearDemoBossFormer() {
    if (MR::isFirstStep(this)) {
        startAppearDemoCommon();
        MR::startActionSound(this, "AppearBossFormer", -1, -1, -1);
    }

    if (MR::isStep(this, 172)) {
        MR::startActionSound(this, "AppearBossLatter", -1, -1, -1);
    }

    applyAppearDemoRiseTrans(mInitPosition, 700.0f, 0.5f * MR::calcNerveEaseOutRate(this, 150));
    f32 rotY = mRotation.y;
    f32 rotAdd = rotY;
    rotAdd += 15.0f;
    mRotation.y = MR::repeat(rotAdd, 0.0f, 360.0f);
    MR::setNerveAtStep(this, GET_NERVE(PowerStar, PowerStarNrvAppearDemoBossLatter), 150);
}

void PowerStar::exeAppearDemoBossLatter() {
    f32 rotSpeed = 15.0f;

    if (MR::isStep(this, 22)) {
        MR::startActionSound(this, "AppearBossLatter", -1, -1, -1);
    }

    if (MR::isLessStep(this, 150)) {
        applyAppearDemoRiseTrans(mInitPosition, 700.0f, 0.5f + 0.5f * MR::calcNerveEaseOutRate(this, 150));
    } else if (MR::isStep(this, 150)) {
        endAppearDemoMove();
    } else {
        startExistLevelSound();
        rotSpeed = MR::getEaseOutValue((f32)(getNerveStep() - 150) / 90.0f, 15.0f, mIsGrandStar ? 2.0f : 3.0f, 1.0f);
    }

    mRotation.y = MR::repeat(mRotation.y + rotSpeed, 0.0f, 360.0f);

    if (MR::isStep(this, 240)) {
        endAppearDemo();
    }
}

// NONMATCHING: lwz of mColor scheduled after the "Light" pool lis in target; ours loads it first (scheduling, tried hoisting)
void PowerStar::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::validateHitSensors(this);
        char buf[0x40];
        const char* pName = "Light";
        MR::emitEffect(this, ::makeColorName(buf, sizeof(buf), pName, static_cast<const volatile s32&>(mColor), mIsGrandStar));
    }

    processWait(mIsGrandStar ? 2.0f : 3.0f);
}

// NONMATCHING: effect argument scheduling and persistent register allocation differ.
void PowerStar::exeStageClearDemo() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);

        if (mIsGreenStar) {
            RaceManagerFunction::killRace();
        }

        const char* pName = getStarGetAnimCameraName();

        if (MR::isPlayerElementModeYoshi()) {
            pName = mIsGrandStar ? "YoshiGrandStarGet" : "YoshiPowerStarGet";
        }

        MR::startAnimCameraTargetSelf(mPowerStarModelObj, mCameraInfo, pName, 0, false, 1.0f);
    }

    if (MR::isStep(this, 1)) {
        calcStageClearDemoMtx();

        char buf[0x40];
        MR::forceDeleteEffect(this, ::makeColorName(buf, sizeof(buf), "Light", static_cast<const volatile s32&>(mColor), mIsGrandStar));
        MR::hideModelIfShown(this);
        MR::startBckPlayer(getStarGetAnimCameraName(), nullptr);
        ::appearAndStartAction(mPowerStarModelObj, getStarGetAnimCameraName());

        bool isGrandStar = mIsGrandStar;
        const char* pBaseName = isGrandStar ? "GrandStarGet" : "PowerStarGet";
        ModelObj* pModel = mPowerStarModelObj;
        char buf2[0x40];
        MR::emitEffect(pModel, ::makeBronzeName(buf2, sizeof(buf2), pBaseName, mColor, isGrandStar));

        bool isSupportPlay = MR::isSupportPlayActive();
        bool isKoopaVs3 = false;
        if (mIsGrandStar && MR::isStageKoopaVs3()) {
            isKoopaVs3 = true;
        }

        if (isKoopaVs3) {
            ::appearAndStartAction(mPeachModel, ::cStageClearAnimNameKoopaVs3);
            ::appearAndStartAction(mTicoBabyModel, ::cStageClearAnimNameKoopaVs3);

            if (isSupportPlay) {
                ::appearAndStartAction(mSupportTicoModel, ::cStageClearAnimNameKoopaVs3);
            }
        } else if (isSupportPlay) {
            ::appearAndStartAction(mSupportTicoModel, getStarGetAnimCameraName());
        }

        MR::startStarPointerModePowerStarGetDemo(this);
        MR::sendMsgToAllLiveActor(ACTMES_START_POWER_STAR_GET, this);
    }
}

PowerStar::~PowerStar() {
}
