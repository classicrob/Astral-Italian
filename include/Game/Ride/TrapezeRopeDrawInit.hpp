#pragma once

#include "Game/NameObj/NameObj.hpp"

class JUTTexture;

class TrapezeRopeDrawInit : public NameObj {
public:
    TrapezeRopeDrawInit(const char*);

    virtual void initDraw() const;

    /* 0x0C */ JUTTexture* mTexture;
};
