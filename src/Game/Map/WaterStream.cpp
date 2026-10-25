#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Map/WaterStream.hpp"

#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Map/WaterAreaHolder.hpp"
#include "Game/Util.hpp"


WaterStream::WaterStream(const char* pName) : LiveActor(pName) {
    mIsGravityBased = true;
    mIsReversed = false;
    mWidth = 0.0f;
    mUnusedParam0 = 0.0f;
    mUnusedParam1 = 0.0f;
    mFlowSpeed = 80.0f;
    mUnusedParam2 = 100.0f;
    mClipMin.zero();
    mClipMax.zero();
    mBoxMin.zero();
    mBoxMax.zero();
}

WaterStream::~WaterStream() {}

void WaterStream::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x21, -1, -1, -1);
    MR::initDefaultPos(this, rIter);
    initRailRider(rIter);

    bool isNoGravity = false;
    MR::getJMapInfoArg3NoInit(rIter, &isNoGravity);
    if (isNoGravity) {
        mIsGravityBased = false;
    }
    MR::getJMapInfoArg4NoInit(rIter, &mIsReversed);

    mWidth = 1200.0f;
    mFlowSpeed = 50.0f;
    mUnusedParam2 = 80.0f;
    calcClippingBox();
    WaterAreaFunction::entryWaterStream(this);

    initSound(1, "WaterStream", NULL, TVec3f(0.0f));

    AudSoundObject* pSound1 = new AudSoundObject(&mSoundPos1, 4, MR::getCurrentHeap());
    mSound1 = pSound1;
    AudSoundObject* pSound2 = new AudSoundObject(&mSoundPos2, 4, MR::getCurrentHeap());
    mSound2 = pSound2;

    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}

void WaterStream::initAfterPlacement() {}

bool WaterStream::isInWater(const TVec3f& rPos) const {
    bool isInside = false;
    if (rPos.x >= mBoxMin.x && rPos.y >= mBoxMin.y && rPos.z >= mBoxMin.z && rPos.x < mBoxMax.x && rPos.y < mBoxMax.y && rPos.z < mBoxMax.z) {
        isInside = true;
    }
    if (!isInside) {
        return false;
    }

    TVec3f nearest;
    f32 halfWidth = mWidth * calcCurrentWidthRate(calcNearestPos(rPos, &nearest, NULL, NULL));
    if (PSVECDistance(&nearest, &rPos) > halfWidth) {
        return false;
    }

    if (mIsGravityBased) {
        TVec3f toPos = rPos - nearest;
        TVec3f gravity;
        MR::calcGravityVector(this, rPos, &gravity, NULL, 0);
        if (toPos.dot(gravity) < 0.0f) {
            return false;
        }
    }
    return true;
}

// NONMATCHING: regalloc; temporary stack allocation and FP scheduling
inline f32 calcWaterStreamBottomAngle(f32 distance, f32 width) {
    f32 angle = distance / width;
    angle *= 3.1415927f;
    return angle * 0.5f;
}

inline TVec3f calcWaterStreamSurface(const TVec3f& position, const TVec3f& gravity, f32 depth) {
    TVec3f scaled(gravity);
    scaled.scale(depth);
    TVec3f surface(position);
    surface.sub(scaled);
    return surface;
}

bool WaterStream::calcWaterInfo(const TVec3f& rPos, const TVec3f& rGravity, WaterInfo* pInfo) const {
    TVec3f nearest;
    TVec3f nearestDir;
    f32 width;
    {
        f32 nearestRate = calcNearestPos(rPos, &nearest, &nearestDir, NULL);
        width = mWidth * calcCurrentWidthRate(nearestRate);

        TVec3f flow;
        calcStreamVec(rPos, nearest, nearestRate, TVec3f(nearestDir), width, &flow);
        pInfo->mStreamVec.set(flow);
        pInfo->mWaveHeight = calcWaveHeight(rPos, nearestRate, &pInfo->mSurfaceNormal);
    }

    TVec3f toPos(rPos);
    toPos.sub(nearest);
    f32 depth = MR::vecKillElement(toPos, -rGravity, &toPos);
    if (mIsReversed) {
        width *= 2.0f;
    }
    f32 angle = calcWaterStreamBottomAngle(PSVECMag(&toPos), width);
    f32 bottomHeight = width * MR::cos(angle);
    f32 edgeBase = mIsReversed ? width : 0.0f;
    pInfo->mBottomDepth = bottomHeight + depth;
    pInfo->mCamWaterDepth = -depth + edgeBase;

    TVec3f gravity = rGravity;
    MR::calcGravityVector(this, rPos, &gravity, NULL, 0);
    pInfo->mSurfaceNormal.set(-gravity);

    f32 camDepth = pInfo->mCamWaterDepth;
    pInfo->mSurfacePos.set(calcWaterStreamSurface(rPos, rGravity, camDepth));

    TVec3f dist(rPos);
    dist.sub(nearest);
    if (MR::isNearZero(dist, 0.001f)) {
        pInfo->mEdgeDistance = width;
        TVec3f edge = rGravity;
        edge.scale(width);
        edge.add(nearest);
        pInfo->mEdgePos.set(edge);
    } else {
        MR::normalize(&dist);
        dist.scale(width);
        dist.add(nearest);
        pInfo->mEdgeDistance = PSVECDistance(&dist, &rPos);
        pInfo->mEdgePos.set(dist);
    }
    return true;
}

