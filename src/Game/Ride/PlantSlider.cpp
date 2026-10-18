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

// PLACEHOLDER(objects): unnamed core helper (copies/sets a TVec3f from a const TVec3f&); resolve when core names fn_800145D0
extern "C" void fn_800145D0(TVec3f*, const TVec3f&);

namespace {
    Color8 sColorWhite(0xFF, 0xFF, 0xFF, 0xFF);
    Color8 sColorGray(0x64, 0x64, 0x64, 0xFF);
    Color8 sColorLight(0x96, 0x96, 0x96, 0xFF);
};  // namespace

namespace NrvPlantSlider {
    NEW_NERVE(PlantSliderNrvWait, PlantSlider, Wait);
    NEW_NERVE(PlantSliderNrvCooldown, PlantSlider, Cooldown);
    NEW_NERVE(PlantSliderNrvStart, PlantSlider, Start);
    NEW_NERVE(PlantSliderNrvSlide, PlantSlider, Slide);
};  // namespace NrvPlantSlider

void writeVertexPos(f32 x, f32 y, f32 z);

PlantSlider::PlantSlider(const char* pName)
    : LiveActor(pName), mNumPoints(0), mSliderPoints(nullptr), mRailInfo(nullptr), mRider(nullptr), mRailCoord(0.0f),
      mRailDirection(0.0f, 0.0f, 1.0f), mUp(0.0f, 1.0f, 0.0f), mSide(1.0f, 0.0f, 0.0f), mSlideSpeed(0.0f), mRideSpeed(0.0f),
      mRideAccel(0.0f), mStartDelay(20.0f), mAppearSpeed(25.0f), mCameraInfo(nullptr), mCameraTargetMtx(nullptr),
      mPlayAppearDemo(false), mSlideSpeedFactor(1.5f), mNumFlowers(0), mFlowerGroup(nullptr), mNumLeaves(0), mLeafGroup(nullptr),
      mLeafMtxs(nullptr), mTexture(nullptr), mClippingCenter(0.0f, 0.0f, 0.0f), mIsNoClipping(false) {
}

void PlantSlider::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToScene(this, 0x28, -1, -1, 5);

    TVec3f zero(0.0f);
    initSound(8, "PlantSlider", nullptr, zero);
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

    f32 ease = 1.0f;
    for (s32 i = 0; i < mNumPoints; i++) {
        f32 n = (f32)(mNumPoints - 1 - i);
        if (n < 16.0f) {
            ease = MR::getEaseOutValue(n / 16.0f, 0.0f, ease, ease);
        }

        PlantPoint* pPoint = mRailInfo->mPlantPoints[i];
        mSliderPoints[i] = new PlantSliderPoint(pPoint->mPosition, pPoint->mUp, axis, ease);
    }

    initHitSensor(3);
    MR::addHitSensorCallbackBinder(this, "Bind", 5, 80.0f);
    TVec3f sensorOffset(0.0f);
    MR::addHitSensorRide(this, "Ride", 8, 50.0f, sensorOffset);

    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "滑り");
    mCameraTargetMtx = new CameraTargetMtx("つるスライダー注視用");

    mTexture = MR::loadTexFromArc(this, "Plant.arc", "PlantStalk.bti");

    mNumFlowers = (mNumPoints - 7) / 10;
    mFlowerGroup = new LiveActorGroup("花保持", mNumFlowers);
    mFlowerGroup->initWithoutIter();

    for (s32 i = 0; i < mNumFlowers; i++) {
        PlantSliderPoint* pPoint = mSliderPoints[7 + i * 10];

        StarPieceFlower* pFlower = new StarPieceFlower("スターピース花");
        TVec3f up(0.0f, 1.0f, 0.0f);
        TVec3f side(pPoint->mSide);
        side.add(up);
        MR::normalize(&side);

        pFlower->mUp.set(side);
        pFlower->mTargetPos = &pPoint->mPosition;
        pFlower->mPosition.set(side);
        pFlower->initWithoutIter();
        mFlowerGroup->registerActor(pFlower);
        if (mIsNoClipping) {
            MR::invalidateClipping(pFlower);
        }
    }

    mNumLeaves = (mNumPoints - 7) / 30;
    mLeafGroup = new LiveActorGroup("葉っぱ保持", mNumLeaves);
    mLeafGroup->initWithoutIter();

    mLeafMtxs = new TPos3f[mNumLeaves];
    for (s32 i = 0; i < mNumLeaves; i++) {
        PlantSliderPoint* pPoint = mSliderPoints[7 + i * 30];

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

        LiveActor* pLeaf = reinterpret_cast<LiveActor*>(MR::createDemoObj("つるスライダーの葉", "PlantSliderLeaf", reinterpret_cast<MtxPtr>(&mLeafMtxs[i])));
        pLeaf->initWithoutIter();
        pLeaf->makeActorAppeared();
        mLeafGroup->registerActor(pLeaf);
        if (mIsNoClipping) {
            MR::invalidateClipping(pLeaf);
        }
    }

    makeActorAppeared();
}

