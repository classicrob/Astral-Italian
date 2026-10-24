#define JGEOMETRY_VEC3_MI_NOINLINE
#include "Game/Ride/PlantSlider.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PlantRailInfo.hpp"
#include "Game/MapObj/StarPieceFlower.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include "Game/Util/Color.hpp"
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXVert.h>
#include <revolution/gx/GXTransform.h>
#include <revolution/gx/GXTev.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/mtx.h>
#include <revolution/types.h>



namespace NrvPlantSlider {
    NEW_NERVE(PlantSliderNrvWait, PlantSlider, Wait);
    NEW_NERVE(PlantSliderNrvCooldown, PlantSlider, Cooldown);
    NEW_NERVE(PlantSliderNrvStart, PlantSlider, Start);
    NEW_NERVE(PlantSliderNrvSlide, PlantSlider, Slide);
};  // namespace NrvPlantSlider

namespace {
    extern s32 sLeafPointStart;
    extern const s32 sSpringRange;
    extern const f32 sTaperRange;
    Color8 sColorWhite(0xFF, 0xFF, 0xFF, 0xFF);
    Color8 sColorGray(0x64, 0x64, 0x64, 0xFF);
    Color8 sColorLight(0x96, 0x96, 0x96, 0xFF);
};  // namespace

void writeVertexPos(f32 x, f32 y, f32 z);

PlantSlider::PlantSlider(const char* pName)
    : LiveActor(pName), mNumPoints(0), mSliderPoints(nullptr), mRailInfo(nullptr), mRider(nullptr), mRailCoord(0.0f),
      mRailDirection(0.0f, 0.0f, 1.0f), mUp(0.0f, 1.0f, 0.0f), mSide(1.0f, 0.0f, 0.0f), mSlideSpeed(0.0f), mRideSpeed(0.0f),
      mRideAccel(0.0f), mStartDelay(20.0f), mAppearSpeed(25.0f), mCameraInfo(nullptr), mCameraTargetMtx(nullptr),
      mPlayAppearDemo(false), mSlideSpeedFactor(1.5f), mNumFlowers(0), mFlowerGroup(nullptr), mNumLeaves(0), mLeafGroup(nullptr),
      mLeafMtxs(nullptr), mTexture(nullptr), mClippingCenter(0.0f, 0.0f, 0.0f), mIsNoClipping(false) {
}

