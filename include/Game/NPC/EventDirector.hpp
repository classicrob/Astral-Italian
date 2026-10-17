#pragma once

#include "revolution/types.h"

namespace MR {
    void addCometTimer(s32);
};

// Merged from Petari (core lane): declarations missing here
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class CometEventKeeper;
class PowerStarEventKeeper;
class PowerStarHolder;
class StageStateKeeper;
class TimeAttackEventKeeper;

namespace MR {
    void declareEventPowerStar(const char*, s32);
    void declareEventPowerStar(const NameObj*, s32);
    void initEventSystemAfterPlacement();
    bool isSuccessEventPowerStar(const char*, s32);
    bool isGreenEventPowerStar(const char*, s32);
    bool isRedEventPowerStar(const char*, s32);
    bool isGrandEventPowerStar(const char*, s32);
    void appearEventPowerStar(const char*, s32, const TVec3f*, bool, bool);
    bool isEndEventPowerStarAppearDemo(const char*);
};  // namespace MR

class EventDirector : public NameObj {
public:
    EventDirector();

    virtual void init(const JMapInfoIter& rIter);

    PowerStarEventKeeper* mPowerStarEventKeeper;
    StageStateKeeper* mStageStateKeeper;
    PowerStarHolder* mPowerStarHolder;
    CometEventKeeper* mCometEventKeeper;
    TimeAttackEventKeeper* mTimeAttackEventKeeper;
};

namespace EventFunction {
    PowerStarEventKeeper* getPowerStarEventKeeper();
    StageStateKeeper* getStageStateKeeper();
    PowerStarHolder* getPowerStarHolder();
    CometEventKeeper* getCometEventKeeper();
    TimeAttackEventKeeper* getTimeAttackEventKeeper();
    bool isStartCometEvent(const char*);
    void startCometEvent();
    void endCometEvent();
    bool isCometEvent();
};  // namespace EventFunction
