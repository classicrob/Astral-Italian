#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

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

const RosettaEventInfo* getRosettaEventInfo(s32 id) {
    for (s32 i = 0; i < 9; i++)
        if (sRosettaEvents[i].mId == id)
            return &sRosettaEvents[i];
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

#pragma dont_inline on

PostmanEventSystem::PostmanEventSystem() : NameObj("郵便屋さんシステム"), mSelectedEvent(18) {}

namespace PostmanEvent {
bool hasReadAnyLetter() {
    for (s32 i = 0; i < 18; i++) {
        if (GameDataFunction::isOnPostmanImportantLetterRead(i)) return true;
        if (GameDataFunction::isOnPostmanTaskLetterRead(i)) return true;
    }
    return false;
}
bool canReadImportantEvent(s32 id) {
    if (!MR::isValidPostmanImportantEvent(id)) return false;
    if (GameDataFunction::isOnPostmanImportantLetterRead(id)) return false;
    return MR::isPostmanImportantEventAvailable(id);
}
}

bool PostmanEventSystem::selectEvent(s32* out) {
    using namespace PostmanEvent;
    s32 selected = 18;
    for (s32 i = 0; i < 18; i++) {
        if (isValidTaskEvent(i) && !MR::isPostmanTaskEventFinished(i) &&
            !GameDataFunction::isOnPostmanTaskLetterRead(i) && MR::isPostmanTaskEventAvailable(i))
            selected = i;
    }
    s32 important;
    if (canReadImportantEvent(13)) important = 13;
    else if (canReadImportantEvent(14)) important = 14;
    else if (canReadImportantEvent(15)) important = 15;
    else if (canReadImportantEvent(16)) important = 16;
    else if (canReadImportantEvent(17)) important = 17;
    else if (canReadImportantEvent(1)) important = 1;
    else if (canReadImportantEvent(0) && !hasReadAnyLetter()) important = 0;
    else if (canReadImportantEvent(4)) important = 4;
    else { important = 18; if (canReadImportantEvent(5)) important = 5; }
    if (selected == 18) selected = important;
    bool gift;
    if (!hasReadAnyLetter()) gift = false;
    else if (!GameSequenceFunction::isFaceShipAfterFileSelect()) gift = false;
    else if (GameDataFunction::getPlayerStatusCounter14() >= 10) gift = true;
    else if (GameDataFunction::getPlayerMissNum() != 0 && GameDataFunction::getPlayerMissNum() % 4 == 0) gift = true;
    else gift = false;
    if (gift && selected != 1) selected = 2;
    if (!hasReadAnyLetter()) gift = false;
    else if (GameSequenceFunction::isFaceShipAfterFileSelect()) gift = GameDataFunction::getPendingSharedPlayerLeft() > 0;
    else gift = false;
    if (gift && selected != 1) selected = 3;
    if (selected != 18) {
        mSelectedEvent = selected;
        *out = selected;
        return true;
    }
    return false;
}
bool PostmanEventSystem::isCurrentTaskUnread() const {
    if (!PostmanEvent::isValidTaskEvent(mSelectedEvent)) return false;
    return !GameDataFunction::isOnPostmanTaskLetterRead(mSelectedEvent);
}
PostmanEventSystem* MR::getPostmanEventSystem() {
    return static_cast<PostmanEventSystem*>(MR::getSceneObjHolder()->getObj(SceneObj_Unk37));
}
PostmanEventSystem::~PostmanEventSystem() {}

namespace PostmanEvent {
s32 Condition::getAlternateConditionIndex() const {
    if (!hasStageCondition()) return -1;
    if (MR::isEqualString(mStage, "WhiteSnowGalaxy") && mScenario == 1) return 0;
    return -1;
}
bool isValidTaskEvent(s32 id) { return getTaskEventInfo(id) != nullptr; }
const TaskEventInfo* getTaskEventInfo(s32 id) {
    for (s32 i = 0; i < 7; i++)
        if (sTaskEvents[i].mId == id) return &sTaskEvents[i];
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
