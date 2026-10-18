#pragma once

#include "Game/MapObj/MarioFaceShipAttentionConversationExecutor.hpp"
#include "Game/MapObj/MarioFaceShipNormalConversationExecutor.hpp"
#include "Game/NameObj/NameObj.hpp"

class ActorCameraInfo;
struct MarioFaceShipEventInfo;
struct MarioFaceShipEventCast;
class LiveActorGroup;
class TalkMessageCtrl;
class Meister;

// Argument block of MarioFaceShipNpcRegister -> director (built on the caller's stack, 0x16 bytes).
struct MarioFaceShipConversationNpcInfo {
    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ ActorCameraInfo* mCameraInfo;
    /* 0x08 */ const char* mDemoName;
    /* 0x0C */ s32 mEventNo;
    /* 0x10 */ s32 mCastId;
    /* 0x14 */ bool mFlag14;
    /* 0x15 */ bool mFlag15;
};

// Handles the "attention" conversations (the ship NPCs that call Mario).  SceneObj 0x95.
class MarioFaceShipAttentionConversationDirector : public NameObj {
public:
    typedef MarioFaceShipAttentionConversationExecutor::Param Param;

    MarioFaceShipAttentionConversationDirector(const char*);
    virtual ~MarioFaceShipAttentionConversationDirector();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();

    void registerNpc(const MarioFaceShipConversationNpcInfo*, const JMapInfoIter&);       // fn_804BF950
    void registerMeister(Meister*, const JMapInfoIter&);                                  // fn_804BFAB0
    bool getSatisfiedParam(Param**, s32);                                                 // fn_804BFBE0
    void calcSatisfiedNum();                                                              // fn_804BFCC0
    s32 getTalkIndex(s32) const;                                                          // fn_804BFD80
    bool makeTalkMessageId(s32, char*, u32);                                                   // fn_804BFDA0
    bool isParamSatisfied(const Param*, const char*, s32, s32) const;                     // fn_804BFE20

    static void registerNpcToConversation(const MarioFaceShipConversationNpcInfo*, const JMapInfoIter&);  // fn_804BFF30
    static s32 getConversationNum();                                                      // fn_804BFFD0
    static void tryStartDemo(s32);                                                        // fn_804C0000
    static s32 getTalkIndexByIndex(s32);                                                  // fn_804C0050
    static bool makeTalkMessageIdByIndex(s32, char*, u32);                                // fn_804C0090
    static bool isWaiting(s32);                                                           // fn_804C00F0
    static bool isDemoEnd(s32);                                                           // fn_804C0140
    static const char* getKindName();                                                     // fn_804C0190
    static bool makeEventName(char*, u32, s32);                                           // fn_804C01A0

    /* 0x14 */ Param** mParams;
    /* 0x18 */ s32 mParamNum;
    /* 0x1C */ Meister* mMeister;
    /* 0x20 */ MarioFaceShipAttentionConversationExecutor** mExecutors;
    /* 0x24 */ s32 mExecutorNum;
};

// Handles the ordinary ("Meister_%dEventNotice") conversations.  SceneObj 0x96.
class MarioFaceShipNormalConversationDirector : public NameObj {
public:
    typedef MarioFaceShipNormalConversationExecutor::Param Param;

    MarioFaceShipNormalConversationDirector(const char*);                                 // fn_804C0270
    virtual ~MarioFaceShipNormalConversationDirector();                                   // fn_804C0E50
    virtual void init(const JMapInfoIter&);                                               // fn_804C02D0
    virtual void initAfterPlacement();                                                    // fn_804C0500

    void registerMeister(Meister*, const JMapInfoIter&);                                  // fn_804C05D0
    bool getSatisfiedParam(Param**, s32);                                                 // fn_804C06A0
    bool getSpecialParam(Param**);                                                        // fn_804C0780
    void calcSatisfiedNum();                                                              // fn_804C0810
    s32 getTalkIndex(s32) const;                                                          // fn_804C08D0
    bool makeTalkMessageId(s32, char*, u32);                                                   // fn_804C08F0
    bool makeSpecialTalkMessageId(s32, char*, u32);                                            // fn_804C0970
    bool isParamSatisfied(const Param*, const char*, s32, s32) const;                     // fn_804C09F0

    static s32 hasSpecialConversation();                                                  // fn_804C0B30
    static s32 getConversationNum();                                                      // fn_804C0B70
    static s32 getTalkIndexByIndex(s32);                                                  // fn_804C0BA0
    static s32 getSpecialTalkIndex(s32);                                                  // fn_804C0BE0
    static void tryStartDemo(s32);                                                        // fn_804C0C10
    static void tryStartSpecialDemo(s32);                                                 // fn_804C0C60
    static bool makeTalkMessageIdByIndex(s32, char*, u32);                                // fn_804C0C90
    static bool makeSpecialTalkMessageIdByIndex(s32, char*, u32);                         // fn_804C0CF0
    static bool isWaiting(s32);                                                           // fn_804C0D50
    static bool isSpecialWaiting(s32);                                                    // fn_804C0DA0
    static bool isDemoEnd(s32);                                                           // fn_804C0DD0
    static bool isSpecialDemoEnd(s32);                                                    // fn_804C0E20

    /* 0x14 */ Param** mParams;
    /* 0x18 */ s32 mParamNum;
    /* 0x1C */ Meister* mMeister;
    /* 0x20 */ MarioFaceShipNormalConversationExecutor* mSpecialExecutor;
    /* 0x24 */ MarioFaceShipNormalConversationExecutor** mExecutors;
    /* 0x28 */ s32 mExecutorNum;
    /* 0x2C */ bool mHasSpecial;
};
