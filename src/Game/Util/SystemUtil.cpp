#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/SystemUtil.hpp"
#ifdef GALAXY_NATIVE
extern "C" void petari_os_preemption_point(void);
#endif
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
#include "Game/Util/FileUtil.hpp"
#include "Game/System/FunctionAsyncExecutor.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
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
    // fn_80060750 / fn_80060760: set MainLoopFramework::mUseAlpha (+0x18), not mUseVFilter (+0x19); used by the LogoScene call sites.
    // The symbol names (onUseVFilterDirect) are historical; the lead may rename to onUseAlpha/offUseAlpha.
    void onUseVFilterDirect() {
        MainLoopFramework::sManager->mUseAlpha = true;
    }

    void offUseVFilterDirect() {
        MainLoopFramework::sManager->mUseAlpha = false;
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
ResourceCsvGroupHolder* getGameSystemObjHolder3C() {
    return getGameSystemObjHolder()->_3C;
}

void tryDestroyUseResourceHeap() {
    if (getUseResourceHeap() != nullptr) {
        if (GameSystemFunction::isCreatedGameDataHolder()) {
            bool isLuigi = GameDataFunction::isLuigiPlayer();
            if (GameSystemFunction::isPlayerArchiveMario() != isLuigi) {
                return;
            }
        }
        removeResourceAndFileHolderIfIsEqualHeap(getUseResourceHeap());
        SingletonHolder<HeapMemoryWatcher>::get()->freeAllFileCache();
        AudInstListMgr::requestStageInstrumentRefresh();
    }
}

ParticleResourceHolder* getParticleResourceHolder() {
    return getGameSystemObjHolder()->mParticleResHolder;
}

bool requestChangeArchivePlayer(bool isPlayerMario) {
    return GameSystemFunction::requestChangeArchivePlayer(isPlayerMario);
}

void waitEndChangeArchivePlayer() {
    while (!GameSystemFunction::isEndChangeArchivePlayer()) {
#ifdef GALAXY_NATIVE
        petari_os_preemption_point();  // the change completes on other threads
#endif
    }
}

bool requestLoadStationedArchiveOthers() {  // was fn_80060290
    return GameSystemFunction::requestLoadStationedArchiveOthers();
}

bool isEndStationedArchiveLoader() {  // was fn_800602A0
    return GameSystemFunction::isEndStationedArchiveLoader();
}

bool isScenarioSelecting() {  // SMG2 fn_80060320 (name from the GameSystemSceneController call)
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

        if (pThread == nullptr) {
            return false;
        }
        OSSuspendThread(pThread);
        return true;
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
