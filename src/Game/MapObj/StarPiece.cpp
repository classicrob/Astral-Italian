#pragma auto_inline off

#include "Game/MapObj/StarPiece.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include <JSystem/J3DGraphBase/J3DStruct.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXStruct.h>
#include <revolution/mtx.h>


const GXColor effectColors[] = {{0x00, 0x55, 0xff, 0xFF}, {0xFF, 0xFF, 0x00, 0xFF}, {0x55, 0xFF, 0x00, 0xFF}, {0x7F, 0x7F, 0x00, 0xFF}};
const GXColor initColors[] = {{0x80, 0x00, 0x99, 0xFF}, {0xE6, 0xA0, 0x00, 0xFF}, {0x46, 0xA1, 0x08, 0xFF},
                              {0x37, 0x5A, 0xA0, 0xFF}, {0xBE, 0x33, 0x0B, 0xFF}, {0x80, 0x80, 0x80, 0xFF}};

namespace NrvStarPiece {
    NEW_NERVE(HostTypeNrvFloating, StarPiece, Floating);
    NEW_NERVE(HostTypeNrvHop, StarPiece, Hop);
    NEW_NERVE(HostTypeNrvFall, StarPiece, Fall);
    NEW_NERVE(HostTypeNrvFallAfterReflect, StarPiece, Fall);
    NEW_NERVE(HostTypeNrvToTarget, StarPiece, ToTarget);
    NEW_NERVE(HostTypeNrvSpinDrained, StarPiece, SpinDrained);
    NEW_NERVE(HostTypeNrvToPlayerEnd, StarPiece, ToPlayerEnd);
    NEW_NERVE_ONEND(HostTypeNrvThrow, StarPiece, Throw, Throw);
    NEW_NERVE_ONEND(HostTypeNrvThrowNoFall, StarPiece, Throw, Throw);
    NEW_NERVE_ONEND(HostTypeNrvThrowFall, StarPiece, ThrowFall, Throw);
    NEW_NERVE(HostTypeNrvFollowPlayer, StarPiece, FollowPlayer);
    NEW_NERVE(HostTypeNrvRailMove, StarPiece, RailMove);
};  // namespace NrvStarPiece

StarPiece::StarPiece(const char* pName)
    : LiveActor(pName), mFront(0, 0, 1), mTargetTurnRate(0.02f), mTargetSpeedRate(), mParabolaA(), mParabolaB(), mThrowStartPos(0, 0, 0), mThrowEndPos(0, 0, 0), mThrowTriangleFilter(nullptr), mTargetSensor(), mGettableEffectTimer(-1),
      mGettableDelayCounter(-1), mPointerCollectDelay(-1), mFallKillTimer(), mColor((GXColor){0, 0, 0, 255}), mGroupType(GroupType_None), mHostInfo(nullptr),
      mReceiverInfo(0), mNumGift(1) {
    MR::incNumStarPieceNewed();

    mFlags.isGoToPlayer = false;
    setCollectionEnabled(true);
    mFlags.isDynamicShadow = false;
    mFlags.isShadowDisabled = false;
    mFlags.isGroup = false;
    mFlags.isInWater = false;
    mFlags.isLightEffect = false;
    mFlags.isLaunched = false;
}

void StarPiece::initAndSetFloatingFromGroup(const JMapInfoIter& rIter) {
    mGroupType = GroupType_Floating;
    MR::incNumStarPieceGettable(0);
    init(rIter);
    MR::invalidateClipping(this);
    mFlags.isGroup = true;
}

void StarPiece::initAndSetRailMoveFromGroup(const JMapInfoIter& rIter) {
    mGroupType = GroupType_RailMove;
    MR::incNumStarPieceGettable(0);
    init(rIter);
    MR::invalidateClipping(this);
    mFlags.isGroup = true;
}

