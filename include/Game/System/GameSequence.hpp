#pragma once

#include "Game/System/NerveExecutor.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class GameSequence;
class GameSequenceEnding;
class GameSequenceSystemStart;
class GameSequenceTitle;
class GameSequenceInGame;
class NameObj;
class InformationMessage;

// SMG2 entry of the "already done" flag table: 15-bit name hash + done bit, zone id, link id (0xC).
class GameSequenceNameEntry {
public:
    GameSequenceNameEntry();

    void set(const char* pName, s32 zoneId, s32 linkId);
    bool operator==(const GameSequenceNameEntry& rOther) const;
    void onDone();

    bool isDone() const {
        return mValue >> 15 & 1;
    }

    /* 0x0 */ u16 mValue;
    /* 0x4 */ s32 mZoneId;
    /* 0x8 */ s32 mLinkId;
};

class JMapInfoIter;

class GameSequenceNameEntryTable {
public:
    GameSequenceNameEntryTable(s32 count);

    void clear();
    bool setupAlreadyDoneEntry(int* pOutIndex, const char* pName, const JMapInfoIter& rIter);
    void onAlreadyDoneEntry(const char* pName);
    void updateAlreadyDoneEntry(s32 index);
    bool isOnAlreadyDoneEntry(const char* pName);
    void registerZoneNames();
    bool registerEntry(int* pOutIndex, const char* pName, s32 zoneId, s32 linkId);
    GameSequenceNameEntry* find(const char* pName);
    GameSequenceNameEntry* getEntry(s32 index);

    /* 0x0 */ GameSequenceNameEntry* mEntries;
    /* 0x4 */ s32 mCapacity;
    /* 0x8 */ s32 mCount;
};

namespace NrvGameSequence {
    class GameSequenceNrvSystemStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceNrvSystemStart sInstance;
    };
    class GameSequenceNrvTitle : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceNrvTitle sInstance;
    };
    class GameSequenceNrvInGame : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceNrvInGame sInstance;
    };
    class GameSequenceNrvEnding : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceNrvEnding sInstance;
    };
};  // namespace NrvGameSequence

class GameSequence : public NerveExecutor {
public:
    GameSequence();
    virtual ~GameSequence();

    void update();
    void exeMain();
    void cleanupInGame();
    void setTitle();
    void setInGame();
    void setEnding();
    void draw() const;
    void drawPlayerMissLeft() const;
    void updatePlayTime();

    /* 0x08 */ GameSequenceSystemStart* mSystemStart;
    /* 0x0C */ GameSequenceTitle* mTitle;
    /* 0x10 */ GameSequenceInGame* mInGame;
    /* 0x14 */ GameSequenceEnding* mEnding;
    /* 0x18 */ GameSequenceNameEntryTable* mNameEntryTable;
    /* 0x1C */ bool mIsReset;
    /* 0x1D */ bool mIsWipeStarted;
};

class GameSequenceEnding : public ActorStateBase< GameSequence > {
public:
    GameSequenceEnding(GameSequence* pHost);
    virtual ~GameSequenceEnding();
    virtual void init();
    virtual void appear();

    void exeInit();
    void exeWipeClose();
    void exeDemoMessage();
    void exeMessage();
    void exeMessageWait();
    void exeMessageNext();
    void exeSaveSequence();
    void startInit();
    void startWipeClose();
    void startDemoMessage();
    void requestSaveDemo();
    void initAfterResourceLoaded();
    void draw() const;
    void killMessage();
    bool updateMessage();

    /* 0x10 */ NameObj* mNameObj;
    /* 0x14 */ InformationMessage* mMessage;
    /* 0x18 */ u32 mMessageIndex;
    /* 0x1C */ bool mIsNormalEnding;
    /* 0x1D */ bool mIs120PowerStarsEnding;
};
