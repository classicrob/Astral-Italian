#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class MapPartsRailMover;
class ModelObj;
class VolumeModelDrawer;

class EyeBeamer : public LiveActor {
public:
    EyeBeamer(const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void draw() const;
    virtual void calcAnim();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    void initStartNerve(const JMapInfoIter& rIter);
    void initModel();
    void initRailMoveFunction(const JMapInfoIter& rIter);
    void updatePoseAndTrans();
    void updateWaterSurfaceMtx();
    void requestStartPatrol();
    bool tryGotoPatrol();
    bool tryPatrol();
    void exeDemoStartWait();
    void exeDemoWait();
    void exeDemoTurn();
    void exeDemoGotoPatrol();
    void exeWait();
    void exeTurn();
    void exeGotoPatrol();
    void exePatrol();
    bool isInBeamRange(const TVec3f& rVec) const;
    bool isOnBeam() const;

    /* 0x08C */ MapPartsRailMover* mRailMover;
    /* 0x090 */ ModelObj* mBeamBloom;
    /* 0x094 */ ModelObj* mBeamMdl;
    /* 0x098 */ VolumeModelDrawer* mBeamVolumeDrawer;
    /* 0xA0 */ TPos3f mBeamMtx;
    /* 0xD0 */ TQuat4f mBasePoseQuat;
    /* 0xE0 */ TQuat4f mPoseQuat;
    /* 0xF0 */ TVec3f mPlanePosition;
    /* 0xFC */ TVec3f mInitialPlanePos;
    /* 0x108 */ TVec3f mRailStartPos;
    /* 0x110 */ TPos3f mWaterSurfaceMtx;
    /* 0x144 */ TVec3f mClippingCenter;
    /* 0x150 */ u32 mReserved150;
    /* 0x154 */ u32 mReserved154;
    /* 0x158 */ u32 mReserved158;
    /* 0x15C */ u32 mReserved15C;
    /* 0x160 */ f32 mBeamLength;
    /* 0x164 */ f32 mWaterOffset;
    /* 0x164 */ bool mIsInMercatorCube;
};
