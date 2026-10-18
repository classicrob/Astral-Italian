class ResourceCsvGroupHolder;
#pragma once

#include "Game/Util/ObjUtil.hpp"

#include <revolution/types.h>

namespace nw4r {
    namespace ut {
        class Font;
    };
};  // namespace nw4r

namespace MR {
    nw4r::ut::Font* getFontOnCurrentLanguage();
    nw4r::ut::Font* getPictureFontNW4R();
    nw4r::ut::Font* getMenuFontNW4R();
    nw4r::ut::Font* getNumberFontNW4R();
    nw4r::ut::Font* getMenuFont96NW4R();
    nw4r::ut::Font* getStaffFontNW4R();
    bool hasPictureFontNW4R();
    bool isScreen16Per9();
    void onUseVFilter();
    void offUseVFilter();
    void onUseVFilterDirect();  // SMG2 fn_80060750; PLACEHOLDER(core4)
    void offUseVFilterDirect();  // SMG2 fn_80060760; PLACEHOLDER(core4)
}  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include "Game/Util/Functor.hpp"
class GameSystemObjHolder;
class JMapIdInfo;
class NameObj;
class ParticleResourceHolder;

namespace MR {
    GameSystemObjHolder* getGameSystemObjHolder();
    nw4r::ut::Font* getCinemaFontNW4R();
    ParticleResourceHolder* getParticleResourceHolder();
    void requestChangeArchivePlayer(bool);
    void waitEndChangeArchivePlayer();
    void callMethodAllSceneNameObj(void (NameObj::*)());
    void syncWithFlagsAllSceneNameObj();
    void setRandomSeedFromStageName();
    void clearFileLoaderRequestFileInfo(bool);
    void startFunctionAsyncExecute(const MR::FunctorBase&, int, const char*);
    bool startFunctionAsyncExecuteOnMainThread(const MR::FunctorBase&, const char*);
    void waitForEndFunctionAsyncExecute(const char*);
    bool isEndFunctionAsyncExecute(const char*);
    bool tryEndFunctionAsyncExecute(const char*);
    bool suspendAsyncExecuteThread(const char*);
    bool trySuspendAsyncExecuteThread(const char*);  // SMG2 fn_80060620
    void resumeAsyncExecuteThread(const char*);
    bool isSuspendedAsyncExecuteThread(const char*);
    void initSceneMessage();
    void destroySceneMessage();
    void resetSystemAndGameStatus();
    void stopAllSound(u32);
    void setLayoutDefaultAllocator();
    ResourceCsvGroupHolder* getGameSystemObjHolder3C();
    bool isEndStationedArchiveLoader();  // SMG2 fn_800602A0
    bool isScenarioSelecting();  // SMG2 fn_80060320; PLACEHOLDER(core4): name guessed
    bool isDisplayEncouragePal60Window();
    JMapIdInfo* getPlayerRestartIdInfo();
    void setPlayerRestartIdInfo(const JMapIdInfo&);
};  // namespace MR
