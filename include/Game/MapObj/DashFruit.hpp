#pragma once
#include "Game/MapObj/StemFruit.hpp"

// The target DashFruit vtable inherits StemFruit's state hooks and overrides only
// the destructor and slots 0xC0, 0xCC and 0xD0. No additional fields are present.
class DashFruit : public StemFruit {
public:
    DashFruit(const char*);
    virtual ~DashFruit();
    virtual const StemFruitParam* getParam();
    virtual void slot0CC();
    virtual void onEat();
};
