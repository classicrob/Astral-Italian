#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoActionKeeper.hpp"
#include "Game/Demo/DemoCameraKeeper.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoPlayerKeeper.hpp"
#include "Game/Demo/DemoSoundKeeper.hpp"
#include "Game/Demo/DemoSubPartKeeper.hpp"
#include "Game/Demo/DemoTalkAnimCtrl.hpp"
#include "Game/Demo/DemoTimeKeeper.hpp"
#include "Game/Demo/DemoWipeKeeper.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#endif
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/LightUtil.hpp"

DemoExecutor::DemoExecutor(const char* pName)
    : DemoCastGroup(pName), mSheetName(), _SMG2_20(), mTimeKeeper(), mSubPartKeeper(), mPlayerKeeper(), mCameraKeeper(), mActionKeeper(), mWipeKeeper(),
      mSoundKeeper(), mSheetKeeper(), mStageSwitchCtrl(), mStarterObj(), mStartPartName(), mSceneStopType(-1), mUnused50(), mActor(), mTalkAnimCtrl(), mTalkMessageCtrl(), mIsStoryBookStarting(), _SMG2_3CD() {
}

void DemoExecutor::init(const JMapInfoIter& rIter) {
    DemoCastGroup::init(rIter);

    mSheetName = MR::getJMapInfoTimeSheetName(rIter);
    _SMG2_20 = MR::getJMapInfoDemoSkip(rIter);
    mTimeKeeper = new DemoTimeKeeper(this);
    mSubPartKeeper = new DemoSubPartKeeper(this);
    mPlayerKeeper = new DemoPlayerKeeper(this);
    mCameraKeeper = new DemoCameraKeeper(this, rIter);
    mActionKeeper = new DemoActionKeeper(this);
    mWipeKeeper = new DemoWipeKeeper(this);
    mSheetKeeper.push_back(mWipeKeeper);
    mSoundKeeper = new DemoSoundKeeper(this);
    mSheetKeeper.push_back(mSoundKeeper);
    mStageSwitchCtrl = MR::createStageSwitchCtrl(this, rIter);

    if (mStageSwitchCtrl->isValidSwitchAppear()) {
        MR::listenNameObjStageSwitchOnAppear(this, mStageSwitchCtrl, MR::FunctorV0M<DemoExecutor*, void (DemoExecutor::*)()>(this, &DemoExecutor::startProperDemoSystem));
    }

    DemoFunction::registerDemoExecutor(this);
}

void DemoExecutor::registerDemoActor(LiveActor* pActor, const JMapInfoIter& rIter) {
    DemoCastGroup::registerDemoActor(pActor, rIter);
    mCameraKeeper->initCast(pActor, rIter);
    mActionKeeper->initCast(pActor, rIter);

    for (DemoSheetKeeperBase** it = mSheetKeeper.begin(); it != mSheetKeeper.end(); it++) {
        (*it)->initCast(pActor, rIter);
    }
}

void DemoSheetKeeperBase::initCast(LiveActor*, const JMapInfoIter&) {
}

void DemoExecutor::movement() {
    if (updateStoryBook()) return;
    mTimeKeeper->update();
    if (updateKeepers(false)) return;
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::updateDemo));
    if (_SMG2_20) MR::tryEndDemoAndSetAlreadyDone(static_cast<LiveActor*>(mStarterObj), mStartPartName);
}

void DemoExecutor::start(NameObj* pParam1, const char* pParam2, s32 param3) {
    mStarterObj = pParam1;
    mStartPartName = pParam2;
    mSceneStopType = param3;

    if (MR::isStoryBookDemoName(pParam2)) {
        MR::startStoryBookTurnPage();
        mIsStoryBookStarting = true;
    } else {
        mTimeKeeper->start();
        mCameraKeeper->start();
        std::for_each(mSheetKeeper.begin(), mSheetKeeper.end(), std::mem_func(&DemoSheetKeeperBase::start));
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::startDemo));
        mIsStoryBookStarting = false;
    }

    mActor.clear();

    for (s32 i = 0; i < mGroup->getObjNum(); i++) {
        LiveActor* actor = mGroup->getActor(i);

        MR::sendMsgStartDemo(actor);
        DemoFunction::requestDemoCastMovementOn(actor);

        if (MR::isInvalidClipping(actor)) {
            continue;
        }

        MR::invalidateClipping(actor);
        mActor.push_back(actor);
    }
}

