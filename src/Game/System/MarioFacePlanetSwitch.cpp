#include "Game/System/MarioFacePlanetSwitch.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/Util/EventUtil.hpp"

MarioFacePlanetSwitch::MarioFacePlanetSwitch(const char* pName) : NameObj(pName) {
    mElementNum = 0;
    mElements = NULL;
    mIsDone = 0;
}

void MarioFacePlanetSwitch::init(const JMapInfoIter&) {
    initElements();
    checkElements();
}

void MarioFacePlanetSwitch::initElements() {
    s32 infoNum = MarioFaceShipEventDataHolder::getEventInfoNum();
    mElementNum = MarioFaceShipEventDataHolder::getValidEventInfoNum();
    mElements = new MarioFacePlanetSwitchElement*[mElementNum];

    s32 n = 0;
    for (s32 i = 0; i < infoNum; i++) {
        MarioFaceShipEventInfo* info = MarioFaceShipEventDataHolder::getEventInfo(i);
        if (info->mSwitchId != -1) {
            MarioFacePlanetSwitchElement* elem = new MarioFacePlanetSwitchElement;
            if (elem != NULL) {
                elem->mKind = 0;
                elem->mName1 = "IslandFleetGalaxy";
                elem->mName2 = NULL;
                elem->mObjId = -1;
                elem->mStarId1 = -1;
                elem->mStarId2 = -1;
                elem->mThreshold = -1;
                elem->mSwitchId = -1;
                elem->mIsDone = 0;
            }
            mElements[n] = elem;
            mElements[n]->mKind = info->mKind;
            mElements[n]->mName1 = info->mName1;
            mElements[n]->mName2 = info->mName2;
            mElements[n]->mObjId = info->mObjId;
            mElements[n]->mStarId1 = info->mStarId1;
            mElements[n]->mStarId2 = info->mStarId2;
            mElements[n]->mThreshold = info->mThreshold;
            mElements[n]->mSwitchId = info->mSwitchId;
            mElements[n]->mIsDone = 0;
            n++;
        }
    }
}

void MarioFacePlanetSwitch::checkElements() {
    for (s32 i = 0; i < mElementNum; i++) {
        if (mElements[i]->mObjId == 0xC9) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0xCB) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0xCC) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0x12F) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0x191) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0x193) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
        if (mElements[i]->mObjId == 0x1F6) {
            MarioFacePlanetSwitchElement* elem = mElements[i];
            if (!isElementSatisfied(elem)) return;
        }
    }
    mIsDone = 1;
}

void MarioFacePlanetSwitch::applySatisfiedElements() {
    for (s32 i = 0; i < mElementNum; i++) {
        MarioFacePlanetSwitchElement* elem = mElements[i];
        if (isElementSatisfied(elem)) {
            StageSwitchFunction::onGlobalSwitchById(elem->mSwitchId - GLOBAL_SWITCH_ID);
            elem->mIsDone = 1;
        }
    }
}

s32 MarioFacePlanetSwitch::checkMarkerElement() {
    for (s32 i = 0; i < mElementNum; i++) {
        MarioFacePlanetSwitchElement* elem = mElements[i];
        if (elem->mObjId == 0x68) {
            return isElementSatisfied(elem);
        }
    }
    return 0;
}

s32 MarioFacePlanetSwitch::isElementSatisfied(MarioFacePlanetSwitchElement* elem) {
    if (elem->mSwitchId < 0) {
        return 0;
    }

    GalaxyStatusAccessor acc = MR::makeGalaxyStatusAccessor(elem->mName1);
    s32 starNum = MR::getPowerStarNum();

    if (MarioFaceShipEventDataHolder::isKindStarOwned(elem->mKind)) {
        return acc.hasPowerStar(elem->mStarId1);
    }
    if (MarioFaceShipEventDataHolder::isKindThresholdReached(elem->mKind)) {
        return elem->mThreshold <= starNum;
    }
    if (MarioFaceShipEventDataHolder::isKindDualStarOwned(elem->mKind)) {
        GalaxyStatusAccessor acc2 = MR::makeGalaxyStatusAccessor(elem->mName2);
        s32 ret = 0;
        if (acc.hasPowerStar(elem->mStarId1) || acc2.hasPowerStar(elem->mStarId2)) {
            ret = 1;
        }
        return ret;
    }
    if (MarioFaceShipEventDataHolder::isKindStarAmountReached(elem->mKind)) {
        return starNum >= 120;
    }
    if (MarioFaceShipEventDataHolder::isKindObjectSet(elem->mKind)) {
        return MarioFaceShipEventDataHolder::getObjectValue(elem->mObjId);
    }
    return 0;
}

void MarioFacePlanetSwitch::notifyApplySatisfied() {
    MR::getSceneObj<MarioFacePlanetSwitch>(0x93)->applySatisfiedElements();
}

s32 MarioFacePlanetSwitch::checkMarkerInScene() {
    if (!MR::isExistSceneObj(0x93)) {
        return 0;
    }
    return MR::getSceneObj<MarioFacePlanetSwitch>(0x93)->checkMarkerElement();
}

bool MarioFacePlanetSwitch::isDoneInScene() {
    if (!MR::isExistSceneObj(0x93)) {
        return 0;
    }
    return MR::getSceneObj<MarioFacePlanetSwitch>(0x93)->mIsDone;
}

MarioFacePlanetSwitch::~MarioFacePlanetSwitch() {}
