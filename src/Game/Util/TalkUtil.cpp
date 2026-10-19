#include "Game/Util/TalkUtil.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/YesNoController.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace MR {
    void registerBranchFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerBranchFunc(rFunc);
    }

    void registerEventFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerEventFunc(rFunc);
    }

    void registerAnimeFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerAnimeFunc(rFunc);
    }

    void registerKillFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerKillFunc(rFunc);
    }

    void setMessageArg(TalkMessageCtrl* pCtrl, int value) {
        CustomTagArg arg(value, CustomTagArg::Type_Int);
        pCtrl->setMessageArg(arg);
    }

    void setMessageArg(TalkMessageCtrl* pCtrl, const wchar_t* pArg) {
        CustomTagArg arg(pArg, CustomTagArg::Type_Char);
        pCtrl->setMessageArg(arg);
    }

    TalkMessageCtrl* createTalkCtrl(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
        pCtrl->createMessage(rIter, pMessageID);
        return pCtrl;
    }

    TalkMessageCtrl* createTalkCtrlDirect(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
        pCtrl->createMessageDirect(rIter, pMessageID);
        return pCtrl;
    }

    TalkMessageCtrl* createTalkCtrlDirect(LiveActor* pActor, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx) {
        JMapInfoIter iter(nullptr, -1);
        TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
        pCtrl->createMessageDirect(iter, pMessageID);
        return pCtrl;
    }

    TalkMessageCtrl* createTalkCtrlDirectOnRootNodeAutomatic(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID,
                                                             const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = createTalkCtrlDirect(pActor, rIter, pMessageID, rOffset, pMtx);
        onRootNodeAutomatic(pCtrl);
        return pCtrl;
    }

    TalkMessageCtrl* tryCreateTalkCtrlDirectOnRootNodeAutomatic(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID,
                                                                const TVec3f& rOffset, MtxPtr pMtx) {
        if (isExistMessageId(rIter)) {
            TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
            pCtrl->createMessage(rIter, pMessageID);
            pCtrl->mIsOnRootNodeAuto = true;
            return pCtrl;
        }

        return nullptr;
    }

    bool tryTalkNearPlayer(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalk()) {
            if (pCtrl->startTalk()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkNearPlayerAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            pCtrl->requestTalk();
            return true;
        } else {
            if (pCtrl->requestTalk()) {
                pCtrl->startTalk();
            }

            return false;
        }
    }

    bool tryTalkForce(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce() && pCtrl->startTalkForce()) {
            ret = true;
        }

        return ret;
    }

    bool tryTalkForceMarioPuppetable(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForcePuppetable()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkForceAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForce();
        }

        return false;
    }

    bool tryTalkForceMarioPuppetableAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForcePuppetable();
        }

        return false;
    }

    bool tryTalkForceWithoutDemo(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce() && pCtrl->startTalkForceWithoutDemo()) {
            ret = true;
        }

        return ret;
    }

    bool tryTalkForceWithoutDemoMarioPuppetable(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForceWithoutDemoPuppetable()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkForceWithoutDemoAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForceWithoutDemo();
        }

        return false;
    }

    bool tryTalkForceWithoutDemoMarioPuppetableAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForceWithoutDemoPuppetable();
        }

        return false;
    }

    bool tryTalkTimeKeepDemo(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForce()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkTimeKeepDemoMarioPuppetable(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForcePuppetable()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkTimeKeepDemoForceWithoutDemo(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForceWithoutDemo()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkTimeKeepDemoWithoutPauseMarioPuppetable(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (pCtrl->requestTalkForce()) {
            if (pCtrl->startTalkForceWithoutDemoPuppetable()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkRequest(TalkMessageCtrl* pCtrl) {
        return pCtrl->requestTalk();
    }

    bool tryTalkSelectLeft(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (MR::isYesNoSelected()) {
            if (MR::isYesNoSelectedYes()) {
                ret = true;
            }
        }

        return ret;
    }

    bool tryTalkSelectRight(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (MR::isYesNoSelected()) {
            if (!MR::isYesNoSelectedYes()) {
                ret = true;
            }
        }

        return ret;
    }

    const MtxPtr getMessageBalloonFollowMatrix(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mMtx;
    }

    const TVec3f& getMessageBalloonFollowOffset(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mMsgBalloonFollowOffs;
    }

    void setMessageBalloonFollowOffset(TalkMessageCtrl* pCtrl, const TVec3f& rOffs) {
        pCtrl->setMessageBallonFollowOffs(rOffs);
    }

    bool isNearPlayer(const TalkMessageCtrl* pCtrl, f32 dist) {
        return pCtrl->isNearPlayer(dist);
    }

    bool inMessageArea(const TalkMessageCtrl* pCtrl) {
        return pCtrl->inMessageArea();
    }

    bool isTalkNone(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mTalkState == TalkMessageState_Entry;
    }

    bool isTalkEntry(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mTalkState == TalkMessageState_None;
    }

    bool isTalkTalking(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mTalkState == TalkMessageState_Talking;
    }

    bool isTalkEnableEnd(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mTalkState == TalkMessageState_EnableEnd;
    }

    void clearTalkState(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
    }

    void resetNode(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->resetFlowNode();
    }

    void readMessage(TalkMessageCtrl* pCtrl) {
        pCtrl->readMessage();
    }

    bool isMessageRead(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mIsAlreadyDone;
    }

    void forwardNode(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardFlowNode();
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }

    void resetAndForwardNode(TalkMessageCtrl* pCtrl, s32 numNode) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->resetFlowNode();

        for (s32 i = 0; i < numNode; i++) {
            TalkFunction::onTalkStateNone(pCtrl);
            pCtrl->mNodeCtrl->forwardFlowNode();
            pCtrl->mNodeCtrl->recordTempFlowNode();
        }
    }

    void forwardNodeNextBranchLeft(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardFlowNode();
        pCtrl->mNodeCtrl->forwardCurrentBranchNode(true);
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }

    void forwardNodeNextBranchRight(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardFlowNode();
        pCtrl->mNodeCtrl->forwardCurrentBranchNode(false);
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }

    void forwardNodeCurrentBranchLeft(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardCurrentBranchNode(true);
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }

    void forwardNodeCurrentBranchRight(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardCurrentBranchNode(false);
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }

    bool tryForwardNode(TalkMessageCtrl* pCtrl) {
        if (pCtrl->mNodeCtrl->isExistNextNode()) {
            TalkFunction::onTalkStateNone(pCtrl);
            TalkFunction::onTalkStateNone(pCtrl);
            pCtrl->mNodeCtrl->forwardFlowNode();
            pCtrl->mNodeCtrl->recordTempFlowNode();
            return true;
        }

        return false;
    }

    bool isExistNextNode(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mNodeCtrl->isExistNextNode();
    }

    bool tryForwardNodeForMessageArea(TalkMessageCtrl* pCtrl) {
        TVec3f* pPlayerPos = getPlayerCenterPos();
        if (getAreaObj("MessageArea", *pPlayerPos) == nullptr) {
            return false;
        }

        s32 messageID = static_cast< s32 >(pCtrl->getMessageID());
        while (true) {
            if (pCtrl->inMessageArea()) {
                break;
            }

            if (!tryForwardNode(pCtrl)) {
                TalkFunction::onTalkStateNone(pCtrl);
                pCtrl->mNodeCtrl->resetFlowNode();
            }

            if (messageID == static_cast< s32 >(pCtrl->getMessageID())) {
                break;
            }
        }

        if (messageID == static_cast< s32 >(pCtrl->getMessageID())) {
            return false;
        }

        return true;
    }

    bool isShortTalk(const TalkMessageCtrl* pCtrl) {
        return TalkFunction::isShortTalk(pCtrl);
    }

    void setDistanceToTalk(TalkMessageCtrl* pCtrl, f32 dist) {
        pCtrl->mTalkDistance = dist;
    }

    void onRootNodeAutomatic(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsOnRootNodeAuto = true;
    }

    void offRootNodeAutomatic(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsOnRootNodeAuto = false;
    }

    void onReadNodeAutomatic(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsOnReadNodeAuto = true;
    }

    void offReadNodeAutomatic(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsOnReadNodeAuto = false;
    }

    void onStartOnlyFront(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsStartOnlyFront = true;
    }

    void offTalkCinemaFrame(TalkMessageCtrl* pCtrl) {
        if (pCtrl != nullptr) {
            pCtrl->mUseCinemaFrame = false;
        }
    }

    void hideBubbleBeak(TalkMessageCtrl* pCtrl) {
        if (pCtrl != nullptr) {
            pCtrl->mHideBalloonBeak = true;
        }
    }

    void showBubbleBeak(TalkMessageCtrl* pCtrl) {
        if (pCtrl != nullptr) {
            pCtrl->mHideBalloonBeak = false;
        }
    }

    void onTalkSmg2Flag52(TalkMessageCtrl* pCtrl) {
        if (pCtrl != nullptr) {
            pCtrl->mCenterBalloonBeak = true;
        }
    }

    void initTalkCamera(TalkMessageCtrl* pCtrl, const JMapInfoIter& rIter) {
        pCtrl->initCamera(rIter);
    }

    void disableTalkCamera(TalkMessageCtrl* pCtrl) {
        pCtrl->disableCamera();
    }

    bool isTalkStart(const TalkMessageCtrl* pCtrl) {
        return TalkFunction::isTalkSystemStart(pCtrl);
    }

    bool isTalkEnd(const TalkMessageCtrl* pCtrl) {
        return TalkFunction::isTalkSystemEnd(pCtrl);
    }
};  // namespace MR
