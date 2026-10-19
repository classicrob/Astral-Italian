#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoActionKeeper.hpp"
#include "Game/Demo/DemoPlayerKeeper.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoCastGroup.hpp"
#include "Game/Demo/DemoCastGroupHolder.hpp"
#include "Game/Demo/DemoDirector.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoExecutorFunction.hpp"
#include "Game/Demo/DemoSubPartKeeper.hpp"
#include "Game/Demo/DemoTimeKeeper.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

namespace DemoFunction {
    DemoDirector* getDemoDirector() {
        return MR::getSceneObj< DemoDirector >(SceneObj_DemoDirector);
    }

    DemoCastGroupHolder* getDemoCastSubGroupHolder() {
        return getDemoDirector()->mCastSubGroupHolder;
    }

    bool isDemoActionAnimRegistered(const LiveActor* actor) {
        DemoActionKeeper* keeper = getDemoDirector()->mExecutor->mActionKeeper;
        DemoActionInfo* info = keeper->findInfo(actor, getCurrentDemoPartNameMain(nullptr));
        return !MR::isNullOrEmptyString(info ? info->mAnimName : nullptr);
    }

    bool isDemoPlayerAnimRegistered() {
        DemoPlayerKeeper* keeper = getDemoDirector()->mExecutor->mPlayerKeeper;
        const DemoPlayerInfo* info = keeper->findPlayerInfo(getCurrentDemoPartNameMain(nullptr));
        if (info) return !MR::isNullOrEmptyString(info->mBckName);
        return false;
    }

    void registerDemoSimpleCastAllFunction(LiveActor* pActor) {
        return getDemoDirector()->registerDemoSimpleCast(pActor);
    }

    void registerDemoSimpleCastAllFunction(LayoutActor* pActor) {
        return getDemoDirector()->registerDemoSimpleCast(pActor);
    }

    void registerDemoSimpleCastAllFunction(NameObj* pObj) {
        return getDemoDirector()->registerDemoSimpleCast(pObj);
    }

    void registerDemoActionFunctorFunction(const LiveActor* pActor, const MR::FunctorBase& rFunctor, const char* pName) {
        DemoExecutorFunction::registerDemoActionFunction(findDemoExecutor(pActor), pActor, rFunctor, pName);
    }

    void registerDemoActionFunctorFunction(const LiveActor* pActor, const MR::FunctorBase& rFunctor, const char* pDemoName, const char* pName) {
        DemoExecutorFunction::registerDemoActionFunction(findDemoExecutor(pDemoName), pActor, rFunctor, pName);
    }

    void registerDemoActionNerveFunction(const LiveActor* pActor, const Nerve* pNerve, const char* pName) {
        DemoExecutorFunction::registerDemoActionNerve(findDemoExecutor(pActor), pActor, pNerve, pName);
    }

    void registerDemoActionNerveFunction(const LiveActor* pActor, const Nerve* pNerve, const char* pDemoName, const char* pName) {
        DemoExecutorFunction::registerDemoActionNerve(findDemoExecutor(pDemoName), pActor, pNerve, pName);
    }

    bool tryStartDemoRegistered(LiveActor* pActor, const char* pPartName) {
        DemoExecutor* executor = findDemoExecutor(pActor);
        if (executor == nullptr) {
            return false;
        }
        if (pPartName != nullptr) {
            return executor->tryStartProperDemoSystemPart(pPartName);
        } else {
            return executor->tryStartProperDemoSystem();
        }
    }

    bool tryStartDemoRegisteredMarioPuppetable(LiveActor* pActor, const char* pPartName) {
        DemoExecutor* executor = findDemoExecutor(pActor);
        if (executor == nullptr) {
            return false;
        }
        if (pPartName != nullptr) {
            return executor->tryStartDemoSystemPart(pPartName, 2);
        } else {
            return MR::tryStartTimeKeepDemoMarioPuppetable(executor, executor->mName, nullptr);
        }
    }

    void registerDemoExecutor(DemoExecutor* pExecutor) {
        getDemoDirector()->_18->registerObj(pExecutor);
    }

    DemoExecutor* findDemoExecutor(const char* pDemoName) {
        DemoCastGroup* group = getDemoDirector()->_18->findCastGroup(pDemoName);
        if (group == nullptr) {
            return nullptr;
        }
        return static_cast< DemoExecutor* >(group);
    }

