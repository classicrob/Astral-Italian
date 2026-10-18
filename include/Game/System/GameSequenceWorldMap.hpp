#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class GameSequenceInGame;
class PlayResultWorldMapHolder;
class NameObj;

namespace NrvGameSequenceWorldMap {
    class GameSequenceWorldMapNrvClose : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceWorldMapNrvClose sInstance;
    };
    class GameSequenceWorldMapNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceWorldMapNrvWait sInstance;
    };
    class GameSequenceWorldMapNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static GameSequenceWorldMapNrvEnd sInstance;
    };
};  // namespace NrvGameSequenceWorldMap

class GameSequenceWorldMap : public ActorStateBase< GameSequenceInGame > {
public:
    GameSequenceWorldMap(GameSequenceInGame* pHost);
    virtual ~GameSequenceWorldMap();

    virtual void init();
    virtual void appear();

    PlayResultWorldMapHolder* getPlayResultHolder() const;
    void exeClose();
    void exeWait();
    void exeEnd();

    /* 0x10 */ PlayResultWorldMapHolder* mPlayResultHolder;
    /* 0x14 */ NameObj* mNameObj;
};
