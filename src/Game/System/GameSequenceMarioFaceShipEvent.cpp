#include "Game/System/GameSequenceMarioFaceShipEvent.hpp"
#include "Game/System/GameSequenceMarioFaceShip.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/MarioFacePlanetSwitch.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/PlayResultMarioFaceShipHolder.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

// ---------------------------------------------------------------------------------------------
// GameSequenceMarioFaceShipEvent (a second translation unit in the target: own __sinit)
// ---------------------------------------------------------------------------------------------

static const char* sDemoNameSave = "セーブ";
static const char* sDemoNameBgmPrepare = "ＢＧＭプリペア";
static const char* sDemoNameEventStartWait = "イベント開始ウェイト";
static const char* sDemoNameGrandStarLaunch = "グランドスター後発進デモ";

static const char* sGrandStarReturnDemoNames[5] = {
    "グランドスター帰還後（ワールド１）", "グランドスター帰還後（ワールド２）", "グランドスター帰還後（ワールド３）",
    "グランドスター帰還後（ワールド４）", "グランドスター帰還後（ワールド５）",
};

namespace {
    class GameSequenceMarioFaceShipEventNrvGrandStarReturn : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvGrandStarReturn sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvResultDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvResultDemo sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvEventStartWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvEventStartWait sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvBgmPrepare : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvBgmPrepare sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvDemoEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvDemoEnd sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvTalkWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvTalkWait sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvWipeWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvWipeWait sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvSave : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvSave sInstance;
    };
    class GameSequenceMarioFaceShipEventNrvStartDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipEventNrvStartDemo sInstance;
    };

    GameSequenceMarioFaceShipEventNrvGrandStarReturn GameSequenceMarioFaceShipEventNrvGrandStarReturn::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvResultDemo GameSequenceMarioFaceShipEventNrvResultDemo::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvEventStartWait GameSequenceMarioFaceShipEventNrvEventStartWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvBgmPrepare GameSequenceMarioFaceShipEventNrvBgmPrepare::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvDemoEnd GameSequenceMarioFaceShipEventNrvDemoEnd::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvTalkWait GameSequenceMarioFaceShipEventNrvTalkWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvWipeWait GameSequenceMarioFaceShipEventNrvWipeWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvSave GameSequenceMarioFaceShipEventNrvSave::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipEventNrvStartDemo GameSequenceMarioFaceShipEventNrvStartDemo::sInstance ATTRIBUTE_WEAK;
};  // namespace

GameSequenceMarioFaceShipEvent::GameSequenceMarioFaceShipEvent(GameSequenceMarioFaceShip* pHost)
    : ActorStateBase< GameSequenceMarioFaceShip >("[シーケンス]マリオ顔惑星イベント", pHost) {
    mDemoStarter = nullptr;
    mCurrentIndex = -1;
    mDemoStarter = new NameObj("顔惑星デモ開始者");
}

MarioFaceShipEventList::MarioFaceShipEventList() {
    mNum = 0;
}

void GameSequenceMarioFaceShipEvent::init() {
    initNerve(&GameSequenceMarioFaceShipEventNrvResultDemo::sInstance, 0);
}

void GameSequenceMarioFaceShipEvent::appear() {
    bool isGrandStarVsKoopa = false;
    mIsDead = false;
    if (GameSequenceFunction::hasStageResultSequence()) {
        if (GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
            isGrandStarVsKoopa = true;
        }
    }

    if (MR::isPendingNormalEndingDemo() || MR::isPending120StarEndingDemo() ||
        (GameDataFunction::isOnGameEventFlagWorld1Entered() && GameSequenceFunction::isFaceShipRespondGameOver())) {
        MR::openWipeFade(0x1E);
    } else if (isGrandStarVsKoopa) {
        MR::forceCloseWipeCircle();
    } else {
        MR::openSystemWipeCircleAtPlayer(-1);
    }

    if (GameSequenceFunction::isFaceShipFlag1C()) {
        MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(mDemoStarter, sDemoNameSave);
        setNerve(&GameSequenceMarioFaceShipEventNrvWipeWait::sInstance);
        return;
    }
    if (!GameSequenceFunction::hasStageResultSequence()) {
        setNerve(&GameSequenceMarioFaceShipEventNrvStartDemo::sInstance);
        return;
    }
    if (isGrandStarVsKoopa) {
        setNerve(&GameSequenceMarioFaceShipEventNrvGrandStarReturn::sInstance);
        return;
    }
    setNerve(&GameSequenceMarioFaceShipEventNrvResultDemo::sInstance);
}

