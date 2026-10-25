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
    MtxPtr base = MR::getPlayerBaseMtx();
    const char* name = "\203x\203r\203`\203R[\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g]";
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
    s32 scenario;
    const char* stage;
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
        stage = getTaskEventInfo(id)->mEnd.mStage;
        if (stage == nullptr) return false;
        scenario = getTaskScenario(id);
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
}
namespace MR {
bool isPostmanSpecialLetter(s32 id) { return id == 2 || id == 3; }
}
namespace PostmanEvent {
const char* getTaskStage(s32 id) { return getTaskEventInfo(id)->mEnd.mStage; }
s32 getTaskScenario(s32 id) {
    const TaskEventInfo* info = getTaskEventInfo(id);
    if (info->mEnd.isValid()) return info->mEnd.mScenario;
    return -1;
}
}

namespace MR {
bool isPostmanTaskEventAvailable(s32 id) { return PostmanEvent::getTaskEventInfo(id)->isAvailable(); }
bool isPostmanTaskEventFinished(s32 id) { return PostmanEvent::getTaskEventInfo(id)->isFinished(); }
bool isPostmanImportantEventAvailable(s32 id) { return PostmanEvent::getRosettaEventInfo(id)->isAvailable(); }
s32 getPostmanImportantEventReward(s32 id) { return PostmanEvent::getRosettaEventInfo(id)->getReward(); }
bool isValidPostmanImportantEvent(s32 id) { return PostmanEvent::getRosettaEventInfo(id) != nullptr; }
}
