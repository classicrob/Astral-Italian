#include "Game/Util/MessageUtil.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include <cstring>
#include "Game/Map/RaceManager.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/Screen/MessageTagSkipTagProcessor.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

#define MESSAGE_ID_BUFFER_SIZE 256

namespace MR {
    const wchar_t* getSystemMessageDirect(const char* pMessageId) {
        TalkMessageInfo messageInfo = TalkMessageInfo();

        MessageSystem::getSystemMessageDirect(&messageInfo, pMessageId);

        return reinterpret_cast< wchar_t* >(messageInfo.mMessageText);
    }

    // NONMATCHING: equivalent null-return branch arrangement
    const wchar_t* getGameMessageDirect(const char* pMessageId) {
        TalkMessageInfo messageInfo = TalkMessageInfo();

        const wchar_t* text = nullptr;
        if (MessageSystem::getGameMessageDirect(&messageInfo, pMessageId)) text = reinterpret_cast<wchar_t*>(messageInfo.mMessageText);
        return text;
    }

    const wchar_t* getLayoutMessageDirect(const char* pMessageId) {
        TalkMessageInfo messageInfo = TalkMessageInfo();

        MessageSystem::getLayoutMessageDirect(&messageInfo, pMessageId);

        return reinterpret_cast< wchar_t* >(messageInfo.mMessageText);
    }

    const wchar_t* getSceneMessageDirect(const char* name, s32 zone) {
        TalkMessageInfo info;
        MessageData* data = MessageSystem::getSceneMessageData(name, zone);
        MessageSystem::getMessageDirect(&info, name, data);
        const wchar_t* text = reinterpret_cast<const wchar_t*>(info.mMessageText);
        return text ? text : L"メッセージ未登録";
    }

    const wchar_t* getCurrentGalaxyNameOnCurrentLanguage() {
        return getGalaxyNameOnCurrentLanguage(getCurrentStageName());
    }

    const wchar_t* getCurrentGalaxyNameShortOnCurrentLanguage() {
        return getGalaxyNameShortOnCurrentLanguage(getCurrentStageName());
    }

    const wchar_t* getCurrentScenarioNameOnCurrentLanguage() {
        s32 scenario = getCurrentOrSelectedScenarioNo();
        return getScenarioNameOnCurrentLanguage(getCurrentStageName(), scenario);
    }

    const wchar_t* getWorldNameOnCurrentLanguage(s32 world) {
        char id[MESSAGE_ID_BUFFER_SIZE];
        snprintf(id, sizeof(id), "GalaxyName_WorldMap%02dGalaxy", world);
        return getGameMessageDirect(id);
    }

    bool isExistGameMessage(const char* id) {
        return MessageSystem::isExistGameMessage(id);
    }

    bool isExistSceneMessage(const char* id) {
        return MessageSystem::isExistSceneMessage(id);
    }

    s32 makeMessageID(char* out, u32 size, const char* name, s32 id) {
        return snprintf(out, size, "%s%03d", name, id);
    }

    bool isExistSceneMessage(const char* name, const JMapInfoIter& iter, bool addId) {
        char id[MESSAGE_ID_BUFFER_SIZE];
        if (addId) {
            s32 index = -1;
            getJMapInfoMessageID(iter, &index);
            if (index == -1) return false;
            snprintf(id, sizeof(id), "%s%03d", name, index);
        } else {
            strncpy(id, name, sizeof(id));
        }
        MessageData* data = MessageSystem::getSceneMessageData(id, getPlacedZoneId(iter));
        return MessageSystem::isExistMessage(id, data);
    }

    const wchar_t* getMessageLine(wchar_t* pDst, u32 capacity, const wchar_t* pMessage, u32 line) {
        u32 currentLine = 0;

        while (*pMessage != 0) {
            if (currentLine == line) {
                break;
            }

            if (*pMessage == 0xE) {
                pMessage++;
                MessageEditorMessageTag tag(pMessage);
                pMessage += tag.getSkipLength();

                if (tag.isGroupTagId(1, 1)) {
                    break;
                }
            } else {
                if (*pMessage == L'\n') {
                    currentLine++;
                }

                pMessage++;
            }
        }

        for (u32 count = 0; count < capacity; count++) {
            if (*pMessage == 0xE) {
                *pDst++ = *pMessage++;
                MessageEditorMessageTag tag(pMessage);

                for (s32 i = 0; i < static_cast< s32 >(tag.getSkipLength()); i++) {
                    *pDst = *pMessage;
                    pMessage++;
                    pDst++;
                }
            }

            if (*pMessage == L'\n' || *pMessage == 0) {
                break;
            }

            *pDst = *pMessage;

            if (count < capacity - 1) {
                pDst++;
                pMessage++;
            }
        }

        *pDst = 0;
        return pDst;
    }

