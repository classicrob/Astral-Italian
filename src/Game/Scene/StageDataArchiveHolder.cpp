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

namespace {
#ifdef GALAXY_NATIVE
    struct IsEqualNameCase {
#else
    struct IsEqualNameCase : std::binary_function< const char*, const char*, bool > {
#endif
        bool operator()(const char* pValue, const char* pName) const {
            return MR::isEqualStringCase(pValue, pName);
        }
    };
};  // namespace

namespace {
    struct ResourcePair {
        /* 0x0 */ const char* mFrom;
        /* 0x4 */ const char* mTo;
    };

    // Player archives that are swapped for their Luigi counterparts.
    ResourcePair sLuigiResourceTable[18] = {
        {"/ObjectData/Mario.arc", "/ObjectData/Luigi.arc"},
        {"/ObjectData/MarioFace.arc", "/ObjectData/LuigiFace.arc"},
        {"/ObjectData/BeeMario.arc", "/ObjectData/BeeLuigi.arc"},
        {"/ObjectData/HopperMario.arc", "/ObjectData/HopperLuigi.arc"},
        {"/ObjectData/IceMario.arc", "/ObjectData/IceLuigi.arc"},
        {"/ObjectData/InvincibleMario.arc", "/ObjectData/InvincibleLuigi.arc"},
        {"/ObjectData/TeresaMario.arc", "/ObjectData/TeresaLuigi.arc"},
        {"/ObjectData/MarioTornado.arc", "/ObjectData/LuigiTornado.arc"},
        {"/ObjectData/BoneMario.arc", "/ObjectData/BoneLuigi.arc"},
        {"/ObjectData/MarioShadow.arc", "/ObjectData/LuigiShadow.arc"},
        {"/ObjectData/CloudMario.arc", "/ObjectData/CloudLuigi.arc"},
        {"/ObjectData/CloudMarioCap.arc", "/ObjectData/CloudLuigiCap.arc"},
        {"/ObjectData/RockMario.arc", "/ObjectData/RockLuigi.arc"},
        {"/ObjectData/RockMarioRollingRock.arc", "/ObjectData/RockLuigiRollingRock.arc"},
        {"/ObjectData/MarioHair.arc", "/ObjectData/LuigiHair.arc"},
        {"/ObjectData/MarioHairAnim.arc", "/ObjectData/LuigiHairAnim.arc"},
        {"/ObjectData/MarioHairAnimVsKoopaLv1Galaxy.arc", "/ObjectData/LuigiHairAnimVsKoopaLv1Galaxy.arc"},
        {"/ObjectData/MarioCap.arc", "/ObjectData/LuigiCap.arc"},
    };

    // Replacement used once the star piece counter has stopped (post-game event flag).
    ResourcePair sStarPieceCounterStopResourceTable[1] = {
        {"/ObjectData/CocoNut.arc", "/ObjectData/Watermelon.arc"},
    };

    // Per-world sky archives, indexed by worldId - 2.
    ResourcePair sWorldSkyResourceTable[6] = {
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap02Sky.arc"},
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap03Sky.arc"},
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap04Sky.arc"},
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap05Sky.arc"},
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap06Sky.arc"},
        {"/ObjectData/WorldMap01Sky.arc", "/ObjectData/WorldMap07Sky.arc"},
    };

    // fn_8045A9E0
    const char* findReplacementResource(const char* pName, const ResourcePair* pTable, s32 count) {
        for (s32 i = 0; i < count; i++) {
            if (MR::isEqualStringCase(pName, pTable[i].mFrom)) {
                return pTable[i].mTo;
            }
        }

        return nullptr;
    }

