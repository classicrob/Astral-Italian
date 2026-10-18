#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/Screen/MessageTagSkipTagProcessor.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

void TalkMessageHistory::entry(u16 msgID) {
    mHistory[mCount++] = msgID;
}

bool TalkMessageHistory::search(u16 msgID) const {
    for (s32 i = 0; i < mCount; i++) {
        if (mHistory[i] == msgID) {
            return true;
        }
    }

    return false;
}

TalkNodeCtrl::TalkNodeCtrl() : _0(), mMessageName(), mCurrentNodeIdx(-1), mMessageInfo() {
    mHistory.mCount = 0;
    _38 = nullptr;
    mCurrentNode = nullptr;
    mFlowNode = nullptr;
    mNodeData = -1;
}

bool TalkNodeCtrl::createFlowNodeDirect(TalkMessageCtrl* ctrl, const JMapInfoIter& iter, const char* name, ActorCameraInfo** camera) {
    if (!_0) {
        _0 = MessageSystem::getSceneMessageData(name, ctrl->mHostActor->mLinkInfo.mZoneID);
        if (!MessageSystem::isExistMessage(name, _0)) {
            _0 = MessageSystem::getSceneMessageData(name, -1);
        }
    }
    TalkNode* node = MessageSystem::findNode(name, _0);
    _38 = node;
    mCurrentNode = node;
    mFlowNode = node;
    mMessageName = new char[strlen(name) + 1];
    MR::copyString(mMessageName, name, strlen(name) + 1);
    if (mCurrentNode) updateMessage();
    else MessageSystem::getMessageDirect(&mMessageInfo, mMessageName, _0);
    if (!MR::isValidInfo(iter)) *camera = new ActorCameraInfo(0, ctrl->mHostActor->mLinkInfo.mZoneID);
    else *camera = new ActorCameraInfo(iter);
    RecursiveHelper helper;
    helper.mIndex = 0;
    s32 switchA = -1;
    s32 switchB = -1;
    initNodeRecursive(ctrl, iter, *camera, &helper, &switchA, &switchB);
    MR::initTalkStageSwitch(ctrl->mHostActor, iter, switchA, true);
    MR::initTalkStageSwitch(ctrl->mHostActor, iter, switchB, false);
    resetFlowNode();
    return true;
}

void TalkNodeCtrl::resetFlowNode() {
    if (mCurrentNode != _38) {
        mCurrentNode = _38;
        mFlowNode = _38;
        updateMessage();
    }
}

void TalkNodeCtrl::resetTempFlowNode() {
    if (mCurrentNode != mFlowNode) {
        mCurrentNode = mFlowNode;
        updateMessage();
    }
}

void TalkNodeCtrl::recordTempFlowNode() {
    mFlowNode = mCurrentNode;
}

void TalkNodeCtrl::forwardFlowNode() {
    TalkNode* node = mCurrentNode;

    if (node == nullptr) {
        return;
    }

    if (node->mNodeType == 1) {
        u16 idx = node->mNextIdx;
        mCurrentNode = MessageSystem::getNode(idx, _0);
    } else if (node->mNodeType == 3) {
        u16 idx = node->mIndex;
        mCurrentNode = MessageSystem::getNode(idx, _0);
    }

    updateMessage();
}

bool TalkNodeCtrl::isExistNextNode() const {
    return getNextNode() != nullptr;
}

bool TalkNodeCtrl::isNextNodeMessage() const {
    TalkNode* next = getNextNode();

    if (next == nullptr || next->mNodeType != 1) {
        return false;
    }

    return true;
}

bool TalkNodeCtrl::isCurrentNodeEvent() const {
    return getCurrentNodeEvent() != nullptr;
}

TalkNode* TalkNodeCtrl::getNextNode() const {
    TalkNode* node = mCurrentNode;
    if (!node) return nullptr;
    if (node->mNodeType == 1) {
        if (node->mNextIdx != 0xFFFF) return MessageSystem::getNode(node->mNextIdx, _0);
    } else if (node->mNodeType == 3) {
        if (node->mIndex != 0xFFFF) return MessageSystem::getNode(node->mIndex, _0);
    } else if (node->mNodeType == 4) {
        if (node->mEventType != 0xFFFF) return MessageSystem::getNode(node->mEventType, _0);
    }
    return nullptr;
}

TalkNode* TalkNodeCtrl::getNextNodeBranch() const {
    TalkNode* node = getNextNode();

    if (node == nullptr) {
        return nullptr;
    }

    return node->mNodeType != 2 ? nullptr : node;
}

TalkNode* TalkNodeCtrl::getCurrentNodeBranch() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 2) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getCurrentNodeMessage() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 1) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getCurrentNodeEvent() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 3) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getNextNodeEvent() const {
    TalkNode* node = getNextNode();

    if (node == nullptr) {
        return nullptr;
    }

    return node->mNodeType != 3 ? nullptr : node;
}

