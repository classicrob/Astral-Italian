#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Map/ProductMapCreator.hpp"
#include "Game/Effect/EffectObj10x10x10SyncClipping.hpp"
#include "Game/Effect/EffectObj20x20x10SyncClipping.hpp"
#include "Game/Effect/EffectObj50x50x10SyncClipping.hpp"
#include "Game/Effect/EffectObjR1000F50.hpp"
#include "Game/Effect/EffectObjR1500F400.hpp"
#include "Game/Effect/EffectObjR500F50.hpp"
#include "Game/Map/Air.hpp"
#include "Game/Map/Sky.hpp"
#include "Game/MapObj/AnmModelObj.hpp"
#include "Game/MapObj/CloudSea.hpp"
#include "Game/MapObj/CloudStep.hpp"
#include "Game/MapObj/Gorogoro.hpp"
#include "Game/MapObj/HipDropMoveBlock.hpp"
#include "Game/MapObj/LavaFloater.hpp"
#include "Game/MapObj/RailMoveObj.hpp"
#include "Game/MapObj/RotateMoveObj.hpp"
#include "Game/MapObj/SimpleBreakableObj.hpp"
#include "Game/MapObj/SimpleClipPartsObj.hpp"
#include "Game/MapObj/SimpleFloaterObj.hpp"
#include "Game/MapObj/SimpleMapObj.hpp"
#include "Game/MapObj/SwitchingMoveBlock.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const char* const sArcName = "ProductMapObjDataTable.arc";
    const char* const sFileName = "ProductMapObjDataTable.bcsv";
}

const PlanetEntry ProductMapCreator::sCreateFuncTable[47] = {
    {"AutoMakeMapObj", createNameObj< SimpleMapObj >},
    {"AnmModelSwitchMove", createNameObj< AnmModelSwitchMove >},
    {"AnmModelSwitchMoveEndKill", createNameObj< AnmModelSwitchMoveEndKill >},
    {"AnmModelSwitchMoveEndKillAnyAnim", createNameObj< AnmModelSwitchMoveEndKillAnyAnim >},
    {"AnmModelSwitchMoveValidateCollision", createNameObj< AnmModelSwitchMoveValidateCollision >},
    {"AnmModelSwitchMoveInvalidateCollision", createNameObj< AnmModelSwitchMoveInvalidateCollision >},
    {"AnmModelSwitchSyncBrk", createNameObj< AnmModelSwitchSyncBrk >},
    {"CloudStep", createNameObj< CloudStep >},
    {"CloudSea", createNameObj< CloudSea >},
    {"HipDropMoveBlock", createNameObj< HipDropMoveBlock >},
    {"RailMoveObj", createNameObj< RailMoveObj >},
    {"RailMoveWithReverseAnimObj", createNameObj< RailMoveWithReverseAnimObj >},
    {"RailMoveWithIndirectModelObj", createNameObj< RailMoveWithIndirectModelObj >},
    {"RailMoveShadowDropYObj", createNameObj< RailMoveShadowDropYObj >},
    {"RailMoveObjClipParts", createNameObj< RailMoveObjClipParts >},
    {"RailMoveDemoActionObj", createNameObj< RailMoveDemoActionObj >},
    {"RailMoveLavaProjmapObj", createNameObj< RailMoveLavaProjmapObj >},
    {"RailRotateMoveObj", createNameObj< RailRotateMoveObj >},
    {"RailAndRotateMoveObj", createNameObj< RailAndRotateMoveObj >},
    {"RotateMoveObj", createNameObj< RotateMoveObj >},
    {"RotateMoveObjClipParts", createNameObj< RotateMoveObjClipParts >},
    {"GorogoroCylinderRock", createNameObj< GorogoroCylinderRock >},
    {"LavaFloater", createNameObj< LavaFloater >},
    {"SimpleBreakableCollisionObj", createNameObj< SimpleBreakableCollisionObj >},
    {"SimpleBreakableDeleteShadowObj", createNameObj< SimpleBreakableDeleteShadowObj >},
    {"SimpleBreakableObj", createNameObj< SimpleBreakableObj >},
    {"SimpleBreakableStrongLightObj", createNameObj< SimpleBreakableStrongLightObj >},
    {"SimpleClipPartsObj", createNameObj< SimpleClipPartsObj >},
    {"SimpleFloaterObj", createNameObj< SimpleFloaterObj >},
    {"SimpleMapObj", createNameObj< SimpleMapObj >},
    {"SimpleMapObjFarMax", createNameObj< SimpleMapObjFarMax >},
    {"SimpleMapObjWithEffect", createNameObj< SimpleMapObjWithEffect >},
    {"SimpleSeesawObj", createNameObj< SimpleSeesawObj >},
    {"SwitchingMoveBlock", createNameObj< SwitchingMoveBlock >},
    {"EffectObjR500F50", createNameObj< EffectObjR500F50 >},
    {"EffectObjR1000F50", createNameObj< EffectObjR1000F50 >},
    {"EffectObjR1500F400", createNameObj< EffectObjR1500F400 >},
    {"EffectObj20x20x10SyncClipping", createNameObj< EffectObj20x20x10SyncClipping >},
    {"EffectObj50x50x10SyncClipping", createNameObj< EffectObj50x50x10SyncClipping >},
    {"EffectObj10x10x10SyncClipping", createNameObj< EffectObj10x10x10SyncClipping >},
    {"Sky", createNameObj< Sky >},
    {"ProjectionMapSky", createNameObj< ProjectionMapSky >},
    {"SoundSyncSky", createNameObj< SoundSyncSky >},
    {"Air", createNameObj< Air >},
    {"PriorDrawAir", createNameObj< PriorDrawAir >},
    {"AfterMapObjDrawAir", createNameObj< AfterMapObjDrawAir >},
    {"ProjectionMapAir", createNameObj< ProjectionMapAir >},
};

