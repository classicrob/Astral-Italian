#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JUTTexture;

class GCaptureRibbon : public LiveActor {
public:
    GCaptureRibbon(const char*);

    virtual ~GCaptureRibbon();
    virtual void init(const JMapInfoIter&);
    virtual void draw() const;

    void reset();
    void lengthen(const TVec3f&, const TVec3f&);
    void shorten(const TVec3f&, const TVec3f&);
    void updateAxis();
    f32 calcLineWidth() const;

    /* 0x090 */ JUTTexture* mTexture;
    /* 0x094 */ TVec3f mPoints[0x40];
    /* 0x394 */ TVec3f mSideVecs[0x40];
    /* 0x694 */ TVec3f mUpVecs[0x40];
    /* 0x994 */ s32 mPointCount;
    /* 0x998 */ s32 _998;
};
