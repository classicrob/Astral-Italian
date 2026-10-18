#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/MiniRouteLine.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/System/WorldMapPointMarks.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/VectorUtil.hpp"
#include <cmath>

// ==== TU C (0x804EB450): MiniRouteLine ====
// ---------------------------------------------------------------------------------------------------------------
// MiniRouteLine: the glowing line between two map nodes
// ---------------------------------------------------------------------------------------------------------------

MiniRouteLine::MiniRouteLine(const char* pName, WorldMapResultWorld*) : LiveActor(pName) {
    mFrom = 0;
    mTo = 0;
    mCondStage = nullptr;
    mCondStarNum = -1;
    mCondFlag = nullptr;
    mUnkA4 = false;
    mColorChange = false;
    mValid = true;
    mOpened = false;
    mHidden = false;
    mAlpha = 1.0f;
    mCenter.x = 0.0f;
    mCenter.y = 0.0f;
    mCenter.z = 0.0f;
    mEffectPos.x = 0.0f;
    mEffectPos.y = 0.0f;
    mEffectPos.z = 0.0f;
    mEffectName = "Progress";
    mShadowActor = nullptr;
    mMtxSetter = nullptr;
}

// NONMATCHING: the three scaled copies of the side vector are low-slot temporaries in the target (chained scaleInline inlines too much)
void MiniRouteLine::init(const JMapInfoIter&) {
    if (!MR::initActor(this, "MiniRouteLine", true)) {
        MR::connectToScene(this, -1, 20, 39, -1);
    }
    mMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    mMtxSetter->updateMtxUseBaseMtx();
    MR::newDifferedDLBuffer(this);
    MR::invalidateClipping(this);
    makeActorDead();

    mEffectActor = new LiveActor("リンク作成中");
    if (!MR::initActor(mEffectActor, "MiniRouteLine", false)) {
        mEffectActor->initEffectKeeper(1, nullptr, false);
        MR::setEffectHostSRT(mEffectActor, "Progress", &mEffectPos, &mEffectActor->mRotation, nullptr);
        MR::setEffectHostSRT(mEffectActor, "ProgressTico", &mEffectPos, &mEffectActor->mRotation, nullptr);
        MR::connectToScene(mEffectActor, -1, 20, 39, -1);
    }
    MR::invalidateClipping(mEffectActor);
    mEffectActor->makeActorDead();

    mShadowActor = new LiveActor("リンク影");
    if (!MR::initActor(mShadowActor, "MiniRouteLineShadow", false)) {
        MR::connectToScene(mShadowActor, -1, 20, 39, -1);
    }
    MR::invalidateClipping(mShadowActor);
    mShadowActor->makeActorDead();

    TVec3f sideVec;
    MR::calcSideVec(&sideVec, this);
    f32 scaleX = mScale.x;
    TVec3f a(sideVec);
    a.scale(scaleX);
    TVec3f b(a);
    b.scale(0.5f);
    TVec3f c(b);
    c.scale(1000.0f);
    mCenter.set(mPosition + c);
    MR::setClippingTypeSphere(this, 1000.0f * (mScale.x * 0.5f), &mCenter);
    MR::setClippingTypeSphere(mShadowActor, 1000.0f * (mScale.x * 0.5f), &mCenter);
    MR::startAction(this, "Close");
    MR::startAction(mEffectActor, "Open");
    MR::startBrk(mEffectActor, "Normal");
    MR::copyTransRotateScale(this, mShadowActor);
    MR::copyTransRotateScale(this, mEffectActor);
    updateShadow(0, mAlpha);
    updateEffect(0, mAlpha);
}

void MiniRouteLine::appear() {
    LiveActor::appear();
    if (mHidden) {
        MR::hideModel(this);
    } else {
        MR::showModel(this);
    }
    updateShadow(0, mAlpha);
    updateEffect(0, mAlpha);
}

void MiniRouteLine::kill() {
    LiveActor::kill();
    mShadowActor->kill();
    mEffectActor->kill();
}

// NONMATCHING: register numbers of the paired-single matrix copy (f0..f5 are assigned in reverse)
void MiniRouteLine::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TPos3f mtx;
    MR::makeMtxTR(mtx, this);
    mtx.invert(mtx);
    mMtxSetter->mEffectMtx.set(mtx);
}

void MiniRouteLine::setCondition(const char* pStage, s32 starNum, const char* pFlag) {
    mCondStage = pStage;
    mCondStarNum = starNum;
    mCondFlag = pFlag;
    if (isLocked()) {
        mHidden = true;
    }
}

bool MiniRouteLine::isLocked() const {
    if (!MR::isNullOrEmptyString(mCondStage)) {
        if (mCondStarNum == 0) {
            if (GameDataFunction::getPowerStarNumOwned(mCondStage) == 0) {
                return true;
            }
        } else if (!GameDataFunction::hasPowerStar(mCondStage, mCondStarNum)) {
            return true;
        }
    }
    if (!MR::isNullOrEmptyString(mCondFlag) && !GameDataFunction::isOnGameEventFlag(mCondFlag)) {
        return true;
    }
    return false;
}

