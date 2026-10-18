#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

// SMG2-only unit (no Petari source). Layout from the ctor stores; fields after 0x100 are not yet named.
class LeafBoard : public LiveActor {
public:
    LeafBoard(const char* pName);

    virtual ~LeafBoard();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);

    /* 0x090 */ TPos3f mMtx90;
    /* 0x0C0 */ TQuat4f mRot;
    /* 0x0D0 */ TVec3f mDir;
    /* 0x0DC */ TVec3f mVelocityDC;
    /* 0x0E8 */ TVec3f mE8;
    /* 0x0F4 */ TVec3f mF4;
    /* 0x100 */ f32 mF100;
    /* 0x104 */ s32 mTimer104;
    /* 0x108 */ bool mFlag108;
};
