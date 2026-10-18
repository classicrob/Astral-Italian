#pragma once

#include <revolution/types.h>

class IntermissionScene;
class JMapIdInfo;
class NameObjHolder;
class NameObjListExecutor;
class Nerve;
class ModelWipeScene;
class ScenarioDataParser;
class ScenarioSelectScene;
class Scene;
class SceneObjHolder;
class Spine;

class SceneControlInfo {
public:
    /// @brief Creates a new `SceneControlInfo`.
    SceneControlInfo();

    void setScene(const char*);
    void setStage(const char*);
    void setStartIdInfo(const JMapIdInfo&);

    /* 0x00 */ char mScene[64];  // SMG2: 64 (SMG1 32)
    /* 0x40 */ char mStage[64];
    /* 0x80 */ s32 mScenarioNo;
    /* 0x84 */ s32 mSelectedScenarioNo;
    /* 0x88 */ JMapIdInfo* mStartIdInfo;
};

enum SceneInitializeState {
    SceneInitializeState_NotInit,
    SceneInitializeState_Init,
    SceneInitializeState_PlacementPlayer,
    SceneInitializeState_PlacementHighPriority,
    SceneInitializeState_Placement,
    SceneInitializeState_AfterPlacement,
    SceneInitializeState_End,
};

class GameSystemSceneController {
public:
    /// @brief Creates a new `GameSystemSceneController`.
    GameSystemSceneController();
    bool isScenarioSelecting() const;  // SMG2 fn_80060320 callee (symbol isScenarioSelecting__25GameSystemSceneControllerCFv)

    void initAfterStationedResourceLoaded();
    void requestChangeScene();
    void checkRequestAndChangeScene();
    void initializeScene();
    void destroyScene();
    bool isStopSound() const;
    bool isReadyToStartScene() const;
    bool isFirstUpdateSceneNerveNormal() const;
    void startScene();
    void updateScene();
    void updateSceneDuringResetProcessing();
    void calcAnimScene();
    void drawScene();
    NameObjListExecutor* getNameObjListExecutor() const;
    SceneObjHolder* getSceneObjHolder() const;
    bool isExistSceneObjHolder() const;
    s32 getCurrentScenarioNo() const;
    s32 getCurrentSelectedScenarioNo() const;
    const char* getNextStageName() const;  // core (h1): fn_804BA850 (`addi r3,r3,0xCC`), not yet defined in GameSystemSceneController.cpp
    void setSceneInitializeState(SceneInitializeState);
    bool isSceneInitializeState(SceneInitializeState) const;
    void startScenarioSelectScene();
    void startScenarioSelectSceneBackground();
    void setCurrentScenarioNo(s32, s32);
    void resetCurrentScenarioNo();
    bool isScenarioDecided() const;
    void loadScenarioWaveData(bool);
    bool isLoadDoneScenarioWaveData() const;
    void exeNotInitialized();
    void exeNormal();
    void exeWaitDrawDoneScene();
    void exeDestroyScene();
    void exeChangeWaveBank();
    void exeInitializeScene();
    void exeInvalidateSystemWipe();
    void exeDestroyed();
    void exeReadyToStartScene();
    void prepareReset();
    bool isPreparedReset();
    void restartGameAfterResetting();
    bool isExistRequest() const;
    bool isSameAtNextSceneAndStage() const;
    void updateSceneControlInfo();
    Scene* getCurrentSceneForExecute() const;
    const Nerve* getNextNerveOnResetProcessing() const;
    void createSceneHeaps();
    bool tryCreateWorldMapHeap(const char*, const char*, s32);
    bool tryCreateFileCacheHeap(const char*, const char*, s32);
    void destroyHeaps();
    bool tryDestroyFileCacheHeapIfNotSameScene();
    bool tryDestroySceneHeap();
    bool tryDestroyWorldMapHeap();
    bool tryDestroyFileCacheHeap();
    bool tryDestroyFileCacheHeap(bool) NO_INLINE;
    void requestChangeNerve(const Nerve*);

    bool isCurrentNerve(const Nerve*) const;
    bool isNextNerve(const Nerve*) const;

    /* 0x00 */ SceneControlInfo mCurrSceneControlInfo;
    /* 0x8C */ SceneControlInfo mNextSceneControlInfo;  // SMG2 offsets (SceneControlInfo is 0x8C)
    /* 0x118 */ Spine* mSpine;
    /* 0x11C */ const Nerve* mNextNerve;
    /* 0x120 */ bool mIsResetProcessing;
    /* 0x121 */ bool mHasWorldMapHeap;  // set by createSceneHeaps(): the world map heap exists
    /* 0x124 */ ScenarioDataParser* mScenarioParser;
    /* 0x128 */ NameObjHolder* mObjHolder;
    /* 0x12C */ Scene* mScene;
    /* 0x130 */ SceneInitializeState mSceneInitializeState;
    /* 0x134 */ IntermissionScene* mIntermissionScene;
    /* 0x138 */ ScenarioSelectScene* mScenarioSelectScene;
    /* 0x13C */ ModelWipeScene* mModelWipeScene;
    /* 0x140 */ bool mIsFileCacheHeapMissing;  // stored by createSceneHeaps() from the tryCreateFileCacheHeap() result (inverted)
};
