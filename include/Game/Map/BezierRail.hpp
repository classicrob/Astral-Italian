#pragma once

#include "Game/Map/RailPart.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class RailPart;

class BezierRailPart {
public:
    inline BezierRailPart(const TVec3f& a1, const TVec3f& a2, const TVec3f& a3, const TVec3f& a4) ALWAYS_INLINE {
        set(a1, a2, a3, a4);
    }

    void set(const TVec3f&, const TVec3f&, const TVec3f&, const TVec3f&);
    void calcPos(TVec3f*, f32) const;
    void calcVelocity(TVec3f*, f32) const;
    f32 getLength(f32, f32, int) const;
    f32 getParam(f32) const;
    f32 getNearestParam(const TVec3f&, f32) const;

    inline f32 getVelocityLength(f32 t) const {
        TVec3f v;
        calcVelocity(&v, t);
        return v.length();
    }

    TVec3f mStart;        // 0x00
    TVec3f mCtrlDegree1;  // 0x0C
    TVec3f mCtrlDegree2;  // 0x18
    TVec3f mCtrlDegree3;  // 0x24
    f32 mLength;          // 0x30
};

class BezierRail {
public:
    BezierRail(const JMapInfoIter&, const JMapInfo*);

    f32 normalizePos(f32, int) const;
    f32 getPartLength(int) const;
    void calcPos(TVec3f*, f32) const;
    void calcDirection(TVec3f*, f32) const;
    void calcPosDir(TVec3f*, TVec3f*, f32) const;
    f32 getNearestRailPosCoord(const TVec3f&) const;
    f32 getRailPosCoord(int) const;
    void calcCurrentRailCtrlPointIter(JMapInfoIter*, f32, bool) const;
    void calcRailCtrlPointIter(JMapInfoIter*, int) const NO_INLINE;
    void getIncludedSection(const RailPart**, f32*, f32, int) const;
    int getCurrentCtrlPointIndex(f32, bool) const;

    f32 getTotalLength() const;

    inline RailPart* getRailPart(s32 idx) const {
        return &mRailParts[idx];
    }

    bool mIsClosed;  // 0x0
    u8 _1;
    u8 _2;
    u8 _3;
    s32 mPointNum;  // 0x4
    s32 mNumRailParts;  // 0x8
    RailPart* mRailParts;  // 0xC
    f32* mPointCoords;  // 0x10
    JMapInfoIter* mIter;  // 0x14
    const JMapInfo* mInfo;  // 0x18
};
