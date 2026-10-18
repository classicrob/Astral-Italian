#pragma once

#include "Game/Player/MarioState.hpp"

class MarioYoshiBalloon : public MarioState {
public:
    MarioYoshiBalloon(MarioActor*);
    virtual bool start();
    virtual bool close();
    virtual bool update();

    void fn_80414570() NO_INLINE;
    bool fn_80414580() NO_INLINE;
    void fn_80414730() NO_INLINE;
    void fn_80414B90(const TVec3f&) NO_INLINE;
    void fn_80414CD0() NO_INLINE;
    void fn_80415030() NO_INLINE;
    void fn_80415150() NO_INLINE;
    void fn_80415330() NO_INLINE;
    void fn_80415390(s32) NO_INLINE;
    bool fn_804153C0(TVec3f*, const TVec3f&, f32, f32) NO_INLINE;

    /* 0x14 */ f32 mAir;
    /* 0x18 */ s32 mRequestedState;
    /* 0x1C */ s32 mState;
    /* 0x20 */ s32 mStateFrame;
    /* 0x24 */ s32 mRequestedAnimation;
    /* 0x28 */ s32 mAnimation;
    /* 0x2C */ s32 mAnimationFrame;
    /* 0x30 */ bool mFirstJet;
    /* 0x34 */ TVec3f mWind;
};
