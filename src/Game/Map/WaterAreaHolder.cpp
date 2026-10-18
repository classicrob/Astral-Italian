#include "Game/Map/WaterAreaHolder.hpp"
#include "Game/Map/OceanSphere.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Map/WaterStream.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/WaterCameraFilter.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace {
    static const s32 sAreaNumMax = 8;

    WaterAreaHolder* getWaterAreaHolder() {
        return MR::getSceneObj< WaterAreaHolder >(SceneObj_WaterAreaHolder);
    }
};  // namespace

WaterAreaHolder::WaterAreaHolder() : NameObj("水エリア保持"), mNumOceanSpheres(0), mOceanSpheres(nullptr), mNumWaterStreams(0), mWaterStreams(nullptr), mCamInWater(false), mCameraFilter(nullptr), mUseBloom(false) {
    mWaterInfo.clear();

    if (MR::isEqualStageName("HeavenlyBeachGalaxy") || MR::isEqualStageName("OceanRingGalaxy")) {
        MR::createNormalBloom();
        mUseBloom = true;
    }

    mOceanSpheres = new OceanSphere*[::sAreaNumMax];
    mWaterStreams = new WaterStream*[::sAreaNumMax];

    for (s32 idx = 0; idx < ::sAreaNumMax; idx++) {
        mOceanSpheres[idx] = nullptr;
    }

    MR::connectToSceneScreenEffectMovement(this);
    mCameraFilter = new WaterCameraFilter();
    mCameraFilter->initWithoutIter();
}

void WaterAreaHolder::entryOceanSphere(OceanSphere* pOceanSphere) {
    mOceanSpheres[mNumOceanSpheres] = pOceanSphere;
    mNumOceanSpheres++;
}

void WaterAreaHolder::entryWaterStream(WaterStream* pWaterStream) {
    mWaterStreams[mNumWaterStreams] = pWaterStream;
    mNumWaterStreams++;
}

void WaterAreaHolder::movement() {
    if (MR::getWaterAreaObj(&mWaterInfo, MR::getCamPos())) {
        if (!mCamInWater) {
            if (mUseBloom) {
                MR::turnOnNormalBloom();
                MR::setNormalBloomIntensity(255);
                MR::setNormalBloomThreshold(128);
                MR::setNormalBloomBlurIntensity1(40);
                MR::setNormalBloomBlurIntensity2(20);
            }
            mCamInWater = true;
        }
        MR::getWaterAreaInfo(&mWaterInfo, MR::getCamPos(), TVec3f(0.0f, -1.0f, 0.0f), false);
    } else {
        if (mCamInWater) {
            if (mUseBloom) {
                MR::setImageEffectControlAuto();
            }
            mCamInWater = false;
        }
    }
}

void WaterAreaFunction::entryOceanSphere(OceanSphere* pOceanSphere) {
    createWaterAreaHolder();
    ::getWaterAreaHolder()->entryOceanSphere(pOceanSphere);
}

void WaterAreaFunction::entryWaterStream(WaterStream* pWaterStream) {
    createWaterAreaHolder();
    ::getWaterAreaHolder()->entryWaterStream(pWaterStream);
}

void WaterAreaFunction::createWaterAreaHolder() {
    MR::createSceneObj(SceneObj_WaterAreaHolder);
}

bool WaterAreaFunction::tryInOceanArea(const TVec3f& rPos, WaterInfo* pWaterInfo) {
    if (!MR::isExistSceneObj(SceneObj_WaterAreaHolder)) {
        return false;
    }

    OceanSphere* sphere;
    WaterStream* stream;

    for (s32 idx = 0; idx < ::getWaterAreaHolder()->mNumOceanSpheres; idx++) {
        sphere = ::getWaterAreaHolder()->mOceanSpheres[idx];
        if (sphere->isInWater(rPos)) {
            pWaterInfo->mOceanSphere = sphere;
            return true;
        }
    }

    for (s32 idx = 0; idx < ::getWaterAreaHolder()->mNumWaterStreams; idx++) {
        stream = ::getWaterAreaHolder()->mWaterStreams[idx];
        if (stream->isInWater(rPos)) {
            pWaterInfo->mWaterStream = stream;
            return true;
        }
    }

    return false;
}

bool WaterAreaFunction::isCameraInWaterForCameraUtil() {
    if (MR::isExistSceneObj(SceneObj_WaterAreaHolder)) {
        if (::getWaterAreaHolder()->mCamInWater) {
            return true;
        }
    }

    return false;
}

f32 WaterAreaFunction::getCameraWaterDepth() {
    if (MR::isExistSceneObj(SceneObj_WaterAreaHolder)) {
        return ::getWaterAreaHolder()->mWaterInfo.mCamWaterDepth;
    }

    return 0.0f;
}

WaterInfo* WaterAreaFunction::getCameraWaterInfo() {
    if (MR::isExistSceneObj(SceneObj_WaterAreaHolder)) {
        return &::getWaterAreaHolder()->mWaterInfo;
    }

    return nullptr;
}

void WaterAreaFunction::pauseOffWaterArea() {
    if (MR::isExistSceneObj(SceneObj_WaterAreaHolder)) {
        MR::requestMovementOn(::getWaterAreaHolder());
        MR::requestMovementOn(::getWaterAreaHolder()->mCameraFilter);
    }
}
