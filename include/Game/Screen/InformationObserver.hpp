#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class InformationObserver : public LiveActor {
public:
    enum TYPE {
        Type_Bee,
        Type_Teresa,
        Type_Hopper,
        Type_Fire,
        Type_Ice,
        Type_Flying,
        Type_Invincible,
        Type_Rock,
        Type_Cloud,
        Type_Drill,
        Type_LifeUp,
        Type_OneUp,
        Type_Yoshi,
        Type_CometMedal,
        Type_MarioFacePlanet,
    };

    /// @brief Creates a new `InformationObserver`.
    InformationObserver();

    virtual void init(const JMapInfoIter& rIter);

    void entry(InformationObserver::TYPE, LiveActor*, bool);
    void exeWait();
    void exeDisp();
    void exeEnd();

private:
    /* 0x8C */ TYPE mType;
    /* 0x90 */ bool mIsPausingDemo;
    /* 0x94 */ LiveActor* mPauseDemoActor;
    /* 0x98 */ s32 mDisplayFrame;
};

namespace InformationObserverFunction {
    bool isActive();
    void explainBee();
    void explainTeresa();
    void explainHopper();
    void explainFire();
    void explainIce();
    void explainFlying();
    void explainInvincible();
    void explainRock();
    void explainCloud();
    void explainDrill();
    void explainLifeUp();
    void explainOneUp();
    void explainYoshi(LiveActor*);
    void explainCometMedal(LiveActor*);
    void explainMarioFacePlanet(LiveActor*);
};  // namespace InformationObserverFunction