void StarPiece::init(const JMapInfoIter& rIter) {
    s32 arg3 = -1;

    if (MR::isValidInfo(rIter)) {
        if (mGroupType == GroupType_None) {
            MR::incNumStarPieceGettable(0);
        }

        MR::initDefaultPos(this, rIter);
        MR::getJMapInfoArg3NoInit(rIter, &arg3);
        // unused
        const char* objName = nullptr;
        MR::getObjectName(&objName, rIter);
        MR::registerDemoSimpleCastAll(this);
    }

    s32 minColor = 0;

    if (MR::isGalaxyPurpleCometAppearInCurrentStage()) {
        minColor = 1;
    }

    if (minColor <= arg3 && arg3 < getNumColor()) {
        mColor = initColors[arg3];
    } else {
        mColor = initColors[MR::getRandom(minColor, getNumColor())];
    }

    MR::initActor(this, rIter, "StarPiece", true);
    MR::initDLMakerMatColor0(this, "StarPiece1_v", &mColor);
    MR::newDifferedDLBuffer(this);
    MR::connectToSceneNoSilhouettedMapObj(this);

    f32 flt = 30.0f;
    MR::setScale(this, 1.0f);
    initBinder(40.0f, 0.0f, 0);

    switch (mGroupType) {
    case GroupType_None:
    case GroupType_Floating:
        initNerve(GET_NERVE(StarPiece, HostTypeNrvFloating));
        break;
    case GroupType_RailMove:
        initNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove));
        break;
    }

    initEffectKeeper(0, "StarPiece", false);
    if (!MR::isStageFileSelect()) {
        initSound(4, "StarPiece", nullptr, TVec3f(0.0f));
    }
    initHitSensor(2);

    MR::addHitSensorEye(this, "attack", 16, flt, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensor(this, "Body", ATYPE_STAR_PIECE, 16, flt, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initShadowVolumeSphere(this, flt);

    MR::offCalcShadow(this, nullptr);
    MR::offCalcShadowDropPrivateGravity(this, nullptr);
    MR::onCalcShadowOneTime(this, nullptr);
    MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);

    if (MR::isValidInfo(rIter)) {
        s32 arg4 = -1;
        MR::getJMapInfoArg4NoInit(rIter, &arg4);

        if (arg4 == 0) {
            MR::onCalcShadow(this, nullptr);
            MR::onCalcShadowDropPrivateGravity(this, nullptr);
            mFlags.isDynamicShadow = true;
            mFlags.isShadowDisabled = false;
        } else if (arg4 == -1) {
            mFlags.isDynamicShadow = false;
            mFlags.isShadowDisabled = false;
        } else if (arg4 == 1) {
            mFlags.isDynamicShadow = false;
            mFlags.isShadowDisabled = true;
            MR::invalidateShadow(this, nullptr);
        }
    }

    MR::initStarPointerTarget(this, 28.0f, TVec3f(0, 0, 0));
    MR::offCalcGravity(this);

    mThrowTriangleFilter = MR::createTriangleFilterDelegator(this, &StarPiece::isIgnoreTriOnThrow);

    MR::tryCreateMirrorActor(this, "StarPiece");
    MR::startBtk(this, "Gift");
    MR::setBtkFrameAndStop(this, 5.0f);
    MR::useStageSwitchAwake(this, rIter);

    if (mGroupType == GroupType_None) {
        appear();
    } else {
        makeActorDead();
    }
}

// NONMATCHING: color comparison operand registers differ.
void StarPiece::initAfterPlacement() {
    if (MR::isExistSceneObj(SceneObj_CollectTicoHolder) && *(u32*)&mColor == *(u32*)&initColors[Color_Silver]) {
        mColor = initColors[MR::getRandom((s32)0, (s32)4)];
    }

    MR::isDead(this);
}

void StarPiece::appear() {
    LiveActor::appear();
    MR::onCalcShadow(this, nullptr);
    MR::onCalcShadowDropPrivateGravity(this, nullptr);
}

void StarPiece::setColor(s32 colorIndex) {
    mColor = initColors[colorIndex];
}

s32 StarPiece::getNumColor() {
    return ARRAY_SIZE(initColors);
}

void StarPiece::appearFromGroup() {
    appear();

    switch (mGroupType) {
    case GroupType_Floating:
        setNerve(GET_NERVE(StarPiece, HostTypeNrvFloating));
        break;
    case GroupType_RailMove:
        setNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove));
        break;
    }
}

void StarPiece::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::invalidateHitSensor(this, "attack");
    mGettableEffectTimer = -1;
}

f32 StarPiece::calcDistToCamera() {
    TVec3f dirToCamera(*MR::getSensorPos(this, "Body"));
    TVec3f camPos(MR::getCamPos());
    dirToCamera.sub(camPos);
    return PSVECMag(&dirToCamera);
}

void StarPiece::kill() {
    MR::forceDeleteEffect(this, "StarPieceFlyingBlur");

    if (200.0f < calcDistToCamera()) {
        if (isEffectLight()) {
            MR::emitEffect(this, "StarPieceBreakS");
        } else {
            MR::emitEffect(this, "StarPieceBreak");
        }
    }

    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::startActionSound(this, "OjStarPieceBreakW");
    } else {
        MR::startActionSound(this, "OjStarPieceBreak");
    }

    LiveActor::kill();
}

void StarPiece::makeActorDead() {
    LiveActor::makeActorDead();
    mTargetSensor = nullptr;
    mReceiverInfo = 0;
    mGettableEffectTimer = -1;

    mFlags.isLightEffect = false;

    if (mFlags.isLaunched == true) {
        MR::getStarPieceDirector()->mNumStarPiecesLaunched--;
        mFlags.isLaunched = false;
    }

    if (mHostInfo != nullptr) {
        mHostInfo->mActiveCount--;
        mHostInfo = nullptr;
    }
}

void StarPiece::startClipped() {
    LiveActor::startClipped();

    if (!isNerve(GET_NERVE(StarPiece, HostTypeNrvFloating)) && !isNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove))) {
        makeActorDead();
    }

    mGettableEffectTimer = -1;
}

void StarPiece::changeScale(f32 scale) {
    MR::setScale(this, scale);
    // probably some clamp inline function
    f32 clampedScale;

    if (scale < 0.1f) {
        clampedScale = 0.1f;
    } else if (scale > 2.0f) {
        clampedScale = 2.0f;
    } else {
        clampedScale = scale;
    }

    clampedScale = (clampedScale - 0.1f) / 1.9f;
    MR::setBtkFrameAndStop(this, MR::getBtkFrameMax(this) * (1.0f - clampedScale));
    MR::setBinderRadius(this, 40.0f * scale);
    MR::setShadowVolumeSphereRadius(this, nullptr, 30.0f * scale);
}

