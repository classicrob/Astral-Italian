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

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
// Unnamed SMG2 helpers: the 2-string ReceiverTagMail ctor, the mail request step, and the
// vtable-driven resource lookups in exeSend.
extern "C" ReceiverTagMail* fn_80500820(ReceiverTagMail*, const char*, const char*);
extern "C" void* getMailSourceRes(ResourceHolder*, const char*);
extern "C" void* createMailData(ResourceHolder*, void*);
extern "C" void* getResTableRes(ResourceHolder*, const char*);


namespace NrvWiiMessageBoardMailSenderState {
    // Nerve order follows the target's sInstances: 6100 sends, 6104 shows, 6108 is the end state.
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvSend, WiiMessageBoardMailSenderState, Send);
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvShow, WiiMessageBoardMailSenderState, Show);
    NEW_NERVE(WiiMessageBoardMailSenderStateNrvEnd, WiiMessageBoardMailSenderState, End);
}  // namespace NrvWiiMessageBoardMailSenderState

WiiMessageBoardMailSenderState::WiiMessageBoardMailSenderState() : NerveExecutor("WiiMessageBoardMailSenderState") {
    mReceiverTagMail = NULL;
    mResourceHolder = NULL;
    mSysInfoWindow = NULL;
    mMessageId = 0;
    mResName = NULL;
    mGameEventFlag = NULL;

    ReceiverTagMail* pMail = (ReceiverTagMail*)operator new(0x14);
    if (pMail != NULL) {
        mReceiverTagMail = fn_80500820(pMail, "WiiMessage_Title", "WiiMessageImage.arc");
    }

    mResourceHolder = MR::createAndAddResourceHolder("WiiMessageImage.arc");
    mSysInfoWindow = MR::createSysInfoWindowMiniExecuteWithChildren();

    if (!MR::isStageStaffRoll()) {
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
        void* pSource = getMailSourceRes(mResourceHolder, mResName);
        void* pData = createMailData(mResourceHolder, pSource);
        void* pRes = getResTableRes(mResourceHolder, mResName);
        mReceiverTagMail->send(mMessageId, static_cast<const u8*>(pRes), reinterpret_cast<u32>(pData), false);
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

WiiMessageBoardMailSender::WiiMessageBoardMailSender() : NameObj("WiiMessageBoardMailSender") {
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
