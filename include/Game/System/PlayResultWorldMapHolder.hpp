#pragma once

#include "Game/NameObj/NameObj.hpp"

class WorldMapResultRoute;

// SMG2: result data carried between the stage-clear sequence and the world map.
class PlayResultWorldMapHolder : public NameObj {
public:
    PlayResultWorldMapHolder();
    virtual ~PlayResultWorldMapHolder();

    void reset();
    void setTicoFatStarPieceId(s32);             // fn_804C96B0
    void setTicoFatStarPieceRequirement(s32);    // fn_804C96C0
    void setGalaxyFlag(s32, s32);                // fn_804C96D0
    bool isGalaxyFlag(s32, s32) const;           // fn_804C96F0
    void clearGalaxyFlags();                     // fn_804C9710
    void storeRoute(const WorldMapResultRoute*);  // fn_804C9750
    void loadRoute(WorldMapResultRoute*) const;   // fn_804C9790
    void clearRouteCount();                      // fn_804C97A0

    /* 0x14 */ s32 mCurrentNode;
    /* 0x18 */ s32 mTicoFatId;             // -1: none
    /* 0x1C */ s32 mTicoFatStarPieceNum;   // star pieces required by the Fat Tico
    /* 0x20 */ bool mGalaxyFlags[8][0x40]; // indexed [worldNo - 1][index]
    /* 0x220 */ WorldMapResultRoute* mRoute;
    /* 0x224 */ bool mIsAfterGameOver;
    /* 0x225 */ bool mIsAfterFileSelect;
    /* 0x228 */ s32 mEnterType;            // 0..5, 5 = default
    /* 0x22C */ bool mFlag22C;
    /* 0x22D */ bool mFlag22D;
};
