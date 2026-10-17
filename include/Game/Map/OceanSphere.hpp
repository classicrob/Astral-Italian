#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JUTTexture;
class OceanSpherePoint;
class WaterInfo;

class OceanSpherePlane {
public:
    OceanSpherePlane(s32 pointCount, const TVec3f* pCenter, const TVec3f& rAxis1, const TVec3f& rAxis2, const TVec2f& rTex1, const TVec2f& rTex2,
                     const TVec2f& rTex3);

    void update(f32 radius, f32 wave1Time, f32 wave2Time);
    OceanSpherePoint* getPoint(int col, int row) const {
        return mPoints[(mAxisPointCount * row) + col];
    }

    /* 0x00 */ s32 mGridPointCount;
    /* 0x04 */ s32 mAxisPointCount;
    /* 0x08 */ OceanSpherePoint** mPoints;
};

class OceanSpherePlaneEdge {
public:
    OceanSpherePlaneEdge(s32 pointCount, const TVec3f* pCenter, const TVec3f& rAxis1, const TVec3f& rAxis2, const TVec2f& rTex1, const TVec2f& rTex2);

    void update(f32 radius, f32 wave1Time, f32 wave2Time);

    /* 0x00 */ s32 mPointCount;
    /* 0x04 */ OceanSpherePoint** mPoints;
};

class OceanSphere : public LiveActor {
public:
    OceanSphere(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void control();
    virtual void draw() const;

    bool isInWater(const TVec3f& rPos) const;
    bool calcWaterInfo(const TVec3f& rPos, const TVec3f& rGravity, WaterInfo* pInfo) const;
    void exeWait();
    void exeRiseUp();
    void initPoints();
    void initDisplayList();
    void updatePoints();
    OceanSpherePoint* getPlanePointLeftUpper(int row, int col) const;
    OceanSpherePoint* getPlanePointRightUpper(int row, int col) const;
    OceanSpherePoint* getPlanePointLeftLower(int row, int col) const;
    OceanSpherePoint* getPlanePointRightLower(int row, int col) const;
    void loadMaterialFace() const;
    void loadMaterialBack() const;
    void drawSphere(bool useEnvMap, bool useGD) const;
    void sendVertex(const OceanSpherePoint* pPoint, bool useEnvMap, bool useGD) const;

    /* 0x90 */ f32 mRadius;
    /* 0x94 */ f32 mRadiusTarget;
    /* 0x98 */ s32 mPointCount;
    /* 0x9C */ OceanSpherePoint* mAxisPointPX;
    /* 0xA0 */ OceanSpherePoint* mAxisPointNX;
    /* 0xA4 */ OceanSpherePoint* mAxisPointPY;
    /* 0xA8 */ OceanSpherePoint* mAxisPointNY;
    /* 0xAC */ OceanSpherePoint* mAxisPointPZ;
    /* 0xB0 */ OceanSpherePoint* mAxisPointNZ;
    /* 0xB4 */ OceanSpherePlane* mPlaneLeftUpper;
    /* 0xB8 */ OceanSpherePlane* mPlaneRightUpper;
    /* 0xBC */ OceanSpherePlane* mPlaneLeftLower;
    /* 0xC0 */ OceanSpherePlane* mPlaneRightLower;
    /* 0xC4 */ OceanSpherePlaneEdge* mEdge0;
    /* 0xC8 */ OceanSpherePlaneEdge* mEdge1;
    /* 0xCC */ OceanSpherePlaneEdge* mEdge2;
    /* 0xD0 */ OceanSpherePlaneEdge* mEdge3;
    /* 0xD4 */ OceanSpherePlaneEdge* mEdge4;
    /* 0xD8 */ OceanSpherePlaneEdge* mEdge5;
    /* 0xDC */ OceanSpherePlaneEdge* mEdge6;
    /* 0xE0 */ OceanSpherePlaneEdge* mEdge7;
    /* 0xE4 */ f32 mWaveTime1;
    /* 0xE8 */ f32 mWaveTime2;
    /* 0xEC */ f32 mTexOffs0X;
    /* 0xF0 */ f32 mTexOffs0Y;
    /* 0xF4 */ f32 mTexOffs1X;
    /* 0xF8 */ f32 mTexOffs1Y;
    /* 0xFC */ JUTTexture* mWaterTex;
    /* 0x100 */ JUTTexture* mWaterEnvTex;
    /* 0x104 */ GXColor mTevReg1Front;
    /* 0x108 */ GXColor mTevReg1Back;
    /* 0x10C */ bool mUseDisplayList;
    /* 0x10D */ u8 _109;
    /* 0x10E */ u8 _10A;
    /* 0x10F */ u8 _10B;
    /* 0x110 */ u32 mDispListFaceLen;
    /* 0x114 */ u8* mDispListFace;
    /* 0x118 */ u32 mDispListBackLen;
    /* 0x11C */ u8* mDispListBack;
    /* 0x120 */ bool mAlwaysUseRealDrawing;
    /* 0x121 */ bool mIsCameraInside;
    /* 0x122 */ bool mIsStartPosCamera;
    /* 0x123 */ bool mIsTearDrop;
    /* 0x124 */ bool mEnableStartPosCameraSwitch;
};
