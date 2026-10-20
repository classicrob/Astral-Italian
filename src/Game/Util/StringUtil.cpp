#include "Game/Util/StringUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/Screen/MessageTagSkipTagProcessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <cctype>
#include <cstdarg>
#include <cstdio>
#ifdef GALAXY_NATIVE
#include <revolution/os.h>
#include <cstring>
#include <cwchar>
#else
#include <wstring.h>
#endif

#define CENTISEC_PER_SEC 100
#define SEC_PER_MIN 60
#define MIN_PER_HOUR 60
#define FRAME_PER_SEC 60
#define FRAME_PER_MIN (FRAME_PER_SEC * SEC_PER_MIN)
#define FRAME_PER_HOUR (FRAME_PER_SEC * SEC_PER_MIN * MIN_PER_HOUR)

extern "C" {
int strcasecmp(const char*, const char*);
int vswprintf(wchar_t*, size_t, const wchar_t*, va_list);
};

namespace MR {
    struct Tag {
        /* 0x00 */ u16 _0;
        /* 0x02 */ u8 mDataSize;
        /* 0x03 */ u8 _3;
        /* 0x04 */ wchar_t mBuffer[1];
    };

    struct TagHeader {
        /* 0x00 */ u8 mDataSize;
        /* 0x01 */ u8 mGroup;
        /* 0x02 */ u16 mTag;
    };

    void addFilePrefix(char* pDst, u32 size, const char* pChild, const char* pParent) {
        if (*pChild == '/') {
            pChild++;
        }

        snprintf(pDst, size, "%s/%s", pParent, pChild);
    }

    void makeDateString(wchar_t* pDst, s32 size, s32 year, s32 month, s32 day) {
        const wchar_t* pMessage = MR::getGameMessageDirect("System_Date000");

        ReplaceTagFunction::ReplaceArgs(pDst, size, pMessage, year, month, day);
    }

    void makeTimeString(wchar_t* pDst, s32 size, s32 hour, s32 minute) {
        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time002");

        ReplaceTagFunction::ReplaceArgs(pDst, size, pMessage, hour, minute);
    }

    void makePlayTimeString(wchar_t* pDst, s32 size, u32 time) {
        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time002");
        s32 minutes = time / FRAME_PER_MIN;
        ReplaceTagFunction::ReplaceArgs(pDst, size, pMessage, time / FRAME_PER_HOUR, minutes % SEC_PER_MIN);
    }

    void makeClearTimeString(wchar_t* pDst, u32 frame) {
        s32 minute;
        s32 second;
        s32 centisecond;

        if (frame >= FRAME_PER_HOUR) {
            minute = 59;
            second = 59;
            centisecond = 99;
        } else {
            s32 signedFrame = frame;

            minute = signedFrame / FRAME_PER_MIN;
            second = (signedFrame / FRAME_PER_SEC) % SEC_PER_MIN;
            centisecond = ((signedFrame % FRAME_PER_SEC) * CENTISEC_PER_SEC) / FRAME_PER_SEC;
        }

        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time000");

        ReplaceTagFunction::ReplaceArgs(pDst, 9, pMessage, 0, minute, second, centisecond);
    }

    void makeHoursMinuteSecondString(wchar_t* pDst, u32 frame) {
        s32 hour, minute, second;
        if (frame >= 1000U * FRAME_PER_HOUR) {
            hour = 999;
            minute = 59;
            second = 59;
        } else {
            hour = frame / FRAME_PER_HOUR;
            minute = (frame % FRAME_PER_HOUR) / FRAME_PER_MIN;
            second = (frame % FRAME_PER_MIN) / FRAME_PER_SEC;
        }
        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time003");
        ReplaceTagFunction::ReplaceArgs(pDst, 10, pMessage, hour, minute, second);
    }

