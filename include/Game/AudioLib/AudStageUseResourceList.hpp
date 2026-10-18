#pragma once

#include <revolution/types.h>

class JMapInfo;

// UseResource CSVs of the hub stages (FileSelect, MarioFaceShipGalaxy, PeachCastleGalaxy)
class AudStageUseResourceList {
public:
    AudStageUseResourceList();  // fn_8008A370
    void load();                // fn_8008A390

    /* 0x00 */ const JMapInfo* mParsers[5];
};
