#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Screen/GalaxyNamePlate.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include <cstdio>
#include <cmath>

namespace NrvGalaxyWorldMapInfo {
    class GalaxyWorldMapInfoNrvHide : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static GalaxyWorldMapInfoNrvHide sInstance;
    };
    GalaxyWorldMapInfoNrvHide GalaxyWorldMapInfoNrvHide::sInstance ATTRIBUTE_WEAK;

    class GalaxyWorldMapInfoNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GalaxyWorldMapInfo* pActor = reinterpret_cast<GalaxyWorldMapInfo*>(pSpine->mExecutor);
            if (pActor->mShowEffect) {
                WorldMapFunction::setFogRange(pActor, 0.6f);
            }
        }
        static GalaxyWorldMapInfoNrvWait sInstance;
    };
    GalaxyWorldMapInfoNrvWait GalaxyWorldMapInfoNrvWait::sInstance ATTRIBUTE_WEAK;

    class GalaxyWorldMapInfoNrvNew : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static GalaxyWorldMapInfoNrvNew sInstance;
    };
    GalaxyWorldMapInfoNrvNew GalaxyWorldMapInfoNrvNew::sInstance ATTRIBUTE_WEAK;

    class GalaxyWorldMapInfoNrvOpened : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static GalaxyWorldMapInfoNrvOpened sInstance;
    };
    GalaxyWorldMapInfoNrvOpened GalaxyWorldMapInfoNrvOpened::sInstance ATTRIBUTE_WEAK;

    class GalaxyWorldMapInfoNrvNewOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast<GalaxyWorldMapInfo*>(pSpine->mExecutor)->exeNewOpen();
        }
        static GalaxyWorldMapInfoNrvNewOpen sInstance;
    };
    GalaxyWorldMapInfoNrvNewOpen GalaxyWorldMapInfoNrvNewOpen::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGalaxyWorldMapInfo

namespace {
    TVec3f sTopOffset(0.0f, 2500.0f, 0.0f);
    TVec3f sShadowOffset(0.0f, -2250.0f, 0.0f);
    TVec3f sBossRotation(0.0f, 312.0f, 0.0f);
    TVec3f sNamePlateOffset(0.0f, 1500.0f, 0.0f);
};  // namespace

GalaxyWorldMapInfo::GalaxyWorldMapInfo(WorldMapResultWorld* pWorld, const char* pGalaxyName, const char* pModelName, const char* pType, s32 pointIndex)
    : LiveActor(pGalaxyName), mWorld(pWorld), mGalaxyName(pGalaxyName), mType(pType), mModelName(pModelName), mPointIndex(pointIndex), mKind(0),
      mShadow(nullptr), mBack(nullptr), mFlag(nullptr), mNamePlate(nullptr), mStatusIcon(nullptr), mShowEffect(true), mCometTimer(0),
      mScaleMin(1.0f), mScaleMax(1.0f), mNamePlateOffset(sNamePlateOffset), mPosOffset(sTopOffset), mRotateSpeed(0.3f), mBasePos(0.0f, 0.0f, 0.0f) {
}