bool GameSequenceMarioFaceShipEvent::hasStartEvent() {
    if (GameSequenceFunction::hasStageResultSequence()) {
        return true;
    }
    if (GameSequenceFunction::isFaceShipFlag1C()) {
        return true;
    }
    MarioFaceShipEventList list;
    makeStartEventList(list);
    return list.mNum > 0;
}

bool GameSequenceMarioFaceShipEvent::isEmpty() {
    MarioFaceShipEventList list;
    makeResultEventList(list);
    return list.mNum > 0;
}

void GameSequenceMarioFaceShipEvent::makeResultEventList(MarioFaceShipEventList& rList) {
    rList.mNum = 0;
    for (s32 i = 0; i < MarioFaceShipEventDataHolder::calcAllTalkNum(); i++) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = nullptr;
        entry.mTalkIndex = i;
        rList.add(entry);
    }

    if (PostmanEvent::isCurrentTaskUnread()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "郵便屋さんイベント[タスク]";
        entry.mTalkIndex = -1;
        rList.add(entry);
    }
    if (PostmanEvent::isCurrentReplyEvent()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "郵便屋さんイベント[ロゼッタ]";
        entry.mTalkIndex = -1;
        rList.add(entry);
    }
    if (GameSequenceFunction::isClearedIslandFleetGalaxyFirstTimeAtResult()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "マリオ顔惑星変化";
        entry.mTalkIndex = -1;
        rList.add(entry);
        return;
    }
    if (GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
        addGrandStarEvents(rList, "VsKoopaJrLv1Galaxy", sGrandStarReturnDemoNames[0]);
        addGrandStarEvents(rList, "VsKoopaLv1Galaxy", sGrandStarReturnDemoNames[1]);
        addGrandStarEvents(rList, "KoopaJrLv2Galaxy", sGrandStarReturnDemoNames[2]);
        addGrandStarEvents(rList, "VsKoopaGravityGalaxy", sGrandStarReturnDemoNames[3]);
        addGrandStarEvents(rList, "VsKoopaJrLv3Galaxy", sGrandStarReturnDemoNames[4]);
        return;
    }

    bool isLuigiNoEnding = false;
    if (GameDataFunction::isLuigiPlayer() && !MR::isOnGameEventFlagNormalEnding()) {
        isLuigiNoEnding = true;
    }
    if (isLuigiNoEnding) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "でしゃばりルイージ終了";
        entry.mTalkIndex = -1;
        rList.add(entry);
    }
}

void GameSequenceMarioFaceShipEvent::makeStartEventList(MarioFaceShipEventList& rList) {
    rList.mNum = 0;
    if (MR::isPendingNormalEndingDemo()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "ノーマルエンディング後デモ";
        entry.mTalkIndex = -1;
        rList.add(entry);
    } else if (MR::isPending120StarEndingDemo()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "スター120個エンディング後デモ";
        entry.mTalkIndex = -1;
        rList.add(entry);
    } else if (GameSequenceFunction::isFaceShipAfterFileSelect()) {
        if (GameDataFunction::isOnGameEventFlagWorld1Entered()) {
            if (GameSequenceFunction::isFaceShipRespondGameOver()) {
                MarioFaceShipEventEntry entry;
                entry.mDemoName = "ゲームオーバー後のデモ";
                entry.mTalkIndex = -1;
                rList.add(entry);
            } else {
                MarioFaceShipEventEntry entry;
                entry.mDemoName = "ファイルセレクトからの開始";
                entry.mTalkIndex = -1;
                rList.add(entry);
            }
        } else {
            MarioFaceShipEventEntry entry;
            entry.mDemoName = "スター１個目直後のゲームスタート";
            entry.mTalkIndex = -1;
            rList.add(entry);
        }
    }

    if (GameSequenceFunction::isTicoFatStarPieceActive()) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "ハラペコスターピースチコ紹介";
        entry.mTalkIndex = -1;
        rList.add(entry);
    } else if (MR::getCurrentMarioStartId() == 1) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "ワールドマップからの開始";
        entry.mTalkIndex = -1;
        rList.add(entry);
    }
    if (MR::getCurrentMarioStartId() == 2) {
        MarioFaceShipEventEntry entry;
        entry.mDemoName = "チェンジルームからスタート";
        entry.mTalkIndex = -1;
        rList.add(entry);
    }
}

