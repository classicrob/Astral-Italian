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

// Unnamed SMG2 helpers (declared locally until their owners name them)
extern "C" {
    void* fn_8001FD30(void*, const char*);
    void fn_8001FEB0(void*, void*, const char*, const Nerve*, s32);
    void fn_80060730();
    void fn_80060740();
    s32 fn_804D8620();
    s32 fn_804D8630();
    s32 fn_804D8650();
    void fn_804D71B0();
    void fn_804D7240();
    void fn_804D85D0();
    void* fn_80500DD0();
    s32 fn_80500D80();
    void fn_804B4360(void*, void*);
};

void fn_804B31D0(GameSequence* pSequence);
void fn_804B3770(GameSequenceEnding* pEnding);
void fn_804B36F0(GameSequenceEnding* pEnding);
bool fn_804B3790(GameSequenceEnding* pEnding);

// Calls the virtual function stored at byte offset `offset` of the object's vtable.
static void callVirtual(void* pObj, s32 offset) {
    typedef void (*VFunc)(void*);
    VFunc func = reinterpret_cast< VFunc >((*reinterpret_cast< void*** >(pObj))[offset / 4]);
    func(pObj);
}

extern const char* lbl_807D03C0;
extern const char* lbl_807D03C4;
extern const char* lbl_807D03C8;

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

    class GameSequenceEndingNrvBlank : public Nerve {
    public:
        virtual void execute(Spine*) const {
            fn_804D71B0();
        }
        static GameSequenceEndingNrvBlank sInstance;
    };
    GameSequenceEndingNrvBlank GameSequenceEndingNrvBlank::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(GameSequenceEndingNrvInit, GameSequenceEnding, Init);
    NEW_NERVE(GameSequenceEndingNrvWipeClose, GameSequenceEnding, WipeClose);
    NEW_NERVE(GameSequenceEndingNrvDemoMessage, GameSequenceEnding, DemoMessage);
    NEW_NERVE(GameSequenceEndingNrvMessage, GameSequenceEnding, Message);
    NEW_NERVE(GameSequenceEndingNrvMessageWait, GameSequenceEnding, MessageWait);
    NEW_NERVE(GameSequenceEndingNrvMessageNext, GameSequenceEnding, MessageNext);
    NEW_NERVE(GameSequenceEndingNrvSaveSequence, GameSequenceEnding, SaveSequence);
};  // namespace

GameSequenceNameEntryTable::GameSequenceNameEntryTable(s32 count) : mEntries(nullptr), mCount(0), _8(0) {
    mEntries = new GameSequenceNameEntry[count];
    mCount = count;
}

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
    MR::initActorState(this, mSystemStart, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvSystemStart), "GameSequenceSystemStart");
    mTitle = new GameSequenceTitle(this);
    MR::initActorState(this, mTitle, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvTitle), "GameSequenceTitle");
    mInGame = new GameSequenceInGame(this);
    MR::initActorState(this, mInGame, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame), "GameSequenceInGame");
    mEnding = new GameSequenceEnding(this);
    MR::initActorState(this, mEnding, GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding), "GameSequenceEnding");
    mNameEntryTable = new GameSequenceNameEntryTable(0x60);
}

GameSequence::~GameSequence() {
}

void GameSequence::update() {
    mIsWipeStarted = false;
    if (mIsReset && fn_804D8620() != 0) {
        fn_804D85D0();
        mIsWipeStarted = true;
    }
    if (!isNerve(GET_NERVE_ANON(GameSequenceNrvMain)) && GameSystemFunction::isResetProcessing()) {
        setNerve(GET_NERVE_ANON(GameSequenceNrvMain));
    }
    updateNerve();
    fn_804B31D0(this);
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
    fn_804B4360(mInGame->mInStage, mInGame);
    fn_804B3770(mEnding);
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

void fn_804B3120(GameSequence* pSequence) {
    if (pSequence->isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding))) {
        fn_804B36F0(pSequence->mEnding);
    }
}

void fn_804B3160(GameSequence* pSequence) {
    if (pSequence->isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame))) {
        LayoutActor* pActor = pSequence->mInGame->mInStage->mPlayerMissLeft;
        if (pActor != nullptr && !MR::isDead(pActor)) {
            pActor->draw();
        }
    }
}

void fn_804B31D0(GameSequence* pSequence) {
    if (pSequence->isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvInGame)) || pSequence->isNerve(GET_NERVE_DIRECT(NrvGameSequence, GameSequenceNrvEnding))) {
        if (!MR::isInitializeStateEnd() || !GameSceneFunction::isExecPauseMenu()) {
            GameDataFunction::incCumulativePlayTime();
        }
    }
}

// Ending helpers

void fn_804B3600(NerveExecutor* pExecutor) {
    pExecutor->setNerve(GET_NERVE_ANON(GameSequenceEndingNrvInit));
}

void fn_804B3610(NerveExecutor* pExecutor) {
    pExecutor->setNerve(GET_NERVE_ANON(GameSequenceEndingNrvWipeClose));
}

void fn_804B3620(NerveExecutor* pExecutor) {
    pExecutor->setNerve(GET_NERVE_ANON(GameSequenceEndingNrvDemoMessage));
}

void fn_804B3630(GameSequenceEnding* pEnding) {
    GameSystemFunction::setPermissionToCheckWiiRemoteConnect(true);
    const Nerve* pNerve = GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence);
    if (pEnding->mIsNormalEnding) {
        pNerve = GET_NERVE_ANON(GameSequenceEndingNrvMessage);
    }
    fn_8001FEB0(pEnding, pEnding->mNameObj, "\x83Z\x81[\x83u", pNerve, 0);
}

