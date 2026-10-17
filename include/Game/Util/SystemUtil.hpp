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
    void suspendAsyncExecuteThread(const char*);
    void resumeAsyncExecuteThread(const char*);
    bool isSuspendedAsyncExecuteThread(const char*);
    void initSceneMessage();
    void destroySceneMessage();
    void resetSystemAndGameStatus();
    void stopAllSound(u32);
    void setLayoutDefaultAllocator();
    bool isDisplayEncouragePal60Window();
    JMapIdInfo* getPlayerRestartIdInfo();
    void setPlayerRestartIdInfo(const JMapIdInfo&);
};  // namespace MR
