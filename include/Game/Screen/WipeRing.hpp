#pragma once

#include "Game/Screen/WipeLayoutBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class WipeRing : public WipeLayoutBase {
public:
    WipeRing(bool, const char*);

    virtual void init(const JMapInfoIter&);
    virtual void wipe(s32);
    virtual void forceClose();
    virtual void forceOpen();
    virtual bool isOpen() const;
    virtual bool isClose() const;
    virtual bool isWipeIn() const;
    virtual bool isWipeOut() const;

    void appearWithColor();
    void setColor();
    void setCenterOnPlayer() { mIsCenterOnPlayerOnce = true; }
    void setCenterPos(const TVec3f&);
    void startAnim(const char*);
    bool getMarioCenterPos(TVec3f*);
    f32 calcRadius() const;
    f32 calcMaxRadius() const;
    void updatePlayerPos();
    void exeWipeIn();
    void exeWipeOut();

private:
    /* 0x20 */ TVec2f mCenterPos;
    /* 0x28 */ s32 mAnimFrame;
    /* 0x2C */ f32 mWipeOutFrame;
    /* 0x30 */ f32 mWipeInFrame;
    /* 0x40 */ bool mIsCenterOnPlayer;
    /* 0x41 */ bool mIsCenterOnPlayerOnce;
};

namespace NrvWipeRing {
    class WipeRingNrvClose : public Nerve { public: WipeRingNrvClose(); virtual void execute(Spine*) const; static WipeRingNrvClose sInstance; };
    class WipeRingNrvWipeIn : public Nerve { public: WipeRingNrvWipeIn(); virtual void execute(Spine*) const; static WipeRingNrvWipeIn sInstance; };
    class WipeRingNrvOpen : public Nerve { public: WipeRingNrvOpen(); virtual void execute(Spine*) const; static WipeRingNrvOpen sInstance; };
    class WipeRingNrvWipeOut : public Nerve { public: WipeRingNrvWipeOut(); virtual void execute(Spine*) const; static WipeRingNrvWipeOut sInstance; };
}
