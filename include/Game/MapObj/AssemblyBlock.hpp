#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class PartsModel;

class ModelObj;

class AssemblyBlock : public LiveActor {
public:
    /// @brief Creates a new `AssemblyBlock`.
    /// @param A pointer to the null-terminated name of the object.
    AssemblyBlock(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    void exeWait();
    void exeAssemble();
    void exeAssembleWait();
    void exeReturn();
    void exeTimer();
    bool tryStartAssemble();
    bool tryStartReturn();

    /* 0x090 */ TPos3f mCurrentMtx;
    /* 0x0C0 */ TPos3f mAssembledMtx;
    /* 0x0F0 */ TPos3f mReturnMtx;
    /* 0x120 */ s32 mObjArg7;
    /* 0x124 */ f32 mPlayerSearchDistance;
    /* 0x128 */ TVec3f mClippingCenter;
    /* 0x134 */ f32 mFloatRotateSpeed;
    /* 0x138 */ s32 mTimer;
    /* 0x13C */ ModelObj* mBloomModel;
    /* 0x140 */ bool mIsTimerModel;
};
