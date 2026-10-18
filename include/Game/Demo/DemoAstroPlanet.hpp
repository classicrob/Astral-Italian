#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Planet actor used by the ending demos (ctor lives in another unit).
class DemoAstroPlanet : public LiveActor {
public:
    DemoAstroPlanet(const char*, MtxPtr);
};