    DemoExecutor* findDemoExecutor(const LiveActor* pActor) {
        DemoExecutor* executor;
        for (s32 i = 0; i < getDemoDirector()->_18->getObjNum(); i++) {
            executor = static_cast< DemoExecutor* >(getDemoDirector()->_18->getCastGroup(i));
            if (DemoExecutorFunction::isRegisteredDemoCast(executor, pActor)) {
                return executor;
            }
        }
        return nullptr;
    }

    DemoExecutor* findDemoExecutorActive(const LiveActor* pActor) {
        DemoExecutor* executor;
        for (s32 i = 0; i < getDemoDirector()->_18->getObjNum(); i++) {
            executor = static_cast< DemoExecutor* >(getDemoDirector()->_18->getCastGroup(i));
            if (DemoExecutorFunction::isRegisteredDemoCast(executor, pActor) && MR::isDemoActive(executor->mName)) {
                return executor;
            }
        }
        return nullptr;
    }

    bool isStoryBookActionActive() {
        if (!MR::isExistSceneObj(0x89)) return false;
        LiveActor* actor = MR::getSceneObj<LiveActor>(0x89);
        DemoExecutor* executor = getDemoDirector()->mExecutor;
        if (!executor) return false;
        if (executor->mIsStoryBookStarting) return false;
        DemoActionKeeper* keeper = executor->mActionKeeper;
        DemoActionInfo* info = keeper->findInfo(actor, getCurrentDemoPartNameMain(nullptr));
        return info ? info->mActionType == 19 : false;
    }

    bool isDemoCast(const DemoExecutor* pExecutor, const LiveActor* pActor) {
        return DemoExecutorFunction::isRegisteredDemoCast(pExecutor, pActor);
    }

    bool isExistDemoPart(const DemoExecutor* pExecutor, const char* pPartName) {
        return DemoExecutorFunction::isExistDemoPart(pExecutor, pPartName);
    }
};  // namespace DemoFunction

namespace {
    DemoTimeKeeper* getCurrentTimeKeeper() NO_INLINE {
        return DemoFunction::getDemoDirector()->mExecutor->mTimeKeeper;
    }

    DemoSubPartKeeper* getCurrentSubPartKeeper() NO_INLINE {
        return DemoFunction::getDemoDirector()->mExecutor->mSubPartKeeper;
    }

    // NONMATCHING: equivalent null-guard block ordering and regalloc
    bool isCurrentMainPart(const char* pPartName) NO_INLINE {
        DemoTimeKeeper* timeKeeper = getCurrentTimeKeeper();
        if (!timeKeeper->mSubPartInfos) return false;
        return MR::isEqualString(pPartName, ::getCurrentTimeKeeper()->mSubPartInfos->mPartName);
    }
};  // namespace

namespace DemoFunction {

    bool isDemoPartActiveFunction(const char* pPartName) {
        if (!MR::isTimeKeepDemoActive()) {
            return false;
        }
        if (::isCurrentMainPart(pPartName)) {
            return true;
        }
        return ::getCurrentSubPartKeeper()->isDemoPartActive(pPartName);
    }

    s32 getDemoPartStepFunction(const char* pPartName) {
        if (::isCurrentMainPart(pPartName)) {
            return ::getCurrentTimeKeeper()->mCurrentStep;
        } else {
            return ::getCurrentSubPartKeeper()->getDemoPartStep(pPartName);
        }
    }

    s32 getDemoPartTotalStepFunction(const char* pPartName) {
        if (::isCurrentMainPart(pPartName)) {
            return ::getCurrentTimeKeeper()->mSubPartInfos[0].mTotalStep;
        } else {
            return ::getCurrentSubPartKeeper()->getDemoPartTotalStep(pPartName);
        }
    }

    // NONMATCHING: regalloc and equivalent equality operand order
    bool isDemoSubPartActiveFunction(const char* name) {
        if (name) return MR::isEqualString(name, ::getCurrentTimeKeeper()->mMainPartInfos->mPartName);
        DemoTimePartInfo* current = ::getCurrentTimeKeeper()->mMainPartInfos;
        return current == ::getCurrentTimeKeeper()->mSubPartInfos;
    }

    bool isDemoSubPartLastStep() {
        return ::getCurrentTimeKeeper()->isLastPart(false);
    }

    bool isDemoLastPartLastStep() {
        if (isMainPartLastStep()) return MR::isDemoSubPartLastStep();
        return false;
    }

    bool isMainPartLastStep() {
        if (!MR::isTimeKeepDemoActive()) return false;
        s32 step = ::getCurrentTimeKeeper()->mCurrentStep;
        if (step == -1) return false;
        return step == ::getCurrentTimeKeeper()->mSubPartInfos->mTotalStep - 1;
    }

