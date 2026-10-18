#include "Game/Map/HeadLight.hpp"
#include "Game/Util.hpp"

HeadLight::HeadLight(const char* pName) : LiveActor(pName), mColor(0xFF, 0xFF, 0xFF, 0xFF) {
    mArg4 = 500;
}

void HeadLight::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);

    s32 arg = 0;
    if (MR::getJMapInfoArg0NoInit(rIter, &arg)) {
        mColor.r = arg;
    }
    if (MR::getJMapInfoArg1NoInit(rIter, &arg)) {
        mColor.g = arg;
    }
    if (MR::getJMapInfoArg2NoInit(rIter, &arg)) {
        mColor.b = arg;
    }
    if (MR::getJMapInfoArg3NoInit(rIter, &arg)) {
        mColor.a = arg;
    }
    MR::getJMapInfoArg4NoInit(rIter, &mArg4);

    MR::invalidateClipping(this);
    MR::initUseStageSwitchReadAppear(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void HeadLight::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();

    TVec3f upVec;
    MR::getPlayerUpVec(&upVec);

    TVec3f frontVec;
    MR::getPlayerFrontVec(&frontVec);

    MR::makeMtxUpFrontPos(&mtx, upVec, frontVec, mPosition);
    MR::setBaseTRMtx(this, mtx);
}

void HeadLight::control() {
    TVec3f upVec;
    MR::getPlayerUpVec(&upVec);

    TVec3f frontVec;
    MR::getPlayerFrontVec(&frontVec);

    TVec3f pos = (*MR::getPlayerPos() + upVec * 120.0f) + frontVec * 50.0f;
    mPosition = pos;

    f32 intensity = MR::getLinerValueFromMinMax((f32)mArg4, 0.0f, 1000.0f, 0.97f, 1.0f);
    MR::requestPointLight(this, mPosition, mColor, intensity, -1, 15.0f);
}

HeadLight::~HeadLight() {
}