// NONMATCHING: regalloc
namespace {
    inline void calcWaterStreamSide(TVec3f* cross, const LiveActor* actor, const TVec3f& up) {
        TVec3f invUp = -up;
        PSVECCrossProduct(&MR::getRailDirection(actor), &invUp, cross);
    }
}

f32 WaterStream::calcNearestPos(const TVec3f& rPos, TVec3f* pPos, TVec3f* pDir, TVec3f* pSide) const {
    f32 coord = MR::calcNearestRailCoord(this, rPos);
    MR::calcRailPosAtCoord(pPos, this, coord);
    if (pSide != NULL) {
        TVec3f railDir;
        TVec3f railPos;
        MR::calcRailPosAndDirectionAtCoord(&railPos, &railDir, this, coord);
        TVec3f up(0.0f, -1.0f, 0.0f);
        MR::calcGravityVector(this, railPos, &up, NULL, 0);
        TVec3f cross;
        calcWaterStreamSide(&cross, this, up);
        f32 halfWidth = mWidth * calcCurrentWidthRate(coord);
        TVec3f offsetA = cross;
        offsetA.scale(-halfWidth);
        offsetA.add(*pPos);
        TVec3f offsetB = cross;
        offsetB.scale(halfWidth);
        offsetB.add(*pPos);
        MR::calcPerpendicFootToLineInside(pSide, rPos, offsetA, offsetB);
    }
    if (pDir != NULL) {
        MR::calcRailDirectionAtCoord(pDir, this, coord);
    }
    return coord;
}

f32 WaterStream::calcWaveHeight(const TVec3f& rPos, f32 rate, TVec3f* pNormal) const {
    TVec3f up(0.0f, -1.0f, 0.0f);
    MR::calcGravityVector(this, rPos, &up, NULL, 0);
    if (MR::isNearZero(up, 0.001f)) {
        pNormal->set<f32>(0.0f, -1.0f, 0.0f);
    } else {
        TVec3f inv = -up;
        pNormal->set(inv);
    }
    return 0.0f;
}

void WaterStream::calcStreamVec(const TVec3f& rPos, const TVec3f& rNearest, f32 rate, const TVec3f& rDir, f32 width, TVec3f* pVec) const {
    pVec->zero();
    f32 speed = 10.0f * calcCurrentFlowSpeedRate(rate);
    TVec3f diff = rPos - rNearest;
    if (PSVECMag(&diff) < width) {
        pVec->set(rDir);
        pVec->scale(speed);
    }
}

void WaterStream::movement() {
    TVec3f camNearest;
    calcNearestPos(MR::getCameraWatchPos(), &camNearest, NULL, &mSoundPos1);
    MR::startSoundObjectLevel(mSound1, "SE_AT_LV_OCEAN_RING", -1);
    mSound1->process();

    TVec3f camPos = MR::getCamPos();
    calcNearestPos(camPos, &camNearest, NULL, &mSoundPos2);
    MR::startSoundObjectLevel(mSound2, "SE_AT_LV_OCEAN_RING_SUB", -1);
    mSound2->process();
}

f32 WaterStream::calcCurrentWidthRate(f32 rate) const {
    f32 a = 0.0f;
    f32 b = 0.0f;
    MR::setRailCoord(const_cast<WaterStream*>(this), rate);
    MR::calcDistanceToCurrentAndNextRailPoint(this, &a, &b);
    f32 sum = a + b;
    if (sum < 1.0f) {
        return 1.0f;
    }
    f32 wa = 12.0f;
    f32 wb = 12.0f;
    MR::getCurrentRailPointArg1NoInit(this, &wa);
    MR::getNextRailPointArg1NoInit(this, &wb);
    return (((wa * b) + (wb * a)) / sum) / 12.0f;
}

f32 WaterStream::calcCurrentFlowSpeedRate(f32 rate) const {
    f32 a = 0.0f;
    f32 b = 0.0f;
    MR::setRailCoord(const_cast<WaterStream*>(this), rate);
    MR::calcDistanceToCurrentAndNextRailPoint(this, &a, &b);
    f32 sum = a + b;
    if (sum < 1.0f) {
        return 1.0f;
    }
    f32 wa = 100.0f;
    f32 wb = 100.0f;
    MR::getCurrentRailPointArg0NoInit(this, &wa);
    MR::getNextRailPointArg0NoInit(this, &wb);
    return (((wa * b) + (wb * a)) / sum) / 100.0f;
}

void WaterStream::calcClippingBox() {
    f32 result = 1200.0f;
    for (s32 i = 0; i < MR::getRailPointNum(this); i++) {
        f32 val = 0.0f;
        MR::getRailPointArg1NoInit(this, i, &val);
        result = MR::max(100.0f * val, result);
    }
    MR::calcBoundingBox(this, reinterpret_cast<TBox3f*>(&mBoxMin), 200.0f);
    TVec3f ext(result);
    mBoxMin.sub(ext);
    mBoxMax.add(ext);
    mClipMin.set(mBoxMin);
    mClipMax.set(mBoxMax);
    TVec3f pad(5000.0f);
    mClipMin.sub(pad);
    mClipMax.add(pad);
    TVec3f width(mWidth);
    mBoxMin.sub(width);
    mBoxMax.add(width);
    JMAVECLerp(&mBoxMax, &mBoxMin, &mClippingCenter, 0.5f);
    MR::setClippingTypeSphere(this, 100.0f + PSVECDistance(&mClippingCenter, &mBoxMin), &mClippingCenter);
    MR::setClippingFarMax(this);
}
