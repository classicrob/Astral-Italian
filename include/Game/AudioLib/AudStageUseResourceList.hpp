#pragma once

#include <revolution/types.h>

class JMapInfo;

// UseResource CSVs of the hub stages (FileSelect, MarioFaceShipGalaxy, PeachCastleGalaxy)
class AudStageUseResourceList {
public:
    AudStageUseResourceList();  // fn_8008A370
    void loadIslandFleetClear();
    void loadGalaxyClear();
    void loadEndingClear();
    void loadKoopaClear();
    static const char* getFileSelectResourceName();
    static const char* getFaceShipResourceName();
    static const char* getPeachCastleResourceName();
    void loadStaticSound();
    void load();                // fn_8008A390

    /* 0x00 */ const JMapInfo* mParsers[5];
};
