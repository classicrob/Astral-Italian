#include "Game/Scene/StageDataArchiveHolder.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Scene/StageFileLoader.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSystemDataHolder.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <algorithm>
#include <cstdio>

#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Array.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/PlacementInfoOrdered.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Scene/StageDataArchiveHolder.hpp"
#include "Game/Scene/StageResourceLoader.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <cstdio>

namespace {
    const char* sSoundNameList[2] = {"ObjInfo", "AreaObjInfo"};
    const char* sAreaNameList[10] = {
        "SoundEmitterPoint", "SoundPlayArea", "AudioEffectArea", "SoundEmitterSphere", "SoundEmitterCube",
        "BgmChangeArea",     "BgmProhibitArea", "BgmVolumeFadeArea", "GravityBgmCtrl", "WorldMapSyncSoundEmitterCube",
    };
    const char* sAreaObjInfo = "AreaObjInfo";
    const char* sLightCtrlArea = "LightCtrlArea";

    static const char* cLayerDirName[0x11] = {"Common", "LayerA", "LayerB", "LayerC", "LayerD", "LayerE", "LayerF", "LayerG", "LayerH",
                                              "LayerI", "LayerJ", "LayerK", "LayerL", "LayerM", "LayerN", "LayerO", "LayerP"};
};  // namespace

void StageFileLoader::makeStageArchivePath(char* pDst, u32 size, const char* pZoneName, const char* pSuffix) {
    snprintf(pDst, size, "/StageData/%s/%s%s.arc", pZoneName, pZoneName, pSuffix);
}

// NONMATCHING: regalloc; equivalent array loops, pooled strings, and local reference slots (FP order/grouping checked: no FP instructions).
StageDataArchiveHolder::StageDataArchiveHolder(const char* pStageName) : mMapArchive(), mSoundArchive(), mDesignArchive(), mZoneInfoArchive(), mUseResourceArchive(), mGhostArchive() {
    mMapArchive = mountArchive(pStageName, "Map");
    mSoundArchive = mountArchive(pStageName, "Sound");
    mDesignArchive = mountArchive(pStageName, "Design");
    mZoneInfoArchive = mountArchive(pStageName, "ZoneInfo");
    mUseResourceArchive = mountArchive(pStageName, "UseResource");
    mGhostArchive = mountArchive(pStageName, "Ghost");

    if (mSoundArchive != nullptr) {
        for (u32 i = 0; i < 2; i++) {
            const char* pName = sSoundNameList[i];
            mSoundArchive->mNameList0.push_back(pName);
        }
        for (u32 i = 0; i < 10; i++) {
            const char* pName = sAreaNameList[i];
            mSoundArchive->mNameList1.push_back(pName);
        }
    }

    if (mDesignArchive != nullptr) {
        const char* pAreaName = sAreaObjInfo;
        mDesignArchive->mNameList0.push_back(pAreaName);
        const char* pLightName = sLightCtrlArea;
        mDesignArchive->mNameList1.push_back(pLightName);
    }

    for (u32 i = 0; i < 10; i++) {
        const char* pName = sAreaNameList[i];
        mMapArchive->mNameList2.push_back(pName);
        if (mDesignArchive != nullptr) {
            const char* pDesignName = sAreaNameList[i];
            mDesignArchive->mNameList2.push_back(pDesignName);
        }
    }

    const char* pLightName = sLightCtrlArea;
    mMapArchive->mNameList2.push_back(pLightName);
    if (mSoundArchive != nullptr) {
        const char* pSoundLightName = sLightCtrlArea;
        mSoundArchive->mNameList2.push_back(pSoundLightName);
    }
}

void StageDataArchiveHolder::initJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir) {
    mMapArchive->initJmpInfo(pList, pDir);
    if (mSoundArchive != nullptr) {
        mSoundArchive->initJmpInfo(pList, pDir);
    }
    if (mDesignArchive != nullptr) {
        mDesignArchive->initJmpInfo(pList, pDir);
    }
}

void StageDataArchiveHolder::initLayerJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            initJmpInfo(pList, path);
        }
    }
}

void StageDataArchiveHolder::initCommonLayerJmpInfo(const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            mMapArchive->initJmpInfo(&mMapArchive->mCommonInfos, path);
            if (mSoundArchive != nullptr) {
                mSoundArchive->initJmpInfo(&mSoundArchive->mCommonInfos, path);
            }
            if (mDesignArchive != nullptr) {
                mDesignArchive->initJmpInfo(&mDesignArchive->mCommonInfos, path);
            }
        }
    }
}

void StageDataArchiveHolder::initScenarioLayerJmpInfo(const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            mMapArchive->initJmpInfo(&mMapArchive->mScenarioInfos, path);
            if (mSoundArchive != nullptr) {
                mSoundArchive->initJmpInfo(&mSoundArchive->mScenarioInfos, path);
            }
            if (mDesignArchive != nullptr) {
                mDesignArchive->initJmpInfo(&mDesignArchive->mScenarioInfos, path);
            }
        }
    }
}

StageDataArchive* StageDataArchiveHolder::mountArchive(const char* pStageName, const char* pDir) {
    char path[256];
    snprintf(path, sizeof(path), "/StageData/%s/%s%s.arc", pStageName, pStageName, pDir);
    JKRArchive* pArchive = MR::receiveArchive(path);

    if (pArchive == nullptr) {
        return nullptr;
    }

    return new StageDataArchive(pArchive, pStageName, pDir);
}


template void MR::Vector< MR::FixedArray< const char*, 16 > >::push_back(const char* const&);