// NONMATCHING: floating-point allocation and vector temporary stack placement differ.
void PlantSlider::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToScene(this, 0x28, -1, -1, 5);

    initSound(8, "PlantSlider", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(PlantSlider, PlantSliderNrvWait));

    initRailRider(rIter);
    MR::initAndSetRailClipping(&mClippingCenter, this, 100.0f, 500.0f);
    MR::calcRailStartPointPos(&mPosition, this);

    MR::getJMapInfoArg0NoInit(rIter, &mIsNoClipping);
    MR::getJMapInfoArg1NoInit(rIter, &mStartDelay);
    MR::getJMapInfoArg2NoInit(rIter, &mAppearSpeed);

    f32 speedFactor = -1.0f;
    MR::getJMapInfoArg3NoInit(rIter, &speedFactor);
    if (speedFactor > 0.0f) {
        mSlideSpeedFactor = speedFactor / 10.0f;
    }

    MR::getJMapInfoArg4NoInit(rIter, &mPlayAppearDemo);

    mRailInfo = new PlantRailInfo(rIter, 30.0f);
    mNumPoints = mRailInfo->mNumPlantPoints;
    mSliderPoints = new PlantSliderPoint*[mNumPoints];

    TVec3f axis(0.0f, 0.0f, 1.0f);
    MR::calcActorAxisZ(&axis, this);
    axis.add(mGravity);
    MR::normalize(&axis);

    for (s32 i = 0; i < mNumPoints; i++) {
        f32 ease = 1.0f;
        f32 n = (f32)(mNumPoints - 1 - i);
        if (n < sTaperRange) {
            ease = MR::getEaseOutValue((f32)(mNumPoints - 1 - i) / sTaperRange, 0.0f, ease, ease);
        }

        mSliderPoints[i] = new PlantSliderPoint(mRailInfo->mPlantPoints[i]->mPosition, mRailInfo->mPlantPoints[i]->mUp, axis, ease);
    }

    initHitSensor(3);
    MR::addHitSensorCallbackBinder(this, "Bind", 5, 80.0f);
    MR::addHitSensorRide(this, "Ride", 8, 50.0f, TVec3f(0.0f, 0.0f, 0.0f));

    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "滑り");
    mCameraTargetMtx = new CameraTargetMtx("つるスライダー注視用");

    mTexture = MR::loadTexFromArc(this, "Plant.arc", "PlantStalk.bti");

    mNumFlowers = (mNumPoints - 7) / 10;
    mFlowerGroup = new LiveActorGroup("花保持", mNumFlowers);
    mFlowerGroup->initWithoutIter();

    for (s32 i = 0; i < mNumFlowers; i++) {
        StarPieceFlower* pFlower = new StarPieceFlower("スターピース花");
        PlantSliderPoint* pPoint = mSliderPoints[7 + i * 10];
        TVec3f up(0.0f, 1.0f, 0.0f);
        TVec3f side(pPoint->mSide);
        side.add(up);
        MR::normalize(&side);

        pFlower->mUp.set(side);
        pFlower->mTargetPos = &pPoint->mPosition;
        pFlower->mPosition.set(pPoint->mPosition);
        pFlower->initWithoutIter();
        mFlowerGroup->registerActor(pFlower);
        if (mIsNoClipping) {
            MR::invalidateClipping(pFlower);
        }
    }

    s32 leafStart = sLeafPointStart;
    mNumLeaves = (mNumPoints - leafStart) / 30;
    mLeafGroup = new LiveActorGroup("葉っぱ保持", mNumLeaves);
    mLeafGroup->initWithoutIter();

    mLeafMtxs = new TPos3f[mNumLeaves];
    for (s32 i = 0; i < mNumLeaves; i++) {
        PlantSliderPoint* pPoint = mSliderPoints[leafStart + i * 30];

        TVec3f gravity(mGravity);
        gravity.scale(2.0f);
        TVec3f side(pPoint->mSide);
        side.scale(1.5f);
        TVec3f up(pPoint->mUp);
        up.add(side);
        TVec3f front(up);
        front.add(gravity);
        MR::normalize(&front);

        MR::makeMtxUpFrontPos(&mLeafMtxs[i], front, pPoint->mSide, pPoint->mPosition);

        LiveActor* pLeaf = reinterpret_cast<LiveActor*>(MR::createDemoObj("つるスライダーの葉", "PlantSliderLeaf", mLeafMtxs[i].toMtxPtr()));
        pLeaf->initWithoutIter();
        pLeaf->makeActorAppeared();
        mLeafGroup->registerActor(pLeaf);
        if (mIsNoClipping) {
            MR::invalidateClipping(pLeaf);
        }
    }

    makeActorAppeared();
}

void PlantSlider::exeCooldown() {
    updatePoints();
    if (MR::isStep(this, 60) || MR::isOnGroundPlayer()) {
        setNerve(GET_NERVE(PlantSlider, PlantSliderNrvWait));
    }
}

// NONMATCHING: equivalent early-return branch layout differs.
void PlantSlider::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("PlantSliderStart", nullptr);
        MR::setRailDirectionToEnd(this);
        MR::startLevelSoundPlayer("Catch", -1, -1);
    }

    updateSlide();

    if (MR::isGreaterEqualStep(this, 15) && tryReachGoal()) {
        return;
    }
    if (!MR::isActionEnd(mRider)) {
        return;
    }
    setNerve(GET_NERVE(PlantSlider, PlantSliderNrvSlide));
}

// NONMATCHING: equivalent early-return branch layout differs.
void PlantSlider::exeSlide() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("PlantSlider", nullptr);
        MR::setRailDirectionToEnd(this);
    }
    MR::startActionSound(this, "SlideDown", -1, -1, -1);

    if (tryReachGoal()) {
        return;
    }
    updateSlide();
}

void PlantSlider::startClipped() {
    LiveActor::startClipped();
    if (mIsNoClipping) {
        for (s32 i = 0; i < mNumFlowers; i++) {
            mFlowerGroup->getActor(i)->startClipped();
        }
        for (s32 i = 0; i < mNumLeaves; i++) {
            mLeafGroup->getActor(i)->startClipped();
        }
    }
}

