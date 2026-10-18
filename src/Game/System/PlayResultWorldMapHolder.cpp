#include "Game/System/PlayResultWorldMapHolder.hpp"

PlayResultWorldMapHolder::PlayResultWorldMapHolder() : NameObj("\x83v\x83\x8c\x83""C\x8c\x8b\x89\xca\x95\xdb\x8e\x9d[\x83\x8f\x81[\x83\x8b\x83h\x83}\x83""b\x83v]") {
    mTable = nullptr;
    mTable = static_cast< u8* >(fn_804ECBC0(new u8[0x404]));
    reset();
}

PlayResultWorldMapHolder::~PlayResultWorldMapHolder() {
}

void PlayResultWorldMapHolder::reset() {
    mUnk14 = 0;
    mUnk18 = -1;
    mUnk1C = 0;
    fn_804C9710(mTable);
    *reinterpret_cast< s32* >(mTable + 0x400) = 0;
    mUnk224 = false;
    mUnk225 = false;
    mUnk228 = 5;
    mUnk22C = false;
    mUnk22D = false;
}

extern "C" void fn_804C96B0(PlayResultWorldMapHolder* pHolder, s32 value) {
    pHolder->mUnk18 = value;
}

extern "C" void fn_804C96C0(PlayResultWorldMapHolder* pHolder, s32 value) {
    pHolder->mUnk1C = value;
}

extern "C" void fn_804C96D0(u8* pTable, s32 a, s32 b) {
    pTable[a * 0x40 + b - 0x20] = 1;
}

extern "C" s32 fn_804C96F0(u8* pTable, s32 a, s32 b) {
    return pTable[a * 0x40 + b - 0x20];
}

extern "C" void fn_804C9710(u8* pTable) {
    for (s32 i = 0; i < 8; i++) {
        for (s32 j = 0; j < 0x40; j++) {
            pTable[i * 0x40 + j + 0x20] = 0;
        }
    }
}

extern "C" void fn_804C9750(PlayResultWorldMapHolder* pHolder, u8* pSrc) {
    fn_804C9760(pHolder->mTable, pSrc);
}

extern "C" void fn_804C9760(u8* pDst, u8* pSrc) {
    for (s32 i = 0; i < 0x400; i++) {
        pDst[i] = pSrc[i];
    }
    *reinterpret_cast< s32* >(pDst + 0x400) = *reinterpret_cast< s32* >(pSrc + 0x400);
}

extern "C" void fn_804C9790(PlayResultWorldMapHolder* pHolder, u8* pDst) {
    fn_804C9760(pDst, pHolder->mTable);
}

extern "C" void fn_804C97A0(PlayResultWorldMapHolder* pHolder) {
    *reinterpret_cast< s32* >(pHolder->mTable + 0x400) = 0;
}
