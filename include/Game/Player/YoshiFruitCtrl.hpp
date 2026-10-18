#pragma once
#include <revolution/types.h>
class Yoshi;
class YoshiFruitCtrl {
public:
    YoshiFruitCtrl(Yoshi*);
    void fn_80415630();
    void fn_804157E0();
    void fn_80415800();
    void fn_80415810();
    void fn_80415820();
    void fn_80415830(const char*);
    void fn_80415880(const char*);
    s32 findFruitIndex(const char*) NO_INLINE;
    const char* findFruitTransition(s32, s32) NO_INLINE;
    Yoshi* mActor;
    bool mLightFruitActive;
    bool _5;
    s32 _8;
    s32 _C;
    bool _10;
    bool _11;
    const char* _14;
    const char* _18;
};
