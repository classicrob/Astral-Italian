#pragma once

#include <JSystem/JGeometry/TVec.hpp>

#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <revolution/gd/GDBase.h>

class J3DShape;
class J3DShapeX;
class J3DShapePacketX;

class J3DModelX : public J3DModel {
public:
    J3DModelX(J3DModelData*, u32, u32);

    virtual ~J3DModelX();

    void viewCalc2();
    void viewCalc3(u32, MtxPtr);
    void viewCalcRef(u32, J3DModel*);
    void viewCalcRefPos(u32, J3DModel*, const TVec3f&, const TVec3f&);
    bool simpleDrawSetup(J3DMaterial*);
    void simpleDrawShape(J3DMaterial*);
    void storeDisplayList(_GDLObj*, u32);
    void shapePacketDrawFast(J3DShapePacketX*) const;
    void shapeDrawFast(J3DShapeX*) const;
    void copyExtraMtxBuffer(const J3DModelX*);
    void copyAnmMtxBuffer(const J3DModelX*);
    void swapDrawBuffer(u32);
    void setDynamicDL(u8*, u32);
    void setDrawViewBuffer(MtxPtr);
    void setDrawView(u32);
    void directDraw(J3DModel*);
    void drawIn(J3DMaterial*, bool, MtxPtr, J3DModel*);

    /* 0xDC */ u8 _DC;
    /* 0xDD */ u8 _DD;
    /* 0xE0 */ Mtx* mExtraMtxBuffer[16];
    /* 0x120 */ void (*_120)(void*, u16);
    /* 0x124 */ void (*mShapeCallback)(J3DShape*);
    /* 0x128 */ void* _128;
    /* 0x12C */ J3DModel* _12C;
    /* 0x130 */ u8* mDisplayLists[16];
    /* 0x170 */ u32 mDisplayListSizes[16];
    /* 0x1B0 */ u32 mDisplayListMask;
    /* 0x1B4 */ u8* _1B4;
    /* 0x1B8 */ u8* _1B8;
    /* 0x1BC */ u32 _1BC;
    /* 0x1C0 */ u16 _1C0;
    /* 0x1C4 */ u8** _1C4;
    /* 0x1C8 */ u8** _1C8;
    /* 0x1CC */ u16* _1CC;
    /* 0x1D0 */ u8 _1D0;
    /* 0x1D4 */ f32 _1D4;
    /* 0x1D8 */ u32 _1D8;
    /* 0x1DC */ u32 _1DC;
    /* 0x1E0 */ s32 _1E0;
    /* 0x1E4 */ u8 _1E4;
    /* 0x1E5 */ u8 _1E5;
};