ProductMapCreator::ProductMapCreator(const char* name) : NameObj(name), mEntries(nullptr) {
    JMapInfo* info = MR::createCsvParser(sArcName, sFileName);
    mEntryCount = MR::getCsvDataElementNum(info);
    mEntries = new ProductMapEntry*[mEntryCount];
    populateEntries(info);
}

void ProductMapCreator::populateEntries(const JMapInfo* info) {
    for (s32 i = 0; i < mEntryCount; i++) {
        mEntries[i] = new ProductMapEntry;
        parseEntry(info, i);
    }
}

void ProductMapCreator::parseEntry(const JMapInfo* info, s32 index) {
    ProductMapEntry* entry = mEntries[index];
    MR::getCsvDataStr(&entry->mModelName, info, "ModelName", index);
    MR::getCsvDataStr(&entry->mClassName, info, "ClassName", index);
}

ProductMapEntry* ProductMapCreator::getProductMapEntry(const char* name) {
    for (s32 i = 0; i < mEntryCount; i++) {
        if (MR::isEqualString(name, mEntries[i]->mModelName)) {
            return mEntries[i];
        }
    }
    return nullptr;
}

CreatorFuncPtr ProductMapCreator::getProductMapCreator(const char* name) {
    for (u32 i = 0; i < 47; i++) {
        const PlanetEntry* entry = &sCreateFuncTable[i];
        if (MR::isEqualString(name, entry->mName)) {
            return entry->mCreateFunc;
        }
    }
    return nullptr;
}

namespace MR {
    CreatorFuncPtr getProductMapCreator(const char* name) {
        ProductMapCreator* creator = getSceneObj<ProductMapCreator>(0x7F);
        return creator->getProductMapCreator(creator->getProductMapEntry(name)->mClassName);
    }

    bool hasProductMapCreator(const char* name) {
        return getSceneObj<ProductMapCreator>(0x7F)->getProductMapEntry(name) != nullptr;
    }
}


bool EffectObj10x10x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj10x10x10SyncClipping::getClippingCenterOffset() const {
#ifdef GALAXY_NATIVE
    static TVec3f offset(0.0f, 580.0f, 0.0f);
    return &offset;
#else
    return &TVec3f(0.0f, 580.0f, 0.0f);
#endif
}

f32 EffectObj10x10x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj10x10x10SyncClipping::getClippingRadius() const {
    return 1000.0f;
}


bool EffectObj50x50x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj50x50x10SyncClipping::getClippingCenterOffset() const {
#ifdef GALAXY_NATIVE
    static TVec3f offset(0.0f, 200.0f, 0.0f);
    return &offset;
#else
    return &TVec3f(0.0f, 200.0f, 0.0f);
#endif
}

f32 EffectObj50x50x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj50x50x10SyncClipping::getClippingRadius() const {
    return 2500.0f;
}


bool EffectObj20x20x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj20x20x10SyncClipping::getClippingCenterOffset() const {
#ifdef GALAXY_NATIVE
    static TVec3f offset(0.0f, 200.0f, 0.0f);
    return &offset;
#else
    return &TVec3f(0.0f, 200.0f, 0.0f);
#endif
}

f32 EffectObj20x20x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj20x20x10SyncClipping::getClippingRadius() const {
    return 1000.0f;
}

f32 EffectObjR1500F400::getFarClipDistance() const {
    return 400.0f;
}

f32 EffectObjR1500F400::getClippingRadius() const {
    return 1500.0f;
}

f32 EffectObjR1000F50::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObjR1000F50::getClippingRadius() const {
    return 1000.0f;
}


EffectObjR1000F50::~EffectObjR1000F50() {
}
