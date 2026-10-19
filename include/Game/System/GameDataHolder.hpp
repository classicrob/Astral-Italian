#pragma once

#include <revolution/types.h>

class BinaryDataChunkHolder;
class GameDataAllGalaxyStorage;
class GameDataPlayerStatus;
class GameDataTicoFatStorage;
class GameDataWorldMapStorage;
class GameEventFlagStorage;
class GameEventValueChecker;

// SMG2 layout (0x2C bytes): the holder owns one save chunk per record and has no UserFile back pointer.
class GameDataHolder {
public:
    GameDataHolder();

    bool isOnGameEventFlag(const char*) const;
    void onGameEventFlag(const char*);
    void offGameEventFlag(const char*);
    u16 getGameEventValue(const char*) const;
    void setGameEventValue(const char*, u16);
    void incPlayerMissNum();
    s32 getPlayerMissNum() const;
    s32 calcCurrentPowerStarNum() const;
    s32 calcNormalPowerStarNum() const;
    bool hasGrandStar(int) const;
    s32 calcTicoCoinNum() const;
    s32 getStocketStarPieceNum() const;
    void addStockedStarPieceNum(int);
    void incCumulativePlayTime();
    s32 getCumulativePlayTime() const;
    void resetAllData();
    u32 makeFileBinary(u8*, u32);
    bool loadFromFileBinary(const char*, const u8*, u32);

    /* 0x00 */ GameEventFlagStorage* mEventFlagStorage;
    /* 0x04 */ GameEventValueChecker* mEventValueChecker;
    /* 0x08 */ GameDataPlayerStatus* mPlayerStatus;
    /* 0x0C */ GameDataAllGalaxyStorage* mAllGalaxyStorage;
    /* 0x10 */ GameDataTicoFatStorage* mTicoFatStorage;
    /* 0x14 */ GameDataWorldMapStorage* mWorldMapStorage;
    /* 0x18 */ BinaryDataChunkHolder* mChunkHolder;
    /* 0x1C */ char mName[16];
};