void TalkNodeCtrl::updateMessage() {
    TalkNode* cur = mCurrentNode;
    if (cur && cur->mNodeType == 4) {
        mCurrentNode = getNextNode();
        updateMessage();
        return;
    }
    if (cur) {
        if (cur->mNodeType == 1) {
            MessageSystem::getMessage(&mMessageInfo, cur, _0);
            mCurrentNodeIdx = cur->mIndex;
            TalkNode* branch = getNextNodeBranch();
            if (branch) mNodeData = branch->mIndex;
            else mNodeData = -1;
        } else mMessageInfo._0 = nullptr;
    } else MessageSystem::getMessageDirect(&mMessageInfo, mMessageName, _0);
    if (mMessageInfo.isAutoEventTalk() && (mHistory.search(mCurrentNodeIdx) ||
        (mMessageInfo._B != -1 && GameDataFunction::isOnMessageAlreadyRead(mMessageInfo._B)))) {
        mMessageInfo.mTalkType = 0;
    }
}

void TalkNodeCtrl::readMessage() {
    if (mMessageInfo.isAutoEventTalk()) {
        if (!mHistory.search(mCurrentNodeIdx)) {
            mHistory.entry(mCurrentNodeIdx);
        }

        mMessageInfo.mTalkType = 0;
    }

    if (mMessageInfo._B != -1) {
        GameDataFunction::setOnMessageAlreadyRead(mMessageInfo._B);
    }
}

const wchar_t* TalkNodeCtrl::getSubMessage() const {
    const TalkMessageInfo info = mMessageInfo;

    if (*((wchar_t*)info._0) != 0x1A) {
        return nullptr;
    }

    MessageEditorMessageTag messageTag = MessageEditorMessageTag((const wchar_t*)&info._0[2]);

    if (((char*)messageTag.mMessage)[1] != 8 || messageTag.mMessage[1] != 0) {
        return nullptr;
    }

    TalkMessageInfo info2 = TalkMessageInfo();
    u16 param = messageTag.getParam32(0);

    if (MessageSystem::getSceneMessageData()->getMessage(&info2, 0, param)) {
        return (const wchar_t*)info2._0;
    }

    return nullptr;
}

void TalkNodeCtrl::forwardCurrentBranchNode(bool storeCurrent) {
    TalkNode* cur = mCurrentNode;

    if (storeCurrent) mCurrentNode = MessageSystem::getBranchTrueNode(cur, _0);
    else mCurrentNode = MessageSystem::getBranchFalseNode(cur, _0);

    updateMessage();
}

bool TalkNodeCtrl::createFlowNode(TalkMessageCtrl* pMsgCtrl, const JMapInfoIter& rIter, const char* pName, ActorCameraInfo** pCameraInf) {
    s32 msgID = -1;
    MR::getJMapInfoMessageID(rIter, &msgID);
    if (msgID == -2) {
        return false;
    }

    char buf[0x100];
    MR::makeMessageID(buf, sizeof(buf), pName, msgID);
    _0 = MessageSystem::getSceneMessageData(buf, pMsgCtrl->mHostActor->mLinkInfo.mZoneID);
    return createFlowNodeDirect(pMsgCtrl, rIter, buf, pCameraInf);
}

bool RecursiveHelper::hasNode(const TalkNode* pNode) const {
    for (int i = 0; i < mIndex; i++) {
        if (mStack[i] == pNode) {
            return true;
        }
    }

    return false;
}

void TalkNodeCtrl::initNodeRecursive(TalkMessageCtrl* ctrl, const JMapInfoIter& iter, ActorCameraInfo* camera, RecursiveHelper* helper, s32* switchA, s32* switchB) {
    TalkNode* current = mCurrentNode;
    if (helper->hasNode(current)) return;
    helper->mStack[helper->mIndex++] = current;
    if (mMessageInfo.isCameraEvent()) {
        camera->mCameraSetID = mMessageInfo.mCameraSetID;
        MR::initMultiActorCamera(ctrl->mHostActor, iter, &camera, "会話");
    }
    if (getNextNodeBranch() || getCurrentNodeBranch()) {
        if (getNextNodeBranch()) forwardFlowNode();
        TalkNode* branch = getCurrentNodeBranch();
        if (branch) {
            if (branch->mIndex == 3) { if (*switchA < 0) *switchA = 0; }
            else if (branch->mIndex == 4 && *switchB < 0) *switchB = 0;
        }
        forwardCurrentBranchNode(true);
        initNodeRecursive(ctrl, iter, camera, helper, switchA, switchB);
        mCurrentNode = branch;
        forwardCurrentBranchNode(false);
        initNodeRecursive(ctrl, iter, camera, helper, switchA, switchB);
        return;
    }
    if (getNextNodeEvent() || isNextNodeMessage()) {
        forwardFlowNode();
        TalkNode* event = getCurrentNodeEvent();
        if (event) {
            if (event->mEventType == 5) *switchA = 1;
            else if (event->mEventType == 6) *switchB = 1;
            else if (event->mEventType == 8) *switchA = 1;
            else if (event->mEventType == 9) *switchB = 1;
        }
        initNodeRecursive(ctrl, iter, camera, helper, switchA, switchB);
    }
}

bool TalkNodeCtrl::isForwardNode() const {
    TalkNode* event = getCurrentNodeEvent();
    return event ? event->mEventType == 3 : false;
}
