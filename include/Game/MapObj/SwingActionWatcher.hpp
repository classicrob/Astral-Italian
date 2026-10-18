#pragma once
#include "Game/NameObj/NameObj.hpp"
class SwingActionWatcher : public NameObj {
public:
    SwingActionWatcher(const char*);
    virtual ~SwingActionWatcher();
    virtual void movement();
    bool mWasSwinging;
    bool mStarted;
};
namespace MR { void createSwingActionWatcher(); bool isPlayerSwingActionStart(); }
