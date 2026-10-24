#include "Game/Util/Functor.hpp"
#include "Game/Util.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemStationedArchiveLoader.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/StationedFileInfo.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRExpHeap.hpp>

namespace {
    const char* sAudioResourceJob = "オーディオ常駐リソース読み込み";
    const char* sPlayerResourceJob = "プレイヤー常駐リソース読み込み";
    const char* sOtherResourceJob1st = "第１常駐リソース読み込み";
    const char* sOtherResourceJob2nd = "第２常駐リソース読み込み";
    const char* sChangePlayerResourceJob = "プレイヤーリソース入れ替え";

    NEW_NERVE(GameSystemStationedArchiveLoaderLoadAudioResource, GameSystemStationedArchiveLoader, LoadAudioResource);
    NEW_NERVE(GameSystemStationedArchiveLoaderLoadAudio1stWaveData, GameSystemStationedArchiveLoader, LoadAudio1stWaveData);
    NEW_NERVE(GameSystemStationedArchiveLoaderLoadStationedArchivePlayer, GameSystemStationedArchiveLoader, LoadStationedArchivePlayer);
    NEW_NERVE(GameSystemStationedArchiveLoaderLoadStationedArchiveOthers, GameSystemStationedArchiveLoader, LoadStationedArchiveOthers);
    NEW_NERVE(GameSystemStationedArchiveLoaderInitializeGameData, GameSystemStationedArchiveLoader, InitializeGameData);
    NEW_NERVE(GameSystemStationedArchiveLoaderInitialized, GameSystemStationedArchiveLoader, End);
    NEW_NERVE(GameSystemStationedArchiveLoaderLoadAudio2ndWaveData, GameSystemStationedArchiveLoader, LoadAudio2ndWaveData);
    NEW_NERVE(GameSystemStationedArchiveLoaderLoadStationedArchiveOthers2nd, GameSystemStationedArchiveLoader, LoadStationedArchiveOthers2nd);
    NEW_NERVE(GameSystemStationedArchiveLoaderInitializeStationedArchives, GameSystemStationedArchiveLoader, InitializeStationedArchives);
    NEW_NERVE(GameSystemStationedArchiveLoaderEnd, GameSystemStationedArchiveLoader, End);
    NEW_NERVE(GameSystemStationedArchiveLoaderSuspended, GameSystemStationedArchiveLoader, Suspended);
    NEW_NERVE(GameSystemStationedArchiveLoaderChangeArchivePlayer, GameSystemStationedArchiveLoader, ChangeArchivePlayer);
};  // namespace

GameSystemStationedArchiveLoader::GameSystemStationedArchiveLoader() : NerveExecutor("常駐データ初期化"), mHeapHolder(), _C() {
    initNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadAudioResource));
}

void GameSystemStationedArchiveLoader::update() {
    updateNerve();
}

bool GameSystemStationedArchiveLoader::isInitialized() const {
    return isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitialized));
}

bool GameSystemStationedArchiveLoader::isDone() const {
    return isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderEnd));
}

// NONMATCHING: regalloc; equivalent nested short-circuit boolean evaluation (FP order/grouping checked: no FP instructions).
bool GameSystemStationedArchiveLoader::isPreparedReset() const {
    bool result = true;
    bool initialized = true;
    if (!isInitialized() &&
        !isDone()) {
        initialized = false;
    }
    if (!initialized && !isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended))) {
        result = false;
    }
    return result;
}

bool GameSystemStationedArchiveLoader::isDataMario() const {
    return mHeapHolder->mIsDataMario;
}

void GameSystemStationedArchiveLoader::prepareReset() {
    if (_C) {
        return;
    }

    if (isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitialized)) || isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderEnd)) ||
        isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended)) ||
        isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderChangeArchivePlayer)) ||
        isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitializeGameData)) ||
        isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitializeStationedArchives))) {
        return;
    }

    _C = true;
}

bool GameSystemStationedArchiveLoader::requestChangeArchivePlayer(bool isDataMario) {
    isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitialized));
    if (isDataMario == mHeapHolder->mIsDataMario) {
        return false;
    }
    mHeapHolder->setIsDataMario(isDataMario);
    mHeapHolder->dispose();
    setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderChangeArchivePlayer));
    return true;
}

bool GameSystemStationedArchiveLoader::requestLoadStationedArchiveOthers() {
    if (isNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderEnd))) {
        return false;
    }
    setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadAudio2ndWaveData));
    return true;
}

