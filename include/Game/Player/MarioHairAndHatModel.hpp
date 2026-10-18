#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BckCtrlData;

/// @brief Mario's hair/cap model (MarioActor + 0x1090). Only the members the Player units call are declared.
class MarioHairAndHatModel : public LiveActor {
public:
    MarioHairAndHatModel();

    // SMG2-only (original names unknown; named by address)
    void fn_803EC760(MtxPtr);
    void fn_803EC770(MtxPtr);
    void fn_803EC780();
    bool fn_803EC7F0(const char*);
    bool fn_803EC8A0(const char*, const BckCtrlData&);

    /* 0x90 */ MtxPtr _90;
    /* 0x94 */ MtxPtr _94;
    /* 0x98 */ void* _98;
    /* 0x9C */ bool _9C;
    /* 0x9D */ bool _9D;
};
