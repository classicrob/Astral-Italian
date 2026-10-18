// SMG2 value of ACTMES_SPHERE_SELECTOR_TARGET_SELECTED is unknown; only SphereSelector::sendSelectedMsgAndSetTarget uses it (not in this unit).
#define ACTMES_SPHERE_SELECTOR_TARGET_SELECTED 0
#include "Game/Map/PlanetMapCreator.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/TombSpiderEnvironment.hpp"
#include "Game/Boss/TombSpiderFunction.hpp"
#include "Game/Map/LavaSunPlanet.hpp"
#include "Game/Map/OceanHomeMapCtrl.hpp"
#include "Game/Map/PlanetMap.hpp"
#include "Game/MapObj/SwitchFreezePlanet.hpp"
#include "Game/MapObj/AnmModelObj.hpp"
#include "Game/MapObj/GravityTimerChangeWall.hpp"
#include "Game/MapObj/RisePictureBookPlanet.hpp"
#include "Game/MapObj/MarioFacePlanet.hpp"
#include "Game/Map/KoopaBattlePlanet.hpp"
#include "Game/Map/KoopaFinalStarPlanet.hpp"
#include "Game/MapObj/CrescentSeesawPlanet.hpp"
#include "Game/MapObj/BlueStarCupsulePlanet.hpp"
#include "Game/MapObj/BendSwanpPlanet.hpp"
#include "Game/Map/PlanetMapWithoutHighModel.hpp"
#include "Game/MapObj/AstroMapObj.hpp"
#include "Game/MapObj/BeamGoRoundPlanet.hpp"
#include "Game/MapObj/BumpAppearPlanet.hpp"
#include "Game/MapObj/FlexibleSphere.hpp"
#include "Game/MapObj/HatchWaterPlanet.hpp"
#include "Game/MapObj/LavaJamboSunPlanet.hpp"
#include "Game/MapObj/MarblePlanet.hpp"
#include "Game/MapObj/PeachCastleGardenPlanet.hpp"
#include "Game/MapObj/QuestionBoxGalleryObj.hpp"
#include "Game/MapObj/ReverseGravityRoomPlanet.hpp"
#include "Game/MapObj/RotateMoveObj.hpp"
#include "Game/MapObj/SandCapsuleInsidePlanet.hpp"
#include "Game/MapObj/SimpleClipPartsObj.hpp"
#include "Game/MapObj/SimpleMapObj.hpp"
#include "Game/MapObj/SnowCapsulePlanet.hpp"
#include "Game/MapObj/WormEatenPlanet.hpp"
#include "Game/MapObj.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

namespace {
    static const char* const sArcName = "PlanetMapDataTable.arc";
    static const char* const sFileName = "PlanetMapDataTable.bcsv";

    const PlanetEntry* findUniquePlanetEntry(const char* pName) {
        for (u32 i = 0; i < ARRAY_SIZE(PlanetMapCreator::sUniquePlanetCreateFuncTable); i++) {
            const PlanetEntry* pEntry = &PlanetMapCreator::sUniquePlanetCreateFuncTable[i];
            if (MR::isEqualString(pName, pEntry->mName)) {
                return pEntry;
            }
        }

        return nullptr;
    }

    static bool isDataForceLow(const PlanetMapData* pMapData) {
        for (int i = 0; i < 8; i++) {
            if (!MR::isEqualString(pMapData->mForceScenarioData[i], "Low")) {
                return true;
            }
        }

        return false;
    }
};  // namespace

// PLACEHOLDER(core): headers for these SMG2 planets do not exist yet; layout is only the allocation size seen in the
// PlanetMapCreator factory thunks. Replace with the real header once the unit gets source.
class FlowerBlossomRellayPlanet : public LiveActor {
public:
    FlowerBlossomRellayPlanet(const char*);
};

class MysteryMountainPlanet : public LiveActor {
public:
    MysteryMountainPlanet(const char*);
};

class MagmaBoatPlanetIce : public LiveActor {
public:
    MagmaBoatPlanetIce(const char*);
    u8 _pad[0xD4 - 0x90];
};

