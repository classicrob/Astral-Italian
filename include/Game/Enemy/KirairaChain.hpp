#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ModelObj;
class SimpleJ3DModelDrawer;
class SwingRopePoint;

class KirairaChain : public LiveActor {
public:
    KirairaChain(const LiveActor*);
    virtual ~KirairaChain();

    virtual void init(const JMapInfoIter& rIter);
    virtual void draw() const;
    virtual void control();

    void initPoints();
    void updatePoints();
    void restrictPointFromTop(s32, s32, const TVec3f&, f32);
    void restrictPointFromBottom(s32, s32, const TVec3f&, f32);
    void restrictPointFromTopAndCalcAxis(s32, s32, const TVec3f&, f32);
    void restrictPointFromBottomAndCalcAxis(s32, s32, const TVec3f&, f32);

    const TVec3f* getCutPos() const {
        return &mCutPos;
    }

    /* 0x90 */ const LiveActor* mHost;
    /* 0x94 */ bool mIsCut;
    /* 0x98 */ TVec3f mCutPos;
    /* 0xA4 */ TVec3f mRailStart;
    /* 0xB0 */ TVec3f mRailEnd;
    /* 0xBC */ TPos3f mFixPointBottomMtx;
    /* 0xEC */ TPos3f mFixPointTopMtx;
    /* 0x11C */ TVec3f mFixPointTopDir;
    /* 0x128 */ TVec3f mFixPointBottomDir;
    /* 0x134 */ s32 mPointCount;
    /* 0x138 */ SwingRopePoint** mPoints;
    /* 0x13C */ s32 mUnusedPointCount;
    /* 0x140 */ u8 mPadding140[4];
    /* 0x144 */ s32 mAccelPointIdx;
    /* 0x148 */ s32 mAccelTimer;
    /* 0x14C */ TVec3f mAccel;
    /* 0x158 */ TVec3f mClippingCenter;
    /* 0x164 */ SimpleJ3DModelDrawer* mModelDrawer;
    /* 0x168 */ ModelObj* mFixPointTop;
};