// NONMATCHING: returns the up vector of a rail point (point->mUp); the caller-visible contract is taken from the asm
TVec3f* PlantSlider::getPointUp(PlantRailInfo* pRailInfo, s32 index) {
    return &pRailInfo->mPlantPoints[index]->mUp;
}

void PlantSlider::exeCooldown() {
    updatePoints();
    if (MR::isStep(this, 60) && MR::isOnGroundPlayer()) {
        setNerve(GET_NERVE(PlantSlider, PlantSliderNrvWait));
    }
}

void PlantSlider::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("PlantSliderStart", nullptr);
        MR::setRailDirectionToEnd(this);
        MR::startLevelSoundPlayer("Catch", -1, -1);
    }

    updateSlide();

    if (!MR::isGreaterEqualStep(this, 15)) {
        return;
    }
    if (tryReachGoal()) {
        return;
    }
    if (!MR::isActionEnd(mRider)) {
        return;
    }
    setNerve(GET_NERVE(PlantSlider, PlantSliderNrvSlide));
}

void PlantSlider::exeSlide() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("PlantSlider", nullptr);
        MR::setRailDirectionToEnd(this);
        MR::startActionSound(this, "SlideDown", -1, -1, -1);
    }

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

void PlantSlider::control() {
    for (s32 i = 0; i < mNumLeaves; i++) {
        PlantSliderPoint* pPoint = mSliderPoints[7 + i * 30];

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
    }
}

void PlantSlider::updateHitSensor(HitSensor* pSensor) {
    MR::setSensorPos(pSensor, mSliderPoints[2]->mPosition);
    f32 minDist = PSVECDistance(MR::getPlayerCenterPos(), &mSliderPoints[2]->mPosition);

    for (s32 i = 3; i < mNumPoints - 2; i++) {
        f32 dist = PSVECDistance(MR::getPlayerCenterPos(), &mSliderPoints[i]->mPosition);
        if (dist < minDist) {
            minDist = dist;
            MR::setSensorPos(pSensor, mSliderPoints[i]->mPosition);
        }
    }
}

void PlantSlider::attackSensor(HitSensor*, HitSensor*) {
}

// NONMATCHING: message handling (rush begin / update base mtx / rush cancel) is recovered but the rail-position branch is not yet matched
bool PlantSlider::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (!isNerve(GET_NERVE(PlantSlider, PlantSliderNrvCooldown)) && MR::isOnGroundPlayer()) {
            return false;
        }

        mRider = MR::getSensorHost(pReceiver);
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

        mCameraTargetMtx->setMtx(MR::getPlayerBaseMtx());
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

