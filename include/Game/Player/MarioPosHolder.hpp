#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class MarioPosHolder {
public:
    MarioPosHolder();

    void fn_80405EB0();
    void fn_80405EF0(s32);
    void fn_80405F90(const TVec3f&);
    void fn_80406010(const TVec3f&);
    void fn_80406040();
    void fn_80406080();
    void fn_804060C0();
    bool fn_804060D0();
    void fn_804062A0();

    TVec3f mPreviousPosition;
    TVec3f mPos;
    bool _18;
    bool _19;
    s32 mPushDepth;
    TVec3f mSavedPosition;
    bool mLocked;
};