    bool isDemoSuspendOrLastPartLastStep() {
        if (!MR::isTimeKeepDemoActive()) {
            return false;
        }
        DemoDirector* director = getDemoDirector();
        if (director->mExecutor == nullptr) {
            return false;
        }
        DemoTimeKeeper* timekeeper = director->mExecutor->mTimeKeeper;
        if (timekeeper->isExistSuspendFlagCurrentPart()) {
            return timekeeper->isCurrentDemoPartLastStep();
        }
        return isDemoLastPartLastStep();
    }

    const char* getCurrentDemoPartNameMain(const char* pDemoName) {
        DemoExecutor* executor = pDemoName ? findDemoExecutor(pDemoName) : getDemoDirector()->mExecutor;
        if (executor == nullptr) {
            return nullptr;
        }
        DemoTimePartInfo* part = &executor->mTimeKeeper->mSubPartInfos[0];
        if (part != nullptr) {
            return part->mPartName;
        }
        return nullptr;
    }

    const char* getCurrentDemoPartNameSub(const char* name) {
        DemoExecutor* executor = name ? findDemoExecutor(name) : getDemoDirector()->mExecutor;
        if (!executor) return nullptr;
        DemoSubPartKeeper* keeper = executor->mSubPartKeeper;
        for (s32 i = 0; i < keeper->mNumSubPartInfos; i++) {
            DemoSubPartInfo* info = &keeper->mSubPartInfos[i];
            if (keeper->isDemoPartActive(info->mSubPartName)) return info->mSubPartName;
        }
        return nullptr;
    }

    bool isTargetDemoCast(LiveActor* pActor, const JMapInfoIter& rIter, const char* pActorName, s32 id) {
        if (pActorName == nullptr) {
            return false;
        }
        if (!MR::isName(pActor, pActorName)) {
            return false;
        }
        if (id >= 0 && MR::getDemoCastID(rIter) != id) {
            return false;
        }
        return true;
    }

    bool isRegisteredDemoActionAppear(const LiveActor* pActor) {
        return DemoExecutorFunction::isRegisteredDemoActionAppear(DemoFunction::findDemoExecutor(pActor), pActor);
    }

    bool isRegisteredDemoActionFunctor(const LiveActor* pActor) {
        return DemoExecutorFunction::isRegisteredDemoActionFunctor(DemoFunction::findDemoExecutor(pActor), pActor);
    }

    bool isRegisteredDemoActionFunctor(const LiveActor* pActor, const char* pDemoName) {
        return DemoExecutorFunction::isRegisteredDemoActionFunctor(DemoFunction::findDemoExecutor(pDemoName), pActor);
    }

    bool isRegisteredDemoActionNerve(const LiveActor* pActor) {
        return DemoExecutorFunction::isRegisteredDemoActionNerve(DemoFunction::findDemoExecutor(pActor), pActor);
    }

    void requestDemoCastMovementOn(LiveActor* pActor) {
        MR::requestMovementOn(pActor);
        if (pActor->mEffectKeeper != nullptr) {
            MR::Effect::requestMovementOn(pActor->mEffectKeeper);
        }
    }

    bool isRegisteredDemoCast(const LiveActor* pActor, const char* pDemoName) {
        DemoExecutor* executor = findDemoExecutor(pDemoName);
        if (executor != nullptr) {
            return DemoExecutorFunction::isRegisteredDemoCast(executor, pActor);
        }
        return false;
    }

    void pauseTimeKeepDemo(LiveActor* pActor) {
        return DemoExecutorFunction::pauseTimeKeepDemo(DemoFunction::findDemoExecutorActive(pActor));
    }

    void resumeTimeKeepDemo(LiveActor* pActor) {
        return DemoExecutorFunction::resumeTimeKeepDemo(DemoFunction::findDemoExecutorActive(pActor));
    }

    bool isPauseTimeKeepDemo(LiveActor* pActor) {
        return DemoExecutorFunction::isPauseTimeKeepDemo(DemoFunction::findDemoExecutorActive(pActor));
    }

    void pauseTimeKeepDemo(const char* pDemoName) {
        return DemoExecutorFunction::pauseTimeKeepDemo(DemoFunction::findDemoExecutor(pDemoName));
    }

    void resumeTimeKeepDemo(const char* pDemoName) {
        return DemoExecutorFunction::resumeTimeKeepDemo(DemoFunction::findDemoExecutor(pDemoName));
    }

