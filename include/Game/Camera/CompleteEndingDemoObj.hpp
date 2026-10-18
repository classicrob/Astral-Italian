#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class DemoObj;
class ModelObj;

class CompleteEndingDemoObj : public LiveActor {
public:
    /// @brief Creates a new `CompleteEndingDemoObj`.
    /// @param pName A pointer to the null-terminated name of the object.
    CompleteEndingDemoObj(const char* pName);

    virtual ~CompleteEndingDemoObj();
    virtual void init(const JMapInfoIter&);
    virtual void kill();

    void exeAppear();

    /* 0x90 */ ModelObj* mRosetta;
    /* 0x94 */ ModelObj* mRosettaChair;
    /* 0x98 */ DemoObj* mTicoBaby;
    /* 0x9C */ ModelObj* mMarioCap;
};

namespace NrvCompleteEndingDemoObj {
    class CompleteEndingDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static CompleteEndingDemoObjNrvWait sInstance;
    };
    CompleteEndingDemoObjNrvWait CompleteEndingDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CompleteEndingDemoObjNrvAppear, CompleteEndingDemoObj, Appear);
};  // namespace NrvCompleteEndingDemoObj
