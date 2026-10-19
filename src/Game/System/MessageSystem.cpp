#include "Game/System/MessageHolder.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/LMS.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>

bool MessageSystem::getSystemMessageDirect(TalkMessageInfo* pMessageInfo, const char* pMessageId) {
    MessageData* pData = MR::getGameSystemObjHolder()->mMessageHolder->mSystemMessageData;
    if (pData != nullptr) {
        return pData->getMessageDirect(pMessageInfo, pMessageId);
    }
    return false;
}

bool MessageSystem::getGameMessageDirect(TalkMessageInfo* pMessageInfo, const char* pMessageId) {
    for (s32 i = 0; i < MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageNum; i++) {
        MessageData* pData = MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageDatas[i];
        bool found = pData != nullptr ? pData->getMessageDirect(pMessageInfo, pMessageId) : false;
        if (found) {
            return true;
        }
    }

    return false;
}

bool MessageSystem::getMessage(TalkMessageInfo* pMessageInfo, const TalkNode* pNode, const MessageData* pData) {
    if (pData != nullptr) {
        return pData->getMessageByIndex(pMessageInfo, pNode->mIndex);
    }

    return false;
}

bool MessageSystem::isExistMessage(const char* pMessageId, const MessageData* pData) {
    if (pData == nullptr) {
        return false;
    }

    bool isFound = LMS_GetTextByLabel(pData->mMessageHandle, pMessageId) != nullptr;
    if (!isFound) {
        isFound = pData->findEntryNodeIndex(pMessageId) >= 0;
    }
    return isFound;
}

bool MessageSystem::isExistGameMessage(const char* pMessageId) {
    for (s32 i = 0; i < MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageNum; i++) {
        if (isExistMessage(pMessageId, MR::getGameSystemObjHolder()->mMessageHolder->mGameMessageDatas[i])) {
            return true;
        }
    }

    return false;
}

bool MessageSystem::isExistSceneMessage(const char* pMessageId) {
    for (s32 i = 0; i < MR::getGameSystemObjHolder()->mMessageHolder->mSceneCommonMessageNum; i++) {
        if (isExistMessage(pMessageId, MR::getGameSystemObjHolder()->mMessageHolder->mSceneCommonMessageDatas[i])) {
            return true;
        }
    }

    return false;
}

TalkNode* MessageSystem::findNode(const char* pMessageId, const MessageData* pData) {
    if (pData == nullptr) {
        return nullptr;
    }

    s32 index = pData->findEntryNodeIndex(pMessageId);
    if (index == -1) {
        return nullptr;
    }

    return reinterpret_cast< TalkNode* >(pData->getNodeDataPtr(index));
}

TalkNode* MessageSystem::getNode(u32 index, const MessageData* pData) {
    if (pData == nullptr) {
        return nullptr;
    }

    if (index == 0xFFFF) {
        return nullptr;
    }

    return reinterpret_cast< TalkNode* >(pData->getNodeDataPtr(index));
}

TalkNode* MessageSystem::getBranchTrueNode(const TalkNode* pNode, const MessageData* pData) {
    if (pData == nullptr) {
        return nullptr;
    }

    u16 flowIndex = pData->getFlowNodeIndex(pNode);
    u16 branchIndex = *reinterpret_cast< u16* >(pData->getNodeBranchPtr(flowIndex));
    return getNode(branchIndex, pData);
}

TalkNode* MessageSystem::getBranchFalseNode(const TalkNode* pNode, const MessageData* pData) {
    if (pData == nullptr) {
        return nullptr;
    }

    u16 flowIndex = pData->getFlowNodeIndex(pNode);
    u16 branchIndex = *reinterpret_cast< u16* >(reinterpret_cast< u8* >(pData->getNodeBranchPtr(flowIndex)) + 2);
    return getNode(branchIndex, pData);
}
