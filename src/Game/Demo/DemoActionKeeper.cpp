#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Demo/DemoActionKeeper.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoTalkAnimCtrl.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

// done due to inlining
template const bool JMapInfo::getValue< s32 >(int, const char*, s32*) const;
template const bool JMapInfo::getValue< const char* >(int, const char*, const char**) const;

namespace {
    TalkMessageCtrl* findTalkMessageCtrl(LiveActor* pActor) {
        DemoExecutor* executor = DemoFunction::findDemoExecutorActive(pActor);
        if (executor != nullptr) {
            return executor->findTalkMessageCtrl(pActor);
        }

        return nullptr;
    }

    void setTalkAnimCtrlInterpole(LiveActor* pActor, s32 interpole) {
        DemoExecutor* executor = DemoFunction::findDemoExecutorActive(pActor);
        for (s32 i = 0; i < executor->mTalkAnimCtrl.size(); i++) {
            executor->mTalkAnimCtrl[i]->mInterpole = interpole;
        }
    }
};  // namespace

DemoActionInfo::DemoActionInfo() {
    mPartName = nullptr;
    mCastName = nullptr;
    mCastID = -1;
    mActionType = 1;
    mPosName = nullptr;
    mAnimName = nullptr;
    mMaxCastCount = 128;
    mCastCount = 0;
    mCastList = nullptr;
    mFunctors = nullptr;
    mNerves = nullptr;
    mCastList = new LiveActor*[128];
    mFunctors = new MR::FunctorBase*[mMaxCastCount];
    mNerves = new const Nerve*[mMaxCastCount];
    for (s32 i = 0; i < mMaxCastCount; i++) {
        mCastList[i] = nullptr;
        mFunctors[i] = nullptr;
        mNerves[i] = nullptr;
    }
}

void DemoActionInfo::registerCast(LiveActor* pActor) {
    mCastList[mCastCount] = pActor;
    mCastCount++;
}

void DemoActionInfo::registerFunctor(const LiveActor* pActor, const MR::FunctorBase& rFunctor) {
    for (s32 i = 0; i < mCastCount; i++) {
        if (mCastList[i] == pActor) {
            mFunctors[i] = rFunctor.clone(nullptr);
            return;
        }
    }
}

void DemoActionInfo::registerNerve(const LiveActor* pActor, const Nerve* pNerve) {
    for (s32 i = 0; i < mCastCount; i++) {
        if (mCastList[i] == pActor) {
            mNerves[i] = pNerve;
            return;
        }
    }
}

void DemoActionInfo::executeActionFirst() const {
    if (mCastCount > 0) {
        ::setTalkAnimCtrlInterpole(mCastList[0], 0);
    }

    LiveActor* actor;
    for (s32 i = 0; i < mCastCount; i++) {
        actor = mCastList[i];
        switch (mActionType) {
        case 3: mFunctors[i]->operator()(); break;
        case 4: actor->setNerve(mNerves[i]); break;
        case 1: actor->appear(); break;
        case 2: actor->kill(); break;
        case 5: MR::onSwitchA(actor); break;
        case 6: MR::onSwitchB(actor); break;
        case 7: MR::showModel(actor); break;
        case 8: MR::hideModel(actor); break;
        case 9: MR::tryTalkTimeKeepDemoMarioPuppetable(::findTalkMessageCtrl(actor)); break;
        case 10: MR::tryTalkTimeKeepDemoWithoutPauseMarioPuppetable(::findTalkMessageCtrl(actor)); break;
        case 11: MR::tryTalkTimeKeepDemoWithoutPauseMarioPuppetable(::findTalkMessageCtrl(actor)); break;
        case 12: MR::offSwitchA(actor); break;
        case 13: MR::offSwitchB(actor); break;
        case 14: MR::requestMovementOff(actor); break;
        case 15: MR::requestMovementOn(actor); break;
        case 16: MR::startStoryBookText(true); break;
        case 17: MR::startStoryBookText(false); break;
        case 18: {
            const char* part = DemoFunction::getCurrentDemoPartNameSub(nullptr);
            s32 duration;
            if (MR::isNullOrEmptyString(part)) {
                duration = -1;
            } else {
                duration = MR::getDemoPartTotalStep(part);
            }
            MR::startStoryBookTimedText(duration);
            break;
        }
        case 19: MR::waitStoryBookDemo(); break;
        }

        if (!MR::isNullOrEmptyString(mAnimName)) {
            MR::startAction(actor, mAnimName);
        }

        if (!MR::isNullOrEmptyString(mPosName)) {
            MR::findNamePos(mPosName, &actor->mPosition, &actor->mRotation);
        }
    }
}

void DemoActionInfo::executeActionLast() const {
    if (mCastCount > 0) {
        ::setTalkAnimCtrlInterpole(mCastList[0], 0);
    }

    for (s32 i = 0; i < mCastCount; i++) {
        LiveActor* const actor = mCastList[i];
        if (mActionType == 10 || mActionType == 18 || mActionType == 20) {
            ::setTalkAnimCtrlInterpole(actor, -1);
        }
    }
}

void DemoActionKeeper::initCast(LiveActor* pActor, const JMapInfoIter& rIter) {
    if (mNumInfos <= 0) {
        return;
    }

    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];
        if (DemoFunction::isTargetDemoCast(pActor, rIter, info->mCastName, info->mCastID)) {
            mInfoArray[i]->registerCast(pActor);
        }
    }
}