void PlantSlider::endClipped() {
    LiveActor::endClipped();
    if (mIsNoClipping) {
        for (s32 i = 0; i < mNumFlowers; i++) {
            mFlowerGroup->getActor(i)->endClipped();
        }
        for (s32 i = 0; i < mNumLeaves; i++) {
            mLeafGroup->getActor(i)->endClipped();
        }
    }
}

// NONMATCHING: loop setup scheduling and stack temporary placement differ.
void PlantSlider::control() {
    const TVec3f* gravityPos;
    s32 leafStart;
    PlantSliderPoint* pPoint;
    s32 i = 0;
    if (mNumLeaves <= 0) return;
    leafStart = sLeafPointStart;
    gravityPos = &mGravity;
    for (; i < mNumLeaves; i++) {
        pPoint = mSliderPoints[leafStart + i * 30];

        TVec3f front = (pPoint->mUp + pPoint->mSide.scaleInline(1.5f)) + gravityPos->scaleInline(2.0f);
        MR::normalize(&front);

        MR::makeMtxUpFrontPos(&mLeafMtxs[i], front, pPoint->mSide, pPoint->mPosition);
    }
}

// NONMATCHING: sensor-position loads are scheduled after the player-position calls.
void PlantSlider::updateHitSensor(HitSensor* pSensor) {
    MR::setSensorPos(pSensor, mSliderPoints[2]->mPosition);
    const TVec3f* first = &mSliderPoints[2]->mPosition;
    f32 minDist = PSVECDistance(first, MR::getPlayerCenterPos());

    for (s32 i = 3; i < mNumPoints - 2; i++) {
        const TVec3f* pos = &mSliderPoints[i]->mPosition;
        f32 dist = PSVECDistance(pos, MR::getPlayerCenterPos());
        if (dist < minDist) {
            minDist = dist;
            MR::setSensorPos(pSensor, mSliderPoints[i]->mPosition);
        }
    }
}

void PlantSlider::attackSensor(HitSensor*, HitSensor*) {
}

// Matrix-copy and vector-subtraction helpers are expanded differently by the compiler.
// NONMATCHING: vector temporary slots and matrix-copy helper inlining differ.
bool PlantSlider::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (!isNerve(GET_NERVE(PlantSlider, PlantSliderNrvCooldown)) && MR::isOnGroundPlayer()) {
            return false;
        }

        mRider = MR::getSensorHost(pSender);
        MR::invalidateClipping(this);
        MR::moveCoordAndTransToNearestRailPos(this, *MR::getPlayerPos());
        mRideAccel = 0.0f;
        mRideSpeed = 0.0f;

        mRailDirection.set(MR::getRailDirection(this));
        TVec3f negGravity(-mGravity);
        mUp.set(negGravity);
        MR::makeAxisFrontUp(&mSide, &mUp, mRailDirection, mUp);

        const TVec3f* pGravity = MR::getPlayerGravity();
        const TVec3f* pVelocity = MR::getPlayerVelocity();
        mSlideSpeed = -pVelocity->dot(*pGravity);

        mRailCoord = MR::calcNearestRailCoord(this, *MR::getPlayerCenterPos());
        mSlideSpeed = MR::clamp(mSlideSpeed, 5.0f, 23.0f);

        MR::setRailDirectionToStart(this);
        MR::tryRumblePadWeak(this, 0);
        MR::startMultiActorCameraNoTarget(this, mCameraInfo, "滑り", -1);

        mCameraTargetMtx->mMatrix.set(MR::getPlayerBaseMtx());
        CameraTargetArg arg(mCameraTargetMtx);
        MR::setGameCameraTarget(arg);

        setNerve(GET_NERVE(PlantSlider, PlantSliderNrvStart));
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        TPos3f mtx;
        mtx.identity();
        if (mPlayAppearDemo) {
            TVec3f negGravity(-mGravity);
            MR::makeMtxFrontUpPos(&mtx, mRailDirection, negGravity, mPosition);
        } else {
            TVec3f front(mRailDirection);
            MR::vecKillElement(front, mSide, &front);
            MR::normalize(&front);
            MR::makeMtxFrontSidePos(&mtx, front, mSide, mPosition);
        }
        MR::getRailTotalLength(this);
        MR::getRailCoord(this);
        TVec3f drop(0.0f, mRideSpeed, 0.0f);
        TVec3f up(mUp);
        up.scale(20.0f);
        TVec3f pos(mPosition);
        pos.add(up);
        TVec3f trans(pos);
        trans -= drop;
        mtx.setTrans(trans);
        MR::setBaseTRMtx(mRider, mtx);
        return true;
    }

    if (MR::isMsgRushCancel(msg)) {
        MR::setGameCameraTargetToPlayer();
        MR::endMultiActorCamera(this, mCameraInfo, "滑り", false, -1);
        mRider = nullptr;
        setNerve(GET_NERVE(PlantSlider, PlantSliderNrvCooldown));
        return true;
    }

    return false;
}

