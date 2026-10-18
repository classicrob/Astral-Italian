#pragma once

#include <revolution/types.h>

class JMapInfoIter;
class LiveActor;
class Nerve;
class DemoExecutor;

namespace MR {
    class FunctorBase;
};  // namespace MR

class DemoActionInfo {
public:
    DemoActionInfo();

    void registerCast(LiveActor*);
    void registerFunctor(const LiveActor*, const MR::FunctorBase&);
    void registerNerve(const LiveActor*, const Nerve*);

    void executeActionFirst() const;
    void executeActionLast() const;
    void executeStoryBookStart() const NO_INLINE;

    /* 0x00 */ const char* mPartName;
    /* 0x04 */ const char* mCastName;
    /* 0x08 */ s32 mCastID;
    /* 0x0C */ s32 mActionType;
    /* 0x10 */ const char* mPosName;
    /* 0x14 */ const char* mAnimName;
    /* 0x18 */ s32 _18;
    /* 0x1C */ s32 mCastCount;
    /* 0x20 */ LiveActor** mCastList;
    /* 0x24 */ MR::FunctorBase** mFunctors;
    /* 0x28 */ const Nerve** mNerves;
};

class DemoActionKeeper {
public:
    DemoActionKeeper(const DemoExecutor*);

    void initCast(LiveActor*, const JMapInfoIter&);
    void registerFunctor(const LiveActor*, const MR::FunctorBase&, const char*);
    void registerNerve(const LiveActor*, const Nerve*, const char*);
    void start();
    void update();
    void skip();
    void updatePartActions(bool) NO_INLINE;
    DemoActionInfo* findInfo(const LiveActor*, const char*) const;
    bool isStoryBookPart(const char*) const;
    bool isRegisteredDemoActionAppear(const LiveActor*) const;
    bool isRegisteredDemoActionFunctor(const LiveActor*) const;
    bool isRegisteredDemoActionNerve(const LiveActor*) const;
    bool isRegisteredDemoAction(const LiveActor*, s32) const;

    /* 0x00 */ const DemoExecutor* mDemoExecutor;
    /* 0x04 */ s32 mNumInfos;
    /* 0x08 */ DemoActionInfo** mInfoArray;
    /* 0x0C */ u8 _C;
};