void GalaxyWorldMapInfo::init(const JMapInfoIter& rIter) {
    MR::setPosition(this, mWorld->getNode(mPointIndex)->mPos);

    if (MR::isEqualString(mType, "Galaxy")) {
        mKind = 0;
    } else if (MR::isEqualString(mType, "MiniGalaxy")) {
        mKind = 1;
    } else if (MR::isEqualString(mType, "HideGalaxy")) {
        mKind = 2;
    } else if (MR::isEqualString(mType, "BossGalaxyLv1")) {
        mKind = 3;
    } else if (MR::isEqualString(mType, "BossGalaxyLv2")) {
        mKind = 4;
    } else if (MR::isEqualString(mType, "BossGalaxyLv3")) {
        mKind = 5;
    }

    const char* pModelName = mModelName;
    char arcPath[0x80];
    snprintf(arcPath, sizeof(arcPath), "/ObjectData/%s.arc", pModelName);
    if (!MR::isFileExist(arcPath, true)) {
        pModelName = "MiniFlowerHighlandGalaxy";
    }

    if (!MR::initActor(this, pModelName, true)) {
        MR::connectToSceneWorldMapMiniObj(this);
        initEffectKeeper(0, nullptr, false);
        initSound(4, nullptr, nullptr, TVec3f(0.0f));
    }

    WorldMapFunction::initDrawFog(this);
    WorldMapFunction::setFogRange(this, 0.6f);
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 1000.0f);

    if (mPointerTarget == nullptr) {
        MR::initStarPointerTarget(this, 2000.0f, TVec3f(0.0f, 0.0f));
    }

    MR::registerDemoSimpleCastAll(this);

    mShadow = new MiniPartsModel(this, "影", "MiniGalaxyShadow", sShadowOffset);

    if (hasFlag()) {
        mFlag = new MiniPartsModel(this, "旗", "MiniFlag", TVec3f(0.0f, 0.0f, 0.0f));
    }

    if (GameSequenceFunction::isGreenStarEventOnly()) {
        mBack = new MiniPartsModel(this, "背景エフェクト", "MiniGalaxyBackEffect01", "GreenStarDemo", TVec3f(0.0f, 0.0f, 0.0f));
    } else {
        mBack = new MiniPartsModel(this, "背景エフェクト", "MiniGalaxyBackEffect01", TVec3f(0.0f, 0.0f, 0.0f));
    }

    mNamePlate = new GalaxyNamePlate(mGalaxyName, true);
    mNamePlate->initWithoutIter();

    if (GameDataFunction::isGalaxyStateOpened(mGalaxyName)) {
        initNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvOpened::sInstance, 0);
    } else if (GameDataFunction::isGalaxyStateNew(mGalaxyName)) {
        initNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance, 0);
    } else if (isMiniOrHide()) {
        initNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance, 0);
    } else {
        initNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvWait::sInstance, 0);
    }

    mStatusIcon = new GalaxyStatusIcon();
    if (isNerveNew()) {
        mStatusIcon->setNewPanes();
    } else if (isNerveOpened()) {
        updateStatusIcon();
    } else {
        mStatusIcon->hideAllPanes();
    }
    mStatusIcon->kill();

    mStatusIcon->setTrans(TVec2f(0.0f, -1000.0f));
    MR::registerDemoSimpleCastAll(mStatusIcon);

    if (isBossGalaxy()) {
        mShowEffect = false;
    }

    if (isBossGalaxy()) {
        mScaleMin = 1.0f;
        mScaleMax = 1.2f;
    } else {
        mScaleMin = 1.3f;
        mScaleMax = 1.55f;
    }

    if (mKind == 5) {
        mRotateSpeed = 0.0f;
        MR::setRotation(this, sBossRotation);
    }

    mBasePos.set(mPosition);
    mPosition.set(mBasePos + mPosOffset);

    MR::tryStartAllAnim(this, mModelName);

    if (isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void GalaxyWorldMapInfo::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mShadow->makeActorAppeared();
    mBack->makeActorAppeared();

    if (isNerveNew() || isNerveOpened()) {
        mStatusIcon->appear();
    }

    if (hasFlag()) {
        mFlag->makeActorAppeared();
        if (mKind == 3) {
            MR::tryStartAllAnim(mFlag, "KoopaJr");
        } else {
            MR::tryStartAllAnim(mFlag, "Koopa");
        }
        if (GameDataFunction::getPowerStarNumOwned(mGalaxyName) > 0) {
            MR::tryStartAllAnim(mFlag, "Clear");
        }
    }

    if (mShowEffect && isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvWait::sInstance)) {
        MR::tryDeleteEffect(this, mModelName);
        WorldMapFunction::setFogDrawOn(this);
    } else {
        MR::tryEmitEffect(this, mModelName);
        WorldMapFunction::setFogDrawOff(this);
    }
}

void GalaxyWorldMapInfo::makeActorDead() {
    LiveActor::makeActorDead();
    MR::forceDeleteEffectAll(this);

    MiniPartsModel* pShadow = mShadow;
    if (pShadow != nullptr) {
        pShadow->makeActorDead();
        MR::forceDeleteEffectAll(pShadow);
    }
    MiniPartsModel* pBack = mBack;
    if (pBack != nullptr) {
        pBack->makeActorDead();
        MR::forceDeleteEffectAll(pBack);
    }
    MiniPartsModel* pFlag = mFlag;
    if (pFlag != nullptr) {
        pFlag->makeActorDead();
        MR::forceDeleteEffectAll(pFlag);
    }
    mStatusIcon->kill();
}

