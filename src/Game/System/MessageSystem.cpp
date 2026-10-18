#include "Game/System/MessageHolder.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
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
