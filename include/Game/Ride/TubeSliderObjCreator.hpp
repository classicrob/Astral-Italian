#pragma once
#include "Game/NameObj/NameObj.hpp"
class TubeSlider;
class TubeSliderObjCreator : public NameObj {
public:
    TubeSliderObjCreator(TubeSlider*, const char*);
    virtual ~TubeSliderObjCreator();
    virtual void init(const JMapInfoIter&);
    TubeSlider* mSlider;
    const char* mObjName;
};