class MagmaBoatPlanetMagma : public LiveActor {
public:
    MagmaBoatPlanetMagma(const char*);
    u8 _pad[0xD4 - 0x90];
};

class RelayDrill2DPlanet : public LiveActor {
public:
    RelayDrill2DPlanet(const char*);
    u8 _pad[0xD0 - 0x90];
};

class QuicksandPlanet : public LiveActor {
public:
    QuicksandPlanet(const char*);
    u8 _pad[0xD4 - 0x90];
};

class PictureJoinPlanet : public SimpleMapObj {
public:
    PictureJoinPlanet(const char* pName) : SimpleMapObj(pName) {
    }
};

const PlanetEntry PlanetMapCreator::sUniquePlanetCreateFuncTable[78] = {
    {"BendSwanpPlanet", createNameObj< BendSwanpPlanet >},
    {"BlueStarCupsulePlanet", createNameObj< BlueStarCupsulePlanet >},
    {"CrescentSeesawPlanet", createNameObj< CrescentSeesawPlanet >},
    {"FlowerBlossomRellayPlanet", createNameObj< FlowerBlossomRellayPlanet >},
    {"KoopaFinalStarPlanet", createNameObj< KoopaFinalStarPlanet >},
    {"MagmaBoatIcePlanet", createNameObj< MagmaBoatPlanetIce >},
    {"MagmaBoatMagmaPlanet", createNameObj< MagmaBoatPlanetMagma >},
    {"MarioFacePlanet", createNameObj< MarioFacePlanet >},
    {"MysteryMountainPlanet", createNameObj< MysteryMountainPlanet >},
    {"PeachCastleNightPlanet", createNameObj< PeachCastleNightPlanet >},
    {"RelayDrill2DPlanet", createNameObj< RelayDrill2DPlanet >},
    {"RisePictureBookPlanet", createNameObj< RisePictureBookPlanet >},
    {"BattleGrapyonPlanet", createNameObj< GravityTimerChangeWall >},
    {"BarrelColonyInnerPlanet", createNameObj< PlanetMapAnimLow >},
    {"BothSideLiftPlanet", createNameObj< PlanetMapPress >},
    {"MagmaSlicePlanet", createNameObj< PlanetMapPress >},
    {"ChallengeGliderStartPlanet", createNameObj< PlanetMapFarClippable >},
    {"ChoConveyorPlanetD", createNameObj< RotateMoveObj >},
    {"CoinRoomBrickBluePlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomBrickBrownPlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomBrickFlatBrownPlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomSfBluePlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomSfPurplePlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomWoodBluePlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomWoodBrownPlanet", createNameObj< SimpleMapObj >},
    {"CoinRoomWoodFlatBrownPlanet", createNameObj< SimpleMapObj >},
    {"DarkHopperPlanetA", createNameObj< SimpleClipPartsObj >},
    {"DarkHopperPlanetB", createNameObj< SimpleClipPartsObj >},
    {"DarkHopperPlanetC", createNameObj< SimpleClipPartsObj >},
    {"DarkHopperPlanetD", createNameObj< SimpleClipPartsObj >},
    {"DarknessRoomPlanet", createNameObj< SimpleClipPartsObj >},
    {"DrumActionPlanetB", createNameObj< PlanetMapAnimLow >},
    {"Drill2DTimingPlanet", createNameObj< PlanetMapFarClippable >},
    {"FlagDiscPlanetB", createNameObj< PlanetMapFarClippable >},
    {"FlagDiscPlanetC", createNameObj< PlanetMapFarClippable >},
    {"FlagDiscPlanetD", createNameObj< RotateMoveObj >},
    {"FlagDiscPlanetE", createNameObj< PlanetMapFarClippable >},
    {"GliderCliffCreekPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderCylinderPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderDossunRoadPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderDownFallOutWardPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderForestPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderForestWaterfallRelayPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderGoalGatePlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderPoleWoodPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderStartWoodPlanet", createNameObj< PlanetMapFarClippable >},
    {"GliderWaterfallPlanet", createNameObj< PlanetMapFarClippable >},
    {"GravityFollowLift2DPlanet", createNameObj< PlanetMapProjmap >},
    {"GravityFollowPanelJumpStandPlanet", createNameObj< PlanetMapProjmap >},
    {"HoneyQueenPlanet", createNameObj< FurPlanetMap >},
    {"ItemCollectionPlanet", createNameObj< PlanetMapFarClippable >},
    {"KoopaBattleLv1Planet", createNameObj< KoopaBattlePlanet >},
    {"KoopaBattleLv2Planet", createNameObj< KoopaBattlePlanet >},
    {"KoopaBattleLv3Planet", createNameObj< KoopaBattlePlanet >},
    {"LongCylinderFlowSandPlanet", createNameObj< QuicksandPlanet >},
    {"LuckyRoomPlanet", createNameObj< PlanetMapFarClippable >},
    {"MarioFaceShipPlanet", createNameObj< MarioFacePlanet >},
    {"MuimuiAttackPlanet", createNameObj< PlanetMapFarClippable >},
    {"MysteryCandyBrightPlanet", createNameObj< SimpleClipPartsObj >},
    {"MysteryFlipPanelBrightPlanet", createNameObj< SimpleClipPartsObj >},
    {"MysteryMountainBrightPlanet", createNameObj< SimpleClipPartsObj >},
    {"PictureJoinPlanet", createNameObj< PictureJoinPlanet >},
    {"ReverseFlowSandPlanet", createNameObj< QuicksandPlanet >},
    {"ReverseGravity2DFireBlockBreakPlanet", createNameObj< AnmModelSwitchSwapVisibility >},
    {"ReverseGravity2DLiftPlanet", createNameObj< AnmModelSwitchSwapVisibility >},
    {"ReverseGravity2DRoofActionPlanet", createNameObj< AnmModelSwitchSwapVisibility >},
    {"SandyFlowSandPlanet", createNameObj< QuicksandPlanet >},
    {"SkateHolePlanet", createNameObj< SwitchFreezePlanet >},
    {"StarCreekPlanet", createNameObj< SwitchFreezePlanet >},
    {"StarDustStartPlanet", createNameObj< RotateMoveObj >},
    {"StoryBookOpeningPlanet", createNameObj< PlanetMapProjmap >},
    {"StoryBookTicoMeetPlanet", createNameObj< PlanetMapProjmap >},
    {"SwitchGravityBombHeiPlanet", createNameObj< PlanetMapProjmap >},
    {"TubeSliderStartPlanet", createNameObj< PlanetMapProjmap >},
    {"UFOShapePassThroughPlanet", createNameObj< UFOShapePassThroughPlanet >},
    {"UnderGroundDangeonPlanetA", createNameObj< PlanetMapProjmap >},
    {"YukkinaBodyPlanet", createNameObj< PlanetMapStrongLight >},
    {"ZeroGravityRoomPlanet", createNameObj< PlanetMapFarClippable >},
};