void DemoActionKeeper::registerFunctor(const LiveActor* pActor, const MR::FunctorBase& rFunctor, const char* pPartName) {
    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];

        if (info && (pPartName == nullptr || MR::isEqualString(info->mPartName, pPartName))) {
            for (s32 j = 0; j < info->mCastCount; j++) {
                if (MR::isSame(info->mCastList[j], pActor)) {
                    info->registerFunctor(pActor, rFunctor);
                }
            }
        }
    }
}

void DemoActionKeeper::registerNerve(const LiveActor* pActor, const Nerve* pNerve, const char* pPartName) {
    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];

        if (info && (pPartName == nullptr || MR::isEqualString(info->mPartName, pPartName))) {
            for (s32 j = 0; j < info->mCastCount; j++) {
                if (MR::isSame(info->mCastList[j], pActor)) {
                    info->registerNerve(pActor, pNerve);
                }
            }
        }
    }
}

void DemoActionKeeper::start() {
    if (mNumInfos > 0) {
        for (s32 i = 0; i < mNumInfos; i++) {
            DemoActionInfo* info = mInfoArray[i];
            if (info->mActionType != 16 && info->mActionType != 17 && info->mActionType != 19 && MR::isDemoSubPartActive(info->mPartName)) {
                mInfoArray[i]->executeActionFirst();
            }
        }
        mIsStoryBookStartPending = 1;
    }
}

void DemoActionKeeper::update() {
    if (mNumInfos > 0) {
        if (mIsStoryBookStartPending) {
            for (s32 i = 0; i < mNumInfos; i++) {
                DemoActionInfo* info = mInfoArray[i];
                if (MR::isDemoSubPartActive(info->mPartName)) {
                    mInfoArray[i]->executeStoryBookStart();
                }
            }
            mIsStoryBookStartPending = 0;
        } else {
            updatePartActions(false);
        }
    }
}

void DemoActionKeeper::skip() {
    updatePartActions(true);
}

bool DemoActionKeeper::isRegisteredDemoActionAppear(const LiveActor* pActor) const {
    return isRegisteredDemoAction(pActor, 1);
}

bool DemoActionKeeper::isRegisteredDemoActionFunctor(const LiveActor* pActor) const {
    return isRegisteredDemoAction(pActor, 3);
}

bool DemoActionKeeper::isRegisteredDemoActionNerve(const LiveActor* pActor) const {
    return isRegisteredDemoAction(pActor, 4);
}

bool DemoActionKeeper::isRegisteredDemoAction(const LiveActor* pActor, s32 actionType) const {
    DemoActionInfo* info;
    const s32 count = mNumInfos;
    for (s32 i = 0; i < count; i++) {
        info = mInfoArray[i];
        for (s32 j = 0; j < info->mCastCount; j++) {
            if (pActor == info->mCastList[j] && info->mActionType == actionType) {
                return true;
            }
        }
    }

    return false;
}

DemoActionKeeper::DemoActionKeeper(const DemoExecutor* pExector) {
    mDemoExecutor = pExector;
    mNumInfos = 0;
    mInfoArray = nullptr;
    mIsStoryBookStartPending = 0;
    JMapInfo* jmap = nullptr;
    mNumInfos = DemoFunction::createSheetParser(mDemoExecutor, "Action", &jmap);
    mInfoArray = new DemoActionInfo*[mNumInfos];

    for (s32 i = 0; i < mNumInfos; i++) {
        mInfoArray[i] = new DemoActionInfo();
        DemoActionInfo* info = mInfoArray[i];
        jmap->getValue< const char* >(i, "PartName", &info->mPartName);
        jmap->getValue< const char* >(i, "CastName", &info->mCastName);
        jmap->getValue< s32 >(i, "CastID", &info->mCastID);
        jmap->getValue< s32 >(i, "ActionType", &info->mActionType);
        jmap->getValue< const char* >(i, "PosName", &info->mPosName);
        jmap->getValue< const char* >(i, "AnimName", &info->mAnimName);
    }
}

void DemoActionInfo::executeStoryBookStart() const {
    if (mActionType == 16) {
        MR::startStoryBookText(true);
    } else if (mActionType == 17) {
        MR::startStoryBookText(false);
    } else if (mActionType == 19) {
        MR::waitStoryBookDemo();
    }
}

bool DemoActionKeeper::isStoryBookPart(const char* part) const {
    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];
        if (MR::isEqualString(info->mPartName, part)) {
            if (info->mActionType == 16) return true;
            if (info->mActionType == 17) return true;
            if (info->mActionType == 19) return true;
        }
    }
    return false;
}

DemoActionInfo* DemoActionKeeper::findInfo(const LiveActor* actor, const char* part) const {
    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];
        if (MR::isEqualString(info->mPartName, part)) {
            for (s32 j = 0; j < info->mCastCount; j++) {
                if (MR::isSame(info->mCastList[j], actor)) {
                    return info;
                }
            }
        }
    }
    return nullptr;
}

void DemoActionKeeper::updatePartActions(bool skip) {
    for (s32 i = 0; i < mNumInfos; i++) {
        DemoActionInfo* info = mInfoArray[i];
        if (skip) {
            for (s32 j = 0; j < info->mCastCount; j++) {
                TalkMessageCtrl* talk = ::findTalkMessageCtrl(info->mCastList[j]);
                if (talk) {
                    MR::isTalkTalking(talk);
                }
            }
        }
        if (skip || MR::isDemoEndWaitCameraInterpolating()) {
            if (info->mActionType >= 9 && info->mActionType <= 11) {
                continue;
            }
        }
        if (MR::isDemoPartFirstStep(info->mPartName)) {
            mInfoArray[i]->executeActionFirst();
        } else if (MR::isDemoPartLastStep(info->mPartName)) {
            mInfoArray[i]->executeActionLast();
        }
    }
}