void DemoSheetKeeperBase::start() {
}

void DemoExecutor::startPart(NameObj* pParam1, const char* pParam2, const char* pParam3, s32 param4) {
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(),
                  std::binder2nd< std::mem_fun1_t< void, DemoTalkAnimCtrl, const char* >, const char* >(
                      std::mem_func(&DemoTalkAnimCtrl::setupStartDemoPart), pParam3));
    start(pParam1, pParam2, param4);
    mTimeKeeper->setStartPart(pParam3);
}

void DemoExecutor::startProperDemoSystem() {
    if (mPlayerKeeper->mNumPlayerInfos > 0) {
        MR::requestStartTimeKeepDemoMarioPuppetable(this, mName, nullptr);
    } else {
        MR::requestStartTimeKeepDemo(this, mName, nullptr);
    }
}

void DemoExecutor::startDemoSystemPart(const char* pParam1, s32 param2) {
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(),
                  std::binder2nd< std::mem_fun1_t< void, DemoTalkAnimCtrl, const char* >, const char* >(
                      std::mem_func(&DemoTalkAnimCtrl::setupStartDemoPart), pParam1));

    switch (param2) {
    case 1:
        MR::startTimeKeepDemo(this, mName, nullptr);
        break;
    case 2:
        MR::startTimeKeepDemoMarioPuppetable(this, mName, nullptr);
        break;
    }

    mTimeKeeper->setStartPart(pParam1);
}

bool DemoExecutor::tryStartProperDemoSystem() {
    if (mPlayerKeeper->mNumPlayerInfos > 0) {
        return MR::tryStartTimeKeepDemoMarioPuppetable(this, mName, nullptr);
    } else {
        return MR::tryStartTimeKeepDemo(this, mName, nullptr);
    }
}

bool DemoExecutor::tryStartDemoSystemPart(const char* pParam1, s32 param2) {
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(),
                  std::binder2nd< std::mem_fun1_t< void, DemoTalkAnimCtrl, const char* >, const char* >(
                      std::mem_func(&DemoTalkAnimCtrl::setupStartDemoPart), pParam1));

    bool result = false;

    switch (param2) {
    case 1:
        result = MR::tryStartTimeKeepDemo(this, mName, nullptr);
        break;
    case 2:
        result = MR::tryStartTimeKeepDemoMarioPuppetable(this, mName, nullptr);
        break;
    }

    if (!result) {
        return false;
    }

    mTimeKeeper->setStartPart(pParam1);
    return true;
}

bool DemoExecutor::tryStartProperDemoSystemPart(const char* pParam1) {
    if (mPlayerKeeper->mNumPlayerInfos > 0) {
        return MR::tryStartTimeKeepDemoMarioPuppetable(this, mName, pParam1);
    } else {
        return MR::tryStartTimeKeepDemo(this, mName, pParam1);
    }
}

void DemoExecutor::pause() {
    mTimeKeeper->mIsPaused = true;
}

void DemoExecutor::resume() {
    mTimeKeeper->mIsPaused = false;
}

void DemoExecutor::skip() {
    if (mTimeKeeper->mIsPaused) mTimeKeeper->mIsPaused = false;
    mTimeKeeper->update();
    while (!mTimeKeeper->isDemoEnd()) {
        mSubPartKeeper->update();
        mPlayerKeeper->update();
        mActionKeeper->skip();
        mTimeKeeper->update();
    }
    mCameraKeeper->endForce();
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::endDemo));
    end();
}

void DemoExecutor::addTalkAnimCtrl(DemoTalkAnimCtrl* pCtrl) {
    mTalkAnimCtrl.push_back(pCtrl);
}

void DemoExecutor::addTalkMessageCtrl(LiveActor* pActor, TalkMessageCtrl* pCtrl) {
    DemoTalkMessageCtrl talkMessageCtrl = {pActor, pCtrl};
    mTalkMessageCtrl.push_back(talkMessageCtrl);
}

TalkMessageCtrl* DemoExecutor::findTalkMessageCtrl(const LiveActor* pActor) const {
    for (const DemoTalkMessageCtrl* it = mTalkMessageCtrl.begin(); it != mTalkMessageCtrl.end(); it++) {
        if (it->mActor != pActor) {
            continue;
        }

        return it->mTalkMessageCtrl;
    }

    return nullptr;
}

