#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/ActorAppearSwitchListener.hpp"
#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Map/SwitchWatcherHolder.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace MR {
    bool useStageSwitchReadA(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchA(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }

    bool initUseStageSwitchReadA(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchReadA(actor, iter);
    }

    bool useStageSwitchReadB(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchB(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }

    bool initUseStageSwitchReadB(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchReadB(actor, iter);
    }

    bool useStageSwitchReadAppear(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchAppear(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }


    bool useStageSwitchAwake(LiveActor* actor, const JMapInfoIter& iter) {
        return AwakeControlFunc::addAwakeControl(actor, iter);
    }

    // Same as useStageSwitchReadA
    bool initUseStageSwitchReadAppear(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchReadAppear(actor, iter);
    }

    bool useStageSwitchWriteA(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchA(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }

    // Same as useStageSwitchReadB
    bool initUseStageSwitchWriteA(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchWriteA(actor, iter);
    }

    bool useStageSwitchWriteB(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchB(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }

    bool initUseStageSwitchWriteB(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchWriteB(actor, iter);
    }

    bool useStageSwitchWriteDead(LiveActor* pActor, const JMapInfoIter& rIter) {
        if (!isValidInfo(rIter)) {
            return false;
        }

        if (!isExistStageSwitchDead(rIter)) {
            return false;
        }

        if (pActor->mStageSwitchCtrl == nullptr) {
            pActor->initStageSwitch(rIter);
        }

        return true;
    }

    bool initUseStageSwitchWriteDead(LiveActor* actor, const JMapInfoIter& iter) {
        return useStageSwitchWriteDead(actor, iter);
    }

    bool useStageSwitchWriteAutoDead(LiveActor* actor, const JMapInfoIter& iter) {
        bool used = useStageSwitchWriteDead(actor, iter);
        if (actor->mStageSwitchCtrl) actor->mStageSwitchCtrl->_10 = true;
        return used;
    }

    bool needStageSwitchReadA(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchReadA(pActor, rIter);
    }

    bool needStageSwitchReadB(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchReadB(pActor, rIter);
    }

    bool needStageSwitchReadAppear(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchReadAppear(pActor, rIter);
    }

    bool needStageSwitchWriteA(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchWriteA(pActor, rIter);
    }

    bool needStageSwitchWriteB(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchWriteB(pActor, rIter);
    }

    bool needStageSwitchWriteDead(LiveActor* pActor, const JMapInfoIter& rIter) {
        return useStageSwitchWriteDead(pActor, rIter);
    }

    void needStageSwitchWriteAutoDead(LiveActor* actor, const JMapInfoIter& iter) {
        useStageSwitchWriteDead(actor, iter);
        if (actor->mStageSwitchCtrl) actor->mStageSwitchCtrl->_10 = true;
    }

    bool isValidSwitchA(const LiveActor* pActor) {
        return pActor->mStageSwitchCtrl != nullptr && pActor->mStageSwitchCtrl->isValidSwitchA();
    }

    bool isValidSwitchB(const LiveActor* pActor) {
        return pActor->mStageSwitchCtrl != nullptr && pActor->mStageSwitchCtrl->isValidSwitchB();
    }

    bool isValidSwitchAppear(const LiveActor* pActor) {
        return pActor->mStageSwitchCtrl != nullptr && pActor->mStageSwitchCtrl->isValidSwitchAppear();
    }

    bool isValidSwitchDead(const LiveActor* pActor) {
        return pActor->mStageSwitchCtrl != nullptr && pActor->mStageSwitchCtrl->isValidSwitchDead();
    }

    bool isOnSwitchA(const LiveActor* pActor) {
        StageSwitchCtrl* ctrl = pActor->mStageSwitchCtrl;
        if (ctrl == nullptr || !ctrl->isValidSwitchA()) return false;
        return ctrl->isOnSwitchA();
    }

    bool isOnSwitchB(const LiveActor* pActor) {
        StageSwitchCtrl* ctrl = pActor->mStageSwitchCtrl;
        if (ctrl == nullptr || !ctrl->isValidSwitchB()) return false;
        return ctrl->isOnSwitchB();
    }

    bool isOnSwitchAppear(const LiveActor* pActor) {
        StageSwitchCtrl* ctrl = pActor->mStageSwitchCtrl;
        if (ctrl == nullptr || !ctrl->isValidSwitchAppear()) return false;
        return ctrl->isOnSwitchAppear();
    }

    void onSwitchA(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->onSwitchA();
    }

    void onSwitchB(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->onSwitchB();
    }

    void onSwitchDead(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->onSwitchDead();
    }

    void offSwitchA(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->offSwitchA();
    }

    void offSwitchB(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->offSwitchB();
    }

    void offSwitchDead(LiveActor* pActor) {
        pActor->mStageSwitchCtrl->offSwitchDead();
    }

    void syncStageSwitchAppear(LiveActor* pActor) {
        ActorAppearSwitchListener* pSwitchListener = new ActorAppearSwitchListener(pActor, true, true);
        getSwitchWatcherHolder()->joinSwitchEventListenerAppear(pActor->mStageSwitchCtrl, pSwitchListener);
    }

    // NONMATCHING: equivalent inline validity check/branch layout
    bool useStageSwitchSyncAppear(LiveActor* actor, const JMapInfoIter& iter) {
        if (isValidSwitchAppear(actor) || useStageSwitchReadAppear(actor, iter)) {
            syncStageSwitchAppear(actor);
            actor->makeActorDead();
            return true;
        }
        actor->makeActorAppeared();
        return false;
    }

    void syncStageSwitchAppearAndSync(LiveActor* actor, const JMapInfoIter& iter) {
        useStageSwitchReadAppear(actor, iter);
        syncStageSwitchAppear(actor);
    }

    bool useStageSwitchReadParam(LiveActor* actor, const JMapInfoIter& iter) {
        if (!isValidInfo(iter)) return false;
        if (!isExistStageSwitchParam(iter)) return false;
        if (actor->mStageSwitchCtrl == nullptr) actor->initStageSwitch(iter);
        return true;
    }

    bool isValidSwitchParam(const LiveActor* actor) {
        return actor->mStageSwitchCtrl != nullptr && actor->mStageSwitchCtrl->isValidSwitchParam();
    }

    bool isOnSwitchParam(const LiveActor* actor) {
        return actor->mStageSwitchCtrl->isOnSwitchParam();
    }

    void listenStageSwitchOnAppear(LiveActor* pActor, const MR::FunctorBase& rFunctor) {
        listenNameObjStageSwitchOnAppear(pActor, pActor->mStageSwitchCtrl, rFunctor);
    }

    void listenStageSwitchOnOffAppear(LiveActor* pActor, const MR::FunctorBase& rFunctor1, const MR::FunctorBase& rFunctor2) {
        listenNameObjStageSwitchOnOffAppear(pActor, pActor->mStageSwitchCtrl, rFunctor1, rFunctor2);
    }

    void listenStageSwitchOnA(LiveActor* pActor, const MR::FunctorBase& rFunctor) {
        listenNameObjStageSwitchOnA(pActor, pActor->mStageSwitchCtrl, rFunctor);
    }

    void listenStageSwitchOnOffA(LiveActor* pActor, const MR::FunctorBase& rFunctor1, const MR::FunctorBase& rFunctor2) {
        listenNameObjStageSwitchOnOffA(pActor, pActor->mStageSwitchCtrl, rFunctor1, rFunctor2);
    }

    void listenStageSwitchOnB(LiveActor* pActor, const MR::FunctorBase& rFunctor) {
        listenNameObjStageSwitchOnB(pActor, pActor->mStageSwitchCtrl, rFunctor);
    }

    void listenStageSwitchOffB(LiveActor* pActor, const MR::FunctorBase& rFunctor) {
        listenNameObjStageSwitchOffB(pActor, pActor->mStageSwitchCtrl, rFunctor);
    }

    void listenStageSwitchOnOffB(LiveActor* pActor, const MR::FunctorBase& rFunctor1, const MR::FunctorBase& rFunctor2) {
        listenNameObjStageSwitchOnOffB(pActor, pActor->mStageSwitchCtrl, rFunctor1, rFunctor2);
    }
};  // namespace MR
