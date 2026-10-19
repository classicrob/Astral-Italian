#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class SwitchingScreenEffect : public LiveActor {
public:
    SwitchingScreenEffect(const char*);
    virtual ~SwitchingScreenEffect();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void exeWaitCapture();
    void exeSwitch();
    void exeDisplay();
    void updateScreenPlacement();
};

namespace NrvSwitchingScreenEffect {
#define DECLARE_SWITCHING_NERVE(State) \
    class SwitchingScreenEffectNrv##State : public Nerve { \
    public: SwitchingScreenEffectNrv##State(); virtual void execute(Spine*) const; \
    static SwitchingScreenEffectNrv##State sInstance; };
    DECLARE_SWITCHING_NERVE(Wait)
    DECLARE_SWITCHING_NERVE(WaitCapture)
    DECLARE_SWITCHING_NERVE(Switch)
    DECLARE_SWITCHING_NERVE(Display)
#undef DECLARE_SWITCHING_NERVE
}

namespace MR {
    void createSwitchingScreenEffect();
    void onSwitchingScreenEffect();
    void offSwitchingScreenEffect();
}
