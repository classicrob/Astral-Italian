#include "Game/System/GameSequenceDirector.hpp"
#include "Game/NWC24/NWC24Messenger.hpp"
#include "Game/System/GalaxyDataTableAccessor.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/StarPointerOnOffController.hpp"

GameSequenceDirector::GameSequenceDirector(GameSystemDataHolder* pDataHolder)
    : mGameSequence(nullptr), mGalaxyDataTableAccessor(nullptr), mSaveDataHandleSequence(nullptr), mStarPointerOnOffController(nullptr),
      mNWC24Messenger(nullptr) {
    mGameSequence = new GameSequence();
    mGalaxyDataTableAccessor = new GalaxyDataTableAccessor(pDataHolder);
    mSaveDataHandleSequence = new SaveDataHandleSequence();
    mStarPointerOnOffController = new StarPointerOnOffController();
    mNWC24Messenger = new NWC24Messenger("NWC24Messenger");
}

void GameSequenceDirector::initAfterResourceLoaded() {
    GameSequenceFunction::initAfterResourceLoaded();
    mSaveDataHandleSequence->initAfterResourceLoaded();
    mNWC24Messenger->initAfterResourceLoaded();
}

void GameSequenceDirector::update() {
    mGameSequence->update();
    mSaveDataHandleSequence->update();
    mStarPointerOnOffController->update();
    mNWC24Messenger->movement();
    mNWC24Messenger->calcAnim();
}

void GameSequenceDirector::draw() const {
    mSaveDataHandleSequence->draw();
    mGameSequence->draw();
    mNWC24Messenger->draw();
}

bool GameSequenceDirector::isInitializedGameDataHolder() const {
    return mSaveDataHandleSequence->isInitializedGameDataHolder();
}
