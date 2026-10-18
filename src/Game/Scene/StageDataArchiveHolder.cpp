#include "Game/Scene/StageDataArchiveHolder.hpp"

// PLACEHOLDER(core): fn_8045AE50 is an unnamed StageDataArchive member in the unowned auto_03 gap (archive, list, name); resolve when named.
extern "C" {
    void fn_8045AE50(StageDataArchive*, void*, const char*);
}

namespace {
    const char* const sSoundNameList[2] = {"ObjInfo", "AreaObjInfo"};
    const char* const sAreaNameList[10] = {
        "SoundEmitterPoint", "SoundPlayArea", "AudioEffectArea", "SoundEmitterSphere", "SoundEmitterCube",
        "BgmChangeArea",     "BgmProhibitArea", "BgmVolumeFadeArea", "GravityBgmCtrl", "WorldMapSyncSoundEmitterCube",
    };
};  // namespace

StageDataArchiveHolder::StageDataArchiveHolder(const char* pStageName) : mMapArchive(), mSoundArchive(), mDesignArchive(), mZoneInfoArchive(), mUseResourceArchive(), mGhostArchive() {
    mMapArchive = mountArchive(pStageName, "Map");
    mSoundArchive = mountArchive(pStageName, "Sound");
    mDesignArchive = mountArchive(pStageName, "Design");
    mZoneInfoArchive = mountArchive(pStageName, "ZoneInfo");
    mUseResourceArchive = mountArchive(pStageName, "UseResource");
    mGhostArchive = mountArchive(pStageName, "Ghost");

    if (mSoundArchive != nullptr) {
        for (s32 i = 0; i < 2; i++) {
            const char* pName = sSoundNameList[i];
            mSoundArchive->mNameList0.push_back(pName);
        }
        for (s32 i = 0; i < 10; i++) {
            const char* pName = sAreaNameList[i];
            mSoundArchive->mNameList1.push_back(pName);
        }
    }

    if (mDesignArchive != nullptr) {
        const char* pAreaName = "AreaObjInfo";
        mDesignArchive->mNameList0.push_back(pAreaName);
        const char* pLightName = "LightCtrlArea";
        mDesignArchive->mNameList1.push_back(pLightName);
    }

    for (s32 i = 0; i < 10; i++) {
        const char* pName = sAreaNameList[i];
        mMapArchive->mNameList2.push_back(pName);
        if (mDesignArchive != nullptr) {
            mDesignArchive->mNameList2.push_back(pName);
        }
    }

    const char* pLightName = "LightCtrlArea";
    mMapArchive->mNameList2.push_back(pLightName);
    if (mSoundArchive != nullptr) {
        mSoundArchive->mNameList2.push_back(pLightName);
    }
}

void StageDataArchiveHolder::addToArchives(void* pList, const char* pName) {
    fn_8045AE50(mMapArchive, pList, pName);
    if (mSoundArchive != nullptr) {
        fn_8045AE50(mSoundArchive, pList, pName);
    }
    if (mDesignArchive != nullptr) {
        fn_8045AE50(mDesignArchive, pList, pName);
    }
}
