#pragma once

#include "Game/NameObj/NameObj.hpp"

struct MarioFacePlanetSwitchElement {
    /* 0x00 */ const char* mKind;
    /* 0x04 */ const char* mName1;
    /* 0x08 */ const char* mName2;
    /* 0x0C */ s32 mObjId;
    /* 0x10 */ s32 mStarId1;
    /* 0x14 */ s32 mStarId2;
    /* 0x18 */ s32 mSwitchId;
    /* 0x1C */ s32 mThreshold;
    /* 0x20 */ u8 mIsDone;
};

class MarioFacePlanetSwitch : public NameObj {
public:
    MarioFacePlanetSwitch(const char*);
    virtual ~MarioFacePlanetSwitch();
    virtual void init(const JMapInfoIter&);

    void initElements();
    void checkElements();
    void applySatisfiedElements();
    s32 checkMarkerElement();
    s32 isElementSatisfied(MarioFacePlanetSwitchElement*);

    static void notifyApplySatisfied();
    static s32 checkMarkerInScene();
    static bool isDoneInScene();

    /* 0x14 */ s32 mElementNum;
    /* 0x18 */ MarioFacePlanetSwitchElement** mElements;
    /* 0x1C */ bool mIsDone;
};