void GameSystemStationedArchiveLoader::exeLoadAudioResource() {
    if (MR::isFirstStep(this)) {
        if (!tryAsyncExecuteIfNotSuspend(MR::Functor(this, &GameSystemStationedArchiveLoader::startToLoadAudioResource),
                                        sAudioResourceJob, 17)) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
            return;
        }
    } else if (trySuspend()) {
        MR::trySuspendAsyncExecuteThread(sAudioResourceJob);
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        return;
    }
    if (MR::tryEndFunctionAsyncExecute(sAudioResourceJob)) {
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadAudio1stWaveData));
    }
}

void GameSystemStationedArchiveLoader::exeLoadAudio1stWaveData() {
    if (MR::isFirstStep(this)) {
        GameSystemFunction::loadAudioStaticWaveData();
    }

    if (GameSystemFunction::isLoadedAudioStaticWaveData()) {
        GameSystemFunction::initAudioSystem();
        if (trySuspend()) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        } else {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadStationedArchivePlayer));
        }
    }
}

void GameSystemStationedArchiveLoader::exeLoadStationedArchivePlayer() {
    if (MR::isFirstStep(this)) {
        if (trySuspend()) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
            return;
        }
        mHeapHolder = new PlayerHeapHolder();
        MR::startFunctionAsyncExecute(MR::Functor(this, &GameSystemStationedArchiveLoader::startToLoadStationedArchivePlayer), 14,
                                     sPlayerResourceJob);
    } else if (trySuspend()) {
        MR::trySuspendAsyncExecuteThread(sPlayerResourceJob);
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        return;
    }
    if (MR::tryEndFunctionAsyncExecute(sPlayerResourceJob)) {
        createAndAddPlayerArchives(mHeapHolder->mIsDataMario);
        mHeapHolder->adjust();
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadStationedArchiveOthers));
    }
}

void GameSystemStationedArchiveLoader::exeLoadStationedArchiveOthers() {
    if (MR::isFirstStep(this)) {
        if (!tryAsyncExecuteIfNotSuspend(MR::Functor(this, &GameSystemStationedArchiveLoader::startToLoadStationedArchiveOthers),
                                        sOtherResourceJob1st, -1)) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
            return;
        }
    } else if (trySuspend()) {
        MR::trySuspendAsyncExecuteThread(sOtherResourceJob1st);
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        return;
    }
    if (MR::tryEndFunctionAsyncExecute(sOtherResourceJob1st)) {
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitializeGameData));
    }
}

void GameSystemStationedArchiveLoader::exeInitializeGameData() {
    createAndAddOtherArchives();
    GameSystemFunction::initAfterStationedResourceLoaded();
    SingletonHolder<HeapMemoryWatcher>::get()->setCurrentHeapToGameHeap();
    GameSystemFunction::setSceneNameObjHolderToNameObjRegister();
    MR::clearFileLoaderRequestFileInfo(false);
    setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitialized));
}

void GameSystemStationedArchiveLoader::exeLoadAudio2ndWaveData() {
    if (MR::isFirstStep(this)) {
        GameSystemFunction::loadAudio2ndWaveData();
    }
    if (GameSystemFunction::isLoadedAudio2ndWaveData()) {
        GameSystemFunction::initAudioSystem();
        GameSystemFunction::initAudioRhythm();
        if (trySuspend()) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        } else {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderLoadStationedArchiveOthers2nd));
        }
    }
}

void GameSystemStationedArchiveLoader::exeLoadStationedArchiveOthers2nd() {
    if (MR::isFirstStep(this)) {
        if (!tryAsyncExecuteIfNotSuspend(MR::Functor(this, &GameSystemStationedArchiveLoader::startToLoadStationedArchiveOthers2nd),
                                        sOtherResourceJob2nd, 17)) {
            setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
            return;
        }
    } else if (trySuspend()) {
        MR::trySuspendAsyncExecuteThread(sOtherResourceJob2nd);
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderSuspended));
        return;
    }
    if (MR::tryEndFunctionAsyncExecute(sOtherResourceJob2nd)) {
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderInitializeStationedArchives));
    }
}