void StarPiece::control() {
    if (MR::isFirstStep(this)) {
        if (!isNerve(GET_NERVE(StarPiece, HostTypeNrvToTarget)) || mFlags.isGoToPlayer) {
            MR::setBtkFrameAndStop(this, 5.0f);
        }
    }

    if (mGettableDelayCounter >= 0) {
        mGettableDelayCounter -= 1;
    }

    if (mPointerCollectDelay >= 0) {
        mPointerCollectDelay -= 1;
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFall)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect))) {
        if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            kill();
            return;
        }

        if (MR::isPressedRoofAndGround(this)) {
            kill();
            return;
        }
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFall)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect)) ||
        isNerve(GET_NERVE(StarPiece, HostTypeNrvThrow)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowNoFall)) ||
        isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowFall))) {
        changeScale(calcNearCameraScale());
    }
}

void StarPiece::exeFloating() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        tryCalcGravity();

        if (!mFlags.isShadowDisabled) {
            MR::validateShadow(this, nullptr);
        } else {
            MR::invalidateShadow(this, nullptr);
        }

        if (mFlags.isGroup) {
            MR::invalidateClipping(this);
        } else {
            MR::validateClipping(this);
        }
    }

    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    tryGotJudge();

    if (mFlags.isDynamicShadow) {
        HitInfo info;
        MR::checkStrikePointToMap(mPosition, &info);

        if (MR::isCodeSand(&info.mParentTriangle)) {
            kill();
            MR::emitEffect(this, "SandColumnS");
        }
    }
}

void StarPiece::appearHop(const TVec3f& rVec1, const TVec3f& rVec2) {
    if (MR::isNearZero(rVec2)) {
        mGravity.set(0.0f, -1.0f, 0.0f);
    } else {
        MR::normalize(-rVec2, &mGravity);
    }

    MR::setPosition(this, rVec1);
    makeActorAppeared();
    mVelocity.set(rVec2 * 30.0f);
    setNerve(GET_NERVE(StarPiece, HostTypeNrvHop));
}

void StarPiece::exeHop() {
    if (MR::isFirstStep(this)) {
        MR::gotByPlayer();
        MR::invalidateClipping(this);
        MR::offBind(this);
        MR::invalidateShadow(this, nullptr);
    }

    mVelocity += mGravity;

    if (MR::isGreaterStep(this, 30)) {
        if (mHostInfo != nullptr) {
            mHostInfo->mCollectedCount++;
        }

        kill();
    }
}

void StarPiece::exeFall() {
    if (MR::isFirstStep(this)) {
        if (isEffectLight()) {
            MR::emitEffect(this, "StarPieceLightS");
        } else {
            MR::emitEffect(this, "StarPieceLight");
        }

        MR::onBind(this);
        MR::invalidateClipping(this);
        MR::validateShadow(this, nullptr);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropGravity(this, nullptr);
        mFallKillTimer = 0;
    }

    tryCalcGravity();

    if (MR::isInWater(mPosition)) {
        if (!mFlags.isInWater) {
            mVelocity.scale(0.5f);
            MR::emitEffect(this, "WaterFall");
        }
        mFlags.isInWater = true;
    } else {
        mFlags.isInWater = false;
    }

    if (mFlags.isInWater) {
        mVelocity *= 0.97f;
        mVelocity += mGravity * 0.1f;
    } else {
        mVelocity *= 0.97f;
        mVelocity += mGravity * 1.0f;
    }

    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    if (MR::reboundVelocityFromCollision(this, 0.99f, 0.0f, 1.0f) == true) {
        if (MR::isBindedGround(this) && !MR::isBindedWall(this) && !MR::isBindedRoof(this)) {
            TVec3f* gravity = &mGravity;
            mVelocity.scale(gravity->dot(mVelocity), *gravity);
        }

        MR::startAction(this, "Land");

        if (!MR::isDemoActive()) {
            if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
                MR::startActionSound(this, "OjStarPieceHitW");
            } else {
                MR::startActionSound(this, "OjStarPieceHit");
            }
        }

        if (mFlags.isInWater) {
            if (mVelocity.squared() < 100.0f) {
                mVelocity.setLength(10.0f);
            }
        } else {
            if (mVelocity.squared() < 400.0f) {
                mVelocity.setLength(20.0f);
            }
        }
    }

    if (!MR::isDemoActive()) {
        mFallKillTimer++;

        if (mFallKillTimer > 600) {
            kill();
            return;
        }
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect))) {
        if (MR::isGreaterStep(this, 9)) {
            kill();
            return;
        }
    }

    if (!isNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect))) {
        tryGotJudge();
    }
}

bool StarPiece::isTouchToTarget(TVec3f* pVec, f32 flt) {
    TVec3f* vec;

    if (mFlags.isGoToPlayer) {
        vec = MR::getPlayerCenterPos();
    } else {
        vec = (TVec3f*)MR::getSensorPos(mTargetSensor);
    }

    TVec3f vec2(*vec);
    vec2 -= mGravity * flt;
    vec2 -= mPosition;

    if (pVec != nullptr) {
        pVec->set(vec2);
    }

    f32 distToTouch;

    if (mFlags.isGoToPlayer) {
        distToTouch = 160.0f;
    } else {
        distToTouch = MR::getSensorRadius(mTargetSensor) + MR::getSensorRadius(this, "Body");
    }

    return vec2.length() < distToTouch;
}

