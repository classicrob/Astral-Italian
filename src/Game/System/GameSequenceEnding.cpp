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

static const char* sNormalEndingMessages[] = {"InformationSpecialWorld"};
static const char* s120StarEndingMessages[] = {"InformationGreenStar"};
static const char* sNormalEndingMailFlag = "ノーマルエンディングメール送信";

namespace {
    NEW_NERVE(GameSequenceEndingNrvInit, GameSequenceEnding, Init);
    NEW_NERVE(GameSequenceEndingNrvWipeClose, GameSequenceEnding, WipeClose);
    class GameSequenceEndingNrvBlank : public Nerve {
    public:
        virtual void execute(Spine*) const {
            GameSequenceFunction::incPlayTimeInStage();
        }
        static GameSequenceEndingNrvBlank sInstance;
    };
#ifdef GALAXY_NATIVE
    GameSequenceEndingNrvBlank GameSequenceEndingNrvBlank::sInstance;
#else
    GameSequenceEndingNrvBlank GameSequenceEndingNrvBlank::sInstance ATTRIBUTE_WEAK;
#endif

    NEW_NERVE(GameSequenceEndingNrvDemoMessage, GameSequenceEnding, DemoMessage);
    NEW_NERVE(GameSequenceEndingNrvMessage, GameSequenceEnding, Message);
    NEW_NERVE(GameSequenceEndingNrvMessageWait, GameSequenceEnding, MessageWait);
    NEW_NERVE(GameSequenceEndingNrvMessageNext, GameSequenceEnding, MessageNext);
    NEW_NERVE(GameSequenceEndingNrvSaveSequence, GameSequenceEnding, SaveSequence);
};  // namespace

GameSequenceEnding::GameSequenceEnding(GameSequence* pHost) : ActorStateBase< GameSequence >("[シーケンス]エンディング", pHost),
      mNameObj(nullptr), mMessage(nullptr), mMessageIndex(0), mIsNormalEnding(false), mIs120PowerStarsEnding(false) {
    mNameObj = new NameObj("エンディングデモ開始者");
}

GameSequenceEnding::~GameSequenceEnding() {
}

void GameSequenceEnding::init() {
    initNerve(GET_NERVE_ANON(GameSequenceEndingNrvInit), 0);
}

void GameSequenceEnding::appear() {
    mIsDead = false;
    MR::onUseVFilter();
    mMessageIndex = 0;
    mIsNormalEnding = false;
    mIs120PowerStarsEnding = false;
    if (GameSequenceFunction::isCollectedAll120PowerStars() != 0) {
        mIs120PowerStarsEnding = !MR::isOnGameEventFlag120PowerStarsEnding();
        if (mIs120PowerStarsEnding) {
            GameDataFunction::setCurrentWorldId(1);
        }
        GameDataFunction::onGameEventFlag("スター120個エンディング実行");
    } else {
        mIsNormalEnding = !MR::isOnGameEventFlagNormalEnding();
        GameDataFunction::onGameEventFlag("ノーマルエンディング実行");
    }
}

void GameSequenceEnding::startInit() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvInit));
}

void GameSequenceEnding::startWipeClose() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvWipeClose));
}

void GameSequenceEnding::startDemoMessage() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvDemoMessage));
}

// NONMATCHING: regalloc; demo argument loads scheduled across the same nerve selection (FP order/grouping checked: no FP instructions).
void GameSequenceEnding::requestSaveDemo() {
    GameSystemFunction::setPermissionToCheckWiiRemoteConnect(true);
    MR::requestStartDemo(this, mNameObj, "セーブ",
                         mIsNormalEnding ? static_cast<const Nerve*>(GET_NERVE_ANON(GameSequenceEndingNrvMessage)) : GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence), nullptr);
}

void GameSequenceEnding::initAfterResourceLoaded() {
    mMessage = new InformationMessage(false);
    mMessage->initWithoutIter();
    mMessage->setWindowType(1);
}

void GameSequenceEnding::draw() const {
    if (mMessage != nullptr && !MR::isDead(mMessage)) {
        mMessage->draw();
    }
    if (isNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessageWait))) {
        MR::getWiiMessageBoardMailSenderWindow()->draw();
    }
}

void GameSequenceEnding::killMessage() {
    if (mMessage != nullptr) {
        mMessage->kill();
    }
}

bool GameSequenceEnding::updateMessage() {
    mMessage->movement();
    mMessage->calcAnim();
    return MR::isDead(mMessage);
}

void GameSequenceEnding::exeInit() {
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        MR::tryStartDemoMarioPuppetableSystemFlags(mNameObj, "ノーマルエンディング１");
        MR::forceCloseSystemWipeWhiteFade();
    }
}

void GameSequenceEnding::exeWipeClose() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::resetPlayTimeInStage();
    }
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        GameSystemFunction::setPermissionToCheckWiiRemoteConnect(false);
        MR::setPlayerSwingPermission(false);
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvBlank));
    }
}

// NONMATCHING: regalloc; demo argument loads scheduled across the same nerve selection (FP order/grouping checked: no FP instructions).
void GameSequenceEnding::exeDemoMessage() {
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        MR::tryStartTimeKeepDemoMarioPuppetable(mNameObj, "コンプリートエンディング", nullptr);
        MR::requestStartDemo(this, mNameObj, "セーブ",
                             mIs120PowerStarsEnding ? static_cast<const Nerve*>(GET_NERVE_ANON(GameSequenceEndingNrvMessageNext)) : GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence), nullptr);
    }
}

void GameSequenceEnding::exeMessage() {
    if (MR::isFirstStep(this)) {
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
        mMessage->setMessage(sNormalEndingMessages[mMessageIndex]);
        mMessage->setAButtonPos();
    }
    if (updateMessage()) {
        mMessageIndex++;
        if (mMessageIndex < 1) {
            setNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessage));
            return;
        }
        if (GameDataFunction::isOnGameEventFlag(sNormalEndingMailFlag)) {
            setNerve(GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence));
            return;
        }
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvMessageWait));
    }
}

void GameSequenceEnding::exeMessageWait() {
    if (MR::isFirstStep(this)) {
        MR::requestWiiMessageBoardMailSend("WiiMessage_NormalEnding", "NormalEndingImage.bin", sNormalEndingMailFlag);
    }
    LayoutActor* pWindow = MR::getWiiMessageBoardMailSenderWindow();
    bool isEnd = MR::updateWiiMessageBoardMailSenderState();
    pWindow->movement();
    pWindow->calcAnim();
    if (isEnd != 0) {
        setNerve(GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence));
    }
}

void GameSequenceEnding::exeMessageNext() {
    if (MR::isFirstStep(this)) {
        mMessage->setMessage(s120StarEndingMessages[mMessageIndex]);
        mMessage->setAButtonPos();
        MR::startSubBGM("BGM_SMG2_JG_GREENSTAR1", false);
    }
    if (updateMessage()) {
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