    bool isPauseTimeKeepDemo(const char* pDemoName) {
        return DemoExecutorFunction::isPauseTimeKeepDemo(DemoFunction::findDemoExecutor(pDemoName));
    }

    bool tryCreateDemoTalkAnimCtrlForActor(LiveActor* pActor, const char* pFileBaseName, const char* a3) {
        DemoExecutor* executor = findDemoExecutor(pActor);
        return DemoExecutorFunction::tryCreateDemoTalkAnimCtrlForActor(executor, pActor, pFileBaseName, a3);
    }

    bool tryCreateDemoTalkAnimCtrlForActorDirect(LiveActor* pActor, const char* pDemoName, const char* pFileBaseName, const char* a4) {
        DemoExecutor* executor = findDemoExecutor(pDemoName);
        return DemoExecutorFunction::tryCreateDemoTalkAnimCtrlForActor(executor, pActor, pFileBaseName, a4);
    }

    bool tryCreateDemoTalkAnimCtrlForScene(LiveActor* pActor, const JMapInfoIter& rIter, const char* pFileBaseName, const char* a4, s32 a5, s32 a6) {
        DemoExecutor* executor = findDemoExecutor(pActor);
        return DemoExecutorFunction::tryCreateDemoTalkAnimCtrlForScene(executor, pActor, rIter, pFileBaseName, a4, a5, a6);
    }

    bool tryCreateDemoTalkAnimCtrlForSceneDirect(LiveActor* pActor, const char* pDemoName, const JMapInfoIter& rIter, const char* pFileBaseName,
                                                 const char* a5, s32 a6, s32 a7) {
        DemoExecutor* executor = findDemoExecutor(pDemoName);
        return DemoExecutorFunction::tryCreateDemoTalkAnimCtrlForScene(executor, pActor, rIter, pFileBaseName, a5, a6, a7);
    }

    void registerDemoTalkMessageCtrl(LiveActor* pActor, TalkMessageCtrl* pTalkCtrl) {
        DemoExecutor* executor = findDemoExecutor(pActor);
        executor->addTalkMessageCtrl(pActor, pTalkCtrl);
    }

    void registerDemoTalkMessageCtrlDirect(LiveActor* pActor, TalkMessageCtrl* pTalkCtrl, const char* pDemoName) {
        DemoExecutor* executor = findDemoExecutor(pDemoName);
        DemoExecutorFunction::registerDemoTalkMessageCtrl(executor, pActor, pTalkCtrl);
    }

    void setDemoTalkMessageCtrlDirect(const LiveActor* pActor, TalkMessageCtrl* pTalkCtrl, const char* pDemoName) {
        DemoExecutor* executor = findDemoExecutor(pDemoName);
        DemoExecutorFunction::setDemoTalkMessageCtrl(executor, pActor, pTalkCtrl);
    }

    ResourceHolder** loadDemoArchive() {
        ResourceHolder** holders = new ResourceHolder*[MR::getZoneNum()];
        for (s32 i = 0; i < MR::getZoneNum(); i++) {
            char path[0x100];
            MR::makeDemoArchiveFileName(path, sizeof(path), MR::getZoneNameFromZoneId(i));
            if (MR::isFileExistOnDisc(path, false)) holders[i] = MR::createAndAddResourceHolderStationed(path);
            else holders[i] = nullptr;
        }
        return holders;
    }

    // NONMATCHING: regalloc and load scheduling
    s32 createSheetParser(const DemoExecutor* executor, const char* suffix, JMapInfo** out) {
        if (!getDemoDirector()->getDemoResourceHolder(executor->mLinkInfo.mZoneID)) return 0;
        const char* sheet = executor->mSheetName;
        JMapInfo* map = MR::tryCreateCsvParser(getDemoDirector()->getDemoResourceHolder(executor->mLinkInfo.mZoneID), "Demo%s%s.bcsv", sheet, suffix);
        *out = map;
        if (map) return map->getNumEntries();
        return map ? map->getNumEntries() : 0;
    }

    bool isTalkWaitUserInputFunction(const char*) {
        DemoExecutor* executor = getDemoDirector()->mExecutor;
        return executor ? executor->mTimeKeeper->isTalkWaitUserInput() : false;
    }

    bool isDemoPartTalk(const char* pDemoName) {
        return MR::isEqualSubString(pDemoName, "会話");
    }
};  // namespace DemoFunction
