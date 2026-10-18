#include "Game/Map/LightDirector.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorLightCtrl.hpp"
#include "Game/Map/LightDataHolder.hpp"
#include "Game/Map/LightFunction.hpp"
#include "Game/Map/LightPointCtrl.hpp"
#include "Game/Map/LightZoneDataHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/ObjUtil.hpp"

LightDirector::LightDirector()
    : NameObj("ライト指揮"), _C(), mDataHolder(), mZoneDataHolder(), mDefaultAreaLight(), _1C(), mPointCtrl(), mPointCtrlSub(), mResourceHolder(),
      mZoneResourceHolders() {
}

void LightDirector::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    LightFunction::loadAllLightWhite();

    mDataHolder = new LightDataHolder();
    mZoneDataHolder = new LightZoneDataHolder();
    mPointCtrl = new LightPointCtrl();
    mPointCtrlSub = new LightPointCtrl();
}

void LightDirector::initData() {
    mResourceHolder = LightFunction::loadLightArchive();
    mZoneResourceHolders = new ResourceHolder*[MR::getZoneNum()];
    for (s32 i = 0; i < MR::getZoneNum(); i++) {
        mZoneResourceHolders[i] = LightFunction::loadZoneLightArchive(MR::getZoneNameFromZoneId(i));
    }
    mDataHolder->initLightData();
    mZoneDataHolder->initZoneData();
    mDefaultAreaLight = mDataHolder->findAreaLight(mZoneDataHolder->getDefaultStageAreaLightName());
}

void LightDirector::loadLightPlayer() const {
    _1C->loadLight();
    mPointCtrl->loadPointLight();
}

void LightDirector::loadPointLightSub() const {
    mPointCtrlSub->loadPointLight();
}

ResourceHolder* LightDirector::getZoneLightResourceHolder(const char* pZoneName) const {
    return mZoneResourceHolders[MR::getZoneIdFromZoneName(pZoneName)];
}

void LightDirector::movement() {
    mPointCtrl->update();
    mPointCtrlSub->update();
}
