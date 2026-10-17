#pragma once

#include <revolution.h>

namespace MR {
    /// @brief Removes the file extension from the given path.
    /// @param pDst The output pointer for the path.
    /// @param size The size of the path buffer, in bytes.
    /// @param pPath The pointer to the null-terminated path.
    /// @return The pointer to the final directory separator character.
    char* removeExtensionString(char* pDst, u32 size, const char* pPath);

    /// @brief Copies `num` characters from `pSrc` to `pDst`.
    /// @param pDst The pointer to the null-terminated character string to copy to.
    /// @param pSrc The pointer to the null-terminated character string to copy from.
    /// @param num The number of characters to copy.
    void copyString(char* pDst, const char* pSrc, u32 num);

    /// @brief Returns the final component of the given path.
    /// @param pPath The pointer to the null-terminated path.
    /// @return The pointer to the null-terminated final component of the path.
    const char* getBasename(const char* pPath);

    /// @brief Copies `num` characters from `pSrc` to `pDst` and terminates the string.
    /// @param pDst The pointer to the null-terminated character string to copy to.
    /// @param pSrc The pointer to the null-terminated character string to copy from.
    /// @param num The number of characters to copy.
    void extractString(char* pDst, const char* pSrc, u32 num, u32);

    /// @brief Compares two character strings for equivalence without regard to case sensitivity.
    /// @param pStr1 The pointer to the left-hand null-terminated character string.
    /// @param pStr2 The pointer to the right-hand null-terminated character string.
    /// @retval -1 The first string contains a character less than the corresponding character in the second string.
    /// @retval 0 The strings are equivalent.
    /// @retval 1 The second string contains a character greater than the corresponding character in the first string.
    int strcasecmp(const char* pStr1, const char* pStr2);

    /// @brief Determines if two character strings are equivalent.
    /// @param pStr1 The pointer to the left-hand null-terminated character string.
    /// @param pStr2 The pointer to the right-hand null-terminated character string.
    /// @return `true` if the strings are equivalent, `false` otherwise.
    bool isEqualString(const char* pStr1, const char* pStr2);

    /// @brief Determines if two character strings are equivalent without regard to case sensitivity.
    /// @param pStr1 The pointer to the left-hand null-terminated character string.
    /// @param pStr2 The pointer to the right-hand null-terminated character string.
    /// @return `true` if the strings are equivalent, `false` otherwise.
    bool isEqualStringCase(const char* pStr1, const char* pStr2);

    /// @brief Determines if the given character string contains the given substring.
    /// @param pStr The pointer to the null-terminated character string.
    /// @param pSubStr The pointer to the null-terminated character substring.
    /// @return `true` if the string contains the substring, `false` otherwise.
    bool isEqualSubString(const char* pStr, const char* pSubStr);

    /// @brief Determines if the given character string contains a space character.
    /// @param pStr The pointer to the null-terminated character string.
    /// @return `true` if the string contains a space character, `false` otherwise.
    bool hasStringSpace(const char* pStr);

    /// @brief Determines if the pointer to the given character string is `nullptr` or contains no characters.
    /// @param pStr The pointer to the null-terminated character string.
    /// @return `true` if the pointer to the string is `nullptr` or contains no characters, `false` otherwise.
    bool isNullOrEmptyString(const char* pStr);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here


namespace MR {
    void addFilePrefix(char* pDst, u32 size, const char* pChild, const char* pParent);
    void makeDateString(wchar_t* pDst, s32 size, s32 year, s32 month, s32 day);
    void makeTimeString(wchar_t* pDst, s32 size, s32 hour, s32 minute);
    void makeClearTimeString(wchar_t* pDst, u32 frame);
    void makeMinuteAndSecondString(wchar_t* pDst, u32 frame);
    wchar_t* addNumberFontTag(wchar_t* pDst, int tag);
    void makeRaceBestTimeString(wchar_t* pDst, int raceId);
    void makeRaceCurrentTimeString(wchar_t* pDst);
    wchar_t* addPictureFontCode(wchar_t* pDst, int code);
    wchar_t* addPictureFontTag(wchar_t* pDst, int tag);
    wchar_t* addPictureFontTagPlayerIcon(wchar_t* pDst);
    wchar_t* addNewLine(wchar_t* pDst);
    int convertUTF16ToASCII(char* pDst, const wchar_t* pSrc, s32 num);
    bool isDigitStringTail(const char* pStr, int digitNum);
    bool isExistString(const char* pStr, const char* const* ppStr, u32 strNum);
    bool isMessageEditorNextTag(const wchar_t*);
    int getStringLengthWithMessageTag(const wchar_t*);
    void scan32(const char* pSrc, const char* pSubStr, s32* pDst);
    void scan16(const char* pSrc, const char* pSubStr, u16* pDst);
    void scan8(const char* pSrc, const char* pSubStr, u8* pDst);
    void scanf32(const char* pSrc, const char* pSubStr, f32* pDst);
    void scanu8x4(const char* pSrc, const char* pSubStr, u8* pDst);
    void scans16x4(const char* pSrc, const char* pSubStr, s16* pDst);
    void scanf32x4(const char* pSrc, const char* pSubStr, f32* pDst);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void copyString(wchar_t* pDst, const wchar_t* pSrc, u32 num);
};  // namespace MR
