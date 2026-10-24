#include "Game/NWC24/NWC24Function.hpp"
#include "Game/Util.hpp"
#include "Game/NWC24/UTF16Util.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/System/GameDataFunction.hpp"
#include <revolution/os.h>

// Wii Message Board monthly send limit (bytes); the target loads it from memory.
static u32 cWiiMailLimit = 0x61A80U;

namespace {
    bool isLastUpdateToday(s64* pTime) {
        OSCalendarTime nowTime;
        OSCalendarTime lastTime;
        OSTicksToCalendarTime(GameDataFunction::getSysConfigFileTimeSent(), &lastTime);
        OSTime now = OSGetTime();
        OSTicksToCalendarTime(now, &nowTime);
        if (pTime != nullptr) {
            *pTime = now;
        }
        return lastTime.year == nowTime.year && lastTime.mon == nowTime.mon && lastTime.mday == nowTime.mday;
    }
}  // namespace

namespace MR {
    u32 calcWiiMailSize(const u16* pTitle, const u16* pBody, u32 pictureSize, u32 letterSize) {
        u32 mailSize = 360;
        mailSize += ((strlenUTF16(pTitle) + 1) * 2 * 4 + 2) / 3 + (strlenUTF16(pTitle) + 1) * 2 / 57 * 2 + 4;
        mailSize += ((strlenUTF16(pBody) + 1) * 2 * 4 + 2) / 3 + (strlenUTF16(pBody) + 1) * 2 / 57 * 2 + 4;
        mailSize += (pictureSize * 4 + 2) / 3 + pictureSize / 57 * 2 + 4;
        mailSize += (letterSize * 4 + 2) / 3 + letterSize / 57 * 2 + 4;

        return mailSize;
    }

    bool checkWiiMailLimit(u32 mailSize) {
        if (isLastUpdateToday(nullptr) && mailSize + GameDataFunction::getSysConfigFileSentBytes() > cWiiMailLimit) {
            return false;
        }
        return true;
    }

    void updateWiiMailSentSize(u32 mailSize) {
        u32 sentBytes = GameDataFunction::getSysConfigFileSentBytes();
        s64 timeSent;
        if (!isLastUpdateToday(&timeSent)) {
            sentBytes = 0;
        }
        sentBytes += mailSize;
        GameDataFunction::setSysConfigFileTimeSent(timeSent);
        GameDataFunction::setSysConfigFileSentBytes(sentBytes);
    }

    const wchar_t* getMailSender(const char* pMessageId) {
        return MR::getGameMessageDirect(pMessageId);
    }
};  // namespace MR
