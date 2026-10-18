#include "Game/AreaObj/SceneChangeArea.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

SceneChangeArea::SceneChangeArea(const char* pName) : AreaObj(pName) {
}

void SceneChangeArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void SceneChangeArea::movement() {
    if (isInVolume(*MR::getPlayerPos())) {
        MR::closeSystemWipeFadeWithCaptureScreen(0x78);
        GameSequenceFunction::requestChangeScenarioSelect("IslandFleetGalaxy");
        invalidate();
    }
}