bool StarPiece::isEffectLight() {
    bool retval = true;

    if (!mFlags.isLightEffect && MR::getStarPieceDirector()->mNumStarPiecesLaunched <= 10) {
        retval = false;
    }

    return retval;
}

// NONMATCHING: sensor-host velocity copy uses a different address-register schedule.
void StarPiece::exeToTarget() {
    if (MR::isFirstStep(this)) {
        if (mFlags.isGoToPlayer) {
            if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
                MR::startActionSound(this, "OjStarPieceFlyW");
            } else {
                MR::startActionSound(this, "OjStarPieceFly");
            }
        }

        MR::offBind(this);
        MR::invalidateClipping(this);
        mVelocity.zero();
        mTargetTurnRate = 0.02f;
        mTargetSpeedRate = 0.0f;
        MR::forceDeleteEffect(this, "StarPieceFlyingBlur");
        MR::emitEffect(this, "StarPieceGetBlur");

        if (mFlags.isGoToPlayer) {
            emitTouchEffect(*MR::getStarPointerLastPointedPort(this), 10.0f);
        } else {
            emitTouchEffect(0, 10.0f);
        }

        MR::invalidateShadow(this, nullptr);
    }

    tryCalcGravity();
    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    if (!mFlags.isGoToPlayer) {
        mScale += (TVec3f(0.5f, 0.5f, 0.5f) - mScale) * 0.05f;
        changeScale(mScale.y);
    }

    TVec3f touchTargetVec;

    if (isTouchToTarget(&touchTargetVec, 0.0f)) {
        if (mFlags.isGoToPlayer) {
            touchPlayer();
            return;
        }

        if (mReceiverInfo != nullptr) {
            mReceiverInfo->mGotCount += mNumGift;
            mReceiverInfo->mPendingGiftCount -= mNumGift;
        }

        MR::sendMsgStarPieceGift(mTargetSensor, getSensor("attack"), mNumGift);
        changeScale(1.0f);
        MR::emitEffectHit(this, *MR::getSensorPos(this, "attack"), "InvalidHitMark");
        makeActorDead();
        return;
    }

    if (mFlags.isGoToPlayer && MR::isResetChasingStarPiece()) {
        kill();
        return;
    }

    // probably two inlined max functions
    f32 max;

    if (mTargetTurnRate + 0.01f >= 1.0f) {
        max = 1.0f;
    } else {
        max = mTargetTurnRate + 0.01f;
    }

    mTargetTurnRate = max;

    f32 max2;

    if (mTargetSpeedRate + 0.03f >= 1.0f) {
        max2 = 1.0f;
    } else {
        max2 = mTargetSpeedRate + 0.03f;
    }

    mTargetSpeedRate = max2;

    MR::normalizeOrZero(&touchTargetVec);
    TQuat4f rotateQuat;
    rotateQuat.setRotate(mFront, touchTargetVec, mTargetTurnRate);
    rotateQuat.rotate(mFront);

    MR::normalizeOrZero(&mFront);

    TVec3f vec2(mFront);
    vec2 *= mTargetSpeedRate * 85.0f + 1.0f;
    mVelocity.set(vec2);

    TVec3f* vel;

    if (mFlags.isGoToPlayer) {
        vel = MR::getPlayerVelocity();
    } else {
        vel = &MR::getSensorHost(mTargetSensor)->mVelocity;
    }

    TVec3f velNormalized(*vel);
    MR::normalizeOrZero(&velNormalized);

    if (!MR::isNearZero(velNormalized)) {
        f32 dot = mFront.dot(velNormalized);

        if (0.0f < dot) {
            if (mFlags.isGoToPlayer) {
                mVelocity += *MR::getPlayerVelocity() * mTargetSpeedRate * dot;
            } else {
                mVelocity += MR::getSensorHost(mTargetSensor)->mVelocity * mTargetSpeedRate * dot;
            }
        }
    }
}

void StarPiece::exeSpinDrained() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::offBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
        MR::emitEffect(this, "StarPieceFlyingBlur");

        if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(this, "OjStarPieceFlyW");
        } else {
            MR::startActionSound(this, "OjStarPieceFly");
        }
    }

    tryCalcGravity();
}

void StarPiece::exeToPlayerEnd() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::invalidateShadow(this, nullptr);

        mThrowEndPos.set(mPosition);
        mThrowEndPos -= (*MR::getPlayerCenterPos());
        mVelocity.zero();

        MR::deleteEffect(this, "StarPieceGetBlur");

        if (mHostInfo != nullptr) {
            mHostInfo->mCollectedCount++;
            mHostInfo->mActiveCount--;
            mHostInfo = nullptr;
        }
    }

    f32 newScale = 1.0f - MR::calcNerveRate(this, 10);
    changeScale(newScale);
    mPosition.set(*MR::getPlayerCenterPos());
    mPosition += mThrowEndPos * newScale;

    if (MR::isGreaterStep(this, 30)) {
        MR::validateHitSensor(this, "Body");
        changeScale(1.0f);
        makeActorDead();
    }
}

