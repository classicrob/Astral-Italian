#include "Game/AreaObj/WaterArea.hpp"
#include "Game/Map/WaterAreaHolder.hpp"

WaterArea::WaterArea(const char* pName) : AreaObj(pName) {
}

void WaterArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    WaterAreaFunction::createWaterAreaHolder();
}

WaterAreaMgr::WaterAreaMgr(s32 maxNum, const char* pName) : AreaObjMgr(maxNum, pName) {
}
