#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

// SMG2-only unit (no Petari source). Layout from the ctor stores.
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
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void updateWaterShadow();  // fn_802C2EB0
    void updateMoveSound();    // fn_802C2FA0
    void reboundFromWall();    // fn_802C31B0
    void exeWait();            // fn_802C3480
    void exeFloat();           // fn_802C3520
    void exeWind();            // fn_802C35B0

    /* 0x090 */ TPos3f mMtx90;
    /* 0x0C0 */ TQuat4f mRot;
    /* 0x0D0 */ TVec3f mDir;           // up direction (= -gravity while active)
    /* 0x0DC */ TVec3f mPlayerPos;     // last player position on floor touch
    /* 0x0E8 */ TVec3f mShadowPos;     // water surface position
    /* 0x0F4 */ TVec3f mShadowNormal;  // water surface normal
    /* 0x100 */ f32 mShadowAlpha;
    /* 0x104 */ s32 mTimer104;
    /* 0x108 */ bool mFlag108;
};