void PlanetMapCreator::makeSubModelName(const char** ppDst, const JMapInfo* pIter, s32 idx, const char* pKey, const char* pSuffix) {
    s32 data;
    MR::getCsvDataS32(&data, pIter, pKey, idx);

    const char* pPlanetName;
    MR::getCsvDataStr(&pPlanetName, pIter, "PlanetName", idx);

    if (data != 0) {
        s32 size = strlen(pPlanetName) + strlen(pSuffix) + 1;
        char* buf = new char[size];
        snprintf(buf, size, "%s%s", pPlanetName, pSuffix);

        *ppDst = buf;
    } else {
        *ppDst = nullptr;
    }
}

// FIXME: Scheduling issues
PlanetMapCreator::PlanetMapCreator(const char* pName) : NameObj(pName), mPlanetMapData() {
    JMapInfo* pIter = MR::createCsvParser(::sArcName, ::sFileName);

    mTableCount = MR::getCsvDataElementNum(pIter);
    mPlanetMapData = new PlanetMapData*[mTableCount];
    createPlanetMapDataTable(pIter);
}

CreatorFuncPtr PlanetMapCreator::getCreateFunc(const char* pParam1) {
    if (isScenarioForceLow(getTableData(pParam1))) {
        return createNameObj< PlanetMapWithoutHighModel >;
    }

    const PlanetEntry* pEntry = ::findUniquePlanetEntry(pParam1);

    if (pEntry != nullptr) {
        return pEntry->mCreateFunc;
    }

    return ::createNameObj< PlanetMap >;
}


