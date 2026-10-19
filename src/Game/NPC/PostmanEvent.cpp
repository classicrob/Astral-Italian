#pragma auto_inline off
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

#include "Game/Screen/PostManLetter.hpp"
#include "Game/LiveActor/DemoObj.hpp"
namespace PostmanEvent {
// NONMATCHING: local string-pool and floating-point relocations differ.
TalkMessageCtrl* createTalkCtrl(LiveActor* actor, s32 type) {
    TVec3f offset;
    offset.x = 0.0f;
    offset.y = 146.40000915527344f;
    offset.z = 0.0f;
    const char* message;
    switch (type) {
    case 0: message = "KinopioPostmanLetterIcon"; break;
    case 1: message = "KinopioPostmanOpen"; break;
    case 3: message = "KinopioPostmanLetterNormal"; break;
    case 2: message = "KinopioPostmanNoNewLetter"; break;
    case 4: message = "KinopioPostmanLetterPeach"; break;
    case 5: message = "KinopioPostmanLetterWitch"; break;
    case 6: message = "KinopioPostmanLetterRosetta"; break;
    default: message = nullptr; break;
    }
    TalkMessageCtrl* ctrl = MR::createTalkCtrlDirect(actor, message, offset, nullptr);
    MR::setDistanceToTalk(ctrl, type == 0 ? 1500.0f : 200.0f);
    MR::onRootNodeAutomatic(ctrl);
    if (type == 5) MR::setMessageArg(ctrl, GameDataFunction::getPendingSharedPlayerLeft());
    return ctrl;
}
PostManLetter* createLetter() {
    s32 id = 18;
    MR::getPostmanEventSystem()->selectEvent(&id);
    PostManLetter* letter = new PostManLetter(id);
    letter->initWithoutIter();
    return letter;
}
// NONMATCHING: argument setup and register allocation differ.
LiveActor* createPlayerDemo(const JMapInfoIter& iter) {
    const char* name = "\203x\203r\203`\203R[\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g]";
    MtxPtr base = MR::getPlayerBaseMtx();
    LiveActor* actor = MR::createDemoObjWithAnimArc(name, "TicoBaby",
                                                  "TicoBabyAnimMarioFaceShipGalaxy2", base);
    MR::tryInitDemoSheetAnim(actor, iter, "\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g[\203\215\203[\203b\203^]", "DemoSpinTeach");
    MR::invalidateClipping(actor);
    actor->makeActorDead();
    return actor;
}
s32 getTalkType(const PostManLetter* letter) {
    s32 id = letter->getCurrentLetterId();
    if (!letter->mHasUnreadLetter) id = 18;
    if (id == 3) goto shared;
    if (id < 3) {
        if (id == 1) goto opening;
        if (id >= 1) goto ordinary;
        if (id >= 0) goto icon;
        goto none;
    }
    if (id >= 18) goto none;
    if (id >= 13) goto rosetta;
    goto task;
icon: return 1;
opening: return 3;
ordinary: return 4;
shared: return 5;
task: return 3;
rosetta: return 6;
none: return 2;
}

// NONMATCHING: equivalent boolean control flow differs.
bool receiveLetter(LiveActor*, s32 id) {
    if (MR::isValidPostmanImportantEvent(id)) {
        GameDataFunction::setOnPostmanImportantLetterRead(id);
        if (isRosettaLetter(id)) {
            MR::addStarPiece(getRosettaEventInfo(id)->getReward());
        } else {
            s32 reward = getRosettaEventInfo(id)->getReward();
            if (reward > 0) MR::addPlayerLeft(reward);
        }
    }
    if (isValidTaskEvent(id)) {
        GameDataFunction::setOnPostmanTaskLetterRead(id);
        const char* stage = getTaskEventInfo(id)->mEnd.mStage;
        if (stage == nullptr) return false;
        s32 scenario = getTaskScenario(id);
        if (GameDataFunction::isOnGameEventFlagOpenScenario(stage, scenario)) return false;
        GameDataFunction::onGameEventFlagOpenScenario(stage, scenario);
        return true;
    }
    if (id == 2) {
        GameDataFunction::updatePostmanPeachLetterDeath();
        GameDataFunction::setOnPostmanImportantLetterRead(id);
        MR::addPlayerLeft(5);
        return true;
    }
    if (id == 3) {
        GameDataFunction::setOnPostmanImportantLetterRead(id);
        MR::addPlayerLeft(GameDataFunction::getPendingSharedPlayerLeft());
        return true;
    }
    return false;
}
bool isAvailable() { return getRosettaEventInfo(0)->isAvailable(); }
bool isCurrentTaskUnread() { return MR::getPostmanEventSystem()->isCurrentTaskUnread(); }
#pragma global_optimizer off
bool isCurrentReplyEvent() {
    s32 id = MR::getPostmanEventSystem()->mSelectedEvent;
    if (!isRosettaLetter(id)) return false;
    else return !GameDataFunction::isOnPostmanTaskLetterRead(id);
}
#pragma global_optimizer reset
#pragma global_optimizer off
bool isTaskInProgress(s32 id) {
    if (!isValidTaskEvent(id)) return false;
    if (!GameDataFunction::isOnPostmanTaskLetterRead(id)) return false;
    else return !getTaskEventInfo(id)->isFinished();
}
#pragma global_optimizer reset
const char* getTaskStage(s32 id) { return getTaskEventInfo(id)->mEnd.mStage; }
s32 getTaskScenario(s32 id) {
    const TaskEventInfo* info = getTaskEventInfo(id);
    if (info->mEnd.isValid()) return info->mEnd.mScenario;
    return -1;
}
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

// NONMATCHING: equivalent event selection uses different registers and sentinel-store scheduling.
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
    else { bool canRead = canReadImportantEvent(5); important = 18; if (canRead) important = 5; }
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
#pragma global_optimizer off
bool PostmanEventSystem::isCurrentTaskUnread() const {
    if (!PostmanEvent::isValidTaskEvent(mSelectedEvent)) return false;
    else return !GameDataFunction::isOnPostmanTaskLetterRead(mSelectedEvent);
}
#pragma global_optimizer reset
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


namespace MR {
bool isPostmanSpecialLetter(s32 id) { return id == 2 || id == 3; }
bool isPostmanTaskEventAvailable(s32 id) { return PostmanEvent::getTaskEventInfo(id)->isAvailable(); }
bool isPostmanTaskEventFinished(s32 id) { return PostmanEvent::getTaskEventInfo(id)->isFinished(); }
bool isPostmanImportantEventAvailable(s32 id) { return PostmanEvent::getRosettaEventInfo(id)->isAvailable(); }
s32 getPostmanImportantEventReward(s32 id) { return PostmanEvent::getRosettaEventInfo(id)->getReward(); }
bool isValidPostmanImportantEvent(s32 id) { return PostmanEvent::getRosettaEventInfo(id) != nullptr; }
}