void GameSequenceMarioFaceShipEvent::addGrandStarEvents(MarioFaceShipEventList& rList, const char* pStageName, const char* pDemoName) {
    if (GameSequenceFunction::hasResultPowerStar()) {
        if (GameSequenceFunction::getClearedPowerStarId() == 1 && MR::isEqualString(GameSequenceFunction::getClearedStageName(), pStageName)) {
            MarioFaceShipEventEntry entry;
            entry.mDemoName = pDemoName;
            entry.mTalkIndex = -1;
            rList.add(entry);
            MarioFaceShipEventEntry launch;
            launch.mDemoName = sDemoNameGrandStarLaunch;
            launch.mTalkIndex = -1;
            rList.add(launch);
        }
    }
}

void GameSequenceMarioFaceShipEvent::tryRequestSaveDemo() {
    MarioFaceShipEventList list;
    makeResultEventList(list);
    if (list.mNum == 0) {
        MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(mDemoStarter, sDemoNameSave);
    }
}

void GameSequenceMarioFaceShipEvent::onResultDemoEnd() {
    for (s32 i = 0; i < MarioFaceShipEventDataHolder::calcAllTalkNum(); i++) {
        if (MR::isMarioFaceShipEventTalkValid(i)) {
            GameDataFunction::tryAddFaceShipEventNo(MarioFaceShipEventDataHolder::getTalkIndexByIndex(i));
        }
    }
    makeResultEventList(mEventList);
    mCurrentIndex = -1;
    if (mEventList.mNum > 0) {
        startNextEvent();
        return;
    }
    finishEvents();
}

bool GameSequenceMarioFaceShipEvent::startNextEvent() {
    bool res = false;
    mCurrentIndex++;
    bool isLaunch = false;
    if (mCurrentIndex < mEventList.mNum) {
        if (isCurrentEventLaunch()) {
            MR::tryStartDemo(mDemoStarter, sDemoNameBgmPrepare);
            setNerve(&GameSequenceMarioFaceShipEventNrvBgmPrepare::sInstance);
            isLaunch = true;
        } else {
            MR::tryStartDemo(mDemoStarter, sDemoNameEventStartWait);
            setNerve(&GameSequenceMarioFaceShipEventNrvEventStartWait::sInstance);
        }
        res = true;
    } else {
        if (GameSequenceFunction::isClearedIslandFleetGalaxyFirstTimeAtResult()) {
            MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(mDemoStarter, sDemoNameSave);
        } else {
            MR::tryStartDemoWithoutCinemaFrame(mDemoStarter, sDemoNameSave);
        }
        finishEvents();
    }
    MR::overlayWithPreviousScreen(3);
    MR::resetCameraMan();
    if (!isLaunch) {
        MR::setSeVolumeSetLevel(0xE, 0);
    }
    return res;
}

void GameSequenceMarioFaceShipEvent::endDemoAndStartNext(const char* pDemoName) {
    MR::endDemo(mDemoStarter, pDemoName);
    const char* pNextName = mEventList.mEntries[mCurrentIndex].mDemoName;
    s32 talkIndex = mEventList.mEntries[mCurrentIndex].mTalkIndex;
    MR::startMarioFaceShipBGM(true);
    if (hasTalkIndex()) {
        MarioFaceShipEventDataHolder::tryStartTalkByIndex(talkIndex);
        setNerve(&GameSequenceMarioFaceShipEventNrvTalkWait::sInstance);
        return;
    }
    MR::tryStartTimeKeepDemoMarioPuppetable(mDemoStarter, pNextName, nullptr);
    setNerve(&GameSequenceMarioFaceShipEventNrvDemoEnd::sInstance);
}

bool GameSequenceMarioFaceShipEvent::hasTalkIndex() const {
    return mEventList.mEntries[mCurrentIndex].mTalkIndex >= 0;
}

bool GameSequenceMarioFaceShipEvent::isCurrentEventLaunch() const {
    if (hasTalkIndex()) {
        return false;
    }
    return MR::isEqualString(mEventList.mEntries[mCurrentIndex].mDemoName, sDemoNameGrandStarLaunch);
}

void GameSequenceMarioFaceShipEvent::clearFlags() {
    if (GameSequenceFunction::isTicoFatStarPieceActive()) {
        GameSequenceFunction::clearTicoFatStarPiece();
    }
    if (GameSequenceFunction::isFaceShipAfterFileSelect()) {
        GameSequenceFunction::clearFaceShipAfterFileSelect();
    }
    if (GameSequenceFunction::isFaceShipRespondGameOver()) {
        GameSequenceFunction::clearFaceShipRespondGameOver();
    }
    if (GameSequenceFunction::isFaceShipFlag1C()) {
        GameSequenceFunction::clearFaceShipFlag1C();
    }
    if (MR::isPendingNormalEndingDemo()) {
        GameDataFunction::onGameEventFlag("ノーマルエンディング後デモ");
    }
    if (MR::isPending120StarEndingDemo()) {
        GameDataFunction::onGameEventFlag("スター120個エンディング後デモ");
    }
    if (GameSequenceFunction::hasStageResultSequence()) {
        if (MR::isCometExplainPendingWithStarGalaxy()) {
            GameDataFunction::onGameEventFlag("コメット解説");
        }
        if (MR::isWorld3NormalStarsGatheredForFaceShip()) {
            GameDataFunction::startLuigiIntrusion();
        }
    }
}

