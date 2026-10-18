#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"

class StemFruitChild;

// SMG2-only unit (no Petari source). Layout and virtual slot order from the
// __vt__9StemFruit table (0x806AFAA0). Virtual slots 0x70..0xD4 are StemFruit's own.
class StemFruit : public LiveActor {
public:
    StemFruit(const char* pName);

    virtual ~StemFruit();                                     // slot 0x08
    virtual void init(const JMapInfoIter&);                   // slot 0x0C
    virtual void control();                                   // slot 0x50
    virtual void exeWait();                                   // slot 0x70
    virtual void exeTrampled();                               // slot 0x74
    virtual void exeSpin();                                   // slot 0x78
    virtual void exeShake();                                  // slot 0x7C
    virtual void exeGet();                                    // slot 0x80
    virtual void exeComeOff();                                // slot 0x84
    virtual void exePull();                                   // slot 0x88
    virtual void exePullEnd();                                // slot 0x8C
    virtual void exeComeOffAlt();                             // slot 0x90, fn_8031C260
    virtual bool tryStartTrampled();                          // slot 0x94, fn_8031C2C0
    virtual bool tryStartSpin();                              // slot 0x98, fn_8031C320
    virtual bool tryStartShake();                             // slot 0x9C, fn_8031C3A0
    virtual void setNerveGet();                               // slot 0xA0, fn_8031C410
    virtual void onGetSwitch(LiveActor*, HitSensor*);         // slot 0xA4, fn_8031C420
    virtual bool tryStartComeOff();                           // slot 0xA8, fn_8031C4A0
    virtual bool tryStartPull();                              // slot 0xAC, fn_8031C530
    virtual bool tryStartPullEnd();                           // slot 0xB0, fn_8031C590
    virtual bool tryStartPullEndAlt();                        // slot 0xB4, fn_8031C5E0
    virtual void setNerveWait();                              // slot 0xB8, fn_8031C640
    virtual void endPull(LiveActor*, HitSensor*);             // slot 0xBC, fn_8031C650
    virtual void slot0C0() = 0;                               // slot 0xC0, zero entry in binary
    virtual void updateChildState();                          // slot 0xC4, fn_8031C6C0
    virtual bool isNerveActive() const;                       // slot 0xC8, fn_8031C780
    virtual void slot0CC();                                   // slot 0xCC, fn_8031BE50 (empty)
    virtual void onEat();                                     // slot 0xD0
    virtual void onEatEnd();                                  // slot 0xD4, fn_80293DE0 (other TU)

    /* 0x090 */ StemFruitChild* mChild;
    /* 0x094 */ s32 mTimer;
};

// Child object owned by StemFruit (allocated as 0xBC bytes, LiveActor based).
class StemFruitChild : public LiveActor {
public:
    StemFruitChild(StemFruit* pParent, const JMapInfoIter& rIter);
};