void GalaxyWorldMapInfo::startClipped() {
    LiveActor::startClipped();
    if (mStatusIcon != nullptr) {
        mStatusIcon->kill();
    }
}

void GalaxyWorldMapInfo::endClipped() {
    LiveActor::endClipped();
    if (isNerveNew() || isNerveOpened()) {
        mStatusIcon->appear();
    }
}

bool GalaxyWorldMapInfo::tryNew() {
    if ((isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvWait::sInstance) || isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) && mWorld->isPathReachable(mPointIndex)) {
        makeActorAppeared();
        GameDataFunction::setGalaxyStateNew(mGalaxyName);
        if (isNerveHide()) {
            MR::hideModel(this);
            MR::hideModel(mBack);
        }
        if (isBossGalaxy()) {
            setNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance);
        } else {
            setNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNewOpen::sInstance);
        }
        return true;
    }
    return false;
}

bool GalaxyWorldMapInfo::tryNewBoss() {
    if ((isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvWait::sInstance) || isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) && mWorld->isPathReachable(mPointIndex)) {
        makeActorAppeared();
        GameDataFunction::setGalaxyStateNew(mGalaxyName);
        mStatusIcon->setNewPanes();
        mStatusIcon->appear();
        setNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance);
        return true;
    }
    return false;
}

bool GalaxyWorldMapInfo::tryOpen() {
    if (isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance) || isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvWait::sInstance) || isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) {
        if (MR::isDead(this)) {
            makeActorAppeared();
        }
        GameDataFunction::setGalaxyStateOpened(mGalaxyName);
        updateStatusIcon();
        mStatusIcon->appear();
        MR::tryEmitEffect(this, mModelName);
        WorldMapFunction::setFogDrawOff(this);
        setNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvOpened::sInstance);
        return true;
    }
    return false;
}

bool GalaxyWorldMapInfo::isNerveOpened() const {
    return isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvOpened::sInstance);
}

bool GalaxyWorldMapInfo::isNerveNew() const {
    return isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance);
}

bool GalaxyWorldMapInfo::isNerveNewOpen() const {
    return isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNewOpen::sInstance);
}

bool GalaxyWorldMapInfo::isNerveHide() const {
    return isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance);
}

bool GalaxyWorldMapInfo::startOpenSound(s32 kind) {
    if (kind == 0) {
        fn_800083F0(this, "OnGalaxy", -1, -1, -1);
        return true;
    }
    if (kind == 1) {
        fn_800083F0(this, "BesideGalaxy", -1, -1, -1);
        return true;
    }
    return false;
}

bool GalaxyWorldMapInfo::isPointing() {
    if (isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) {
        return false;
    }
    return WorldMapFunction::isPointing(this);
}

bool GalaxyWorldMapInfo::updatePointing() {
    if (isNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvHide::sInstance)) {
        return false;
    }
    WorldMapFunction::approachScale(this, 0.1f, mScaleMax);
    showNamePlate(true);
    return true;
}

void GalaxyWorldMapInfo::showNamePlate(bool unknown) {
    if (!MR::isClipped(this) && mNamePlate != nullptr) {
        if (isNerveOpened()) {
            mNamePlate->showOpen(true);
        } else if (unknown) {
            mNamePlate->showUnknown(true);
        }
    }
}

void GalaxyWorldMapInfo::startPointingAnim() {
    mStatusIcon->showForTime();
}

void GalaxyWorldMapInfo::endPointingAnim() {
    mStatusIcon->startCometHideAnim();
}

void GalaxyWorldMapInfo::setComet(bool green) {
    mCometTimer = 90;
    if (green) {
        MR::emitEffect(mBack, "CometIconGreenAppear");
    } else {
        MR::emitEffect(mBack, "CometIconAppear");
    }
}

void GalaxyWorldMapInfo::setAllEffectDrawOrder(s32 order) {
    MR::setAllEffectDrawOrder(this, order);
    MR::setAllEffectDrawOrder(mBack, order);
}

