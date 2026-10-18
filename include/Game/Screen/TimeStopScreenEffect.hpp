#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class JUTTexture;

class TimeStopScreenEffect : public LiveActor {
public:
    TimeStopScreenEffect(const char*);
    virtual ~TimeStopScreenEffect();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void draw() const;
    void exeStart();
    void exeEnd();

    static TVec3f sCameraOffset;
    /* 0x90 */ JUTTexture* mCaptureTexture;
};

namespace NrvTimeStopScreenEffect {
#define DECLARE_TIMESTOP_NERVE(State) \
    class TimeStopScreenEffectNrv##State : public Nerve { \
    public: TimeStopScreenEffectNrv##State(); virtual void execute(Spine*) const; \
    static TimeStopScreenEffectNrv##State sInstance; };
    DECLARE_TIMESTOP_NERVE(Wait)
    DECLARE_TIMESTOP_NERVE(Start)
    DECLARE_TIMESTOP_NERVE(Active)
    DECLARE_TIMESTOP_NERVE(End)
#undef DECLARE_TIMESTOP_NERVE
}

namespace MR {
    JUTTexture* getTimeStopScreenCaptureTexture();
}
