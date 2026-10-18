#pragma once

#include "Game/Effect/EffectObjR500F50.hpp"

class CaveStream : public EffectObjR500F50 {
public:
    CaveStream(const char* pName);

    virtual ~CaveStream();
    virtual void init(const JMapInfoIter& rIter);
};
