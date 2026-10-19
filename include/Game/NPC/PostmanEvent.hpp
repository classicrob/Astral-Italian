#pragma once
#include "Game/NameObj/NameObj.hpp"

namespace PostmanEvent {
struct Condition {
    bool hasStageCondition() const NO_INLINE;
    bool isValid() const NO_INLINE;
    bool isSatisfied() const NO_INLINE;
    s32 getAlternateConditionIndex() const NO_INLINE;
    const char* mStage;
    s32 mScenario;
    s32 mStarCount;
};
struct RosettaEventInfo {
    bool isAvailable() const NO_INLINE;
    s32 getReward() const NO_INLINE;
    s32 mId;
    const char* mName;
    Condition mCondition;
    s32 mReward;
};
struct TaskEventInfo {
    bool isAvailable() const NO_INLINE;
    bool isFinished() const NO_INLINE;
    s32 mId;
    const char* mName;
    Condition mStart;
    Condition mAdditional;
    Condition mEnd;
};
const RosettaEventInfo* getRosettaEventInfo(s32) NO_INLINE;
const TaskEventInfo* getTaskEventInfo(s32) NO_INLINE;
bool isValidTaskEvent(s32) NO_INLINE;
bool hasReadAnyLetter() NO_INLINE;
bool canReadImportantEvent(s32) NO_INLINE;
}

class PostmanEventSystem : public NameObj {
public:
    PostmanEventSystem();
    virtual ~PostmanEventSystem();
    bool selectEvent(s32*) NO_INLINE;
    bool isCurrentTaskUnread() const NO_INLINE;
    s32 mSelectedEvent;
};
namespace MR {
    PostmanEventSystem* getPostmanEventSystem() NO_INLINE;
}

class LiveActor;
class TalkMessageCtrl;
class PostManLetter;
class JMapInfoIter;
namespace PostmanEvent {
    TalkMessageCtrl* createTalkCtrl(LiveActor*, s32) NO_INLINE;
    PostManLetter* createLetter() NO_INLINE;
    LiveActor* createPlayerDemo(const JMapInfoIter&) NO_INLINE;
    s32 getTalkType(const PostManLetter*) NO_INLINE;
    bool receiveLetter(LiveActor*, s32) NO_INLINE;
    bool isAvailable() NO_INLINE;
    bool isCurrentReplyEvent() NO_INLINE;
}

namespace PostmanEvent {
    bool isCurrentTaskUnread() NO_INLINE;
    bool isTaskInProgress(s32) NO_INLINE;
    const char* getTaskStage(s32) NO_INLINE;
    s32 getTaskScenario(s32) NO_INLINE;
    bool isRosettaLetter(s32) NO_INLINE;
}

namespace MR {
    bool isValidPostmanImportantEvent(s32) NO_INLINE;
    bool isPostmanImportantEventAvailable(s32) NO_INLINE;
    bool isPostmanTaskEventAvailable(s32) NO_INLINE;
    bool isPostmanTaskEventFinished(s32) NO_INLINE;
}
