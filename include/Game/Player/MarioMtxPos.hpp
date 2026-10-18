#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

/// @brief SMG2 position matrix wrapper with a vtable (3x4 matrix at +0x4, size 0x34).
/// Original name unknown; members are by address (0x804095C0..0x80409700).
class MarioMtxPos {
public:
    MarioMtxPos();                      // fn_804095C0
    void fn_80409700(MtxPtr);           // copy a matrix in
    void fn_80409660(TVec3f*) const;    // get translation
    void fn_80409680(TVec3f*) const;    // get Y axis
    void fn_804096A0(TVec3f*) const;    // get Z axis

    virtual void fn_vf0();

    /* 0x04 */ TMtx34f mMtx;
};