bool PlantSlider::tryReachGoal() {
    if (!MR::isRailReachedGoal(this)) {
        return false;
    }

    MR::moveCoordToEndPos(this);

    TVec3f endPos;
    MR::calcRailEndPointPos(&endPos, this);
    endPos.y -= mRideSpeed;
    MR::setPlayerPos(endPos);

    TVec3f killed(*getPointUp(mRailInfo, mRailInfo->mNumPlantPoints - 1));
    MR::vecKillElement(killed, mGravity, &killed);
    if (!MR::isNearZero(killed, 0.001f)) {
        MR::normalize(&killed);
        MR::setPlayerFrontTargetVec(killed, 1);
    }

    TVec3f camUp(*getPointUp(mRailInfo, mRailInfo->mNumPlantPoints - 1));
    camUp.scale(mStartDelay);
    f32 speed = -mAppearSpeed;
    TVec3f negGravity(mGravity);
    negGravity.scale(speed);
    camUp.add(negGravity);

    MR::startBckPlayer("PlantSliderJump", nullptr);
    MR::startLevelSoundPlayer("JumpFromPlant", -1, -1);
    MR::endMultiActorCamera(this, mCameraInfo, "滑り", false, -1);
    MR::setGameCameraTargetToPlayer();

    MR::restrictVelocityToPlaneModeArea(&camUp, mPosition);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, camUp);

    mRider = nullptr;
    setNerve(GET_NERVE(PlantSlider, PlantSliderNrvCooldown));
    return true;
}

// NONMATCHING: matrix-copy helper inlining and register scheduling differ.
void PlantSlider::updateSlide() {
    f32 gravityComp = MR::clamp(-MR::getRailDirection(this).y, 0.0f, 1.0f);
    mSlideSpeed = mSlideSpeed * 0.998f + gravityComp * 1.3f;
    mSlideSpeed = MR::clamp(mSlideSpeed, 5.0f, 23.0f);
    MR::moveCoordAndFollowTrans(this, mSlideSpeed);
    mRailDirection.set(MR::getRailDirection(this));
    TVec3f dir(mRailDirection);
    MR::vecKillElement(dir, mSide, &dir);
    if (!MR::isNearZero(dir, 0.001f)) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::normalize(&dir);
        MR::vecKillElement(mUp, mSide, &up);
        if (!MR::isNearZero(up, 0.001f)) {
            MR::normalize(&up);
            mUp.set(up);
        }
    }
    f32 length = MR::getRailTotalLength(this);
    f32 ease = MR::getEaseOutValue(MR::getRailCoord(this) / length, 0.0f, 1.0f, 1.0f);
    f32 flat = MR::getEaseOutValue(1.0f - __fabsf(MR::getRailDirection(this).y), 0.0f, 1.0f, 1.0f);
    mRideAccel = mRideAccel * 0.95f - mRideSpeed * 0.005f;
    mRideAccel += ease * (mSlideSpeedFactor * MR::clamp(flat + 0.2f, 0.0f, 1.0f));
    mRideSpeed += mRideAccel;
    updateSpringPoints();
    TPos3f mtx;
    mtx.set(MR::getPlayerBaseMtx());
    mtx.setTrans(mPosition);
    mCameraTargetMtx->mMatrix.set(mtx);
}

#pragma dont_inline on
TVec3f* PlantSlider::getPointUp(PlantRailInfo* pRailInfo, s32 index) {
    return &pRailInfo->mPlantPoints[index]->mUp;
}