void PlanetMapCreator::createPlanetMapDataTable(JMapInfo* pInfo) {
    for (int i = 0; i < mTableCount; i++) {
        mPlanetMapData[i] = new PlanetMapData();

        addTableData(pInfo, i);
    }
}

#pragma push
#pragma opt_loop_invariants off
void PlanetMapCreator::addTableData(const JMapInfo* pInfo, s32 idx) {
    PlanetMapData* curData = mPlanetMapData[idx];
    MR::getCsvDataStr(&curData->mPlanetName, pInfo, "PlanetName", idx);
    PlanetMapCreator::makeSubModelName(&curData->mData[0], pInfo, idx, "LowFlag", "Low");
    PlanetMapCreator::makeSubModelName(&curData->mData[1], pInfo, idx, "MiddleFlag", "Middle");
    PlanetMapCreator::makeSubModelName(&curData->mData[2], pInfo, idx, "BloomFlag", "Bloom");
    PlanetMapCreator::makeSubModelName(&curData->mData[3], pInfo, idx, "WaterFlag", "Water");
    PlanetMapCreator::makeSubModelName(&curData->mData[4], pInfo, idx, "IndirectFlag", "Indirect");

    for (int i = 0; i < 5; i++) {
        if (curData->mData[i] != nullptr) {
            if (!MR::isExistModel(curData->getData(i))) {
                curData->setData(i, nullptr);
            }
        }
    }

    for (int i = 0; i < 8; i++) {
        char data[32];
        snprintf(data, sizeof(data), "ForceLowScenarioName%d", i);

        MR::getCsvDataStr(&curData->mForceScenarioData[i], pInfo, data, idx);
    }
}
#pragma pop

PlanetMapData* PlanetMapCreator::getTableData(const char* pName) const {
    for (int i = 0; i < mTableCount; i++) {
        if (MR::isEqualString(pName, mPlanetMapData[i]->mPlanetName)) {
            return mPlanetMapData[i];
        }
    }

    return nullptr;
}

bool PlanetMapCreator::isScenarioForceLow(const PlanetMapData* pData) const {
    bool isForce = ::isDataForceLow(pData);

    if (!isForce) {
        return false;
    }

    char name[64];
    snprintf(name, sizeof(name), "%s_%d", MR::getCurrentStageName(), MR::getCurrentScenarioNo());

    for (int i = 0; i < 8; i++) {
        if (MR::isEqualString(pData->mForceScenarioData[i], "Low")) {
            return false;
        }

        if (MR::isEqualString(name, pData->mForceScenarioData[i])) {
            return true;
        }
    }

    return false;
}


CreatorFuncPtr PlanetMapCreatorFunction::getPlanetMapCreator(const char* pParam1) {
    return MR::getSceneObj< PlanetMapCreator >(SceneObj_PlanetMapCreator)->getCreateFunc(pParam1);
}

bool PlanetMapCreatorFunction::isLoadArchiveAfterScenarioSelected(const char* pArchive) {
    bool isExistTableData = MR::getSceneObj< PlanetMapCreator >(SceneObj_PlanetMapCreator)->getTableData(pArchive) != nullptr;

    if (!isExistTableData) {
        return false;
    }

    return ::isDataForceLow(MR::getSceneObj< PlanetMapCreator >(SceneObj_PlanetMapCreator)->getTableData(pArchive));
}

bool PlanetMapCreatorFunction::isRegisteredObj(const char* pArchive) {
    return MR::getSceneObj< PlanetMapCreator >(SceneObj_PlanetMapCreator)->getTableData(pArchive) != nullptr;
}
