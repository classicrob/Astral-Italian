#include "Game/AreaObj/CubeCamera.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <revolution/types.h>

#include <cstring>

CubeCameraArea::CubeCameraArea(const char* pName) : AreaObj(pName) {
    mCategory = 0;
    mZoneID = 0;
}

// needed to get sCubeCategory emitted in sbss;
s32 CubeCameraArea::sCubeCategory = 0;

void CubeCameraArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    const char* valid;

    if (rIter.getValue("Validity", &valid)) {
        if (strcmp(valid, "Invalid") == 0) {
            mIsValid = false;
        }
    }

    if (isValidSwitchA()) {
        mIsValid = false;
    }

    if (rIter.isValid()) {
        mZoneID = MR::getPlacedZoneId(rIter);
    } else {
        mZoneID = 0;
    }

    mCategory = getCategoryArg();
    if (getCategoryArg() < 0) {
        mCategory = 1 << ECategory_Normal;
    }

    if (mCategory == 0) {
        mCategory = 1 << ECategory_Normal;
    }

    MR::connectToSceneAreaObj(this);
}

void CubeCameraArea::movement() {
    if (!isValid()) {
        if (isValidSwitchA() && isOnSwitchA()) {
            mIsValid = true;
        }
    } else if (isValidSwitchB() && isOnSwitchB()) {
        mIsValid = false;
    }
}

s32 CubeCameraArea::getAreaPriority() const {
    return mObjArg2;
}

inline const char* CubeCameraArea::getManagerName() const {
    return "CubeCamera";
}

bool CubeCameraArea::isValidCameraID() const {
    return mObjArg0 >= 0;
}

u16 CubeCameraArea::getCameraID() const {
    return mObjArg0;
}

bool CubeCameraArea::isInVolume(const TVec3f& rVec) const {
    return (mCategory & sCubeCategory) != 0 && AreaObj::isInVolume(rVec);
}

void CubeCameraArea::setCurrentCategory(s32 category) {
    sCubeCategory = 1 << category;
}

s32 CubeCameraArea::getCategoryArg() const {
    return mObjArg3;
}

CubeCameraMgr::CubeCameraMgr(s32 maxNum, const char* pName) : AreaObjMgr(maxNum, pName) {
    CubeCameraArea::sCubeCategory = 1 << CubeCameraArea::ECategory_Normal;
}

void CubeCameraMgr::initAfterLoad() {
}

void CubeCameraMgr::sort() {
    if (mArray.size() == 0) {
        return;
    }

    for (u32 i = 0; i < mArray.size() - 1; i++) {
        int swapIndex = i;
        AreaObj* swapObj = getAreaObj(i);
        AreaObj* curObj = swapObj;
        for (u32 j = i + 1; j < mArray.size(); j++) {
            AreaObj* nextObj = getAreaObj(j);
            if (swapObj->mObjArg2 > nextObj->mObjArg2) {
                swapIndex = j;
                swapObj = nextObj;
            }
        }

        if (swapIndex != i) {
            mArray[i] = swapObj;
            mArray[swapIndex] = curObj;
        }
    }
}