bool MiniRouteLine::hasFlag() const {
    return !MR::isNullOrEmptyString(mCondFlag);
}

void MiniRouteLine::setup(const TVec3f& rFrom, const TVec3f& rTo) {
    TQuat4f quat;
    TVec3f dir(rTo);
    dir.sub(rFrom);
    mScale.set<f32>(dir.length() / 1000.0f, 1.0f, 1.0f);
    MR::normalize(&dir);
    MR::makeQuatSideUp(&quat, dir, TVec3f(0.0f, 1.0f, 0.0f));
    MR::setRotation(this, quat);
    MR::setPosition(this, TVec3f(rFrom.x, rFrom.y, rFrom.z));
}

// NONMATCHING: target tests the argument with clrlwi. instead of cmpwi
void MiniRouteLine::setColorChange(u8 flag) {
    mColorChange = flag;
    if (flag) {
        MR::startBrk(mEffectActor, "TicoBuild");
    } else {
        MR::startBrk(mEffectActor, "Normal");
    }
}

void MiniRouteLine::setAlpha(s32 flag, f32 alpha) {
    mAlpha = alpha;
    updateShadow(flag, alpha);
    updateEffect(flag, alpha);
    updateEffectPos();
}

void MiniRouteLine::updateShadow(s32 doTransform, f32 alpha) {
    if (!mOpened || alpha == 0.0f) {
        mShadowActor->makeActorDead();
        return;
    }
    mShadowActor->makeActorAppeared();
    MR::copyTransRotateScale(this, mShadowActor);
    mShadowActor->mScale.x *= alpha;
    if (doTransform != 0) {
        updateTransform(mShadowActor);
    }
    mShadowActor->mPosition.y += -700.0f;
}

void MiniRouteLine::updateEffect(s32 doTransform, f32 alpha) {
    if (!mOpened || alpha == 0.0f) {
        mEffectActor->makeActorDead();
        return;
    }
    mEffectActor->makeActorAppeared();
    MR::copyTransRotateScale(this, mEffectActor);
    mEffectActor->mScale.x *= alpha;
    if (doTransform != 0) {
        updateTransform(mEffectActor);
    }
}

void MiniRouteLine::updateTransform(LiveActor* pPart) {
    TVec3f xDir;
    MR::extractMtxXDir(getBaseMtx(), &xDir);
    f32 scaleX = mScale.x;
    MR::setPosition(pPart, mPosition + xDir.scaleInline(scaleX).scaleInline(1000.0f));
    f32 angle = 180.0f + mRotation.y;
    wrapAngle(angle, 0.0f);
    MR::setRotation(pPart, mRotation.x, angle, mRotation.z);
}

void MiniRouteLine::updateEffectPos() {
    TVec3f xDir;
    MR::extractMtxXDir(mEffectActor->getBaseMtx(), &xDir);
    LiveActor* pEffect = mEffectActor;
    f32 length = 1000.0f * pEffect->mScale.x;
    mEffectPos.set(pEffect->mPosition + xDir.scaleInline(length));
}

void MiniRouteLine::emitProgressEffect() {
    MR::emitEffect(mEffectActor, mEffectName);
}

void MiniRouteLine::deleteProgressEffect() {
    MR::deleteEffect(mEffectActor, mEffectName);
}

void MiniRouteLine::setEffectDrawOrder(s32 order) {
    MR::setAllEffectDrawOrder(mEffectActor, order);
}

bool MiniRouteLine::getNextNode(s32 node, s32* pOther) {
    *pOther = -1;
    if (!mValid) {
        return false;
    }
    if (mOpened) {
        return false;
    }
    if (mAlpha < 1.0f) {
        return false;
    }
    if (isLocked()) {
        return false;
    }
    if (mFrom == node) {
        *pOther = mTo;
    }
    if (mTo == node) {
        *pOther = mFrom;
    }
    if (*pOther < 0) {
        return false;
    }
    return true;
}

bool MiniRouteLine::open() {
    mOpened = true;
    appear();
    return true;
}

bool MiniRouteLine::isBetween(s32 a, s32 b, bool strict) {
    if (!mValid) {
        return false;
    }
    if (strict) {
        if (!mOpened) {
            return false;
        }
        if (mAlpha < 1.0f) {
            return false;
        }
        if (isLocked()) {
            return false;
        }
    }
    if (mFrom == a && mTo == b) {
        return true;
    }
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

bool MiniRouteLine::getOther(s32 node, s32* pOther) const {
    *pOther = -1;
    if (mFrom == node) {
        *pOther = mTo;
    }
    if (mTo == node) {
        *pOther = mFrom;
    }
    if (*pOther < 0) {
        return false;
    }
    return true;
}

bool MiniRouteLine::isBetweenNodes(s32 a, s32 b) const {
    if (mFrom == a && mTo == b) {
        return true;
    }
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

bool MiniRouteLine::isDirected(s32 a, s32 b) const {
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

MiniRouteLine::~MiniRouteLine() {}
