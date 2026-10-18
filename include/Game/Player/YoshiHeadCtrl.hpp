#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class YoshiModelCtrl;
class YoshiHeadCtrl {
public:
    YoshiHeadCtrl(YoshiModelCtrl*);
    void fn_8041A9F0();
    bool fn_8041AD70(s32) const;
    void fn_8041AA70(s32, const TVec3f*);
    void fn_8041AB50(s32);
    void fn_8041AD20(s32);
    void fn_8041AC40(s32);
    YoshiModelCtrl* mModelCtrl;
    s32 _4;
    s32 mTargetCount;
    bool mEnabled[2];
    bool _E[2];
    const TVec3f* mTargets[2];
};
