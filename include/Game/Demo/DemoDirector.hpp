#pragma once

#include "Game/NameObj/NameObj.hpp"

class DemoCastGroupHolder;
class DemoExecutor;
class DemoSimpleCastHolder;
class DemoStartRequestHolder;
class LayoutActor;
class LiveActor;
class ResourceHolder;

class DemoDirector : public NameObj {
public:
    /// @brief Creates a new `DemoDirector`.
    /// @param pName A pointer to the null-terminated name of the object.
    DemoDirector(const char* pName);

    virtual void movement();

    ResourceHolder* getDemoResourceHolder(s32) const;
    void movementOnAllCasts();
    void movementOffAllCasts();
    void startDemoProgrammable(NameObj*, const char*, bool, s32);
    void startDemoTimeKeep(NameObj*, const char*, s32, bool, const char*);
    void startDemoExecutor(NameObj*, const char*, s32, const char*);
    const char* getCurrentDemoName() const;
    void endDemo(NameObj*, const char*, bool);
    bool isExistTimeKeepDemo(const char*) const;
    bool registerDemoCast(LiveActor*, const JMapInfoIter&);
    bool registerDemoCast(LiveActor*, const char*, const JMapInfoIter&);
    void registerDemoSimpleCast(LiveActor*);
    void registerDemoSimpleCast(LayoutActor*);
    void registerDemoSimpleCast(NameObj*);
    bool tryStartDemoRequested();
    void startDemo(NameObj*, const char*, bool, s32);
    void startDemoRequested();
    void doDemoEndRequest();

    /* 0x12 */ bool mIsActive;
    /* 0x13 */ bool _13;  // SMG2: only initialised in the ctor (no other access in the target)
    /* 0x14 */ DemoExecutor* mExecutor;
    /* 0x18 */ bool _14;
    /* 0x1C */ DemoCastGroupHolder* _18;
    /* 0x20 */ DemoCastGroupHolder* mCastSubGroupHolder;
    /* 0x24 */ DemoSimpleCastHolder* _20;
    /* 0x28 */ ResourceHolder** mResourceHolder;
    /* 0x2C */ DemoStartRequestHolder* mStartRequestHolder;
    /* 0x30 */ NameObj* _2C;
    /* 0x34 */ const char* _30;
    /* 0x38 */ s32 _34;
    /* 0x3C */ bool _38;
};
