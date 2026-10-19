#include "Game/AreaObj/ImageEffectArea.hpp"
#include "Game/Util.hpp"

ImageEffectArea::ImageEffectArea(int effectType, const char* pName) : AreaObj(pName), mEffectType(effectType) {
}

ImageEffectAreaMgr::ImageEffectAreaMgr(s32 maxNum, const char* pName) : AreaObjMgr(maxNum, pName) {
}

void ImageEffectAreaMgr::initAfterPlacement() {
    sort();
}

void ImageEffectAreaMgr::sort() {
    if (mArray.size() == 0) {
        return;
    }

    for (u32 i = 0; i < mArray.size() - 1; i++) {
        AreaObj* pOriginal = getAreaObj(i);
        u32 selected = i;
        AreaObj* pSelected = pOriginal;
        for (u32 j = i + 1; j < mArray.size(); j++) {
            AreaObj* pCandidate = getAreaObj(j);
            if (pSelected->mObjArg7 > pCandidate->mObjArg7) {
                selected = j;
                pSelected = pCandidate;
            }
        }

        if (selected != i) {
            mArray[i] = pSelected;
            mArray[selected] = pOriginal;
        }
    }
}

void ImageEffectArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
}