    // NONMATCHING: regalloc (worldId/path share one register in the target) (FP order/grouping checked: no FP instructions).
    // fn_8045AA60: mounts every archive named in the "ResourceName" column of the given UseResource list.
    void mountUseResourceList(const char* pListName) {
        void* pData = nullptr;
        s32 size = 0;
        JKRHeap* pFileCacheHeap = MR::getFileCacheHeap();
        JKRHeap* pUseResourceHeap = MR::getUseResourceHeap();
        JKRHeap* pWorldMapHeap = MR::getWorldMapHeap();

        if (!MR::tryGetUseResourceArchiveResource(&pData, &size, pListName)) {
            return;
        }

        JMapInfo info;
        info.attach(pData);
        s32 num = info.getNumEntries();

        for (s32 i = 0; i < num; i++) {
            const char* pResName;
            if (!info.getValue(i, "ResourceName", &pResName)) {
                continue;
            }

            const char* pRes = pResName;
            s32 worldId = GameDataFunction::getCurrentWorldId();
            const char* pPath = nullptr;
            if (worldId >= 2 && findReplacementResource(pRes, sWorldSkyResourceTable, 6) != nullptr) {
                pPath = sWorldSkyResourceTable[worldId - 2].mTo;
            }

            if (pPath == nullptr && GameDataFunction::isOnGameEventFlagStarPieceCounterStop()) {
                pPath = findReplacementResource(pRes, sStarPieceCounterStopResourceTable, 1);
            }

            if (pPath == nullptr && MR::isPlayerLuigi()) {
                pPath = findReplacementResource(pRes, sLuigiResourceTable, 18);
            }

            if (pPath == nullptr) {
                pPath = pRes;
            }

            pRes = pResName;
            bool isWorldMapResource;
            if (MR::getWorldMapHeap() == nullptr) {
                isWorldMapResource = false;
            } else {
                isWorldMapResource = MR::isExistElement(GameSystemDataFunction::getWorldMapHeapResource(), "ResourceName", pRes);
            }

            bool isLuigiResource = findReplacementResource(pResName, sLuigiResourceTable, 18) != nullptr;

            if (MR::isFileExist(pPath, true)) {
                JKRHeap* pHeap = pFileCacheHeap;
                if (isWorldMapResource) {
                    pHeap = pWorldMapHeap;
                } else if (isLuigiResource) {
                    pHeap = pUseResourceHeap;
                }

                MR::mountAsyncArchive(pPath, pHeap, false, true);
            }
        }
    }
}  // namespace

void StageDataHolder::requestFileLoadCommon() {
    mountUseResourceList("common.bcsv");
    AudWrap::getInstListMgr()->requestStageUseInstList("sound_common.bcsv");
}

void StageDataHolder::requestFileLoadScenario() {
    char scenarioName[256];
    snprintf(scenarioName, sizeof(scenarioName), "scenario_%d.bcsv", MR::getCurrentScenarioNo());
    mountUseResourceList(scenarioName);
    AudInstListMgr* pInstListMgr = AudWrap::getInstListMgr();
    char soundScenarioName[256];
    snprintf(soundScenarioName, sizeof(soundScenarioName), "sound_scenario_%d.bcsv", MR::getCurrentScenarioNo());
    pInstListMgr->requestStageUseInstList(soundScenarioName);
}

// fn_8045AD60
JMapInfo* StageDataArchive::createUseResourceJMapInfo(const char* pName) {
    void* pData = nullptr;
    s32 size = 0;
    MR::getZoneStageArchiveResource(&pData, &size, pName, 0);

    if (!MR::tryGetUseResourceArchiveResource(&pData, &size, pName)) {
        return nullptr;
    }

    JMapInfo* pInfo = new JMapInfo();
    pInfo->attach(pData);
    return pInfo;
}

StageDataArchive::StageDataArchive(JKRArchive* pArchive, const char*, const char*) : mArchive(pArchive) {
}

// NONMATCHING: regalloc (loop variable order) only (FP order/grouping checked: no FP instructions).
void StageDataArchive::initJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir) {
    const char* pName;
    s32 count = mArchive->countFile(pDir) - 2;
    s32 num = count > 0 ? count : 0;

    if (num != 0) {
        JKRArcFinder* pFinder = mArchive->getFirstFile(pDir);

        for (s32 i = 0; i < num; i++) {
            pName = pFinder->mName;

            if (isNameListed(pName)) {
                JMapInfo* pInfo = new JMapInfo();
                pInfo->attach(mArchive->getIdxResource(pFinder->mDirIndex));
                pInfo->setName(pName);
                MR::BothDirPtrLink* pLink = new MR::BothDirPtrLink(pInfo);
                pList->append(pLink);
            }

            pFinder->findNextFile();
        }

        delete pFinder;
    }
}

// fn_8045AFA0
bool StageDataArchive::isNameListed(const char* pName) const {
    if (mNameList0.size() == 0) {
        return true;
    }

#ifdef GALAXY_NATIVE
    return std::find_if(mNameList0.begin(), mNameList0.end(), [pName](const char* value) {
        return IsEqualNameCase()(value, pName);
    }) != mNameList0.end();
#else
    return std::find_if(mNameList0.begin(), mNameList0.end(), std::binder2nd< IsEqualNameCase, const char* >(IsEqualNameCase(), pName)) != mNameList0.end();
#endif
}

// SMG2 0x8045B050
void StageFileLoader::makeStageArchivePath(char* pDst, u32 size, const char* pZoneName, const char* pSuffix) {
    snprintf(pDst, size, "/StageData/%s/%s%s.arc", pZoneName, pZoneName, pSuffix);
}

namespace {
    const char* sAreaObjInfo = "AreaObjInfo";
    const char* sLightCtrlArea = "LightCtrlArea";
    const char* sSoundNameList[2] = {"ObjInfo", "AreaObjInfo"};
    const char* sAreaNameList[10] = {
        "SoundEmitterPoint", "SoundPlayArea", "AudioEffectArea", "SoundEmitterSphere", "SoundEmitterCube",
        "BgmChangeArea",     "BgmProhibitArea", "BgmVolumeFadeArea", "GravityBgmCtrl", "WorldMapSyncSoundEmitterCube",
    };
};  // namespace

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