void GameSystemStationedArchiveLoader::exeInitializeStationedArchives() {
    if (tryCreateAndAddOtherArchives()) {
        HeapMemoryWatcher* watcher = SingletonHolder<HeapMemoryWatcher>::get();
        watcher->adjustStationedHeapNapa();
        watcher->adjustStationedHeapGDDR3();
        MR::clearFileLoaderRequestFileInfo(false);
        GameSystemFunction::setResetOperationApplicationReset();
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderEnd));
    }
}

void GameSystemStationedArchiveLoader::exeEnd() {
}

void GameSystemStationedArchiveLoader::exeSuspended() {
}

void GameSystemStationedArchiveLoader::exeChangeArchivePlayer() {
    if (MR::isFirstStep(this)) {
        MR::startFunctionAsyncExecute(MR::Functor(this, &GameSystemStationedArchiveLoader::startToLoadStationedArchivePlayer), 14,
                                     sChangePlayerResourceJob);
    }
    if (MR::tryEndFunctionAsyncExecute(sChangePlayerResourceJob)) {
        createAndAddPlayerArchives(mHeapHolder->mIsDataMario);
        setNerve(GET_NERVE_ANON(GameSystemStationedArchiveLoaderEnd));
    }
}

bool GameSystemStationedArchiveLoader::trySuspend() {
    if (!_C) {
        return false;
    }

    _C = true;

    return true;
}

bool GameSystemStationedArchiveLoader::tryAsyncExecuteIfNotSuspend(const MR::FunctorBase& rFunctor, const char* pName, s32 priority) {
    if (trySuspend()) {
        return false;
    }

    if (priority < 0) {
        priority = 14;
    }
    MR::startFunctionAsyncExecute(rFunctor, priority, pName);

    return true;
}


void GameSystemStationedArchiveLoader::startToLoadAudioResource() {
    GameSystemFunction::loadAudioResource();
    {
        MR::CurrentHeapRestorer restorer(MR::getStationedHeapGDDR3());
        GameSystemFunction::loadAudioStageUseResource();
    }
    GameSystemFunction::createStationedGameSystemObjects();
}

void GameSystemStationedArchiveLoader::startToLoadStationedArchivePlayer() {
    StationedArchiveLoader::loadResourcesFromTable(*mHeapHolder->getCondition());
}

void GameSystemStationedArchiveLoader::startToLoadStationedArchiveOthers() {
    {
        ConditionIfIsNotPlayer1st cond;
        StationedArchiveLoader::loadResourcesFromTable(cond);
    }
    StationedArchiveLoader::loadScenarioData(MR::getStationedHeapGDDR3());
}

ConditionIfIsNotPlayer1st::ConditionIfIsNotPlayer1st() {
}

ConditionIfIsNotPlayer1st::~ConditionIfIsNotPlayer1st() {
}

void GameSystemStationedArchiveLoader::startToLoadStationedArchiveOthers2nd() {
    ConditionIfIsNotPlayer cond;
    StationedArchiveLoader::loadResourcesFromTable(cond);
}

ConditionIfIsNotPlayer::ConditionIfIsNotPlayer() {
}



void GameSystemStationedArchiveLoader::createAndAddPlayerArchives(bool isDataMario) {
    ConditionIsEqualType cond(isDataMario);
    StationedArchiveLoader::createAndAddResourcesFromTable(cond);

    ResourceHolder* pMarioAnimeHolder = MR::createAndAddResourceHolder("MarioAnime.arc");

    if (isDataMario) {
        pMarioAnimeHolder->newMaterialAnmBuffer(
            static_cast< J3DModelData* >(MR::createAndAddResourceHolder("Mario.arc")->mModelResTable->getRes("mario")));
    } else {
        pMarioAnimeHolder->newMaterialAnmBuffer(
            static_cast< J3DModelData* >(MR::createAndAddResourceHolder("Luigi.arc")->mModelResTable->getRes("luigi")));
    }
}


bool ConditionIsEqualType::isExecute(const MR::StationedFileInfo* pStationedFileInfo) const {
    return pStationedFileInfo->mLoadType == mLoadType;
}

void GameSystemStationedArchiveLoader::createAndAddOtherArchives() {
    ConditionIfIsNotPlayer1st cond;
    StationedArchiveLoader::createAndAddResourcesFromTable(cond);
}

bool GameSystemStationedArchiveLoader::tryCreateAndAddOtherArchives() {
    s32 start = getNerveStep() * 20;
    ConditionIfIsNotPlayer cond;
    return StationedArchiveLoader::createAndAddResourcesFromTable(cond, start, start + 20);
}

