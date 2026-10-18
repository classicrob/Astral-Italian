#include "Game/Util/SystemUtil.hpp"
#include "Game/System/GameSequenceInGame.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceInStage.hpp"
#include "Game/System/GameSequenceMarioFaceShip.hpp"
#include "Game/System/GameSequenceWorldMap.hpp"
#include "Game/System/PlayResultInStageHolder.hpp"
#include "Game/System/PlayResultMarioFaceShipHolder.hpp"
#include "Game/System/PlayResultWorldMapHolder.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace NrvGameSequenceInGame {
    GameSequenceInGameNrvOpening GameSequenceInGameNrvOpening::sInstance ATTRIBUTE_WEAK;
    GameSequenceInGameNrvMarioFace GameSequenceInGameNrvMarioFace::sInstance ATTRIBUTE_WEAK;
    GameSequenceInGameNrvWorldMap GameSequenceInGameNrvWorldMap::sInstance ATTRIBUTE_WEAK;
    GameSequenceInGameNrvInStage GameSequenceInGameNrvInStage::sInstance ATTRIBUTE_WEAK;
    GameSequenceInGameNrvBeforeKoopaVs3 GameSequenceInGameNrvBeforeKoopaVs3::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGameSequenceInGame

GameSequenceInGame::GameSequenceInGame(GameSequence* pHost) : ActorStateBase< GameSequence >("GameSequenceInGame", pHost) {
    mMarioFace = nullptr;
    mWorldMap = nullptr;
    mInStage = nullptr;
}

GameSequenceInGame::~GameSequenceInGame() {
}

void GameSequenceInGame::init() {
    initNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvMarioFace), 4);

    mMarioFace = new GameSequenceMarioFaceShip(this);
    MR::initActorState(this, mMarioFace, GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvMarioFace), "GameSequenceMarioFaceShip");
    mWorldMap = new GameSequenceWorldMap(this);
    MR::initActorState(this, mWorldMap, GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvWorldMap), "GameSequenceWorldMap");
    mInStage = new GameSequenceInStage(this);
    MR::initActorState(this, mInStage, GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvInStage), "GameSequenceInStage");
}

void GameSequenceInGame::appear() {
    mIsDead = false;
    MR::onUseVFilter();
}

void GameSequenceInGame::setOpening() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvOpening));
}

void GameSequenceInGame::setMarioFace() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvMarioFace));
}

void GameSequenceInGame::setWorldMap() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvWorldMap));
}

void GameSequenceInGame::setInStage(const char* pStageName) {
    mInStage->getPlayResultHolder()->setStageName(pStageName);
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvInStage));
}

void GameSequenceInGame::setBeforeKoopaVs3() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvBeforeKoopaVs3));
}

bool GameSequenceInGame::isMarioFace() const {
    return isNerve(GET_NERVE_DIRECT(NrvGameSequenceInGame, GameSequenceInGameNrvMarioFace));
}

PlayResultMarioFaceShipHolder* GameSequenceInGame::getPlayResultMarioFaceShipHolder() {
    return mMarioFace->getPlayResultHolder();
}

PlayResultWorldMapHolder* GameSequenceInGame::getPlayResultWorldMapHolder() {
    return mWorldMap->getPlayResultHolder();
}

PlayResultInStageHolder* GameSequenceInGame::getPlayResultInStageHolder() {
    return mInStage->getPlayResultHolder();
}

void GameSequenceInGame::exeOpening() {
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        MR::openWipeFade(-1);
        if (!GameDataFunction::isOnGameEventFlag("\x83I\x81[\x83v\x83j\x83\x93\x83O\x8e\xc0\x8ds")) {
            MR::setPlayerSwingPermission(false);
            GameDataFunction::onGameEventFlag("\x83I\x81[\x83v\x83j\x83\x93\x83O\x8e\xc0\x8ds");
        }
    }
}
