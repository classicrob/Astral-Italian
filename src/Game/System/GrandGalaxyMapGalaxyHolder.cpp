#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/GrandGalaxyMapGalaxy.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sGalaxyTableName = "Galaxy";
    TVec3f sBossGalaxyRotate(0.0f, 320.0f, 0.0f);
};  // namespace

GrandGalaxyMapGalaxyHolder::GrandGalaxyMapGalaxyHolder() : mNum(0), mGalaxies(nullptr) {}

// NONMATCHING: register allocation only (loop counters/totals colored differently)
void GrandGalaxyMapGalaxyHolder::init() {
    JMapInfo* parsers[8];
    s32 num = 0;
    for (s32 i = 0; i < 8; i++) {
        parsers[i] = WorldMapFunction::createWorldParser(i + 1, sGalaxyTableName);
        num += MR::getCsvDataElementNum(parsers[i]);
    }

    mNum = num;
    s32 index = 0;
    mGalaxies = new GrandGalaxyMapGalaxy*[num];

    for (s32 world = 0; world < 8; world++) {
        JMapInfo* pParser = parsers[world];
        for (s32 i = 0; i < MR::getCsvDataElementNum(pParser); i++) {
            TVec2f offset;
            const char* pStageName = nullptr;
            const char* pMiniatureName = "MiniFlowerHighlandGalaxy";
            const char* pStageType = "Galaxy";
            MR::getCsvDataStrOrNULL(&pStageName, parsers[world], "StageName", i);
            MR::getCsvDataStrOrNULL(&pMiniatureName, parsers[world], "MiniatureName", i);
            MR::getCsvDataStrOrNULL(&pStageType, parsers[world], "StageType", i);
            GrandGalaxyMapGalaxy* pGalaxy = new GrandGalaxyMapGalaxy(pStageName, pMiniatureName);
            if (MR::isEqualString(pStageType, "BossGalaxyLv3")) {
                pGalaxy->stopRotate(sBossGalaxyRotate);
                pGalaxy->setFitRadius(6000.0f);
            }
            if (MR::hasCsvItem(parsers[world], "IconOffsetX")) {
                MR::getCsvDataF32(&offset.x, parsers[world], "IconOffsetX", i);
                MR::getCsvDataF32(&offset.y, parsers[world], "IconOffsetY", i);
                pGalaxy->mLayoutOffset.x = offset.x;
                pGalaxy->mLayoutOffset.y = offset.y;
            }
            mGalaxies[index] = pGalaxy;
            index++;
        }
    }
}

GrandGalaxyMapGalaxy* GrandGalaxyMapGalaxyHolder::find(const char* pStageName) const {
    for (s32 i = 0; i < mNum; i++) {
        if (MR::isEqualString(pStageName, mGalaxies[i]->mName)) {
            return mGalaxies[i];
        }
    }
    return nullptr;
}
