#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GameSequenceInGame;
class GameSequenceMarioFaceShipEvent;
class PlayResultMarioFaceShipHolder;

class GameSequenceMarioFaceShip : public ActorStateBase< GameSequenceInGame > {
public:
    GameSequenceMarioFaceShip(GameSequenceInGame* pHost);
    virtual ~GameSequenceMarioFaceShip();

    virtual void init();
    virtual void appear();

    PlayResultMarioFaceShipHolder* getPlayResultHolder() const;
    void exeStart();  // fn_804B4810

    /* 0x10 */ GameSequenceMarioFaceShipEvent* mEvent;
    /* 0x14 */ PlayResultMarioFaceShipHolder* mPlayResultHolder;
};
