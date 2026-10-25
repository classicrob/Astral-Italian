#pragma auto_inline off
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

#include "Game/Screen/PostManLetter.hpp"
#include "Game/LiveActor/DemoObj.hpp"
namespace PostmanEvent {
bool isRosettaLetter(s32 id) {
    const RosettaEventInfo* info = getRosettaEventInfo(id);
    if (!info) return false;
    if (info->mId == 13) return true;
    if (info->mId == 14) return true;
    if (info->mId == 15) return true;
    if (info->mId == 16) return true;
    return info->mId == 17;
}
}

namespace PostmanEvent {
RosettaEventInfo sRosettaEvents[] = {
    {0, "郵便屋さんオープン", {0, -1, 13}, 1},
    {5, "星ウサギ", {"WhiteSnowGalaxy", 2, 0}, 0},
    {4, "ハニービー", {"FlowerHighlandGalaxy", 1, 0}, 1},
    {1, "チェンジルーム", {"VsKoopaLv3Galaxy", 1, 0}, 0},
    {13, "ロゼッタ1", {"GoroRockGalaxy", 2, 0}, 50},
    {14, "ロゼッタ2", {"TamakoroPlanetGalaxy", 1, 0}, 50},
    {15, "ロゼッタ3", {"OmoteuLandGalaxy", 1, 0}, 50},
    {16, "ロゼッタ4", {"KachikochiLavaGalaxy", 1, 0}, 50},
    {17, "ロゼッタ5", {"UnderGroundDangeonGalaxy", 2, 0}, 50},
};
const RosettaEventInfo* getRosettaEventInfo(s32 id) {
    for (s32 i = 0; i < 9; i++)
        {
            const RosettaEventInfo* info = &sRosettaEvents[i];
            if (info->mId == id) return info;
        }
    return nullptr;
}
bool RosettaEventInfo::isAvailable() const { return mCondition.isSatisfied(); }
s32 RosettaEventInfo::getReward() const { return mReward; }
bool Condition::hasStageCondition() const { return mStage && mScenario > 0; }
bool Condition::isValid() const { return mStarCount > 0 || (mStage && mScenario > 0); }
bool Condition::isSatisfied() const {
    bool stage = false;
    if (mStage && mScenario > 0) stage = true;
    if (stage)
        return MR::isOnGameEventFlagPowerStarSuccess(mStage, mScenario);
    return mStarCount <= MR::getPowerStarNum();
}
}