    void makeMinuteAndSecondString(wchar_t* pDst, u32 frame) {
        s32 minute;
        s32 second;

        if (frame >= FRAME_PER_HOUR) {
            minute = 59;
            second = 59;
        } else {
            minute = frame / FRAME_PER_MIN;
            second = (frame % FRAME_PER_MIN) / FRAME_PER_SEC;
        }

        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time001");

        ReplaceTagFunction::ReplaceArgs(pDst, 16, pMessage, 0, minute, second);
    }


    char* removeExtensionString(char* pDst, u32 size, const char* pPath) {
        snprintf(pDst, size, "%s", pPath);

        char* pExtSeparator = strrchr(pDst, '.');
        char* pDirSeparator = strrchr(pDst, '/');

        if (pExtSeparator < pDirSeparator || pDirSeparator + 1 == pExtSeparator) {
            return pDirSeparator;
        }

        if (pExtSeparator != nullptr) {
            *pExtSeparator = '\0';
        }

        return pDirSeparator;
    }

    void makeRaceBestTimeString(wchar_t* pDst, int raceId) {
        MR::makeClearTimeString(pDst, getRaceBestTime(raceId));
    }

    void makeRaceCurrentTimeString(wchar_t* pDst) {
        MR::makeClearTimeString(pDst, getRaceCurrentTime());
    }

    void copyString(char* pDst, const char* pSrc, u32 num) {
        strncpy(pDst, pSrc, num);
    }

    void copyString(wchar_t* pDst, const wchar_t* pSrc, u32 num) {
        wcsncpy(pDst, pSrc, num);
    }

    wchar_t* addPictureFontCode(wchar_t* pDst, int code) {
        pDst[0] = code;
        pDst[1] = '\0';

        return &pDst[1];
    }

    inline wchar_t* addPictureFontTag(wchar_t* pDst, int tag) {
#ifdef GALAXY_NATIVE
        // Text is host-order code units natively: the size and group bytes share one unit.
        pDst[0] = 26;
        pDst[1] = static_cast< wchar_t >(sizeof(Tag) << 8 | 3);
        pDst[2] = tag - '0';
        pDst[3] = '\0';

        return &pDst[3];
#else
        Tag* pTag = reinterpret_cast< Tag* >(pDst);

        pTag->_0 = 26;
        pTag->mDataSize = sizeof(Tag);
        pTag->_3 = 3;
        pTag->mBuffer[0] = tag - '0';
        pTag->mBuffer[1] = '\0';

        return &pTag->mBuffer[1];
#endif
    }


    wchar_t* addNewLine(wchar_t* pDst) {
        pDst[0] = '\n';
        pDst[1] = '\0';

        return &pDst[1];
    }

    inline wchar_t* addNumberFontTag(wchar_t* pDst, const wchar_t* pFmt, ...) {
        *pDst++ = 0x1A;
#ifdef GALAXY_NATIVE
        // Host-order code units: unit 0 is (size << 8 | group), unit 1 is the tag.
        wchar_t* pTagHeader = pDst;
#else
        TagHeader* pTag = reinterpret_cast< TagHeader* >(pDst);
#endif
        pDst += sizeof(TagHeader) / sizeof(wchar_t);

        va_list args;
        va_start(args, pFmt);

        int num = vswprintf(pDst, 256, pFmt, args);
        va_end(args);

        pDst += num;
#ifdef GALAXY_NATIVE
        const u32 dataSize = num * sizeof(wchar_t) + sizeof(TagHeader) + sizeof(wchar_t);
        pTagHeader[0] = static_cast< wchar_t >((dataSize & 0xFF) << 8 | 10);
        pTagHeader[1] = 0;
#else
        pTag->mGroup = 10;
        pTag->mTag = 0;
        pTag->mDataSize = num * sizeof(wchar_t) + sizeof(TagHeader) + sizeof(wchar_t);
#endif
        *pDst = '\0';

        return pDst;
    }

    const char* getBasename(const char* pPath) {
        const char* pBasename = strrchr(pPath, '/');

        if (pBasename == nullptr) {
            pBasename = pPath;
        } else {
            pBasename++;
        }

        return pBasename;
    }

