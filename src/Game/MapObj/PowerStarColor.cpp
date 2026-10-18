#include "Game/MapObj/PowerStarColor.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/EventUtil.hpp"
s32 PowerStarColor::calcCurrentStage(const NameObj* declarer, s32 starId, bool* collected) {
    const char* name = declarer->mName;
    bool hasStar = MR::hasPowerStarInCurrentStageWithDeclarer(name, starId);
    s32 color = 0;
    if (MR::isPowerStarGreenInCurrentStageWithDeclarer(name, starId)) color = 2;
    else if (MR::isPowerStarBronzeInCurrentStageWithDeclarer(name, starId)) color = 1;
    if (color != 2) {
        if (!hasStar) { if (GameSequenceFunction::isSuperDreamer()) color = 1; }
        else if (color == 1 && !GameSequenceFunction::isSuperDreamer()) { hasStar = false; color = 0; }
    }
    if (collected) *collected = hasStar;
    return color;
}
s32 PowerStarColor::calcResult(bool* uncollected) {
    const char* stage = GameSequenceFunction::getClearedStageName();
    s32 starId = GameSequenceFunction::getClearedPowerStarId();
    bool hasStar = GameSequenceFunction::hasResultPowerStar();
    s32 color = 0;
    if (MR::isPowerStarGreen(stage, starId)) color = 2;
    else if (GameSequenceFunction::isResultStarNotGreen()) { color = 0; hasStar = true; }
    else if (MR::isPowerStarBronze(stage, starId)) color = 1;
    if (uncollected) *uncollected = !hasStar;
    return color;
}