void StarPiece::exeThrow() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        MR::offBind(this);
        MR::setBinderRadius(this, 20.0f);
        MR::setBindTriangleFilter(this, mThrowTriangleFilter);

        MR::validateHitSensor(this, "attack");
        MR::emitEffect(this, "StarPieceFlyingBlur");
        MR::invalidateClipping(this);

        mGravity.set(*MR::getPlayerGravity());

        MR::validateShadow(this, nullptr);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    TVec3f vec1(mPosition);
    TVec3f vec2(0, 0, 0);
    f32 flt = getNerveStep() / 30.0f;
    TVec3f vec3;

    vec3.set(mThrowEndPos);
    vec3 -= mThrowStartPos;

    f32 flt2 = 0.0f;
    vec2 = mThrowStartPos + vec3 * flt + (-mGravity) * flt2;

    if (mTargetSensor != nullptr) {
        f32 flt3 = MR::calcNerveRate(this, 30);
        vec2 = *MR::getSensorPos(mTargetSensor) * flt3 + vec2 * (1.0f - flt3);
    }

    mVelocity.set(vec2);
    mVelocity -= vec1;

    TVec3f camZDir(MR::getCamZdir());
    TVec3f camPos(MR::getCamPos());
    TVec3f vec10(mPosition);
    TVec3f playerCenterPos(*MR::getPlayerCenterPos());

    vec10 -= camPos;
    playerCenterPos -= camPos;

    if (MR::isNoBind(this)) {
        if (1500.0f < MR::getCamPos().distance(mPosition) || camZDir.dot(playerCenterPos) < camZDir.dot(vec10) || MR::isGreaterStep(this, 28)) {
            MR::onBind(this);
        }
    }

    if (MR::isBinded(this)) {
        if (MR::sendMsgToBindedSensor(ACTMES_IS_STAR_PIECE_REFLECT, this, getSensor("attack"))) {
            setReflect(MR::getBindedNormal(this), MR::getBindedHitPos(this));
            return;
        }

        if (MR::sendMsgToBindedSensor(ACTMES_STAR_PIECE_ATTACK, this, getSensor("attack"))) {
            MR::startActionSound(this, "OjStarPieceHitEnemy");
        }

        kill();
        return;
    }

    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(StarPiece, HostTypeNrvThrowFall));
    }
}

void StarPiece::endThrow() {
    MR::invalidateHitSensor(this, "attack");
    MR::onBind(this);
    MR::setBinderRadius(this, 40.0f);
    MR::setBindTriangleFilter(this, nullptr);
}

void StarPiece::exeThrowFall() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensor(this, "attack");
        MR::invalidateClipping(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    tryCalcGravity();
    mVelocity *= 0.97f;
    mVelocity += mGravity * 1.0f;

    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    if (MR::isBinded(this)) {
        if (MR::sendMsgToBindedSensor(ACTMES_IS_STAR_PIECE_REFLECT, this, getSensor("attack"))) {
            setReflect(MR::getBindedNormal(this), MR::getBindedHitPos(this));
            return;
        }

        if (MR::sendMsgToBindedSensor(ACTMES_STAR_PIECE_ATTACK, this, getSensor("attack"))) {
            MR::startActionSound(this, "OjStarPieceHitEnemy");
        }

        kill();
        return;
    }

    if (MR::isGreaterStep(this, 60)) {
        kill();
    }
}

void StarPiece::exeFollowPlayer() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::emitEffect(this, "StarPieceFlyingBlur");
        MR::offBind(this);
        MR::invalidateShadow(this, nullptr);
        MR::offCalcShadow(this, nullptr);
    }

    tryGotJudge();
}

void StarPiece::exeRailMove() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::invalidateClipping(this);

        if (!mFlags.isShadowDisabled) {
            MR::validateShadow(this, nullptr);
        } else {
            MR::invalidateShadow(this, nullptr);
        }

        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    tryCalcGravity();
    mRotation.y += 15.0f;
    MR::repeatDegree(&mRotation.y);

    tryGotJudge();
}

void StarPiece::setFollowPlayerAndAppear() {
    appear();
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(StarPiece, HostTypeNrvFollowPlayer));
}

bool StarPiece::tryCalcGravity() {
    TVec3f gravity;
    MR::calcGravityVectorOrZero(this, &gravity, nullptr, 0);

    if (!MR::isNearZero(gravity)) {
        mGravity.set(gravity);
        return true;
    }

    return false;
}

void StarPiece::trySetGravityAndFront(const TVec3f& rVec1) {
    if (!tryCalcGravity()) {
        if (MR::isNearZero(rVec1)) {
            mGravity.set(*MR::getPlayerGravity());
        } else {
            mGravity.set(rVec1);
        }
        MR::normalizeOrZero(&mGravity);
    }

    if (MR::isSameDirection(mGravity, mFront)) {
        mFront.set< f32 >(mGravity.x, mGravity.z, mGravity.y);
    }
}

bool StarPiece::throwToTarget(HitSensor* pSensor, const TVec3f& launchPos, const TVec3f& gravity, f32 arcHeight) {
    throwToTargetCore(*MR::getSensorPos(pSensor), launchPos, gravity, arcHeight, true);
    mTargetSensor = pSensor;
    return true;
}

