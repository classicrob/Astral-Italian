#include "Game/AreaObj/ClipDrawArea.hpp"
#include "Game/MapObj/FallOutFieldDraw.hpp"

ClipDrawArea::ClipDrawArea(const char* pName) : AreaObj(pName) {
}

void ClipDrawArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::createFallOutFieldDraw();
}
