#include "Game/Demo/DemoStartRequestUtil.hpp"
#include "Game/Demo/DemoDirector.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoStartRequestHolder.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/MapObj/ScoreAttack.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Util.hpp"

namespace {
    void setDemoStartInfoCommon(DemoStartInfo* pStartInfo, const char* pDemoName, const char* pChar2, s32 a4, DemoStartInfo::DemoType demoType,
                                DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                                DemoStartInfo::DeleteEffectType deleteEffectType);

};  // namespace

namespace DemoStartRequestUtil {

    void startDemoSystem(NameObj* pObj, const char* pDemoName, s32 a3, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                         DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType, const char* pChar2) {
        MR::getSceneNameObjMovementController()->requestStopSceneFor(MR::MovementControlType(a3), pObj);

        bool isCinemaFrame = false;
        if (frameType == DemoStartInfo::CinemaFrameType_0 && !MR::isExistSceneObj(0x89)) {
            isCinemaFrame = true;
        }

        if (demoType == DemoStartInfo::DemoType_TimeKeep) {
            DemoFunction::getDemoDirector()->startDemoTimeKeep(pObj, pDemoName, a3, isCinemaFrame, pChar2);
        } else {
            DemoFunction::getDemoDirector()->startDemoProgrammable(pObj, pDemoName, isCinemaFrame, a3);
        }

        bool isRemoteDemo = a3 == 2 || a3 == 3;
        if (isRemoteDemo) {
            if (MR::isExistSceneObj(0x35)) {
                MR::killScoreAttackResults();
            }

            if (MR::isExistSceneObj(0x26)) {
                MR::hidePlayerActionGuidance();
            }

            if (MR::isExistSceneObj(4)) {
                MR::hideCometTimer();
            }
        }

        if (isCinemaFrame) {
            MR::tryScreenToFrameCinemaFrame();
        }

        switch (pointerType) {
        case DemoStartInfo::StarPointerType_1:
            MR::startStarPointerModeDemoWithStarPointer(pObj);
            break;
        case DemoStartInfo::StarPointerType_2:
            MR::startStarPointerModeDemoWithHandPointerFinger(pObj);
            break;
        default:
            MR::startStarPointerModeDemo(pObj);
            break;
        }

        MR::requestMovementOn(pObj);
        MR::pauseOffCameraDirector();
        MR::pauseOffLensFlare();

        if (deleteEffectType == DemoStartInfo::DeleteEffectType_1) {
            MR::Effect::forceDeleteAllOneTimeEmitter();
        }

        bool isRemoteDemoEnd = a3 == 2 || a3 == 3;
        if (isRemoteDemoEnd) {
            MarioAccess::readyRemoteDemo();
        }

        MR::sendMsgToAllLiveActor(0x7f /* start-demo message */, nullptr);
        MR::deactivateDefaultGameLayout();
    }

    void startDemoSystem(LiveActor* pActor, const char* pDemoName, s32 a3, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                         DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType, const char* pChar2) {
        startDemoSystem(static_cast< NameObj* >(pActor), pDemoName, a3, demoType, frameType, pointerType, deleteEffectType, pChar2);
        MR::requestMovementOn(pActor);
    }

    void startDemoSystem(LayoutActor* pActor, const char* pDemoName, s32 a3, DemoStartInfo::DemoType demoType,
                         DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                         DemoStartInfo::DeleteEffectType deleteEffectType, const char* pChar2) {
        startDemoSystem(static_cast< NameObj* >(pActor), pDemoName, a3, demoType, frameType, pointerType, deleteEffectType, pChar2);
        MR::requestMovementOn(pActor);
    }

    bool requestStartDemo(LiveActor* pActor, const char* pDemoName, const Nerve* pDemoNerve, const Nerve* pNoDemoNerve, s32 a5,
                          DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                          DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pActor->setNerve(pDemoNerve);
            }

