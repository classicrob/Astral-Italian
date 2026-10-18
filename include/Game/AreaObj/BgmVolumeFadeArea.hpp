#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class BgmVolumeFadeArea : public AreaObj {
public:
    BgmVolumeFadeArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    s32 getFadeTime() const;
    f32 getVolume() const;

    /* 0x48 */ bool _48;
    /* 0x49 */ bool _49;
};
