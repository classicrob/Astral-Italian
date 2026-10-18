#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class GameSequenceInGame;
class PlayResultInStageHolder;
class PlayerMissLeft;

namespace NrvGameSequenceInStage {
    class GameSequenceInStageNrvMissLeft : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceInStageNrvMissLeft sInstance;
    };
    class GameSequenceInStageNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceInStageNrvWait sInstance;
    };
    class GameSequenceInStageNrvStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceInStageNrvStart sInstance;
    };
    class GameSequenceInStageNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceInStageNrvEnd sInstance;
    };
};  // namespace NrvGameSequenceInStage

class GameSequenceInStage : public ActorStateBase< GameSequenceInGame > {
public:
    GameSequenceInStage(GameSequenceInGame* pHost);
    virtual ~GameSequenceInStage();

    virtual void init();
    virtual void appear();

    PlayResultInStageHolder* getPlayResultHolder() const;
    void initPlayerMissLeft();
    void exeMissLeft();
    void exeWait();
    void exeStart();
    void exeEnd();
    void killPlayerMissLeft();  // was fn_804B4360

    /* 0x10 */ PlayResultInStageHolder* mPlayResultHolder;
    /* 0x14 */ PlayerMissLeft* mPlayerMissLeft;
    /* 0x18 */ bool mIsRetry;
    /* 0x19 */ bool mIsKillReady;
};
