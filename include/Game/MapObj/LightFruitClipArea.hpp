#pragma once
#include "Game/MapObj/ClipArea.hpp"
class ClipAreaShapeCylinder;
class LightFruitClipArea : public ClipArea {
public:
    LightFruitClipArea();
    virtual ~LightFruitClipArea();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    void killGlow();
    void exeAppear();
    void exeTimer();
    void exeDisappear();
    void playTimerSound();
    ClipAreaShapeCylinder* mCylinder; // 0xC4
    LiveActor* mVisual;
    TVec3f mFront;
};
namespace MR { void createLightFruitClipArea(); void startYoshiLightFruit(); void endYoshiLightFruit(); }
