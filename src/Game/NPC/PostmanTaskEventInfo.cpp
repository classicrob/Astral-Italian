#pragma auto_inline off
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

#include "Game/Screen/PostManLetter.hpp"
#include "Game/LiveActor/DemoObj.hpp"
namespace PostmanEvent {
TaskEventInfo sTaskEvents[] = {
    {6, "パマタリアンからの依頼状1", {"GoroRockGalaxy", 1, 0}, {"WanwanFactoryGalaxy", 1, 0}, {"GoroRockGalaxy", 3, 0}},
    {7, "パマタリアンからの依頼状2", {"TeresaLabyrinthGalaxy", 1, 0}, {"ThunderFleetGalaxy", 2, 0}, {"ThunderFleetGalaxy", 3, 0}},
    {8, "ピーチャンからの挑戦状1", {"JungleGliderGalaxy", 1, 0}, {"TimerSwitchingPlatformGalaxy", 1, 0}, {"JungleGliderGalaxy", 2, 0}},
    {9, "ピーチャンからの挑戦状2", {"MagicGalaxy", 1, 0}, {"ChallengeGliderGalaxy", 1, 0}, {"ChallengeGliderGalaxy", 2, 0}},
    {10, "さすらいの遊び人からの招待状1", {"HoneyBeeVillageGalaxy", 1, 0}, {"AncientExcavationGalaxy", 1, 0}, {"HoneyBeeVillageGalaxy", 2, 0}},
    {11, "さすらいの遊び人からの招待状2", {"UnderGroundDangeonGalaxy", 1, 0}, {"VsKoopaLv3Galaxy", 2, 0}, {"UnderGroundDangeonGalaxy", 2, 0}},
    {12, "さすらいの遊び人からの招待状3", {0, 0, 15}, {"MokumokuValleyGalaxy", 1, 0}, {"MokumokuValleyGalaxy", 2, 0}},
};
Condition sAlternateConditions[] = {{"GhostConveyorGalaxy", 1, 0}};

}

namespace PostmanEvent {
s32 Condition::getAlternateConditionIndex() const {
    if (!hasStageCondition()) return -1;
    if (MR::isEqualString(mStage, "WhiteSnowGalaxy") && mScenario == 1) return 0;
    return -1;
}
bool isValidTaskEvent(s32 id) { return getTaskEventInfo(id) != nullptr; }
const TaskEventInfo* getTaskEventInfo(s32 id) {
    for (s32 i = 0; i < 7; i++)
        {
            const TaskEventInfo* info = &sTaskEvents[i];
            if (info->mId == id) return info;
        }
    return nullptr;
}
bool TaskEventInfo::isAvailable() const {
    if (!mStart.isSatisfied()) return false;
    if (!mAdditional.isValid()) return true;
    bool hasAlternate = mAdditional.getAlternateConditionIndex() >= 0;
    if (!hasAlternate) return mAdditional.isSatisfied();
    s32 index = mAdditional.getAlternateConditionIndex();
    const Condition* alternate;
    if (index < 0) alternate = nullptr;
    else alternate = &sAlternateConditions[index];
    return mAdditional.isSatisfied() || alternate->isSatisfied();
}
bool TaskEventInfo::isFinished() const {
    if (mEnd.isValid()) return mEnd.isSatisfied();
    return false;
}
}
