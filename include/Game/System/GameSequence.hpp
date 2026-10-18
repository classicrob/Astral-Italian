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

// SMG2 hash-keyed 0xC entry table (fn_804AD740 / fn_804AD660 / fn_804AD680 / fn_804AD6D0)
struct GameSequenceNameEntry {
    /* 0x0 */ u16 mHash;
    /* 0x4 */ s32 mValue0;
    /* 0x8 */ s32 mValue1;
};

class GameSequenceNameEntryTable {
public:
    GameSequenceNameEntryTable(s32 count);

    void clear();               // fn_804AD7C0
    void registerZoneNames();   // fn_804AD8E0

    /* 0x0 */ GameSequenceNameEntry* mEntries;
    /* 0x4 */ s32 mCount;
    /* 0x8 */ s32 _8;
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
    /* 0x18 */ s32 mMessageIndex;
    /* 0x1C */ bool mIsNormalEnding;
    /* 0x1D */ bool mIs120PowerStarsEnding;
};
