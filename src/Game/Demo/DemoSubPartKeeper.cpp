#include "Game/Demo/DemoSubPartKeeper.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Util.hpp"
#include <cstring>

DemoSubPartKeeper::DemoSubPartKeeper(const DemoExecutor* pExecutor) : mExecutor(pExecutor), mNumSubPartInfos(), mSubPartInfos() {
    JMapInfo* map = nullptr;
    mNumSubPartInfos = DemoFunction::createSheetParser(mExecutor, "SubPart", &map);
    mSubPartInfos = new DemoSubPartInfo[mNumSubPartInfos];
    for (int i = 0; i < mNumSubPartInfos; i++) {
        DemoSubPartInfo* subpart = &mSubPartInfos[i];
        MR::getCsvDataStrOrNULL(&subpart->mSubPartName, map, "SubPartName", i);
        MR::getCsvDataS32(&subpart->mSubPartTotalStep, map, "SubPartTotalStep", i);
        MR::getCsvDataStrOrNULL(&subpart->mMainPartName, map, "MainPartName", i);
        MR::getCsvDataS32(&subpart->mMainPartStep, map, "MainPartStep", i);
    }
}

DemoSubPartInfo::DemoSubPartInfo() : mSubPartName(nullptr), mSubPartTotalStep(1), mMainPartName(nullptr), mMainPartStep(1), mRemainStep(-1) {
}

void DemoSubPartKeeper::update() {
    if (mNumSubPartInfos > 0) {
        for (int i = 0; i < mNumSubPartInfos; i++) {
            DemoSubPartInfo* subpart = &mSubPartInfos[i];
            if (subpart->mRemainStep > 0) {
                subpart->mRemainStep -= 1;
            }
            if (MR::isDemoPartStep(subpart->mMainPartName, subpart->mMainPartStep)) {
                subpart->mRemainStep = subpart->mSubPartTotalStep;
            }
        }
    }
}

void DemoSubPartKeeper::end() {
    for (int i = 0; i < mNumSubPartInfos; i++) {
        mSubPartInfos[i].mRemainStep = -1;
    }
}

bool DemoSubPartKeeper::isDemoPartActive(const char* pSubPartName) const {
    DemoSubPartInfo* subpart = findSubPart(pSubPartName);
    if (subpart == nullptr) {
        return false;
    } else {
        return subpart->mRemainStep > 0;
    }
}

s32 DemoSubPartKeeper::getDemoPartStep(const char* pSubPartName) const {
    DemoSubPartInfo* subpart = findSubPart(pSubPartName);
    return subpart->mSubPartTotalStep - subpart->mRemainStep;
}

s32 DemoSubPartKeeper::getDemoPartTotalStep(const char* pSubPartName) const {
    DemoSubPartInfo* subpart = findSubPart(pSubPartName);
    return subpart->mSubPartTotalStep;
}

DemoSubPartInfo* DemoSubPartKeeper::findSubPart(const char* pSubPartName) const {
    DemoSubPartInfo* subpart;
    s32 count = mNumSubPartInfos;
    for (int i = 0; i < count; i++) {
        subpart = &mSubPartInfos[i];
        if (!strcmp(subpart->mSubPartName, pSubPartName)) {
            return subpart;
        }
    }
    return nullptr;
}
