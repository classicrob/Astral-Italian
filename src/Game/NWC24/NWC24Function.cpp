#include "Game/NWC24/NWC24Function.hpp"
#include "Game/Util.hpp"
#include "Game/NWC24/UTF16Util.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <revolution/os.h>

namespace MR {
    u32 calcWiiMailSize(const u16* pTitle, const u16* pBody, u32 pictureSize, u32 letterSize) {
        u32 mailSize = 360;
        mailSize += ((strlenUTF16(pTitle) + 1) * 2 * 4 + 2) / 3 + (strlenUTF16(pTitle) + 1) * 2 / 57 * 2 + 4;
        mailSize += ((strlenUTF16(pBody) + 1) * 2 * 4 + 2) / 3 + (strlenUTF16(pBody) + 1) * 2 / 57 * 2 + 4;
        mailSize += (pictureSize * 4 + 2) / 3 + pictureSize / 57 * 2 + 4;
        mailSize += (letterSize * 4 + 2) / 3 + letterSize / 57 * 2 + 4;

        return mailSize;
    }

    const wchar_t* getMailSender(const char* pMessageId) {
        return MR::getGameMessageDirect(pMessageId);
    }
};  // namespace MR
