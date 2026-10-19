#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class AstroChangeStageCube : public AreaObj {
public:
    AstroChangeStageCube(int, const char*);

    virtual ~AstroChangeStageCube();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual bool isInVolume(const TVec3f&) const;

    /* 0x3C */ u32 _3C;
    /* 0x40 */ u32 _40;
    s32 mWaitFrame;
};