void fn_804B3690(GameSequenceEnding* pEnding) {
    pEnding->mMessage = new InformationMessage(false);
    pEnding->mMessage->initWithoutIter();
    *reinterpret_cast< s32* >(reinterpret_cast< u8* >(pEnding->mMessage) + 0x30) = 1;  // mWindowType (private in the header)
}

void fn_804B36F0(GameSequenceEnding* pEnding) {
    if (pEnding->mMessage != nullptr && !MR::isDead(pEnding->mMessage)) {
        pEnding->mMessage->draw();
    }
    if (pEnding->isNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessageWait))) {
        callVirtual(fn_80500DD0(), 0x18);
    }
}

void fn_804B3770(GameSequenceEnding* pEnding) {
    if (pEnding->mMessage != nullptr) {
        pEnding->mMessage->kill();
    }
}

bool fn_804B3790(GameSequenceEnding* pEnding) {
    pEnding->mMessage->movement();
    pEnding->mMessage->calcAnim();
    return MR::isDead(pEnding->mMessage);
}

GameSequenceEnding::GameSequenceEnding(GameSequence* pHost) : ActorStateBase< GameSequence >("\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O", pHost) {
    mNameObj = new NameObj("\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O\x83""f\x83\x82\x8aJ\x8en\x8e\xd2");
    mMessage = nullptr;
    mMessageIndex = 0;
    mIsNormalEnding = false;
    mIs120PowerStarsEnding = false;
}

GameSequenceEnding::~GameSequenceEnding() {
}

void GameSequenceEnding::init() {
    initNerve(GET_NERVE_ANON(GameSequenceEndingNrvInit), 0);
}

void GameSequenceEnding::appear() {
    mIsDead = false;
    fn_80060730();
    mMessageIndex = 0;
    mIsNormalEnding = false;
    mIs120PowerStarsEnding = false;
    if (fn_804D8650() != 0) {
        bool is120 = MR::isOnGameEventFlag120PowerStarsEnding();
        mIs120PowerStarsEnding = !is120;
        if (!is120) {
            GameDataFunction::setCurrentWorldId(1);
        }
        GameDataFunction::onGameEventFlag("\x83X\x83^\x81[\x83g120\x8c\xc2\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O\x8e\xc0\x8ds");
    } else {
        mIsNormalEnding = !MR::isOnGameEventFlagNormalEnding();
        GameDataFunction::onGameEventFlag("\x83m\x81[\x83}\x83\x8b\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O\x8e\xc0\x8ds");
    }
}

void GameSequenceEnding::exeInit() {
    if (fn_804D8630() != 0) {
        fn_8001FD30(mNameObj, "\x83m\x81[\x83}\x83\x8b\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O\x82P");
        MR::forceCloseSystemWipeWhiteFade();
    }
}

void GameSequenceEnding::exeWipeClose() {
    if (MR::isFirstStep(this)) {
        fn_804D7240();
    }
    if (fn_804D8630() != 0) {
        GameSystemFunction::setPermissionToCheckWiiRemoteConnect(false);
        MR::setPlayerSwingPermission(false);
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvBlank));
    }
}

void GameSequenceEnding::exeDemoMessage() {
    if (fn_804D8630() != 0) {
        MR::tryStartTimeKeepDemoMarioPuppetable(mNameObj, "\x83R\x83\x93\x83v\x83\x8a\x81[\x83g\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O", nullptr);
        const Nerve* pNerve = GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence);
        if (mIs120PowerStarsEnding) {
            pNerve = GET_NERVE_ANON(GameSequenceEndingNrvMessageNext);
        }
        fn_8001FEB0(this, mNameObj, "\x83Z\x81[\x83u", pNerve, 0);
    }
}

void GameSequenceEnding::exeMessage() {
    if (MR::isFirstStep(this)) {
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
        mMessage->setMessage(lbl_807D03C0);
        mMessage->setAButtonPos();
    }
    if (fn_804B3790(this)) {
        mMessageIndex++;
        if (mMessageIndex < 1) {
            setNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessage));
            return;
        }
        if (GameDataFunction::isOnGameEventFlag(lbl_807D03C8)) {
            setNerve(GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence));
            return;
        }
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessageWait));
    }
}

void GameSequenceEnding::exeMessageWait() {
    if (MR::isFirstStep(this)) {
        MR::requestWiiMessageBoardMailSend("WiiMessage_NormalEnding", "NormalEndingImage.bin", lbl_807D03C8);
    }
    void* pObj = fn_80500DD0();
    s32 isEnd = fn_80500D80();
    callVirtual(pObj, 0x14);
    callVirtual(pObj, 0x1C);
    if (isEnd != 0) {
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence));
    }
}

void GameSequenceEnding::exeMessageNext() {
    if (MR::isFirstStep(this)) {
        mMessage->setMessage(lbl_807D03C4);
        mMessage->setAButtonPos();
        MR::startSubBGM("BGM_SMG2_JG_GREENSTAR1", false);
    }
    if (fn_804B3790(this)) {
        mMessageIndex++;
        if (mMessageIndex < 1) {
            setNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessageNext));
            return;
        }
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence));
    }
}

void GameSequenceEnding::exeSaveSequence() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startGameDataSaveSequence(true, false, false);
    }
    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        kill();
    }
}