    void extractString(char* pDst, const char* pSrc, u32 num, u32) {
        strncpy(pDst, pSrc, num);

        pDst[num] = '\0';
    }

    int convertUTF16ToASCII(char* pDst, const wchar_t* pSrc, s32 num) {
        u32 i;

        for (i = 0; i < num - 1; pDst++, pSrc++, i++) {
#ifdef GALAXY_NATIVE
            // The Wii accepts nonzero units with a zero high byte (0x01..0xFF).
            // Native message strings hold host-order UTF-16, so inspect the unit.
            const u16 code = static_cast< u16 >(*pSrc);
            if (code == 0 || code > 0xFF) {
                break;
            }

            *pDst = static_cast< char >(code);
#else
            const char* p = reinterpret_cast< const char* >(pSrc);

            if (p[0] != '\0') {
                break;
            }

            if (p[1] == '\0') {
                break;
            }

            *pDst = p[1];
#endif
        }

        *pDst = '\0';

        return i;
    }

    int strcasecmp(const char* pStr1, const char* pStr2) {
        return ::strcasecmp(pStr1, pStr2);
    }

    bool isEqualString(const char* pStr1, const char* pStr2) {
        return strcmp(pStr1, pStr2) == 0;
    }

    bool isEqualStringCase(const char* pStr1, const char* pStr2) {
        return strcasecmp(pStr1, pStr2) == 0;
    }

    bool isEqualSubString(const char* pStr, const char* pSubStr) {
        return strstr(pStr, pSubStr) != nullptr;
    }

    bool hasStringSpace(const char* pStr) {
        return strchr(pStr, ' ') != nullptr;
    }

    bool isDigitStringTail(const char* pStr, int digitNum) {
        for (int i = 1; i <= digitNum; i++) {
            int ch = pStr[strlen(pStr) - i];

            if (isdigit(ch) == 0) {
                return false;
            }
        }

        return true;
    }

    bool isExistString(const char* pStr, const char* const* ppStr, u32 strNum) {
        for (int i = 0; i < strNum; i++) {
            if (MR::isEqualString(ppStr[i], pStr)) {
                return true;
            }
        }

        return false;
    }

    bool isNullOrEmptyString(const char* pStr) {
        return pStr == nullptr || MR::isEqualString(pStr, "");
    }

    bool isMessageEditorNextTag(const wchar_t* pStr) {
        if (*pStr != 0xE) {
            return false;
        }
        MessageEditorMessageTag groupTag(pStr + 1);
        u32 group = groupTag.getGroup();
        MessageEditorMessageTag tagTag(pStr + 1);
        u32 tag = tagTag.getTag();
        return group == 1 && tag == 1;
    }

    int getStringLengthWithMessageTag(const wchar_t* pMessage) {
        int length = 0;
        while (*pMessage != 0) {
            if (*pMessage == 0xE) {
                if (isMessageEditorNextTag(pMessage)) {
                    break;
                }
                MessageEditorMessageTag tag(pMessage + 1);
                u32 skip = static_cast<u16>(tag.mMessage[2]) / sizeof(u16) + 3;
                pMessage += skip;
                length += skip;
            }
            pMessage++;
            length++;
        }
        return length;
    }

    void scan32(const char* pSrc, const char* pSubStr, s32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%d", pDst);
    }



    void scanf32(const char* pSrc, const char* pSubStr, f32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%ff", pDst);
    }

    void scanu8x4(const char* pSrc, const char* pSubStr, u8* pDst) {
        int temp[4];

        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t{%d,%d,%d,%d}", &temp[0], &temp[1], &temp[2], &temp[3]);

        for (int i = 0; i < ARRAY_SIZE(temp); i++) {
            pDst[i] = temp[i];
        }
    }


    void scanf32x4(const char* pSrc, const char* pSubStr, f32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t{%ff,%ff,%ff,%ff}", &pDst[0], &pDst[1], &pDst[2], &pDst[3]);
    }
};  // namespace MR
