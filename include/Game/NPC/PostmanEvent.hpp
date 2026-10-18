#pragma once
#include "Game/NameObj/NameObj.hpp"

namespace PostmanEvent {
struct Condition {
    bool hasStageCondition() const;
    bool isValid() const;
    bool isSatisfied() const;
    s32 getAlternateConditionIndex() const;
    const char* mStage;
    s32 mScenario;
    s32 mStarCount;
};
struct RosettaEventInfo {
    bool isAvailable() const;
    s32 getReward() const;
    s32 mId;
    const char* mName;
    Condition mCondition;
    s32 mReward;
};
struct TaskEventInfo {
    bool isAvailable() const;
    bool isFinished() const;
    s32 mId;
    const char* mName;
    Condition mStart;
    Condition mAdditional;
    Condition mEnd;
};
const RosettaEventInfo* getRosettaEventInfo(s32);
const TaskEventInfo* getTaskEventInfo(s32);
bool isValidTaskEvent(s32);
bool hasReadAnyLetter();
bool canReadImportantEvent(s32);
}

class PostmanEventSystem : public NameObj {
public:
    PostmanEventSystem();
    virtual ~PostmanEventSystem();
    bool selectEvent(s32*);
    bool isCurrentTaskUnread() const;
    s32 mSelectedEvent;
};
namespace MR {
    PostmanEventSystem* getPostmanEventSystem();
}