void GameSequenceMarioFaceShipEvent::finishEvents() {
    if (GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
        GameSequenceFunction::setWorldMapCurrentNode(0);
        u8 worldId = GameDataFunction::getCurrentWorldId();
        worldId++;
        GameSequenceFunction::requestChangeStageWorldMapMoveToWorld(worldId);
        clearFlags();
        kill();
        GameSequenceFunction::resetPlayResultInStage();
        return;
    }

    bool isLuigiNoEnding = false;
    if (GameDataFunction::isLuigiPlayer()) {
        if (!MR::isOnGameEventFlagNormalEnding()) {
            isLuigiNoEnding = true;
        }
    }
    if (isLuigiNoEnding) {
        GameSequenceFunction::setFaceShipFlag1C();
        GameSequenceFunction::prepareStageResetChangeRoomMario(3);
        return;
    }

    MR::setPlayerStateWait();
    if (MR::isSystemWipeActive() || (MR::isWipeOpen() && MR::isSystemWipeOpen())) {
        setNerve(&GameSequenceMarioFaceShipEventNrvWipeWait::sInstance);
        return;
    }
    setNerve(&GameSequenceMarioFaceShipEventNrvSave::sInstance);
}

void GameSequenceMarioFaceShipEvent::exeGrandStarReturn() {
    if (MR::isFirstStep(this)) {
        MR::startGrandStarReturnBGM();
        MR::tryStartDemo(mDemoStarter, sDemoNameBgmPrepare);
    }
    if (MR::isPreparedStageBgm()) {
        MR::openWipeCircle(-1);
        setNerve(&GameSequenceMarioFaceShipEventNrvResultDemo::sInstance);
    }
}

void GameSequenceMarioFaceShipEvent::exeResultDemo() {
    if (MR::isFirstStep(this)) {
        const char* pStageName = GameSequenceFunction::getClearedStageName();
        const char* pDemoName;
        if (GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
            MR::endDemo(mDemoStarter, sDemoNameBgmPrepare);
            if (MR::isEqualString(pStageName, "VsKoopaJrLv1Galaxy")) {
                pDemoName = "グランドスター帰還";
            } else {
                pDemoName = "グランドスター帰還[ワールド２以降]";
            }
        } else if (GameDataConst::isGrandStar(pStageName, GameSequenceFunction::getClearedPowerStarId())) {
            pDemoName = "グランドスター帰還[２回目以降]";
        } else if (GameSequenceFunction::isClearedIslandFleetGalaxyFirstTimeAtResult()) {
            pDemoName = "パワースター帰還[初回]";
        } else if (MR::hasMarioFaceShipItemEventAtResult()) {
            pDemoName = "パワースター帰還[アイテム惑星]";
        } else {
            if (MarioFaceShipEventDataHolder::hasEventAtResultStatic()) {
                pDemoName = "パワースター帰還[NPC紹介]";
            } else {
                pDemoName = "パワースター帰還";
            }
        }
        MR::tryStartTimeKeepDemoMarioPuppetable(mDemoStarter, pDemoName, nullptr);
        tryRequestSaveDemo();
    }
    if (!MR::isTimeKeepDemoActive()) {
        onResultDemoEnd();
    }
}

static bool isGrandStarReturnDemoName(const char* pName) {
    for (u32 i = 0; i < 5; i++) {
        if (MR::isEqualString(pName, sGrandStarReturnDemoNames[i])) {
            return true;
        }
    }
    return false;
}

void GameSequenceMarioFaceShipEvent::exeEventStartWait() {
    if (!hasTalkIndex()) {
        if (isGrandStarReturnDemoName(mEventList.mEntries[mCurrentIndex].mDemoName)) {
            MR::setSeVolumeSetLevel(0xA, 0x14);
        }
    }
    if (MR::isStep(this, 0x3C)) {
        endDemoAndStartNext(sDemoNameEventStartWait);
    }
}

void GameSequenceMarioFaceShipEvent::exeBgmPrepare() {
    if (MR::isFirstStep(this)) {
        MR::startFirstAstroStageBgm();
    }
    if (MR::isPreparedStageBgm() && MR::isGreaterStep(this, 0x3C)) {
        endDemoAndStartNext(sDemoNameBgmPrepare);
    }
}

