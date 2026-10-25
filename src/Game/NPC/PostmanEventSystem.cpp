#pragma auto_inline off
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

#include "Game/Screen/PostManLetter.hpp"
#include "Game/LiveActor/DemoObj.hpp"
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
    s32 i;
    s32 selected = 18;
    for (i = 0; i < 18; i++) {
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
