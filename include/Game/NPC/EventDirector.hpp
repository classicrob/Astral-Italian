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
    bool isBronzeEventPowerStar(const char*, s32);
    s32 getEventPowerStarID(const char*);
    void pauseCometTimer();
    void resumeCometTimer();
    void showCometTimer();
    void hideCometTimer();
    void decCometTimer(s32);
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

    /* 0x10 */ StageStateKeeper* mStageStateKeeper;  // not set in SMG2 init; see report
    /* 0x14 */ PowerStarEventKeeper* mPowerStarEventKeeper;
    /* 0x18 */ PowerStarHolder* mPowerStarHolder;
    /* 0x1C */ CometEventKeeper* mCometEventKeeper;
};

namespace EventFunction {
    PowerStarEventKeeper* getPowerStarEventKeeper();
    StageStateKeeper* getStageStateKeeper();
    PowerStarHolder* getPowerStarHolder();
    CometEventKeeper* getCometEventKeeper();
    bool isStartCometEvent(const char*);
    void startCometEvent();
    void endCometEvent();
    bool isCometEvent();
};  // namespace EventFunction
