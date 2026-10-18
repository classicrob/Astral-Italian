#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class HitSensor;

// SMG2-only unit (Nyoropon: searchlight-wielding enemy). LiveActor = 0x90 bytes. Layout from the ctor stores.
class Nyoropon : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool fn_801D8130();
    bool fn_801D81E0();
    bool fn_801D8260();
    bool fn_801D82D0();
    f32 fn_801D8350();
    void fn_801D8370();
    void fn_801D8460();
    void fn_801D8520();
    void fn_801D8640();
    void fn_801D8730();
    void fn_801D8830();
    void fn_801D8910();
    void fn_801D89F0();
    void fn_801D8AD0();
    void fn_801D8BB0();
    void fn_801D8C80();
    void fn_801D8CF0();
    void fn_801D8D60();
    void fn_801D8DD0();
    void fn_801D8EE0();
    void fn_801D9070();
    void fn_801D91A0();
    void fn_801D93A0();
    void fn_801D9C60();
    bool fn_801DA150();
    bool fn_801DA300();
    void fn_801DA370();
    Nyoropon(const char* pName);

    virtual ~Nyoropon();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual void calcAndSetBaseMtx();

public:
    /* 0x90 */ s32 _90;
    /* 0x94 */ s32 _94;
    /* 0x98 */ s32 _98;
    /* 0x9C */ s32 _9C;
    /* 0xA0 */ void* mArrayA0;
    /* 0x110 */ void* mArrayB;
    /* 0x114 */ s32 _114;
    /* 0x118 */ s32 _118;
    /* 0x11C */ f32 _11C;
    /* 0x120 */ f32 _120;
    /* 0x124 */ f32 _124;
    /* 0x128 */ f32 _128;
    /* 0x12C */ f32 _12C;
    /* 0x130 */ f32 _130;
    /* 0x134 */ f32 _134;
    /* 0x138 */ f32 _138;
    /* 0x13C */ f32 _13C;
    /* 0x140 */ f32 _140;
    /* 0x144 */ f32 _144;
    /* 0x148 */ f32 _148;
    /* 0x14C */ f32 _14C;
    /* 0x150 */ f32 _150;
    /* 0x154 */ f32 _154;
    /* 0x158 */ f32 _158;
    /* 0x15C */ f32 _15C;
    /* 0x160 */ s32 _160;
    /* 0x164 */ s32 _164;
    /* 0x168 */ bool _168;
    /* 0x169 */ bool _169;
    /* 0x16C */ TVec3f mFront;
    /* 0x178 */ TVec3f mUp;
};
