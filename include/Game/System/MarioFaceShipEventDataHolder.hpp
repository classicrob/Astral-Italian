#pragma once

#include <revolution/types.h>

// Event-data table of the MarioFaceShip planet switches (class lives in the MarioFaceShipEventDataHolder unit).
// Only the static queries used by MarioFacePlanetSwitch are declared here; the instance layout is not recovered yet.

// One planet record as returned by MarioFaceShipEventDataHolder::getEventInfo().
struct MarioFaceShipEventInfo {
    /* 0x00 */ s32 _00;
    /* 0x04 */ s32 mKind;
    /* 0x08 */ const char* mName1;
    /* 0x0C */ const char* mName2;
    /* 0x10 */ s32 _10;
    /* 0x14 */ s32 mStarId1;
    /* 0x18 */ s32 mStarId2;
    /* 0x1C */ s32 mThreshold;
    /* 0x20 */ s32 _20;
    /* 0x24 */ s32 mSwitchId;
    /* 0x28 */ s32 mObjId;
};

class MarioFaceShipEventDataHolder {
public:
    static s32 getEventInfoNum();                   // fn_804C21D0
    static s32 getValidEventInfoNum();              // fn_804C2200
    static MarioFaceShipEventInfo* getEventInfo(s32);  // fn_804C2290
    static bool isKindStarOwned(s32);               // fn_804C1FF0
    static bool isKindThresholdReached(s32);        // fn_804C2040
    static bool isKindDualStarOwned(s32);           // fn_804C20E0
    static bool isKindStarAmountReached(s32);       // fn_804C2130
    static bool isKindObjectSet(s32);               // fn_804C2180
    static s32 getObjectValue(s32);                 // fn_804C2610
};

namespace MR { s32 getMarioFaceShipEventTalkCount(); bool isMarioFaceShipEventTalkValid(u32); void getMarioFaceShipEventTalkMessageId(u32,char*,u32); }

namespace MR { bool isMarioFaceShipNpcTalkAllowed(); }
