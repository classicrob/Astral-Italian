#include "Game/AreaObj/CancelBeeMorphArea.hpp"
#include "Game/Util.hpp"

CancelBeeMorphArea::CancelBeeMorphArea(const char* pName) : AreaObj(pName) {
}

void CancelBeeMorphArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void CancelBeeMorphArea::movement() {
    if (!MR::isPlayerElementModeBee()) {
        return;
    }

    TVec3f up(0.0f);
    MR::getPlayerUpVec(&up);

    TVec3f pos = *MR::getPlayerPos() + up.scaleInline(150.0f);

    if (isInVolume(*MR::getPlayerPos()) || isInVolume(pos)) {
        // the target evaluates the player position once more here; the result is unused
        MR::getPlayerPos();
        MR::curePlayerElementMode();
    }
}