bool StarPiece::throwToTargetCore(const TVec3f& targetPos, const TVec3f& launchPos, const TVec3f& gravity, f32 arcHeight, bool fallAfterThrow) {
    mGravity.set(gravity);
    mThrowEndPos.set(targetPos);
    mThrowStartPos.set(launchPos);
    // The coefficients are retained in the original state, although exeThrow uses a linear path.
    MR::calcParabolicFunctionParam(&mParabolaA, &mParabolaB, arcHeight, 0.0f);

    MR::showModel(this);
    MR::resetPosition(this, mThrowStartPos);
    MR::invalidateClipping(this);
    makeActorAppeared();

    if (fallAfterThrow) {
        setNerve(GET_NERVE(StarPiece, HostTypeNrvThrow));
    } else {
        setNerve(GET_NERVE(StarPiece, HostTypeNrvThrowNoFall));
    }

    MR::addStarPiece(-1);
    return true;
}

void StarPiece::giftToTarget(StarPieceReceiverInfo* pReceiverInfo, u32 numGift, HitSensor* pSensor, const TVec3f& launchPos) {
    mReceiverInfo = pReceiverInfo;
    mNumGift = numGift;
    mFlags.isGoToPlayer = false;
    mTargetSensor = pSensor;
    MR::setPosition(this, launchPos);
    mFront.set(*MR::getSensorPos(pSensor) - launchPos);
    MR::normalizeOrZero(&mFront);

    MR::showModel(this);
    MR::resetPosition(this);
    appear();
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(StarPiece, HostTypeNrvToTarget));

    f32 scale = 1.0f;

    if (numGift != 1) {
        scale = 2.0f;
    }

    changeScale(scale);
    MR::addStarPiece(-numGift);
}

void StarPiece::goToPlayer(TVec3f initialVelocity) {
    mFront.set(mVelocity);
    MR::normalizeOrZero(&mFront);

    if (MR::isNearZero(mFront)) {
        MR::calcFrontVec(&mFront, this);
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFloating))) {
        if (isEffectLight()) {
            MR::emitEffect(this, "StarPieceLightS");
        } else {
            MR::emitEffect(this, "StarPieceLight");
        }
    } else if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFollowPlayer))) {
        f32 dot = mFront.dot(mGravity);

        if (0.0f < dot) {
            mFront -= mGravity * dot * 2.0f;
        }
    }

    mFlags.isGoToPlayer = true;
    mGettableDelayCounter = 15;
    mPointerCollectDelay = 15;
    mVelocity.set(initialVelocity);
    mFront.set(initialVelocity);

    MR::invalidateClipping(this);
    setNerve(GET_NERVE(StarPiece, HostTypeNrvToTarget));
}

void StarPiece::launch(const TVec3f& launchPos, f32 randomSpeed, f32 verticalSpeed, bool ignoreWater, bool lightEffect, bool quickCollect) {
    MR::setPosition(this, launchPos);
    MR::fn_800145C0(this);
    tryCalcGravity();

    if (MR::isNearZero(mGravity)) {
        MR::calcUpVec(&mGravity, this);
        mGravity *= -1.0f;
        trySetGravityAndFront(mGravity);
    }

    TVec3f randomVelocity(MR::getRandom(-randomSpeed, randomSpeed), MR::getRandom(-randomSpeed, randomSpeed), MR::getRandom(-randomSpeed, randomSpeed));
    mVelocity.set(-mGravity * verticalSpeed);
    mVelocity += randomVelocity;

    if (ignoreWater) {
        mFlags.isInWater = false;
    } else {
        mFlags.isInWater = MR::isInWater(mPosition);

        if (mFlags.isInWater) {
            mVelocity *= 0.5f;
        }
    }

    MR::restrictVelocityToPlaneModeArea(&mVelocity, launchPos);
    MR::resetPosition(this);
    appear();
    setNerve(GET_NERVE(StarPiece, HostTypeNrvFall));
    MR::invalidateClipping(this);

    if (quickCollect) {
        mGettableDelayCounter = 5;
        mPointerCollectDelay = 5;
    } else {
        mGettableDelayCounter = 25;
        mPointerCollectDelay = 25;
    }

    mFlags.isLightEffect = lightEffect;
    MR::getStarPieceDirector()->mNumStarPiecesLaunched++;

    mFlags.isLaunched = true;
}

void StarPiece::launch(const TVec3f& launchPos, const TVec3f& launchDirection, f32 randomSpeed, f32 speed, bool ignoreWater, bool lightEffect, bool quickCollect) {
    MR::setPosition(this, launchPos);
    MR::fn_800145C0(this);
    trySetGravityAndFront(-launchDirection);

    TVec3f randomVelocity(MR::getRandom(-randomSpeed, randomSpeed), MR::getRandom(-randomSpeed, randomSpeed), MR::getRandom(-randomSpeed, randomSpeed));
    mVelocity.set(launchDirection * speed);
    mVelocity += randomVelocity;

    if (ignoreWater) {
        mFlags.isInWater = false;
    } else {
        mFlags.isInWater = MR::isInWater(mPosition);

        if (mFlags.isInWater) {
            mVelocity *= 0.5f;
        }
    }

    MR::restrictVelocityToPlaneModeArea(&mVelocity, launchPos);
    MR::resetPosition(this);
    appear();
    setNerve(GET_NERVE(StarPiece, HostTypeNrvFall));
    MR::invalidateClipping(this);

    if (quickCollect) {
        mGettableDelayCounter = 5;
        mPointerCollectDelay = 5;
    } else {
        mGettableDelayCounter = 25;
        mPointerCollectDelay = 25;
    }

    mFlags.isLightEffect = lightEffect;
    MR::getStarPieceDirector()->mNumStarPiecesLaunched++;

    mFlags.isLaunched = true;
}