void DemoExecutor::setTalkMessageCtrl(const LiveActor* pActor, TalkMessageCtrl* pCtrl) {
    for (DemoTalkMessageCtrl* it = mTalkMessageCtrl.begin(); it != mTalkMessageCtrl.end(); it++) {
        if (it->mActor != pActor) {
            continue;
        }
        it->mTalkMessageCtrl = pCtrl;
        break;
    }
}

bool DemoExecutor::updateKeepers(bool skipCamera) {
    if (mTimeKeeper->isDemoEnd()) {
        end();
        return true;
    }
    if (!mTimeKeeper->mIsPaused) {
        mSubPartKeeper->update();
        mPlayerKeeper->update();
        if (!skipCamera) mCameraKeeper->update();
        mActionKeeper->update();
        std::for_each(mSheetKeeper.begin(), mSheetKeeper.end(), std::mem_func(&DemoSheetKeeperBase::update));
    }
    return false;
}

void DemoSheetKeeperBase::update() {
}

bool DemoExecutor::updateStoryBookStart() {
    if (!mIsStoryBookStarting) return false;
    if (MR::isStoryBookTurnPageStart()) {
        MR::resetPointLight();
        mTimeKeeper->start();
        mCameraKeeper->start();
        mPlayerKeeper->start();
        mActionKeeper->start();
        std::for_each(mSheetKeeper.begin(), mSheetKeeper.end(), std::mem_func(&DemoSheetKeeperBase::start));
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::startDemo));
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::updateStoryBook));
        return true;
    }
    if (MR::isStoryBookTurnPageEnd()) {
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::resume));
        mIsStoryBookStarting = false;
        return true;
    }
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::pause));
    return true;
}

bool DemoExecutor::updateStoryBook() {
    if (updateStoryBookStart()) return true;
    if (_SMG2_3CD) {
        if (MR::isStoryBookTurnPageEnd()) {
            for (s32 i = 0; i < mGroup->getObjNum(); i++) MR::requestMovementOn(mGroup->getActor(i));
            MR::requestMovementOnPlayer();
            endDemoSystem();
            _SMG2_3CD = false;
        }
        return true;
    }
    if (!MR::isStoryBookDemoActive()) return false;
    if (MR::isDemoSubPartLastStep() && !MR::isDemoPartFirstStep(nullptr) && MR::isStoryBookTurningPage()) return false;
    if (MR::isStoryBookTurnPageStart()) {
        mTimeKeeper->update();
        mCameraKeeper->update();
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::updateStoryBook));
        return true;
    }
    if (MR::isStoryBookTurnPageEnd()) {
        std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::resume));
        updateKeepers(true);
        return true;
    }
    std::for_each(mTalkAnimCtrl.begin(), mTalkAnimCtrl.end(), std::mem_func(&DemoTalkAnimCtrl::pause));
    return true;
}

void DemoExecutor::end() {
    mTimeKeeper->end();
    mSubPartKeeper->end();
    mCameraKeeper->end();
    std::for_each(mSheetKeeper.begin(), mSheetKeeper.end(), std::mem_func(&DemoSheetKeeperBase::end));
    if (mActionKeeper->isStoryBookPart(mTimeKeeper->mMainPartInfos[static_cast<u32>(mTimeKeeper->mNumPartInfos - 1)].mPartName)) {
        for (s32 i = 0; i < mGroup->getObjNum(); i++) MR::requestMovementOff(mGroup->getActor(i));
        MR::requestMovementOff(MR::getPlayerDemoActor());
        MR::requestMovementOnStoryBook();
        _SMG2_3CD = true;
    } else endDemoSystem();
}

void DemoSheetKeeperBase::end() {
}

void DemoExecutor::endDemoSystem() {
    MR::endDemo(mStarterObj, mStartPartName);
    mStarterObj = nullptr;
    mStartPartName = nullptr;
    mSceneStopType = -1;
    if (mStageSwitchCtrl->isValidSwitchDead()) mStageSwitchCtrl->onSwitchDead();
    std::for_each(mActor.begin(), mActor.end(), std::ptr_fun(MR::validateClipping));
    mActor.clear();
}
