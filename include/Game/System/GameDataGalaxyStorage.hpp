#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"
#include "Game/Util/Array.hpp"

class BinaryDataContentAccessor;
class BinaryDataContentHeaderSerializer;
class GalaxyStatusAccessor;

// Per-scenario save record (SMG2, 0x0C bytes; header attributes "mMissNum", "mBestTime", "mFlag").
class GameDataSomeScenarioAccessor {
public:
    GameDataSomeScenarioAccessor();

    void resetAllData();
    bool testFlag(u32) const;
    void serialize(const BinaryDataContentAccessor&, u8*) const;
    s32 deserialize(const BinaryDataContentAccessor&, const u8*);

    /* 0x00 */ u8 mMissNum;
    /* 0x04 */ u32 mBestTime;
    /* 0x08 */ u8 mFlag;
};

// Per-galaxy save record (SMG2, 0x10 bytes). Its scenario records are a heap array of GameDataSomeScenarioAccessor.
class GameDataSomeGalaxyStorage {
public:
    GameDataSomeGalaxyStorage(const GalaxyStatusAccessor&);

    void resetAllData();
    GameDataSomeScenarioAccessor* getScenarioAccessor(s32) const;
    s32 getPowerStarNumOwned() const;
    s32 serialize(const BinaryDataContentAccessor&, const BinaryDataContentAccessor&, u8*) const;
    u32 deserialize(const BinaryDataContentAccessor&, const BinaryDataContentAccessor&, const u8*);

    /* 0x00 */ const char* mGalaxyName;
    /* 0x04 */ u8 mScenarioNum;
    /* 0x05 */ u8 mGalaxyState;
    /* 0x06 */ u8 mFlag;
    /* 0x08 */ GameDataSomeScenarioAccessor* mScenarioAccessors;
    /* 0x0C */ s32 mScenarioAccessorNum;
};

// NOTE/TODO: This dummy is here to ensure vtable alignment in GameDataAllGalaxyStorage.
// This double-inheritance with BinaryDataChunkBase behavior is repeated for a few
// different classes. What actually is the class that takes this role?
class Dummy1 {
public:
    Dummy1() {};
};

class GameDataAllGalaxyStorage : public Dummy1, public BinaryDataChunkBase {
public:
    GameDataAllGalaxyStorage();

    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    GameDataSomeGalaxyStorage* makeAccessor(const char*) const;
    s32 calcCurrentPowerStarNum() const;
    s32 calcNormalPowerStarNum() const;
    s32 getPowerStarNumOwned(const char*) const;
    s32 calcTicoCoinNum() const;
    void initHeaderSerializer();
    s32 findIndex(const char*) const;
    s32 findIndex(u16) const;

    s32 getGalaxyNum() const {
        return mSomeGalaxyStorages.size();
    }

    GameDataSomeGalaxyStorage* getGalaxyStorage(s32 index) const {
        return mSomeGalaxyStorages[index];
    }

    BinaryDataContentHeaderSerializer* getSerializer() const {
        return mHeaderSerializer;
    }

    /* 0x08 */ MR::Vector< MR::AssignableArray< GameDataSomeGalaxyStorage* > > mSomeGalaxyStorages;
    /* 0x14 */ BinaryDataContentHeaderSerializer* mHeaderSerializer;
    /* 0x18 */ BinaryDataContentHeaderSerializer* mScenarioHeaderSerializer;
};