// NONMATCHING: goal-reached handling (camera end, rider release, nerve change) recovered from asm; scheduling differs
bool PlantSlider::tryReachGoal() {
    if (!MR::isRailReachedGoal(this)) {
        return false;
    }

    MR::moveCoordToEndPos(this);

    TVec3f endPos;
    MR::calcRailEndPointPos(&endPos, this);
    endPos.y -= mRideSpeed;
    MR::setPlayerPos(endPos);

    TVec3f killed(*getPointUp(mRailInfo, mNumPoints - 1));
    MR::vecKillElement(killed, mGravity, &killed);
    if (!MR::isNearZero(killed, 0.001f)) {
        MR::normalize(&killed);
        MR::setPlayerFrontTargetVec(killed, 1);
    }

    TVec3f camUp(*getPointUp(mRailInfo, mNumPoints - 1));
    camUp.scale(mStartDelay);
    TVec3f negGravity(mGravity);
    negGravity.scale(-mAppearSpeed);
    camUp.add(negGravity);

    MR::startBckPlayer("PlantSliderJump", nullptr);
    MR::startLevelSoundPlayer("JumpFromPlant", -1, -1);
    MR::endMultiActorCamera(this, mCameraInfo, "滑り", false, -1);
    MR::setGameCameraTargetToPlayer();

    TVec3f jumpPos;
    fn_800145D0(&jumpPos, mPosition);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, camUp);

    mRider = nullptr;
    setNerve(GET_NERVE(PlantSlider, PlantSliderNrvCooldown));
    return true;
}

// NONMATCHING: rail speed update; the up-vector branch is recovered from asm but not yet matched
void PlantSlider::updateSlide() {
    f32 gravityComp = MR::clamp(-MR::getRailDirection(this).y, 0.0f, 1.0f);
    mSlideSpeed = mSlideSpeed * 0.998f + gravityComp * 1.3f;
    mSlideSpeed = MR::clamp(mSlideSpeed, 5.0f, 23.0f);
    MR::moveCoordAndFollowTrans(this, mSlideSpeed);

    mRailDirection.set(MR::getRailDirection(this));

    TVec3f dir(mRailDirection);
    MR::vecKillElement(dir, mSide, &dir);
    if (MR::isNearZero(dir, 0.001f)) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::normalize(&dir);
        mUp.set(up);
    } else {
        MR::normalize(&dir);
        mUp.set(dir);
    }

    f32 coord = MR::getRailCoord(this) / MR::getRailTotalLength(this);
    f32 ease = MR::getEaseOutValue(coord, 0.0f, 1.0f, 1.0f);
    f32 kill = MR::clamp(ease * (mSlideSpeed - 5.0f), 0.0f, 1.0f);
    mRideAccel = mRideAccel * 0.95f - kill * 0.2f + 0.15f;
    mRideSpeed += mRideAccel;

    MR::setRailDirectionToStart(this);
    updateSpringPoints();
}

void PlantSlider::updatePoints() {
    for (s32 i = 0; i < mNumPoints; i++) {
        f32 t = (f32)i / (f32)(mNumPoints - 1);
        f32 value = MR::getLinerValue(t, 0.15f, 0.035f, 1.0f);
        mSliderPoints[i]->updateSpring(value, 0.92f);
    }
}

// NONMATCHING: rail-coordinate driven spring pass (fn_80437CA0); loop bounds follow the asm, the per-point formulas are approximated
void PlantSlider::updateSpringPoints() {
    s32 coord = (s32)(MR::getRailCoord(this) / 30.0f);
    if (coord < 0) {
        coord = 0;
    }
    if (coord > mNumPoints - 1) {
        coord = mNumPoints - 1;
    }

    for (s32 i = 1; i < coord; i++) {
        f32 ease = MR::getEaseOutValue((f32)i / (f32)(coord - 1), 0.0f, 1.0f, 1.0f);
        f32 lin = MR::getLinerValue((f32)(coord - i) / (f32)(mNumPoints - 1), 0.01f, 0.001f, 1.0f);
        f32 clamped = MR::clamp(ease, 0.0f, 1.0f);
        mSliderPoints[i]->updateSprings(mRideSpeed * clamped, ease, lin, 0.9f);
    }

    for (s32 i = coord; i < mNumPoints; i++) {
        f32 lin = MR::getLinerValue((f32)(i - coord) / (f32)(mNumPoints - 1), 0.15f, 0.035f, 1.0f);
        mSliderPoints[i]->updateSprings(mRideSpeed, 0.0f, lin, 0.9f);
    }
}

