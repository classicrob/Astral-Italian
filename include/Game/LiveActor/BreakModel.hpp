#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class BreakModel : public LiveActor {
public:
    BreakModel(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx, const char* pAnimName);

    virtual ~BreakModel();
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void calcAndSetBaseMtx();

    void exeBreak();

    LiveActor* mHost;       // _90
    const char* mAnimName;  // _94
    MtxPtr mMtx;            // _98
};

namespace NrvBreakModel {
    class BreakModelNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static BreakModelNrvWait sInstance;
    };

    class BreakModelNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static BreakModelNrvBreak sInstance;
    };
};  // namespace NrvBreakModel
