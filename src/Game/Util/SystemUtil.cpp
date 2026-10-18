#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/SystemUtil.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemFontHolder.hpp"
#include "Game/System/RenderMode.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <nw4r/lyt/layout.h>
#include <nw4r/ut/ResFont.h>
#include <revolution/types.h>
#include "Game/NameObj/NameObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemStationedArchiveLoader.hpp"
#include "Game/System/AudSystemWrapper.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/System/FunctionAsyncExecutor.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameDataTemporaryInGalaxy.hpp"
#include "Game/Util/ScreenUtil.hpp"

nw4r::ut::Font* MR::getFontOnCurrentLanguage() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->getMessageFont();
}

nw4r::ut::Font* MR::getPictureFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mPictureFont;
}

nw4r::ut::Font* MR::getMenuFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mMenuFont64;
}

nw4r::ut::Font* MR::getNumberFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mNumberFont;
}

nw4r::ut::Font* MR::getMenuFont96NW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mMenuFont96;
}

nw4r::ut::Font* MR::getStaffFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mStaffFont;
}

bool MR::hasPictureFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mPictureFont != nullptr;
}

bool MR::isScreen16Per9() {
    return MR::isAspectRatioFlag16Per9();
}

void MR::onUseVFilter() {
    MainLoopFramework::sManager->mUseVFilter = true;
}

void MR::offUseVFilter() {
    MainLoopFramework::sManager->mUseVFilter = false;
}

namespace MR {
    // PLACEHOLDER(core4): SMG2 fn_80060750 / fn_80060760 are duplicates of onUseVFilter / offUseVFilter at other addresses (LogoScene call sites); rename when the owner is identified
    void onUseVFilterDirect() {
        MainLoopFramework::sManager->mUseVFilter = true;
    }

    void offUseVFilterDirect() {
        MainLoopFramework::sManager->mUseVFilter = false;
    }
};  // namespace MR

namespace {
FunctionAsyncExecutor* getFunctionAsyncExecutor() {
    return MR::getGameSystemObjHolder()->mFunctionAsyncExecutor;
}

NameObjHolder* getSceneNameObjHolder() NO_INLINE {
    return SingletonHolder< GameSystem >::get()->mSceneController->mObjHolder;
}
};  // namespace

namespace MR {
    u32 getHashCode(const char*);
    const char* getCurrentStageName();
u32 getGameSystemObjHolder3C() {  // PLACEHOLDER(core4): SMG2 fn_800601F0 returns GameSystemObjHolder::_3C (ActionSound/AutoEffectList); rename when the owner names it
    return getGameSystemObjHolder()->_3C;
}

ParticleResourceHolder* getParticleResourceHolder() {
    return getGameSystemObjHolder()->mParticleResHolder;
}

void requestChangeArchivePlayer(bool isPlayerMario) {
    GameSystemFunction::requestChangeArchivePlayer(isPlayerMario);
}

void waitEndChangeArchivePlayer() {
    while (!GameSystemFunction::isEndChangeArchivePlayer()) {
    }
}

// NONMATCHING: SMG2 fn_800602A0 is a bare tail jump to GameSystemFunction's unnamed fn_804B8680 (lwz 0x28 + b isDone); callee not named yet.
bool isEndStationedArchiveLoader() {  // SMG2 fn_800602A0
    return SingletonHolder< GameSystem >::get()->mStationedArchiveLoader->isDone();
}

bool isScenarioSelecting() {  // SMG2 fn_80060320; PLACEHOLDER(core4): name guessed from the GameSystemSceneController call
    return SingletonHolder< GameSystem >::get()->mSceneController->isScenarioSelecting();
}

void callMethodAllSceneNameObj(NameObjMethod pMethod) {
    ::getSceneNameObjHolder()->callMethodAllObj(pMethod);
}

void suspendAllSceneNameObj() {
    ::getSceneNameObjHolder()->suspendAllObj();
}

void resumeAllSceneNameObj() {
    ::getSceneNameObjHolder()->resumeAllObj();
}

void syncWithFlagsAllSceneNameObj() {
    ::getSceneNameObjHolder()->syncWithFlags();
}

void setRandomSeedFromStageName() {
    u32 seed = getHashCode(getCurrentStageName());

    getGameSystemObjHolder()->mRandom.mSeed = seed;
}

void clearFileLoaderRequestFileInfo(bool param1) {
    getGameSystemObjHolder()->clearRequestFileInfo(param1);
}
    void startFunctionAsyncExecute(const MR::FunctorBase& rFunc, int threadPriority, const char* pThreadName) {
        ::getFunctionAsyncExecutor()->start(rFunc, threadPriority, pThreadName);
    }

    bool startFunctionAsyncExecuteOnMainThread(const MR::FunctorBase& rFunc, const char* pThreadName) {
        return ::getFunctionAsyncExecutor()->startOnMainThread(rFunc, pThreadName);
    }

    void waitForEndFunctionAsyncExecute(const char* pThreadName) {
        ::getFunctionAsyncExecutor()->waitForEnd(pThreadName);
    }

    bool isEndFunctionAsyncExecute(const char* pThreadName) {
        return ::getFunctionAsyncExecutor()->isEnd(pThreadName);
    }

    bool tryEndFunctionAsyncExecute(const char* pThreadName) {
        if (isEndFunctionAsyncExecute(pThreadName)) {
            waitForEndFunctionAsyncExecute(pThreadName);

            return true;
        }

        return false;
    }

    bool suspendAsyncExecuteThread(const char* pThreadName) {
        return trySuspendAsyncExecuteThread(pThreadName);
    }

    bool trySuspendAsyncExecuteThread(const char* pThreadName) {
        OSThread* pThread = ::getFunctionAsyncExecutor()->getOSThread(pThreadName);

        if (pThread != nullptr) {
            OSSuspendThread(pThread);

            return true;
        }

        return false;
    }

    void resumeAsyncExecuteThread(const char* pThreadName) {
        OSThread* pThread = ::getFunctionAsyncExecutor()->getOSThread(pThreadName);

        OSResumeThread(::getFunctionAsyncExecutor()->getOSThread(pThreadName));
    }

    bool isSuspendedAsyncExecuteThread(const char* pThreadName) {
        OSThread* pThread = ::getFunctionAsyncExecutor()->getOSThread(pThreadName);

        if (pThread == nullptr) {
            return false;
        }

        return OSIsThreadSuspended(pThread);
    }

    void initSceneMessage() {
        getGameSystemObjHolder()->mMessageHolder->initSceneData();
    }

    void destroySceneMessage() {
        getGameSystemObjHolder()->mMessageHolder->destroySceneData();
    }

    void resetSystemAndGameStatus() {
        GameDataFunction::resetAllGameData();
        GameSystemFunction::resetAllControllerRumble();
    }

    void stopAllSound(u32 param1) {
        getGameSystemObjHolder()->mAudioSystem->stopAllSound(param1);
    }

    void setLayoutDefaultAllocator() {
        nw4r::lyt::Layout::mspAllocator = &MR::NewDeleteAllocator::sAllocator;
    }

};  // namespace
