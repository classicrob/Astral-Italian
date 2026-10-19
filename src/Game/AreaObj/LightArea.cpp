#include "Game/AreaObj/LightArea.hpp"
#include "Game/Util.hpp"

LightArea::LightArea(const char* pName) : AreaObj(pName), mIsLinked(false) {
}

void LightArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    s32 id = 0;
    if (MR::getLinkObjID(rIter, &id)) {
        if (id != -1) {
            mIsLinked = true;
        }
    } else if (MR::getMapPartsID(rIter, &id)) {
        if (id != -1) {
            mIsLinked = true;
        }
    }
}
