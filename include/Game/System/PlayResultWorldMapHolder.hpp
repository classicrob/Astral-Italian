#pragma once

#include "Game/NameObj/NameObj.hpp"

// Opaque 0x404-byte table allocated as a WorldMapResultRoute (flags at +0x20, 8 x 0x40, counter at +0x400)
class PlayResultWorldMapHolder : public NameObj {
public:
    PlayResultWorldMapHolder();
    virtual ~PlayResultWorldMapHolder();

    void reset();

    /* 0x14 */ s32 mUnk14;
    /* 0x18 */ s32 mUnk18;
    /* 0x1C */ s32 mUnk1C;
    /* 0x220 */ u8* mTable;
    /* 0x224 */ bool mUnk224;
    /* 0x225 */ bool mUnk225;
    /* 0x228 */ s32 mUnk228;
    /* 0x22C */ bool mUnk22C;
    /* 0x22D */ bool mUnk22D;
};

extern "C" {
    void fn_804C96B0(PlayResultWorldMapHolder*, s32);
    void fn_804C96C0(PlayResultWorldMapHolder*, s32);
    void fn_804C96D0(u8*, s32, s32);
    s32 fn_804C96F0(u8*, s32, s32);
    void fn_804C9710(u8*);
    void fn_804C9750(PlayResultWorldMapHolder*, u8*);
    void fn_804C9760(u8*, u8*);
    void fn_804C9790(PlayResultWorldMapHolder*, u8*);
    void fn_804C97A0(PlayResultWorldMapHolder*);
};
