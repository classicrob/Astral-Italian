#pragma once
#include "Game/NameObj/NameObj.hpp"
class YoshiFruitCounter : public NameObj {
public:
    YoshiFruitCounter();
    virtual ~YoshiFruitCounter();
    virtual void movement();
    void increment();
    u32 mCount;
    u32 mLifeDelay;
};
