#pragma once

#include <revolution/types.h>

namespace MR {
    s32 makeGalaxyCompletionMark(wchar_t*, s32, const char*, bool);
    s32 makeGalaxyStarList(wchar_t*, s32, const char*, bool, bool);
    const wchar_t* getWorldNameOnCurrentLanguage(s32);
    s32 makeMessageID(char*, u32, const char*, s32);
    const wchar_t* getGameMessageDirect(const char*);
    void getLayoutMessageID(char*, const char*, const char*);
};

// Merged from Petari (core lane): declarations missing here
#include <revolution/types.h>

namespace MR {
    const wchar_t* getSystemMessageDirect(const char*);
    const wchar_t* getLayoutMessageDirect(const char*);
    const wchar_t* getCurrentGalaxyNameOnCurrentLanguage();
    const wchar_t* getCurrentGalaxyNameShortOnCurrentLanguage();
    const wchar_t* getCurrentScenarioNameOnCurrentLanguage();
    const wchar_t* getRaceNameOnCurrentLanguage(int);
    bool isExistGameMessage(const char*);
    const wchar_t* getMessageLine(wchar_t*, u32, const wchar_t*, u32);
    s32 countMessageLine(const wchar_t*);
    s32 countMessageChar(const wchar_t*);
    s32 countMessageFigure(s32);
    const wchar_t* getNextMessagePage(const wchar_t*);
    const wchar_t* getSceneMessageDirect(const char*, s32);
    bool isStoryBookPageKindA(const char*, s32);
    bool isStoryBookPageKindB(const char*, s32);
    bool isStoryBookPageKindC(const char*, s32);
    const wchar_t* getGalaxyNameOnCurrentLanguage(const char*);
    const wchar_t* getGalaxyNameShortOnCurrentLanguage(const char*);
    const wchar_t* getScenarioNameOnCurrentLanguage(const char*, s32);
    void makeCometMessageID(char*, u32, const char*);
};  // namespace MR
