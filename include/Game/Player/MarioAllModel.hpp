#pragma once

#include <revolution.h>
#include <JSystem/JGeometry/TVec.hpp>

class J3DModel;
class J3DModelX;
class LiveActor;
class MarioActor;
class Mario;
struct DLholder;

class MarioModelDL {
public:
    MarioModelDL(LiveActor*, const char*, const char*);
    void setDisplayList(u32);

    J3DModelX* mModel;
    u16 mMaterialIndex;
    DLholder* mDisplayLists;
};

class MarioAllModel {
public:
    MarioAllModel(MarioActor*);
    void registerModel(s32, LiveActor*, bool, bool);
    J3DModel* getJ3DModel() const;

    void fn_803E7880(s32);
    void fn_803E78F0();
    u16 fn_803E7B30();
    void fn_803E7B80();
    void fn_803E7BB0(MarioActor*);
    void fn_803E7BF0(u32);
    void fn_803E7C10(s32, LiveActor*);

    /* 0x00 */ MarioActor* mActor;
    /* 0x04 */ s32 mCurrentModel;
    /* 0x08 */ s32 mPreviousModel;
    /* 0x0C */ J3DModelX* mModels[9];
    /* 0x30 */ MarioModelDL* mModelDLs[9];
    /* 0x54 */ u16 mEyeLidTextureIndex;
};

class MarioScreenBlur {
public:
    MarioScreenBlur();
    void draw() const;

    u16 mEnabled;
    f32 mStrength;
    f32 mScale;
    u8 mAlpha;
    u8 mMode;
};

class MarioFrontHistory {
public:
    MarioFrontHistory(Mario*);
    void update();
    void getFront(TVec3f*) const;

    TVec3f* mValues;
    u16 mIndex;
    u16 mCount;
    Mario* mMario;
};
