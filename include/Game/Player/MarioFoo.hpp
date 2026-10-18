#pragma once

#include "Game/Player/MarioState.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class MarioActor;
class AreaObj;
class JUTTexture;

class MarioFoo : public MarioState {
public:
    MarioFoo(MarioActor*);

    virtual f32 getStickY() const;
    virtual void init();
    virtual bool start();
    virtual bool close();
    virtual bool update();
    virtual bool notice();
    virtual void hitWall(const TVec3f&, HitSensor*);
    virtual bool passRing(const HitSensor*);
    virtual f32 getBlurOffset() const;
    virtual void draw3D() const;
    virtual const TVec3f& getGravityVec() const;

    void setStartSpeed(f32 speed) {
        _28 = speed;
        _AC = 20;
    }

    void jet();
    void updateTilt();
    void spin();
    f32 calcRingAcc();

    /* 0x11 */ u8 _pad11;
    /* 0x12 */ u16 _A8;
    /* 0x14 */ u8 _11;
    /* 0x15 */ u8 _12;
    /* 0x16 */ char _pad16[2];  // SMG2: align _14 (+4 total vs SMG1)
    /* 0x18 */ u32 _14;
    /* 0x1C */ u16 _18;
    /* 0x1E */ u16 _1A;
    /* 0x20 */ u16 _1C;
    /* 0x22 */ u16 _1E;
    /* 0x24 */ f32 _20;
    /* 0x28 */ f32 _24;
    /* 0x2C */ f32 _28;
    /* 0x30 */ f32 _2C;
    /* 0x34 */ TVec3f _30;
    /* 0x40 */ TVec3f _3C;
    /* 0x4C */ AreaObj* _48;
    /* 0x50 */ u16 _4C;
    /* 0x52 */ u16 _4E;
    /* 0x54 */ u16 _50;
    /* 0x58 */ f32 _54;
    /* 0x5C */ u8 _58;
    /* 0x5D */ u8 _59;
    /* 0x5E */ u16 _5A;
    /* 0x60 */ f32 _5C;
    /* 0x64 */ u16 _60;
    /* 0x68 */ TVec3f _64;
    /* 0x74 */ f32 _70;
    /* 0x78 */ f32 _74;
    /* 0x7C */ TMtx34f _78;
    /* 0xAC */ u16 _AA;
    /* 0xAE */ u16 _AC;
    /* 0xB0 */ u8 _AE;
    /* 0xB4 */ TVec3f _B0[64];
    /* 0x3B4 */ TVec3f _3B0[64];
    /* 0x6B4 */ u32 _6B0;
    /* 0x6B8 */ u32 _6B4;
    /* 0x6BC */ JUTTexture* _6B8;
};
