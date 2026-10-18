#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GameSequenceInGame;
class PlayResultMarioFaceShipHolder;

class GameSequenceMarioFaceShip : public ActorStateBase< GameSequenceInGame > {
public:
    GameSequenceMarioFaceShip(GameSequenceInGame* pHost);
    virtual ~GameSequenceMarioFaceShip();

    PlayResultMarioFaceShipHolder* getPlayResultHolder() const;

    /* 0x10 */ PlayResultMarioFaceShipHolder* mPlayResultHolder;
};
