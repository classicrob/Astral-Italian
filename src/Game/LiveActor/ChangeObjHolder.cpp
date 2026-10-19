#include "Game/LiveActor/ChangeObjHolder.hpp"
#include "Game/LiveActor/ChangeObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

bool ChangeObjPair::tryRegister(ChangeObj* pObj, s32 zoneId, s32 linkId) {
    if (mZoneId != zoneId) return false;
    if (linkId == mFirstLinkId) {
        mFirst = pObj;
        return true;
    }
    if (linkId == mSecondLinkId) {
        mSecond = pObj;
        return true;
    }
    return false;
}

ChangeObjHolder::ChangeObjHolder(const char* pName) : NameObj(pName) {
    mChangeObjs = nullptr;
    mCapacity = 0;
    mCount = 0;
    mChangeObjs = new ChangeObjPair*[0x40];
    mCapacity = 0x40;
}

ChangeObjHolder::~ChangeObjHolder() {
    delete[] mChangeObjs;
}

void ChangeObjHolder::init(const JMapInfoIter&) {
}

void ChangeObjHolder::registerChangeObj(ChangeObj* pObj, const JMapInfoIter& rIter) {
    s32 zoneId = MR::getPlacedZoneId(rIter);
    s32 linkId;
    MR::getJMapInfoLinkID(rIter, &linkId);

    for (s32 i = 0; i < mCount; i++) {
        if (mChangeObjs[i]->tryRegister(pObj, zoneId, linkId)) {
            break;
        }
    }
}

namespace MR {
    void registerChangeObj(ChangeObj* pObj, const JMapInfoIter& rIter) {
        if (MR::isExistSceneObj(0x85)) {
            MR::getSceneObj< ChangeObjHolder >(0x85)->registerChangeObj(pObj, rIter);
        }
    }
};