void StarPiece::launch(const TVec3f& launchPos, const TVec3f& launchVelocity, bool ignoreWater, bool lightEffect, bool quickCollect) {
    MR::setPosition(this, launchPos);
    MR::fn_800145C0(this);
    trySetGravityAndFront(-launchVelocity);
    mVelocity.set(launchVelocity);

    if (ignoreWater) {
        mFlags.isInWater = false;
    } else {
        mFlags.isInWater = MR::isInWater(mPosition);

        if (mFlags.isInWater) {
            mVelocity *= 0.5f;
        }
    }

    MR::restrictVelocityToPlaneModeArea(&mVelocity, launchPos);
    MR::resetPosition(this, launchPos);
    appear();
    setNerve(GET_NERVE(StarPiece, HostTypeNrvFall));
    MR::invalidateClipping(this);

    if (quickCollect) {
        mGettableDelayCounter = 5;
        mPointerCollectDelay = 5;
    } else {
        mGettableDelayCounter = 25;
        mPointerCollectDelay = 25;
    }

    mFlags.isLightEffect = lightEffect;
    MR::getStarPieceDirector()->mNumStarPiecesLaunched++;

    mFlags.isLaunched = true;
}

f32 StarPiece::calcEffectScale(f32 minScreenRadius, f32 baseScale, bool forceScreenRadius) {
    f32 effectScale = baseScale;
    f32 pointRadius2D = MR::calcPointRadius2D(mPosition, 30.0f);

    // Preserve the original branch order when selecting the screen-space effect size.

    if (forceScreenRadius) {
        effectScale = minScreenRadius / pointRadius2D;
    } else if (pointRadius2D * baseScale < minScreenRadius) {
        effectScale = minScreenRadius / pointRadius2D;
    }

    return effectScale;
}

f32 StarPiece::calcNearCameraScale() {
    f32 pointRadius2D = MR::calcPointRadius2D(mPosition, 30.0f);

    if (30.0f < pointRadius2D) {
        return 30.0f / pointRadius2D;
    }

    return 1.0f;
}

void StarPiece::emitTouchEffect(s32 colorIndex, f32 f1) {
    f32 effectScale = calcEffectScale(f1, 1.0f, false);

    if (200.0f < calcDistToCamera()) {
        MR::emitEffectWithScale(this, "StarPieceTouchLight", effectScale, -1);
        MR::setEffectPrmColor(this, "StarPieceTouchLight", effectColors[colorIndex].r, effectColors[colorIndex].g, effectColors[colorIndex].b);
    }
}

void StarPiece::emitGettableEffect(f32 f1) {
    f32 effectScale = calcEffectScale(f1, 0.8f, true);

    if (200.0f < calcDistToCamera()) {
        MR::emitEffectWithScale(this, "GetAble", effectScale, -1);
    }

    u8 clampedR = MR::clamp((s32)mColor.r + 0xFF, 0x00, 0xFF);
    u8 clampedG = MR::clamp((s32)mColor.g + 0xFF, 0x00, 0xFF);
    u8 clampedB = MR::clamp((s32)mColor.b + 0xFF, 0x00, 0xFF);
    u8 clampedR2 = MR::clamp((s32)mColor.r + 0x20, 0x00, 0xFF);
    u8 clampedG2 = MR::clamp((s32)mColor.g + 0x20, 0x00, 0xFF);
    u8 clampedB2 = MR::clamp((s32)mColor.b + 0x20, 0x00, 0xFF);
    MR::setEffectColor(this, "GetAble", clampedR, clampedG, clampedB, clampedR2, clampedG2, clampedB2);
}

void StarPiece::tryGotJudge() {
    if (MR::isNormalTalking()) {
        return;
    }

    if (225000000.0f < (*MR::getPlayerCenterPos() - mPosition).squared() || !isCollectionEnabled()) {
        mGettableEffectTimer = -1;
        return;
    }

    mGettableEffectTimer += 91;
    mGettableEffectTimer %= 90;

    if (mGettableEffectTimer == 20) {
        emitGettableEffect(4.0f);
    }

    if (mPointerCollectDelay < 0) {
        bool isPointing = MR::isStarPointerPointing1Por2P(this, "弱", false, false);
        MR::getStarPointerLastPointedPort(this);

        if (isPointing == true) {
            goToPlayer(TVec3f(mFront));
        }
    }
}

bool StarPiece::isFollowing() {
    return isNerve(GET_NERVE(StarPiece, HostTypeNrvFollowPlayer));
}

bool StarPiece::setFall() {
    if (MR::isDead(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvFloating))) {
        setCollectionEnabled(true);
        setNerve(GET_NERVE(StarPiece, HostTypeNrvFall));
        return true;
    }

    return false;
}

bool StarPiece::isOnRailMove() {
    if (MR::isDead(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove))) {
        return true;
    }

    return false;
}

bool StarPiece::isFloat() {
    if (MR::isDead(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFloating))) {
        return true;
    }

    return false;
}

