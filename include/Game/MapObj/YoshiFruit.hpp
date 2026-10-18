#pragma once
#include "Game/MapObj/StemFruit.hpp"
class YoshiFruit : public StemFruit {
public:
    YoshiFruit(const char* name) : StemFruit(name) {}
    virtual ~YoshiFruit();
    virtual const StemFruitParam* getParam();
    virtual void slot0CC();
    virtual void onEat();
    virtual void onEatEnd();
};
class YoshiFruitBig : public StemFruit {
public:
    YoshiFruitBig(const char* name) : StemFruit(name) {}
    virtual ~YoshiFruitBig();
    virtual const StemFruitParam* getParam();
    virtual void slot0CC();
    virtual void onEat();
    virtual void onEatEnd();
};
