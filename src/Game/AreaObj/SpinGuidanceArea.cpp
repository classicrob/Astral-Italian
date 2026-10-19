#include "Game/AreaObj/SpinGuidanceArea.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/StringUtil.hpp"

SpinGuidanceArea::SpinGuidanceArea(const char* pName) : AreaObj(pName) {
}

SpinGuidanceArea::~SpinGuidanceArea() {
}

void SpinGuidanceArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::createSceneObj(SceneObj_PlayerActionGuidance);

    if (MR::isObjectName(rIter, "SpinGuidanceArea")) {
        MR::getPlayerActionGuidance()->createSpinLayout();
    } else if (MR::isObjectName(rIter, "TamakoroMoveGuidanceArea") || MR::isObjectName(rIter, "TamakoroJumpGuidanceArea")) {
        MR::getPlayerActionGuidance()->createTamakoroLayout();
    }
}
