#pragma once
#include "Game/Screen/LayoutActor.hpp"
class YoshiTargetMark : public LayoutActor {
public:
    YoshiTargetMark(const char*);
    virtual ~YoshiTargetMark();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    void exeAppear();
    void exeDisappear();
    void disappear();
    TVec3f mTargetPosition;
};
