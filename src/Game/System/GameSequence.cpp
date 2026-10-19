#include "Game/Util/SystemUtil.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/NWC24/WiiMessageBoardMailSenderState.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/GameSequenceInStage.hpp"
#include "Game/Screen/PlayerMissLeft.hpp"
#include "Game/System/GameSequenceSystemStart.hpp"
#include "Game/System/GameSequenceTitle.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvGameSequence {
    void GameSequenceNrvSystemStart::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
    void GameSequenceNrvTitle::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
    void GameSequenceNrvInGame::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
    void GameSequenceNrvEnding::execute(Spine* pSpine) const {
        if (MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor))) {
            GameSequenceFunction::notifyToGameSequenceProgressToEndScene();
        }
    }

    GameSequenceNrvSystemStart GameSequenceNrvSystemStart::sInstance ATTRIBUTE_WEAK;
    GameSequenceNrvTitle GameSequenceNrvTitle::sInstance ATTRIBUTE_WEAK;
    GameSequenceNrvInGame GameSequenceNrvInGame::sInstance ATTRIBUTE_WEAK;
    GameSequenceNrvEnding GameSequenceNrvEnding::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGameSequence

namespace {
    NEW_NERVE(GameSequenceNrvMain, GameSequence, Main);
};  // namespace

GameSequence::GameSequence() : NerveExecutor("\x83Q\x81[\x83\x80\x83V\x81[\x83P\x83\x93\x83X") {
    mSystemStart = nullptr;
    mTitle = nullptr;
    mInGame = nullptr;
    mEnding = nullptr;
    mNameEntryTable = nullptr;
    mIsReset = false;
    mIsWipeStarted = false;

    initNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvSystemStart), 8);

    mSystemStart = new GameSequenceSystemStart(this);
    MR::initActorState(this, mSystemStart, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvSystemStart), "SystemStart");
    mTitle = new GameSequenceTitle(this);
    MR::initActorState(this, mTitle, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvTitle), "Title");
    mInGame = new GameSequenceInGame(this);
    MR::initActorState(this, mInGame, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame), "InGame");
    mEnding = new GameSequenceEnding(this);
    MR::initActorState(this, mEnding, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding), "Ending");
    mNameEntryTable = new GameSequenceNameEntryTable(0x60);
}

GameSequence::~GameSequence() {
}

void GameSequence::update() {
    mIsWipeStarted = false;
    if (mIsReset && GameSequenceFunction::isReadyToStartScene() != 0) {
        GameSequenceFunction::startScene();
        mIsWipeStarted = true;
    }
    if (!isNerve(GET_NERVE_ANON(GameSequenceNrvMain)) && GameSystemFunction::isResetProcessing()) {
        setNerve(GET_NERVE_ANON(GameSequenceNrvMain));
    }
    updateNerve();
    updatePlayTime();
}

void GameSequence::setTitle() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvTitle));
}

void GameSequence::setInGame() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame));
}

void GameSequence::setEnding() {
    setNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding));
}

void GameSequence::cleanupInGame() {
    mInGame->mInStage->killPlayerMissLeft();
    mEnding->killMessage();
}

void GameSequence::exeMain() {
    if (MR::isFirstStep(this)) {
        mIsReset = false;
        cleanupInGame();
    }
    if (!GameSystemFunction::isResetProcessing()) {
        GameSequenceFunction::notifyToGameSequenceProgressToEndScene();
    }
}

void GameSequence::draw() const {
    if (isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding))) {
        mEnding->draw();
    }
}

void GameSequence::drawPlayerMissLeft() const {
    if (isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame))) {
        LayoutActor* pActor = mInGame->mInStage->mPlayerMissLeft;
        if (pActor != nullptr && !MR::isDead(pActor)) {
            pActor->draw();
        }
    }
}

void GameSequence::updatePlayTime() {
    if (isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame)) || isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding))) {
        if (!MR::isInitializeStateEnd() || !GameSceneFunction::isExecPauseMenu()) {
            GameDataFunction::incCumulativePlayTime();
        }
    }
}