    s32 countMessageLine(const wchar_t* pMessage) {
        s32 count = 1;

        while (*pMessage != 0) {
            if (*pMessage == 0xE) {
                pMessage++;
                MessageEditorMessageTag tag(pMessage);
                pMessage += tag.getSkipLength();

                if (tag.isGroupTagId(1, 1)) {
                    break;
                }
            } else {
                if (*pMessage == L'\n') {
                    count++;
                }

                pMessage++;
            }
        }

        return count;
    }

    s32 countMessageChar(const wchar_t* pMessage) {
        if (pMessage == nullptr) {
            return 0;
        }

        s32 count = 0;

        while (*pMessage != 0) {
            if (*pMessage == 0xE) {
                pMessage++;
                MessageEditorMessageTag tag(pMessage);
                pMessage += tag.getSkipLength();

                if (tag.getGroup() == 3) {
                    count++;
                } else if (tag.getGroup() == 6) {
                    count += countMessageFigure(*reinterpret_cast< const s32* >(tag.getParamPtr(0)));
                } else if (tag.getGroup() == 5) {
                    count += 3;
                }

                else if (tag.getGroup() == 7) {
                    count += countMessageChar(*reinterpret_cast< const wchar_t* const* >(tag.getParamPtr(0)));
                } else if (tag.isGroupTagId(1, 1)) {
                    break;
                } else if (tag.getGroup() == 10) {
                    count += tag.getParam16(0);
                }
            } else {
                pMessage++;
                count++;
            }
        }

        return count;
    }

    s32 countMessageFigure(s32 value) {
        u32 magnitude = MR::abs(value);
        s32 count = 1;

        while ((magnitude /= 10) != 0) {
            count++;
        }

        return count;
    }

    const wchar_t* getNextMessagePage(const wchar_t* pMessage) {
        while (*pMessage != 0) {
            if (*pMessage == 0xE) {
                pMessage++;
                MessageEditorMessageTag tag(pMessage);
                pMessage += tag.getSkipLength();

                if (tag.isGroupTagId(1, 1)) {
                    if (*pMessage == L'\n') {
                        pMessage++;
                    }

                    return pMessage;
                }
            } else {
                pMessage++;
            }
        }

        return nullptr;
    }

    const wchar_t* getGalaxyNameOnCurrentLanguage(const char* pGalaxyName) {
        char messageId[MESSAGE_ID_BUFFER_SIZE];
        snprintf(messageId, sizeof(messageId), "GalaxyName_%s", pGalaxyName);

        return getGameMessageDirect(messageId);
    }

    const wchar_t* getGalaxyNameShortOnCurrentLanguage(const char* pGalaxyName) {
        char messageId[MESSAGE_ID_BUFFER_SIZE];
        snprintf(messageId, sizeof(messageId), "GalaxyNameShort_%s", pGalaxyName);

        return getGameMessageDirect(messageId);
    }

    const wchar_t* getScenarioNameOnCurrentLanguage(const char* galaxy, s32 scenario) {
        char id[MESSAGE_ID_BUFFER_SIZE];
        if (isValidStageName(galaxy)) {
            GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(galaxy);
            if (accessor.isGreenStar(scenario)) {
                snprintf(id, sizeof(id), "ScenarioName_GreenStar%d", scenario - accessor.getPowerStarNum() / 2);
                return getGameMessageDirect(id);
            }
        }
        snprintf(id, sizeof(id), "ScenarioName_%s%d", galaxy, scenario);
        return getGameMessageDirect(id);
    }

    void getLayoutMessageID(char* pDst, const char* pSuperMessageId, const char* pSubMessageId) {
        snprintf(pDst, MESSAGE_ID_BUFFER_SIZE, "%s_%s", pSuperMessageId, pSubMessageId);
    }

    void makeCometMessageID(char* pDst, u32 bufferSize, const char* pCometName) {
        snprintf(pDst, bufferSize, "CometName_%s", pCometName);
    }
    bool isStoryBookPageKindA(const char* name, s32 zone) {
        TalkMessageInfo info;
        MessageData* data = MessageSystem::getSceneMessageData(name, zone);
        if (MessageSystem::getMessageDirect(&info, name, data)) {
            return info.mBalloonType == 5;
        }
        return false;
    }

    bool isStoryBookPageKindB(const char* name, s32 zone) {
        TalkMessageInfo info;
        MessageData* data = MessageSystem::getSceneMessageData(name, zone);
        if (MessageSystem::getMessageDirect(&info, name, data)) {
            return info.mBalloonType == 6;
        }
        return false;
    }

    bool isStoryBookPageKindC(const char* name, s32 zone) {
        TalkMessageInfo info;
        MessageData* data = MessageSystem::getSceneMessageData(name, zone);
        if (MessageSystem::getMessageDirect(&info, name, data)) {
            return info.mBalloonType == 7;
        }
        return false;
    }

};  // namespace MR