// NONMATCHING: GX draw setup recovered from asm; the TEV/texgen argument values are copied verbatim, not yet verified
void PlantSlider::initDrawSetting() const {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, static_cast<GXCompType>(5), 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXLoadPosMtxImm(MR::getCameraViewMtx(), 0);
    GXSetCurrentMtx(0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
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
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);
}

// NONMATCHING: three draw passes; pass 1 follows the asm, passes 2 and 3 reuse its vertex layout with their own colors
void PlantSlider::draw() const {
    if (!MR::isValidDraw(this)) {
        return;
    }

    initDrawSetting();

    const f32 kScale = 10.0f;
    const Color8* kColors[3] = {&sColorWhite, &sColorGray, &sColorLight};
    for (s32 pass = 0; pass < 3; pass++) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, mNumPoints * 2);
        for (s32 i = 0; i < mNumPoints; i++) {
            const PlantSliderPoint* pPoint = mSliderPoints[i];
            f32 th = pPoint->mThickness;

            f32 fx = pPoint->mFront.x * th * kScale;
            f32 fy = pPoint->mFront.y * th * kScale;
            f32 fz = pPoint->mFront.z * th * kScale;
            f32 sx = pPoint->mSide.x * th * kScale;
            f32 sy = pPoint->mSide.y * th * kScale;
            f32 sz = pPoint->mSide.z * th * kScale;

            writeVertexPos(pPoint->mPosition.x + fx, pPoint->mPosition.y + fy, pPoint->mPosition.z + fz);
            GXWGFifo.u32 = *reinterpret_cast<const u32*>(kColors[pass]);
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = (f32)i;

            writeVertexPos(pPoint->mPosition.x + sx - fx, pPoint->mPosition.y + sy - fy, pPoint->mPosition.z + sz - fz);
            GXWGFifo.u32 = *reinterpret_cast<const u32*>(kColors[pass]);
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = (f32)i;
        }
        GXEnd();
    }
}

// out-of-line FIFO write of one position (fn_804386A0)
void writeVertexPos(f32 x, f32 y, f32 z) {
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
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
    : PlantPoint(pPos, pUp, thickness), mBasePos(pPos), mVelocity(0.0f, 0.0f, 0.0f), mTargetPos(pPos) {
}

// NONMATCHING: spring step; temporaries mirror the asm (base - pos, copy, scale, add)
void PlantSliderPoint::updateSpring(f32 pScale, f32 pDamping) {
    mVelocity.scale(pDamping);
    TVec3f diff = mBasePos - mPosition;
    TVec3f offset(diff);
    offset.scale(pScale);
    mVelocity.add(offset);
    mPosition.add(mVelocity);
    mTargetPos.set(mPosition);
}

// NONMATCHING: four-argument spring step, body follows the asm order but the copy temporaries are not verified
void PlantSliderPoint::updateSprings(f32 pHeight, f32 pInterp, f32 pDamping, f32 pAccel) {
    TVec3f base(mBasePos);
    base.y -= pHeight;
    mVelocity.scale(pAccel);
    TVec3f diff = mBasePos - mPosition;
    TVec3f offset(diff);
    offset.scale(pDamping);
    mVelocity.add(offset);
    mTargetPos.add(mVelocity);
    TVec3f blend(mTargetPos);
    blend.scale(1.0f - pInterp);
    TVec3f vel(mVelocity);
    vel.scale(pInterp);
    vel.add(blend);
    mPosition.set(vel);
}