void StarPiece::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEye(pSender)) {
        return;
    }

    if (!isNerve(GET_NERVE(StarPiece, HostTypeNrvThrow)) && !isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowNoFall)) &&
        !isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowFall))) {
        return;
    }

    if (MR::sendArbitraryMsg(ACTMES_IS_STAR_PIECE_REFLECT, pReceiver, pSender)) {
        TVec3f dirReceiverToSender(*MR::getSensorPos(pSender));
        dirReceiverToSender -= *MR::getSensorPos(pReceiver);
        MR::normalizeOrZero(&dirReceiverToSender);

        TVec3f vec2(*MR::getSensorPos(pReceiver));
        vec2.add(dirReceiverToSender * MR::getSensorRadius(pReceiver));
        setReflect(dirReceiverToSender, vec2);
        return;
    }

    if (MR::sendMsgStarPieceAttack(pReceiver, pSender)) {
        MR::startActionSound(this, "OjStarPieceHitEnemy");
        kill();
    }
}

void StarPiece::setReflect(const TVec3f& rVec1, const TVec3f& rVec2) {
    if (!MR::isNearZero(rVec1)) {
        MR::calcReflectionVector(&mVelocity, rVec1, 0.5f, 0.0f);
    }

    mVelocity.add(mGravity * -35.0f);

    if (200.0f < calcDistToCamera()) {
        MR::emitEffectHit(this, rVec2, "InvalidHitMark");
    }

    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::startActionSound(this, "OjStarPieceReflectW");
    } else {
        MR::startActionSound(this, "OjStarPieceReflect");
    }

    setNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect));
}

bool StarPiece::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInhaleBlackHole(msg)) {
        if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFall)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvFallAfterReflect)) ? true : false) {
            kill();
            return true;
        }
    }

    if (MR::isMsgItemGet(msg)) {
        if (isNerve(GET_NERVE(StarPiece, HostTypeNrvThrow)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowNoFall)) ||
            isNerve(GET_NERVE(StarPiece, HostTypeNrvThrowFall))) {
            return false;
        }

        if (isNerve(GET_NERVE(StarPiece, HostTypeNrvToPlayerEnd))) {
            return true;
        }

        if (mFlags.isGoToPlayer && isNerve(GET_NERVE(StarPiece, HostTypeNrvToTarget))) {
            return true;
        }

        if (isNerve(GET_NERVE(StarPiece, HostTypeNrvFloating)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvFall)) ||
            isNerve(GET_NERVE(StarPiece, HostTypeNrvSpinDrained)) || isNerve(GET_NERVE(StarPiece, HostTypeNrvRailMove))) {
            if (mGettableDelayCounter < 0) {
                touchPlayer();
                MR::notifyDirectGetStarPiecePlayer();
            }

            return true;
        }
    }

    if (MR::isMsgStartPowerStarGet(msg)) {
        if ((isNerve(GET_NERVE(StarPiece, HostTypeNrvToTarget)) && mFlags.isGoToPlayer) || isNerve(GET_NERVE(StarPiece, HostTypeNrvSpinDrained)) ||
            isNerve(GET_NERVE(StarPiece, HostTypeNrvToPlayerEnd))) {
            MR::gotByPlayer();
        }

        makeActorDead();
        return true;
    }

    return false;
}

bool StarPiece::isIgnoreTriOnThrow(const Triangle* pTriangle) {
    return 0.0f <= pTriangle->getFaceNormal()->dot(mVelocity);
}

bool StarPiece::touchPlayer() {
    if (MR::gotByPlayer()) {
        if (200.0f < calcDistToCamera()) {
            if (isEffectLight()) {
                MR::emitEffect(this, "StarPieceGetS");
            } else {
                MR::emitEffect(this, "StarPieceGet");
            }
        }

        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    }

    MR::stopSound(this, "SE_OJ_STAR_PIECE_FLY_W");
    MR::stopSound(this, "SE_OJ_STAR_PIECE_FLY");

    MR::forceDeleteEffect(this, "StarPieceFlyingBlur");
    setNerve(GET_NERVE(StarPiece, HostTypeNrvToPlayerEnd));
    MR::invalidateHitSensor(this, "Body");
    return true;
}

void StarPiece::calcAndSetBaseMtx() {
    TPos3f pos;

    bool gravityZero = MR::isNearZero(mGravity);
    bool frontZero = MR::isNearZero(mFront);

    if (gravityZero && frontZero) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        MR::makeMtxUpNoSupportPos(&pos, up, mPosition);
    } else if (gravityZero) {
        MR::makeMtxFrontNoSupportPos(&pos, mFront, mPosition);
    } else if (frontZero || MR::isSameDirection(mGravity, mFront)) {
        MR::makeMtxUpNoSupportPos(&pos, -mGravity, mPosition);
    } else {
        MR::calcMtxFromGravityAndZAxis(&pos, this, mGravity, mFront);
    }

    TPos3f rotate;
    rotate.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), MR::toRadian(mRotation.y));
    pos.concat(pos, rotate);
    MR::setBaseTRMtx(this, pos);
}

bool StarPiece::isNerveToPlayerEnd() {
    return isNerve(GET_NERVE(StarPiece, HostTypeNrvToPlayerEnd));
}

void StarPiece::setHostInfo(StarPieceHostInfo* pHostInfo) {
    mHostInfo = pHostInfo;
}
