#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class WaterFortressRain : public LiveActor {
public:
    /// @brief Creates a new `WaterFortressRain`.
    /// @param pName A pointer to the null-terminated name of the object.
    WaterFortressRain(const char* pName);

    virtual ~WaterFortressRain();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void exeRain();
    void exeRainSlow();

    /* 0x90 */ TPos3f mEffectMtx;
};

namespace NrvWaterFortressRain {
    NEW_NERVE(WaterFortressRainNrvRain, WaterFortressRain, Rain);
    NEW_NERVE(WaterFortressRainNrvRainSlow, WaterFortressRain, RainSlow);
};  // namespace NrvWaterFortressRain