void GalaxyWorldMapInfo::exeNewOpen() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(mBack, "Select");
        MR::emitEffect(mBack, "NewOpenSign");
        MR::startActionSound(mBack, "Burst", -1, -1, -1);
    }
    if (MR::isStep(this, 45)) {
        MR::emitEffect(mBack, "NewOpen");
        MR::tryRumblePadWeak(this, 0);
        MR::shakeCameraNormal();
    }
    if (mShowEffect) {
        if (MR::isLessStep(this, 154)) {
            f32 t = 1.0f - MR::calcNerveRate(this, 45, 154);
            t *= 0.6f;
            WorldMapFunction::setFogRange(this, t);
        } else if (MR::isStep(this, 154)) {
            MR::tryEmitEffect(this, mModelName);
            WorldMapFunction::setFogDrawOff(this);
        }
    }
    if (MR::isLessStep(this, 75)) {
        MR::tryRumblePadVeryWeak(this, 0);
    }
    if (MR::isStep(this, 110)) {
        MR::showModel(this);
        MR::showModel(mBack);
        mStatusIcon->setNewPanes();
        mStatusIcon->appear();
    }
    if (MR::isStep(this, 150)) {
        setNerve(&NrvGalaxyWorldMapInfo::GalaxyWorldMapInfoNrvNew::sInstance);
    }
}

void GalaxyWorldMapInfo::control() {
    WorldMapFunction::returnScale(this, 0.05f, mScaleMin);
    mRotation.y = mRotation.y + mRotateSpeed;
    wrapAngle(mRotation.y, 0.0f);

    if (mNamePlate != nullptr && !MR::isDead(mNamePlate)) {
        TVec3f pos(mPosition);
        pos.add(mNamePlateOffset);
        mNamePlate->setPos3D(pos);
    }

    if (mStatusIcon != nullptr && !MR::isDead(mStatusIcon)) {
        TVec3f pos(mBasePos);
        pos.add(sTopOffset);
        TVec2f screenPos;
        MR::calcScreenPosition(&screenPos, pos);
        mStatusIcon->setTrans(screenPos);
        if (mCometTimer > 0) {
            mCometTimer--;
            if (mCometTimer == 0) {
                mStatusIcon->startCometAppear();
                MR::startActionSound(mBack, "CometMark", -1, -1, -1);
            }
        }
    }

    if (mFlag != nullptr) {
        updateFlag();
    }

    TVec3f pos(mBasePos);
    pos.add(mPosOffset);
    mPosition.set(pos);
}

void GalaxyWorldMapInfo::updateFlag() {
    TVec3f offset(0.0f);
    mFlag->mCalcOwnMatrix = false;
    if (mKind == 5) {
        MR::extractMtxTrans(MR::getJointMtx(this, "FlagPoint"), &mFlag->mPosition);
        return;
    }
    if (mKind == 3) {
        TVec3f add(0.0f, 2510.0f, 0.0f);
        offset.add(add);
    } else if (mKind == 4) {
        TVec3f add(0.0f, 3420.0f, 0.0f);
        offset.add(add);
    }
    mFlag->mCalcOwnMatrix = false;
    MR::setPosition(mFlag, mPosition);
    offset.mul(mScale);
    mFlag->mPosition.add(offset);
}

bool GalaxyWorldMapInfo::isBossGalaxy() const {
    return (u32)(mKind - 3) <= 2;
}

bool GalaxyWorldMapInfo::hasFlag() const {
    return (u32)(mKind - 3) <= 2;
}

void GalaxyWorldMapInfo::updateStatusIcon() {
    MR::makeGalaxyStatusAccessor(mGalaxyName);
    s32 a = MR::getGalaxyStarTotalNum(mGalaxyName);
    s32 b = WorldMapFunction::hasIncompleteGreenStar(mGalaxyName);
    s32 c = MR::isGalaxyCompletedWithGreen(mGalaxyName);
    s32 d = MR::isGalaxyCompletedNoGreen(mGalaxyName);
    mStatusIcon->setCometPanes(0, a, b, c, d);
    WorldMapFunction::setStatusIconKind(mStatusIcon, mGalaxyName);
}

GalaxyWorldMapInfo::~GalaxyWorldMapInfo() {}
