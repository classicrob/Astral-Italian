#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class GravityBgmCtrl : public LiveActor {
public:
    GravityBgmCtrl(const char*);
    virtual ~GravityBgmCtrl();
    virtual void init(const JMapInfoIter&);
    static bool checkGravityDir(const TVec3f&) NO_INLINE;
    void exeChangeBack();
    void exeChange();
    bool tryChangeBack();
    bool tryChange();
    bool tryWait();
};
