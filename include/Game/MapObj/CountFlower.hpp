#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class PlantGroup;
class CountFlower : public LiveActor {
public:
    CountFlower(const char*);
    virtual ~CountFlower();
    virtual void init(const JMapInfoIter&);
    void exeCount();
    void activateSwitch();
    PlantGroup** mPlants;
    s32 mPlantCount;
    bool mAppearPowerStar;
};