            startDemoSystem(pActor, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, nullptr);
            if (MR::isRegisteredEffect(pActor, nullptr)) {
                MR::pauseOffEffectAll(pActor);
            }

            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pActor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mLiveActor = pActor;
        startInfo.mNerve = pDemoNerve;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, nullptr, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pActor, pDemoName);

        return false;
    }

    bool requestStartDemo(LayoutActor* pActor, const char* pDemoName, const Nerve* pDemoNerve, const Nerve* pNoDemoNerve, s32 a5,
                          DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                          DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pActor->setNerve(pDemoNerve);
            }

            startDemoSystem(pActor, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, nullptr);
            if (MR::isRegisteredEffect(pActor, nullptr)) {
                MR::pauseOffEffectAll(pActor);
            }

            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pActor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mLayoutActor = pActor;
        startInfo.mNerve = pDemoNerve;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, nullptr, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pActor, pDemoName);

        return false;
    }

    bool requestStartDemo(NerveExecutor* pExecutor, NameObj* pObj, const char* pDemoName, const Nerve* pDemoNerve, const Nerve* pNoDemoNerve, s32 a6,
                          DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                          DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pExecutor->setNerve(pDemoNerve);
            }

            startDemoSystem(pObj, pDemoName, a6, demoType, frameType, pointerType, deleteEffectType, nullptr);
            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pExecutor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mNerveExecutor = pExecutor;
        startInfo.mNerve = pDemoNerve;
        startInfo.mRelatedObj = pObj;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, nullptr, a6, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pExecutor, pDemoName);

        return false;
    }

    bool requestStartDemo(NerveExecutor* pExecutor, LiveActor* pActor, const char* pDemoName, const Nerve* pDemoNerve, const Nerve* pNoDemoNerve,
                          s32 a6, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                          DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pExecutor->setNerve(pDemoNerve);
            }

            startDemoSystem(pActor, pDemoName, a6, demoType, frameType, pointerType, deleteEffectType, nullptr);

            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pExecutor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mNerveExecutor = pExecutor;
        startInfo.mNerve = pDemoNerve;
        startInfo.mRelatedObj = pActor;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, nullptr, a6, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pExecutor, pDemoName);

        return false;
    }

    bool requestStartDemo(NameObj* pObj, const char* pDemoName, s32 a5, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                          DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            startDemoSystem(pObj, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, nullptr);
            return true;
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mNameObj = pObj;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, nullptr, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pObj, pDemoName);

        return false;
    }

    bool requestStartTimeKeepDemo(LiveActor* pActor, const char* pDemoName, const char* pChar, const Nerve* pDemoNerve, const Nerve* pNoDemoNerve,
                                  s32 a5, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                                  DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pActor->setNerve(pDemoNerve);
            }

            startDemoSystem(pActor, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, pChar);

            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pActor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mLiveActor = pActor;
        startInfo.mNerve = pDemoNerve;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, pChar, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pActor, pDemoName);

        return false;
    }

    bool requestStartTimeKeepDemo(NerveExecutor* pExecutor, LiveActor* pActor, const char* pDemoName, const char* pChar, const Nerve* pDemoNerve,
                                  const Nerve* pNoDemoNerve, s32 a5, DemoStartInfo::DemoType demoType, DemoStartInfo::CinemaFrameType frameType,
                                  DemoStartInfo::StarPointerType pointerType, DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            if (pDemoNerve != nullptr) {
                pExecutor->setNerve(pDemoNerve);
            }

            startDemoSystem(pActor, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, pChar);

            return true;
        }

        if (pNoDemoNerve != nullptr) {
            pExecutor->setNerve(pNoDemoNerve);
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mNerveExecutor = pExecutor;
        startInfo.mNerve = pDemoNerve;
        startInfo.mRelatedObj = pActor;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, pChar, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pExecutor, pDemoName);

        return false;
    }

    bool requestStartTimeKeepDemo(NameObj* pObj, const char* pDemoName, const char* pChar, s32 a5, DemoStartInfo::DemoType demoType,
                                  DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                                  DemoStartInfo::DeleteEffectType deleteEffectType) {
        if (MR::canStartDemo()) {
            startDemoSystem(pObj, pDemoName, a5, demoType, frameType, pointerType, deleteEffectType, pChar);

            return true;
        }

        DemoStartRequestHolder* requestHolder = DemoFunction::getDemoDirector()->mStartRequestHolder;

        DemoStartInfo startInfo;
        startInfo.mNameObj = pObj;
        ::setDemoStartInfoCommon(&startInfo, pDemoName, pChar, a5, demoType, frameType, pointerType, deleteEffectType);

        requestHolder->registerStartDemoInfo(startInfo);

        requestHolder->pushRequest(pObj, pDemoName);

        return false;
    }


    void startDemo(const DemoStartInfo* pStartInfo) {
        NameObj* pStarter = getDemoStarter(*pStartInfo);
        MR::getSceneNameObjMovementController()->requestStopSceneFor(MR::MovementControlType(pStartInfo->mSceneStopType), pStarter);

        if (pStartInfo->mFrameType == DemoStartInfo::CinemaFrameType_0) {
            MR::tryScreenToFrameCinemaFrame();
        }

        switch (pStartInfo->mPointerType) {
        case DemoStartInfo::StarPointerType_1:
            MR::startStarPointerModeDemoWithStarPointer(pStarter);
            break;
        case DemoStartInfo::StarPointerType_2:
            MR::startStarPointerModeDemoWithHandPointerFinger(pStarter);
            break;
        default:
            MR::startStarPointerModeDemo(pStarter);
            break;
        }

        requestMovementOn(pStartInfo);

        if (pStartInfo->mNerve != nullptr) {
            setNerveToStarter(pStartInfo);
        }

        MR::pauseOffCameraDirector();
        MR::pauseOffLensFlare();

        if (pStartInfo->mDeleteEffectType == DemoStartInfo::DeleteEffectType_1) {
            MR::Effect::forceDeleteAllOneTimeEmitter();
        }

        if (pStartInfo->mLiveActor != nullptr && MR::isRegisteredEffect(pStartInfo->mLiveActor, nullptr)) {
            MR::pauseOffEffectAll(pStartInfo->mLiveActor);
        }

        if (pStartInfo->mLayoutActor != nullptr && MR::isRegisteredEffect(pStartInfo->mLayoutActor, nullptr)) {
            MR::pauseOffEffectAll(pStartInfo->mLayoutActor);
        }

        bool isRemoteDemoEnd = pStartInfo->mSceneStopType == 2 || pStartInfo->mSceneStopType == 3;
        if (isRemoteDemoEnd) {
            MarioAccess::readyRemoteDemo();
        }

        startDemoExecutorIfExist(*pStartInfo);
        MR::sendMsgToAllLiveActor(0x7f /* start-demo message */, nullptr);
        MR::deactivateDefaultGameLayout();
    }

    void startDemo(DemoStartRequestHolder* pStartRequestHolder) {
        if (pStartRequestHolder->getCurrentInfo() != nullptr) {
            startDemo(pStartRequestHolder->getCurrentInfo());
        }
    }

    bool popStartDemoRequest(DemoStartRequestHolder* pStartRequestHolder) {
        if (pStartRequestHolder->getCurrentInfo() == nullptr) {
            return false;
        }

        pStartRequestHolder->popRequest();
        return true;
    }

};  // namespace DemoStartRequestUtil

