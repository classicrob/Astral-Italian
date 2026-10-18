#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GameSequenceMarioFaceShip;
class NameObj;

// One queued demo of the Mario face ship event sequence.
struct MarioFaceShipEventEntry {
    /* 0x0 */ const char* mDemoName;
    /* 0x4 */ s32 mTalkIndex;  // >= 0: a face ship talk (MarioFaceShipEventDataHolder), -1: plain demo
};

// Array element of the event queue: initialised (name = NULL, no talk) by the queue constructor loop.
struct MarioFaceShipEventSlot : public MarioFaceShipEventEntry {
    MarioFaceShipEventSlot() {
        mDemoName = 0;
        mTalkIndex = -1;
    }
};

// Queue of up to four events (0x24 bytes).
class MarioFaceShipEventList {
public:
    MarioFaceShipEventList();  // fn_804B4B30
    void add(const MarioFaceShipEventEntry&) NO_INLINE;  // fn_804B5C10

    /* 0x00 */ MarioFaceShipEventSlot mEntries[4];
    /* 0x20 */ s32 mNum;
};

// Event queue of the Mario face ship (0x3C bytes, see GameSequenceMarioFaceShip::init).
class GameSequenceMarioFaceShipEvent : public ActorStateBase< GameSequenceMarioFaceShip > {
public:
    GameSequenceMarioFaceShipEvent(GameSequenceMarioFaceShip* pHost);
    virtual ~GameSequenceMarioFaceShipEvent();

    virtual void init();
    virtual void appear();

    bool hasStartEvent();                                                           // fn_804B4C90
    bool isEmpty();                                                                 // fn_804B4D10
    void makeResultEventList(MarioFaceShipEventList&);                              // fn_804B4D60
    void makeStartEventList(MarioFaceShipEventList&);                               // fn_804B4F10
    void addGrandStarEvents(MarioFaceShipEventList&, const char*, const char*);     // fn_804B50B0
    void tryRequestSaveDemo();                                                      // fn_804B5150
    void onResultDemoEnd();                                                         // fn_804B51B0
    bool startNextEvent();                                                          // fn_804B5250
    void endDemoAndStartNext(const char*);                                          // fn_804B5340
    bool hasTalkIndex() const NO_INLINE;                                                      // fn_804B53E0
    bool isCurrentEventLaunch() const;                                              // fn_804B5400
    void clearFlags();                                                              // fn_804B5460
    void finishEvents();                                                            // fn_804B5530

    void exeGrandStarReturn();                                                      // fn_804B5620
    void exeResultDemo();                                                           // fn_804B5690
    void exeEventStartWait();                                                       // fn_804B57A0
    void exeBgmPrepare();                                                           // fn_804B5860
    void exeDemoEnd();                                                              // fn_804B58D0
    void exeTalkWait();                                                             // fn_804B5980
    void exeWipeWait();                                                             // fn_804B59E0
    void exeSave();                                                                 // fn_804B5A40
    void exeStartDemo();                                                            // fn_804B5AD0

    /* 0x10 */ NameObj* mDemoStarter;
    /* 0x14 */ MarioFaceShipEventList mEventList;
    /* 0x38 */ s32 mCurrentIndex;
};
