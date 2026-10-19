#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class GameSequence;
class GameSequenceInStage;
class GameSequenceWorldMap;
class GameSequenceMarioFaceShip;
class PlayResultInStageHolder;
class PlayResultWorldMapHolder;
class PlayResultMarioFaceShipHolder;

namespace NrvGameSequenceInGame {
    class GameSequenceInGameNrvOpening : public Nerve {
    public:
        GameSequenceInGameNrvOpening();
        virtual void execute(Spine*) const;
        static GameSequenceInGameNrvOpening sInstance;
    };
    class GameSequenceInGameNrvMarioFace : public Nerve {
    public:
        GameSequenceInGameNrvMarioFace();
        virtual void execute(Spine*) const;
        static GameSequenceInGameNrvMarioFace sInstance;
    };
    class GameSequenceInGameNrvWorldMap : public Nerve {
    public:
        GameSequenceInGameNrvWorldMap();
        virtual void execute(Spine*) const;
        static GameSequenceInGameNrvWorldMap sInstance;
    };
    class GameSequenceInGameNrvInStage : public Nerve {
    public:
        GameSequenceInGameNrvInStage();
        virtual void execute(Spine*) const;
        static GameSequenceInGameNrvInStage sInstance;
    };
    class GameSequenceInGameNrvBeforeKoopaVs3 : public Nerve {
    public:
        GameSequenceInGameNrvBeforeKoopaVs3();
        virtual void execute(Spine*) const;
        static GameSequenceInGameNrvBeforeKoopaVs3 sInstance;
    };
};  // namespace NrvGameSequenceInGame

class GameSequenceInGame : public ActorStateBase< GameSequence > {
public:
    GameSequenceInGame(GameSequence* pHost);
    virtual ~GameSequenceInGame();

    virtual void init();
    virtual void appear();

    void setOpening();
    void setMarioFace();
    void setWorldMap();
    void setInStage(const char*);
    void setBeforeKoopaVs3();
    bool isMarioFace() const;
    PlayResultMarioFaceShipHolder* getPlayResultMarioFaceShipHolder();
    PlayResultWorldMapHolder* getPlayResultWorldMapHolder();
    PlayResultInStageHolder* getPlayResultInStageHolder();
    void exeOpening();

    /* 0x10 */ GameSequenceMarioFaceShip* mMarioFace;
    /* 0x14 */ GameSequenceWorldMap* mWorldMap;
    /* 0x18 */ GameSequenceInStage* mInStage;
};
