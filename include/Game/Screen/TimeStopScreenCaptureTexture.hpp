#pragma once
#include "Game/NameObj/NameObj.hpp"
class JUTTexture;
class TimeStopScreenCaptureTexture : public NameObj {
public:
    TimeStopScreenCaptureTexture(const char*);
    virtual ~TimeStopScreenCaptureTexture();
    /* 0x14 */ JUTTexture* mTexture;
};
