#pragma once

class GameDataHolder;
class GameSequence;
class GameSystemDataHolder;
class NWC24Messenger;
class SaveDataHandleSequence;
class StarPointerOnOffController;
class GalaxyDataTableAccessor;

// SMG2: the SMG1 GameSequenceProgress / GameDataTemporaryInGalaxy are gone; GameSequence owns the scene flow.
class GameSequenceDirector {
public:
    GameSequenceDirector(GameSystemDataHolder* pDataHolder);

    void initAfterResourceLoaded();
    void update();
    void draw() const;
    bool isInitializedGameDataHolder() const;

    /* 0x00 */ GameSequence* mGameSequence;
    /* 0x04 */ GalaxyDataTableAccessor* mGalaxyDataTableAccessor;
    /* 0x08 */ SaveDataHandleSequence* mSaveDataHandleSequence;
    /* 0x0C */ StarPointerOnOffController* mStarPointerOnOffController;
    /* 0x10 */ NWC24Messenger* mNWC24Messenger;
};