#pragma dont_inline reset
// NONMATCHING: loop load scheduling and register allocation differ.
void PlantSlider::updatePoints() {
    PlantSliderPoint* point;
    for (s32 i = 0; i < mNumPoints; i++) {
        point = mSliderPoints[i];
        f32 t = (f32)i / (f32)(mNumPoints - 1);
        f32 value = MR::getLinerValue(t, 0.15f, 0.035f, 1.0f);
        point->updateSpring(value, 0.92f);
    }
}

// NONMATCHING: integer conversion spills and register allocation differ.
void PlantSlider::updateSpringPoints() {
    f32 railCoord = MR::getRailCoord(this);
    s32 coord = MR::clamp((s32)(railCoord / 30.0f), 0, mNumPoints - 1);

    for (s32 i = 1; i < coord; i++) {
        f32 ease = MR::getEaseOutValue((f32)i / (f32)(coord - 1), 0.0f, 1.0f, 1.0f);
        f32 lin = MR::getLinerValue((f32)i / (f32)(mNumPoints - 1), 0.01f, 0.001f, 1.0f);
        PlantSliderPoint* point = mSliderPoints[i];
        f32 clamped = MR::clamp(ease, 0.0f, 1.0f);
        point->updateSprings(mRideSpeed * clamped, ease, lin, 0.9f);
    }
    if (coord > 0) mSliderPoints[coord]->updateSprings(mRideSpeed, 1.0f, 0.0f, 1.0f);
    s32 end = coord + 60;
    if (end >= mNumPoints) end = mNumPoints - 1;
    for (s32 i = coord + 1; i <= end; i++) {
        f32 ease = MR::getEaseOutValue((f32)(coord + 61 - i) / (f32)(s32)sSpringRange, 0.0f, 1.0f, 1.0f);
        f32 lin = MR::getLinerValue((f32)i / (f32)(mNumPoints - 1), 0.01f, 0.001f, 1.0f);
        PlantSliderPoint* point = mSliderPoints[i];
        f32 blend = MR::getLinerValue(ease, 0.8f, 1.0f, 1.0f);
        point->updateSprings(mRideSpeed, blend, lin, 0.9f);
    }
    for (s32 i = end + 1; i < mNumPoints; i++) {
        f32 lin = MR::getLinerValue((f32)i / (f32)(mNumPoints - 1), 0.01f, 0.001f, 1.0f);
        mSliderPoints[i]->updateSprings(mRideSpeed, 0.8f, lin, 0.9f);
    }
}

void PlantSlider::initDrawSetting() const {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXLoadPosMtxImm(MR::getCameraViewMtx(), 0);
    GXSetCurrentMtx(0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXColor color = {0xFF, 0x00, 0x00, 0xFF};
    GXSetChanMatColor(GX_COLOR0A0, color);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60, GX_FALSE, 125);
    mTexture->load(GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);
}

