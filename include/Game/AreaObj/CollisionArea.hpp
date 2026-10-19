#pragma once

#include "Game/AreaObj/AreaObj.hpp"
#include "Game/MapObj/DynamicCollisionObj.hpp"

class AreaPolygon : public DynamicCollisionObj {
public:
    AreaPolygon();

    virtual ~AreaPolygon();

    virtual void init(const JMapInfoIter& rIter);

    void setMtx(MtxPtr pMatrix, const TVec3f& rSize, f32 offset);
    void setSurfaceAndSync(s32 surface);
    void setSurface(s32 surface);
    void invalidate();
    void validate();

    /* 0x128 */ AreaForm* mForm;
    /* 0x12C */ MtxPtr _128;
    /* 0x130 */ TVec3f _12C;
    /* 0x13C */ f32 _138;
};

class CollisionArea : public AreaObj {
public:
    CollisionArea(int formType, const char* pName);

    virtual ~CollisionArea();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    bool hitCheck(const TVec3f& rPos, f32 radius, TVec3f* pPoint, TVec3f* pNormal);

    /* 0x48 */ u32 _48;
    /* 0x4C */ f32 _4C;
    /* 0x50 */ TVec3f _50;
    /* 0x5C */ s32 _5C;
    /* 0x60 */ s32 _60;
    /* 0x64 */ bool _64;
    /* 0x68 */ s32 _68;
    /* 0x6C */ s32 _6C;
    /* 0x70 */ AreaPolygon* mPolygon;
    /* 0x74 */ bool mIsValid;
};