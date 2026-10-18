#pragma once
#include "Game/Screen/LayoutActor.hpp"
class DreamerButtonGuidance : public LayoutActor {
public:
    DreamerButtonGuidance(const char*);
    virtual ~DreamerButtonGuidance();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    void exeAppear();
    void exeEnd();
    void showDreamer();
    void showSuperDreamer();
    void forceAppear();
    void disappear();
    void hideExit();
    void showExit();
};
