#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class HazeCube : public AreaObj {
public:
    HazeCube(s32, const char*);

    virtual void init(const JMapInfoIter&);
    virtual bool isInVolume(const TVec3f&) const;

    virtual const char* getManagerName() const {
        return "HazeCube";
    }

    /* 0x3C */ f32 mDistance;
};
