#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

/// @brief SMG2 position matrix wrapper with a vtable (3x4 matrix at +0x4, size 0x34).
/// Original name unknown; members are by address (0x804095C0..0x80409700).
class MarioMtxPos {
public:
    MarioMtxPos();                      // fn_804095C0
    MarioMtxPos(const TMtx34f&);
    MarioMtxPos(TPos3f*);
    bool invert();
    void preConcat(const MarioMtxPos&);
    void setTranslation(const TVec3f&);
    void identity();
    void fn_80409650(const MarioMtxPos&);
    void fn_804096E0(TVec3f*) const;
    void fn_80409710(const TVec3f&, const TVec3f&);
    void fn_80409700(MtxPtr);           // copy a matrix in
    void getTranslation(TVec3f*) const;
    void getYAxis(TVec3f*) const;
    void getZAxis(TVec3f*) const;

    virtual TMtx34f* fn_vf0() NO_INLINE;

    /* 0x04 */ TPos3f mMtx;
};