void GameSequenceMarioFaceShipEvent::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::forceToFrameCinemaFrame();
        if (!GameSequenceFunction::isClearedIslandFleetGalaxyFirstTimeAtResult()) {
            MR::openWipeFade(0x1E);
        }
    }
    const char* pEndName = "終了";
    if (MR::isDemoPartActive(pEndName)) {
        s32 closeStep = MR::getDemoPartTotalStep(pEndName) - 0x1E;
        if (MR::isDemoPartStep(pEndName, closeStep)) {
            MR::closeWipeFade(0x1E);
        }
    }
    if (!MR::isTimeKeepDemoActive()) {
        startNextEvent();
    }
}

void GameSequenceMarioFaceShipEvent::exeTalkWait() {
    if (MR::isFirstStep(this)) {
        MR::forceToFrameCinemaFrame();
    }
    if (MarioFaceShipEventDataHolder::isTalkEndByIndex(mEventList.mEntries[mCurrentIndex].mTalkIndex)) {
        startNextEvent();
    }
}

void GameSequenceMarioFaceShipEvent::exeWipeWait() {
    if (MR::isWipeOpen() && MR::isSystemWipeOpen() && MR::isGreaterStep(this, 0x1E)) {
        setNerve(&GameSequenceMarioFaceShipEventNrvSave::sInstance);
    }
}

void GameSequenceMarioFaceShipEvent::exeSave() {
    if (MR::isFirstStep(this)) {
        clearFlags();
        GameSequenceFunction::startGameDataSaveSequence(true, false, false);
    }
    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        MR::endDemo(mDemoStarter, sDemoNameSave);
        if (MR::isWipeBlank()) {
            MR::openWipeFade(0x3C);
        }
        kill();
    }
}

void GameSequenceMarioFaceShipEvent::exeStartDemo() {
    if (MR::isFirstStep(this)) {
        bool isStartFlow = false;
        MarioFaceShipEventList list;
        makeStartEventList(list);
        const char* pName = list.mEntries[0].mDemoName;
        if (MR::isEqualString(pName, "ファイルセレクトからの開始") || MR::isEqualString(pName, "ワールドマップからの開始") ||
            MR::isEqualString(pName, "チェンジルームからスタート")) {
            isStartFlow = true;
        }
        if (isStartFlow) {
            if (MR::isEqualString(pName, "チェンジルームからスタート") == false) {
                MR::tryStartDemoMarioPuppetableSystemFlags(mDemoStarter, pName);
            } else {
                MR::tryStartDemoSystemFlags111(mDemoStarter, pName);
            }
        } else {
            MR::tryStartTimeKeepDemoMarioPuppetable(mDemoStarter, pName, nullptr);
        }
        MR::startMarioFaceShipBGM(true);
    }
    if (!MR::isTimeKeepDemoActive()) {
        if (MR::isWipeBlank()) {
            MR::openWipeFade(0x1E);
        }
        clearFlags();
        kill();
    }
}

void MarioFaceShipEventList::add(const MarioFaceShipEventEntry& rEntry) {
    mEntries[mNum].mDemoName = rEntry.mDemoName;
    mEntries[mNum].mTalkIndex = rEntry.mTalkIndex;
    mNum++;
}

GameSequenceMarioFaceShipEvent::~GameSequenceMarioFaceShipEvent() {
}

namespace {
    void GameSequenceMarioFaceShipEventNrvStartDemo::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeStartDemo();
    }
    void GameSequenceMarioFaceShipEventNrvSave::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeSave();
    }
    void GameSequenceMarioFaceShipEventNrvWipeWait::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeWipeWait();
    }
    void GameSequenceMarioFaceShipEventNrvTalkWait::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeTalkWait();
    }
    void GameSequenceMarioFaceShipEventNrvDemoEnd::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeDemoEnd();
    }
    void GameSequenceMarioFaceShipEventNrvBgmPrepare::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeBgmPrepare();
    }
    void GameSequenceMarioFaceShipEventNrvEventStartWait::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeEventStartWait();
    }
    void GameSequenceMarioFaceShipEventNrvResultDemo::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeResultDemo();
    }
    void GameSequenceMarioFaceShipEventNrvGrandStarReturn::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceMarioFaceShipEvent* >(pSpine->mExecutor)->exeGrandStarReturn();
    }
};  // namespace

namespace MR {
    void setPlayerStateWait();  // PlayerUtil.hpp (not included: it is broken in the shared tree at the moment)
}
