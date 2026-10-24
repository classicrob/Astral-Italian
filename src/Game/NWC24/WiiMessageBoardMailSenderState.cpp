#include "Game/NWC24/WiiMessageBoardMailSenderState.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NWC24/ReceiverTagMail.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/SysInfoWindow.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

namespace NrvWiiMessageBoardMailSenderState {
    // Nerve order follows the target's sInstances: 6100 sends, 6104 shows, 6108 is the end state.
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvSend, WiiMessageBoardMailSenderState, Send);
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvShow, WiiMessageBoardMailSenderState, Show);
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvEnd, WiiMessageBoardMailSenderState, End);
}  // namespace NrvWiiMessageBoardMailSenderState

WiiMessageBoardMailSenderState::WiiMessageBoardMailSenderState() : NerveExecutor("Wii伝言板メール送信ステート") {
    mReceiverTagMail = NULL;
    mResourceHolder = NULL;
    mSysInfoWindow = NULL;
    mMessageId = 0;
    mResName = NULL;
    mGameEventFlag = NULL;

    mReceiverTagMail = new ReceiverTagMail("Wii伝言板メール", "WiiMessage_Title");

    mResourceHolder = MR::createAndAddResourceHolder("WiiMessageImage.arc");
    mSysInfoWindow = MR::createSysInfoWindowMiniExecuteWithChildren();

    bool isOutsideStaffRoll = !MR::isStageStaffRoll();
    if (isOutsideStaffRoll) {
        MR::connectToSceneLayout(mSysInfoWindow);
    }

    initNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvSend::sInstance, 0);
}

void WiiMessageBoardMailSenderState::setup(const char* messageId, const char* pResName, const char* pGameEventFlag) {
    mMessageId = messageId;
    mResName = pResName;
    mGameEventFlag = pGameEventFlag;
    setNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvSend::sInstance);
}

void WiiMessageBoardMailSenderState::exeSend() {
    if (MR::isFirstStep(this)) {
        void* pSource = mResourceHolder->mArchive->getResource(mResName);
        u32 resourceSize = mResourceHolder->mArchive->getExpandedResSize(pSource);
        void* pRes = mResourceHolder->mFileInfoTable->getRes(mResName);
        mReceiverTagMail->send(mMessageId, static_cast<const u8*>(pRes), resourceSize, false);
    }

    mReceiverTagMail->update();
    if (mReceiverTagMail->isDone()) {
        if (mReceiverTagMail->isError()) {
            setNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvEnd::sInstance);
        } else {
            setNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvShow::sInstance);
        }
    }
}

void WiiMessageBoardMailSenderState::exeShow() {
    if (MR::isFirstStep(this)) {
        GameDataFunction::onGameEventFlag(mGameEventFlag);
        MR::requestMovementOn(mSysInfoWindow);
        static_cast<SysInfoWindow*>(mSysInfoWindow)->appear("WiiMessage_SendDone", SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center,
                                                            SysInfoWindow::MessageType_System);
    }

    if (MR::isDead(mSysInfoWindow)) {
        setNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvEnd::sInstance);
    }
}

void WiiMessageBoardMailSenderState::exeEnd() {
}

WiiMessageBoardMailSenderState::~WiiMessageBoardMailSenderState() {
}

WiiMessageBoardMailSender::WiiMessageBoardMailSender() : NameObj("Wii伝言板メール送信者") {
    mState = new WiiMessageBoardMailSenderState();
}

WiiMessageBoardMailSender::~WiiMessageBoardMailSender() {
}

namespace MR {
    // Starts a mail send with the given message ID, resource name, and game event flag.
    void requestWiiMessageBoardMailSend(const char* messageId, const char* pResName, const char* pGameEventFlag) {
        WiiMessageBoardMailSender* pSender =
            (WiiMessageBoardMailSender*)MR::getSceneObjHolder()->getObj(SceneObj_WiiMessageBoardMailSender);
        pSender->mState->setup(messageId, pResName, pGameEventFlag);
    }

    // Updates the sender state and returns true while it is showing the result.
    bool updateWiiMessageBoardMailSenderState() {
        WiiMessageBoardMailSenderState* pState =
            ((WiiMessageBoardMailSender*)MR::getSceneObjHolder()->getObj(SceneObj_WiiMessageBoardMailSender))->mState;
        pState->updateNerve();
        return pState->isNerve(&NrvWiiMessageBoardMailSenderState::WiiMessageBoardMailSenderStateNrvEnd::sInstance);
    }

    LayoutActor* getWiiMessageBoardMailSenderWindow() {
        WiiMessageBoardMailSenderState* pState =
            ((WiiMessageBoardMailSender*)MR::getSceneObjHolder()->getObj(SceneObj_WiiMessageBoardMailSender))->mState;
        return pState->mSysInfoWindow;
    }
}  // namespace MR