// NONMATCHING: the first strip exchanges the point and loop-index registers.
void PlantSlider::draw() const {
    if (!MR::isValidDraw(this)) {
        return;
    }
    initDrawSetting();
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, mNumPoints * 2);
    const PlantSliderPoint* pPoint;
    for (s32 i = 0; i < mNumPoints; i++) {
        pPoint = mSliderPoints[i];
        TVec3f side(pPoint->mSide);
        TVec3f front(mSliderPoints[i]->mFront);
        side.scale(mSliderPoints[i]->mThickness);
        front.scale(mSliderPoints[i]->mThickness);
        writeVertexPos(pPoint->mPosition.x + 10.0f * front.x, pPoint->mPosition.y + 10.0f * front.y, pPoint->mPosition.z + 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorWhite.r, sColorWhite.g, sColorWhite.b, sColorWhite.a);
        GXTexCoord2f32(0.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorWhite.mColor;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32)i;
#endif
        writeVertexPos(pPoint->mPosition.x + 10.0f * side.x - 10.0f * front.x, pPoint->mPosition.y + 10.0f * side.y - 10.0f * front.y, pPoint->mPosition.z + 10.0f * side.z - 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorGray.r, sColorGray.g, sColorGray.b, sColorGray.a);
        GXTexCoord2f32(1.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorGray.mColor;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = (f32)i;
#endif
    }
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, mNumPoints * 2);
    for (s32 i = 0; i < mNumPoints; i++) {
        const PlantSliderPoint* pPoint = mSliderPoints[i];
        TVec3f side(pPoint->mSide);
        TVec3f front(mSliderPoints[i]->mFront);
        side.scale(mSliderPoints[i]->mThickness);
        front.scale(mSliderPoints[i]->mThickness);
        writeVertexPos(pPoint->mPosition.x - 10.0f * side.x - 10.0f * front.x, pPoint->mPosition.y - 10.0f * side.y - 10.0f * front.y, pPoint->mPosition.z - 10.0f * side.z - 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorLight.r, sColorLight.g, sColorLight.b, sColorLight.a);
        GXTexCoord2f32(0.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorLight.mColor;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32)i;
#endif
        writeVertexPos(pPoint->mPosition.x + 10.0f * front.x, pPoint->mPosition.y + 10.0f * front.y, pPoint->mPosition.z + 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorWhite.r, sColorWhite.g, sColorWhite.b, sColorWhite.a);
        GXTexCoord2f32(1.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorWhite.mColor;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = (f32)i;
#endif
    }
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, mNumPoints * 2);
    for (s32 i = 0; i < mNumPoints; i++) {
        const PlantSliderPoint* pPoint = mSliderPoints[i];
        TVec3f side(pPoint->mSide);
        TVec3f front(mSliderPoints[i]->mFront);
        side.scale(mSliderPoints[i]->mThickness);
        front.scale(mSliderPoints[i]->mThickness);
        writeVertexPos(pPoint->mPosition.x + 10.0f * side.x - 10.0f * front.x, pPoint->mPosition.y + 10.0f * side.y - 10.0f * front.y, pPoint->mPosition.z + 10.0f * side.z - 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorGray.r, sColorGray.g, sColorGray.b, sColorGray.a);
        GXTexCoord2f32(0.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorGray.mColor;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32)i;
#endif
        writeVertexPos(pPoint->mPosition.x - 10.0f * side.x - 10.0f * front.x, pPoint->mPosition.y - 10.0f * side.y - 10.0f * front.y, pPoint->mPosition.z - 10.0f * side.z - 10.0f * front.z);
#ifdef GALAXY_NATIVE
        GXColor4u8(sColorLight.r, sColorLight.g, sColorLight.b, sColorLight.a);
        GXTexCoord2f32(1.0f, static_cast< f32 >(i));
#else
        GXWGFifo.u32 = sColorLight.mColor;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = (f32)i;
#endif
    }
}

// out-of-line FIFO write of one position (fn_804386A0)
void writeVertexPos(f32 x, f32 y, f32 z) {
#ifdef GALAXY_NATIVE
    GXPosition3f32(x, y, z);
#else
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
#endif
}

PlantSlider::~PlantSlider() {
}

void PlantSlider::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
    updatePoints();
}

PlantSliderPoint::PlantSliderPoint(const TVec3f& pPos, const TVec3f& pUp, const TVec3f& pAxis, f32 thickness)
    : PlantPoint(pPos, pUp, thickness, pAxis), mBasePos(pPos), mVelocity(0.0f, 0.0f, 0.0f), mTargetPos(pPos) {
}

#pragma dont_inline on
// NONMATCHING: vector temporary stack placement differs.
void PlantSliderPoint::updateSpring(f32 pScale, f32 pDamping) {
    mVelocity.scale(pDamping);
    TVec3f diff = mBasePos - mPosition;
    TVec3f offset(diff);
    offset.scale(pScale);
    mVelocity += offset;
    mPosition += mVelocity;
    mTargetPos.x = mPosition.x;
    mTargetPos.y = mPosition.y;
    mTargetPos.z = mPosition.z;
}

#pragma dont_inline reset

// Vector operators are expanded differently by the compiler; the spring and blend operands follow the target.
// NONMATCHING: vector temporary stack placement and operator+= inlining differ.
void PlantSliderPoint::updateSprings(f32 pHeight, f32 pInterp, f32 pDamping, f32 pAccel) {
    TVec3f base(mBasePos);
    base.y -= pHeight;
    mVelocity.scale(pAccel);
    TVec3f offset = (mBasePos - mPosition).scaleInline(pDamping);
    mVelocity += offset;
    mTargetPos += mVelocity;
    TVec3f pos = base.scaleInline(pInterp) + mTargetPos.scaleInline(1.0f - pInterp);
    mPosition = pos;
}

namespace {
    s32 sLeafPointStart = 0;
    const s32 sSpringRange = 60;
    const f32 sTaperRange = 16.0f;
}
