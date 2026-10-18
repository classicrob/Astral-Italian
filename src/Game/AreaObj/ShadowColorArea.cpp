#include "Game/AreaObj/ShadowColorArea.hpp"

ShadowColorArea::ShadowColorArea(const char* pName) : AreaObj(pName) {
}

void ShadowColorArea::init(const JMapInfoIter& rIter) {
    MR::createShadowColorSetter();
    AreaObj::init(rIter);
}