namespace {
    inline void setDemoStartInfoCommon(DemoStartInfo* pStartInfo, const char* pDemoName, const char* pChar2, s32 a4, DemoStartInfo::DemoType demoType,
                                       DemoStartInfo::CinemaFrameType frameType, DemoStartInfo::StarPointerType pointerType,
                                       DemoStartInfo::DeleteEffectType deleteEffectType) {
        pStartInfo->mDemoName = pDemoName;
        pStartInfo->mDemoType = demoType;
        pStartInfo->mSceneStopType = a4;
        pStartInfo->mFrameType = frameType;
        pStartInfo->mPointerType = pointerType;
        pStartInfo->mDeleteEffectType = deleteEffectType;
        pStartInfo->mDemoExecutor = DemoFunction::findDemoExecutor(pDemoName);
        pStartInfo->mSubName = pChar2;
    }
};  // namespace

namespace DemoStartRequestUtil {
    bool isExistStartDemoRequest(const DemoStartRequestHolder* pStartRequestHolder) {
        return pStartRequestHolder->isExistRequest();
    }

    NameObj* getDemoStarter(const DemoStartInfo& rStartInfo) {
        if (rStartInfo.mLiveActor != nullptr) {
            return rStartInfo.mLiveActor;
        }

        if (rStartInfo.mLayoutActor != nullptr) {
            return rStartInfo.mLayoutActor;
        }

        if (rStartInfo.mNameObj != nullptr) {
            return rStartInfo.mNameObj;
        }

        if (rStartInfo.mNerveExecutor != nullptr) {
            return rStartInfo.mRelatedObj;
        }

        return nullptr;
    }

    void setNerveToStarter(const DemoStartInfo* pStartInfo) {
        if (pStartInfo->mLiveActor != nullptr) {
            pStartInfo->mLiveActor->setNerve(pStartInfo->mNerve);
            return;
        }

        if (pStartInfo->mLayoutActor != nullptr) {
            pStartInfo->mLayoutActor->setNerve(pStartInfo->mNerve);
            return;
        }

        if (pStartInfo->mNerveExecutor != nullptr) {
            pStartInfo->mNerveExecutor->setNerve(pStartInfo->mNerve);
        }
    }

    void requestMovementOn(const DemoStartInfo* pStartInfo) {
        if (pStartInfo->mLiveActor != nullptr) {
            MR::requestMovementOn(pStartInfo->mLiveActor);
        }

        if (pStartInfo->mLayoutActor != nullptr) {
            MR::requestMovementOn(pStartInfo->mLayoutActor);
        }

        if (pStartInfo->mNameObj != nullptr) {
            MR::requestMovementOn(pStartInfo->mNameObj);
        }

        if (pStartInfo->mRelatedObj != nullptr) {
            MR::requestMovementOn(pStartInfo->mRelatedObj);
        }
    }

    bool isEmpty(const DemoStartInfo* pStartInfo) {
        if (pStartInfo->mLiveActor != nullptr) {
            return false;
        }

        if (pStartInfo->mLayoutActor != nullptr) {
            return false;
        }

        if (pStartInfo->mNerveExecutor != nullptr) {
            return false;
        }

        if (pStartInfo->mNameObj != nullptr) {
            return false;
        }

        return true;
    }

    bool startDemoExecutorIfExist(const DemoStartInfo& rStartInfo) {
        if (rStartInfo.mDemoExecutor == nullptr || rStartInfo.mDemoType != DemoStartInfo::DemoType_TimeKeep) {
            return false;
        }

        NameObj* pStarter = getDemoStarter(rStartInfo);
        if (pStarter == nullptr) {
            pStarter = DemoFunction::getDemoDirector()->mStartRequestHolder->mProxyObj;
        }

        DemoFunction::getDemoDirector()->startDemoExecutor(pStarter, rStartInfo.mDemoName, rStartInfo.mSceneStopType, nullptr);
        return true;
    }

};  // namespace DemoStartRequestUtil
