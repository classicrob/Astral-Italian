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

extern const char* lbl_807D03C0;
extern const char* lbl_807D03C4;
extern const char* lbl_807D03C8;

namespace {
    class GameSequenceEndingNrvBlank : public Nerve {
    public:
        virtual void execute(Spine*) const {
            GameSequenceFunction::incPlayTimeInStage();
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
    MR::onUseVFilter();
    mMessageIndex = 0;
    mIsNormalEnding = false;
    mIs120PowerStarsEnding = false;
    if (GameSequenceFunction::isCollectedAll120PowerStars() != 0) {
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

void GameSequenceEnding::startInit() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvInit));
}

void GameSequenceEnding::startWipeClose() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvWipeClose));
}

void GameSequenceEnding::startDemoMessage() {
    setNerve(GET_NERVE_ANON(GameSequenceEndingNrvDemoMessage));
}

void GameSequenceEnding::requestSaveDemo() {
    GameSystemFunction::setPermissionToCheckWiiRemoteConnect(true);
    const Nerve* pNerve = GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence);
    if (mIsNormalEnding) {
        pNerve = GET_NERVE_ANON(GameSequenceEndingNrvMessage);
    }
    MR::requestStartDemo(this, mNameObj, "\x83Z\x81[\x83u", pNerve, nullptr);
}

void GameSequenceEnding::initAfterResourceLoaded() {
    mMessage = new InformationMessage(false);
    mMessage->initWithoutIter();
    *reinterpret_cast< s32* >(reinterpret_cast< u8* >(mMessage) + 0x30) = 1;  // mWindowType (private in the header)
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
        MR::tryStartDemoMarioPuppetableSystemFlags(mNameObj, "\x83m\x81[\x83}\x83\x8b\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O\x82P");
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

void GameSequenceEnding::exeDemoMessage() {
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        MR::tryStartTimeKeepDemoMarioPuppetable(mNameObj, "\x83R\x83\x93\x83v\x83\x8a\x81[\x83g\x83G\x83\x93\x83""f\x83""B\x83\x93\x83O", nullptr);
        const Nerve* pNerve = GET_NERVE_ANON(GameSequenceEndingNrvSaveSequence);
        if (mIs120PowerStarsEnding) {
            pNerve = GET_NERVE_ANON(GameSequenceEndingNrvMessageNext);
        }
        MR::requestStartDemo(this, mNameObj, "\x83Z\x81[\x83u", pNerve, nullptr);
    }
}

void GameSequenceEnding::exeMessage() {
    if (MR::isFirstStep(this)) {
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
        mMessage->setMessage(lbl_807D03C0);
        mMessage->setAButtonPos();
    }
    if (updateMessage()) {
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
        mMessage->setMessage(lbl_807D03C4);
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
