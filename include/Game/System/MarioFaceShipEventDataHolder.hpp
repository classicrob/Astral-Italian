#pragma once

#include <revolution/types.h>
#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/StringUtil.hpp"

// Event-data table of the MarioFaceShip planet switches / conversations.
// Instance layout recovered from the target (0x38 bytes).

// One cast row (MarioFaceShipEventCast.csv): the NPC names / ids that take part in an event.
struct MarioFaceShipEventCast {
    MarioFaceShipEventCast();  // fn_804C0EB0

    /* 0x00 */ s32 mEventNo;
    /* 0x04 */ s32 mCastNum;
    /* 0x08 */ const char** mCastNames;
    /* 0x0C */ s32* mCastIds;
};

// One event row (MarioFaceShipEventData.csv), 0x34 bytes.
struct MarioFaceShipEventInfo {
    MarioFaceShipEventInfo()
        : mEventType("スイッチのみ"), mKind("シナリオ"), mName1("IslandFleetGalaxy"), mName2("IslandFleetGalaxy"), mReturnType("下"),
          mStarId1(-1), mStarId2(-1), mThreshold(-1), mWatchedEventNo(-1), mSwitchId(-1), mObjId(-1) {
        mCast = 0;
        mUseTimeKeepDemo = 0;
    }

    bool isAttentionPriorTalk() const;  // fn_804C1FE0


    /* 0x00 */ const char* mEventType;   // EventType
    /* 0x04 */ const char* mKind;        // FlagType
    /* 0x08 */ const char* mName1;       // StageName
    /* 0x0C */ const char* mName2;       // OrStageName
    /* 0x10 */ const char* mReturnType;  // ReturnType
    /* 0x14 */ s32 mStarId1;             // ScenarioNo
    /* 0x18 */ s32 mStarId2;             // OrScenarioNo
    /* 0x1C */ s32 mThreshold;           // PowerStarNum
    /* 0x20 */ s32 mWatchedEventNo;      // WatchedEventNo
    /* 0x24 */ s32 mSwitchId;            // SwitchNo
    /* 0x28 */ s32 mObjId;               // EventNo
    /* 0x2C */ u8 mUseTimeKeepDemo;
    /* 0x30 */ MarioFaceShipEventCast* mCast;
};

class MarioFaceShipEventDataHolder : public NameObj {
public:
    MarioFaceShipEventDataHolder(const char*);
    virtual ~MarioFaceShipEventDataHolder();
    virtual void init(const JMapInfoIter&);

    void initCastTable();                                               // fn_804C0F70
    void initEventTable();                                              // fn_804C1110
    void findCast(s32, MarioFaceShipEventCast**) const;                 // fn_804C1400
    bool makeNoticeMessageId(s32, char*, u32) const;                    // fn_804C1440
    bool isPairedStarNotOwned(const char*, s32, const char*, s32, const char*, s32) const;  // fn_804C1560
    bool isSpecialEventSatisfied(s32, const char*, s32) const;          // fn_804C1610
    s32 getSpecialEventValue(s32) const;                                // fn_804C1640
    s32 calcTalkNum();                                                  // fn_804C1660
    void tryStartTalk(s32);                                             // fn_804C16C0
    bool isTalkValid(s32) const;                                        // fn_804C1740
    s32 getTalkIndex(s32) const;                                        // fn_804C17D0
    bool makeTalkMessageId(s32, char*, u32) const;                      // fn_804C1850
    bool isTalkEnd(s32) const;                                          // fn_804C1910
    bool isTalkWaiting(s32) const;                                      // fn_804C1990
    bool isNpcTalkAllowed() const;                                      // fn_804C1A10
    bool hasEventAtResult() const;                                      // fn_804C1AB0
    bool hasItemEventAtResult() const;                                  // fn_804C1D00

    static bool isAttentionTalkEvent(const MarioFaceShipEventInfo*);    // fn_804C1F10
    static bool isNormalTalkEvent(const MarioFaceShipEventInfo*);       // fn_804C1F90
    static bool isKindStarOwned(const char*);                           // fn_804C1FF0
    static bool isKindThresholdReached(const char*);                    // fn_804C2040
    static bool isKindCometOpen(const char*);                           // fn_804C2090
    static bool isKindDualStarOwned(const char*);                       // fn_804C20E0
    static bool isKindStarAmountReached(const char*);                   // fn_804C2130
    static bool isKindObjectSet(const char*);                           // fn_804C2180
    static s32 getEventInfoNum();                                       // fn_804C21D0
    static s32 getValidEventInfoNum();                                  // fn_804C2200
    static s32 getAttentionTalkNum();                                   // fn_804C2230
    static s32 getNormalTalkNum();                                      // fn_804C2260
    static MarioFaceShipEventInfo* getEventInfo(s32);                   // fn_804C2290
    static s32 calcAllTalkNum();                                        // fn_804C22D0
    static void tryStartTalkByIndex(s32);                               // fn_804C2350
    static bool isTalkValidByIndex(s32);                                // MR::isMarioFaceShipEventTalkValid
    static s32 getTalkIndexByIndex(s32);                                // fn_804C23D0
    static bool isTalkEndByIndex(s32);                                  // fn_804C2470
    static bool isTalkWaitingByIndex(s32);                              // fn_804C24B0
    static bool hasEventAtResultStatic();                               // fn_804C24E0
    static bool isPairedStarNotOwnedStatic(const char*, s32, const char*, s32, const char*, s32);  // fn_804C2540
    static bool isSpecialEventSatisfiedStatic(s32, const char*, s32);   // fn_804C25B0
    static s32 getObjectValue(s32);                                     // fn_804C2610

    /* 0x14 */ s32 mEventInfoNum;
    /* 0x18 */ s32 mValidEventInfoNum;
    /* 0x1C */ s32 mAttentionTalkNum;
    /* 0x20 */ s32 mNormalTalkNum;
    /* 0x24 */ MarioFaceShipEventInfo** mEventInfos;
    /* 0x28 */ s32 mCastNum;
    /* 0x2C */ MarioFaceShipEventCast** mCasts;
    /* 0x30 */ s32 mAttentionConversationNum;
    /* 0x34 */ s32 mNormalConversationNum;
};

namespace MR { s32 getMarioFaceShipEventTalkCount(); bool isMarioFaceShipEventTalkValid(u32); void getMarioFaceShipEventTalkMessageId(u32,char*,u32); }

namespace MR { bool isMarioFaceShipNpcTalkAllowed(); }
