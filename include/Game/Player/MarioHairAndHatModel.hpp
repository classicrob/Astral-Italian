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
};
