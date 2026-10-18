#include "Game/AreaObj/ClipDrawArea.hpp"
#include "Game/MapObj/FallOutFieldDraw.hpp"

ClipDrawArea::ClipDrawArea(const char* pName) : AreaObj(pName) {
}

// NONMATCHING: target passes whatever is still in r3 after AreaObj::init (no literal arg).
void ClipDrawArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::createFallOutFieldDraw(nullptr);
}
