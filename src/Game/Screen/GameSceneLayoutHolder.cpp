#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/ChipHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/CameraInfo.hpp"
#include "Game/Screen/DestroyCounter.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Screen/CounterLayoutController.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Screen/MarioMeter.hpp"
#include "Game/Screen/MarioSubMeter.hpp"
#include "Game/Screen/MissLayout.hpp"
#include "Game/Screen/NoteCounter.hpp"
#include "Game/Screen/OneUpBoard.hpp"
#include "Game/Screen/PurpleCoinCounter.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

// NONMATCHING: constructor string-base and stage guard register allocation.
GameSceneLayoutHolder::GameSceneLayoutHolder()
    : NameObj("ゲームシーンのレイアウト保持"), mInformationMessage(nullptr), mYesNoLayout(nullptr), mCameraInfo(nullptr),
      mMarioMeter(nullptr), mMarioSubMeter(nullptr), mMissLayout(nullptr), mOneUpBoard(nullptr), mNoteCounter(nullptr),
      mCounterLayoutCtrl(nullptr), mPurpleCoinCounter(nullptr), mDestroyCounter(nullptr), mIsActiveDefaultGameLayout(false) {
    MR::joinToNameObjGroup(this, "IgnorePauseNameObj");
    mInformationMessage = new InformationMessage(true);
    mInformationMessage->initWithoutIter();
    mYesNoLayout = new YesNoLayout();
    mYesNoLayout->initWithoutIter();
    mMarioSubMeter = new MarioSubMeter("マリオサブメーター");
    mMarioSubMeter->initWithoutIter();
    mMissLayout = new MissLayout();
    mMissLayout->initWithoutIter();
    const bool skip = MR::isStageStoryBook() || GameSequenceFunction::isPadModeDreamer();
    if (skip) return;
    mIsActiveDefaultGameLayout = true;
    mCameraInfo = new CameraInfo("カメラ情報表示");
    mCameraInfo->initWithoutIter();
    mMarioMeter = new MarioMeter("マリオメーター");
    mMarioMeter->initWithoutIter();
    mOneUpBoard = new OneUpBoard();
    mOneUpBoard->initWithoutIter();
    mNoteCounter = new NoteCounter("音符カウンタ");
    mNoteCounter->initWithoutIter();
    mCounterLayoutCtrl = new CounterLayoutController();
    mCounterLayoutCtrl->initWithoutIter();
    mCounterLayoutCtrl->setHPMeter(mMarioMeter);
    if (GameSequenceFunction::isPadModeSuperDreamer()) deactivateDefaultGameLayout();
}

void GameSceneLayoutHolder::createPurpleCoinCounter() {
    mPurpleCoinCounter = new PurpleCoinCounter("パープルコインカウンタ");
    mPurpleCoinCounter->initWithoutIter();
}

void GameSceneLayoutHolder::initLifeCount(s32 lifeCount) {
    if (mMarioMeter != nullptr) mMarioMeter->initLifeCount(lifeCount);
}

void GameSceneLayoutHolder::setLifeCount(u32 lifeCount) {
    if (mMarioMeter != nullptr) {
        mMarioMeter->setLifeCount(lifeCount);
    }
}

void GameSceneLayoutHolder::setOxygenRatio(f32 oxygenRatio) {
    mMarioSubMeter->setWaterLifeRatio(oxygenRatio);
}

void GameSceneLayoutHolder::setBeePowerRatio(f32 beePowerRatio) {
    mMarioSubMeter->setBeeLifeRatio(beePowerRatio);
}

void GameSceneLayoutHolder::changeLifeMeterModeGround() {
    mMarioSubMeter->frameOutSubMeter();
}

void GameSceneLayoutHolder::changeLifeMeterModeWater() {
    mMarioSubMeter->activeSubMeterWater();
}

void GameSceneLayoutHolder::changeLifeMeterModeBee() {
    mMarioSubMeter->activeSubMeterBee();
}

void GameSceneLayoutHolder::requestOneUp(s32 oneUpNum) {
    if (mOneUpBoard != nullptr) {
        mOneUpBoard->popup(oneUpNum);
    }
}

void GameSceneLayoutHolder::requestPowerUpHPMeter() {
    mMarioMeter->powerUp();
}

void GameSceneLayoutHolder::requestForceAppearHPMeter() {
    if (mMarioMeter != nullptr && !GameSequenceFunction::isPadModeSuperDreamer())
        mMarioMeter->requestForceAppearHPMeter();
}

void GameSceneLayoutHolder::startMiss() {
    if (mMissLayout->isActive()) {
        return;
    }

    mMissLayout->start();
}

bool GameSceneLayoutHolder::isMissAnimEnd() const {
    return !mMissLayout->isActive();
}

void GameSceneLayoutHolder::activateDefaultGameLayout() {
    mMarioSubMeter->activate();
    bool skip = false;
    if (MR::isStageStoryBook() || GameSequenceFunction::isPadModeDreamer()) skip = true;
    if (skip) return;
    if (GameSequenceFunction::isPadModeSuperDreamer()) return;
    mIsActiveDefaultGameLayout = true;
    mCameraInfo->activate();
    if (mPurpleCoinCounter != nullptr) mPurpleCoinCounter->appear();
    MR::activateChipLayout();
    mMarioMeter->activate();
    mCounterLayoutCtrl->appear();
}

void GameSceneLayoutHolder::deactivateDefaultGameLayout() {
    mMarioSubMeter->deactivate();
    bool skip = false;
    if (MR::isStageStoryBook() || GameSequenceFunction::isPadModeDreamer()) skip = true;
    if (skip) return;
    mIsActiveDefaultGameLayout = false;
    mCameraInfo->deactivate();
    mMarioMeter->deactivate();
    if (!MR::isDead(mCounterLayoutCtrl)) mCounterLayoutCtrl->kill();
    if (mPurpleCoinCounter != nullptr && !MR::isDead(mPurpleCoinCounter)) mPurpleCoinCounter->kill();
    MR::deactivateChipLayout();
}

void GameSceneLayoutHolder::createDestroyCounter(s32 maximum) {
    mDestroyCounter = new DestroyCounter(maximum);
    mDestroyCounter->initWithoutIter();
}

DestroyCounter* GameSceneLayoutHolder::getDestroyCounter() const {
    return mDestroyCounter;
}

void GameSceneLayoutHolder::appearDestroyCounter() {
    if (mDestroyCounter != nullptr) mDestroyCounter->appear();
}

void GameSceneLayoutHolder::killDestroyCounter() {
    if (mDestroyCounter != nullptr && !MR::isDead(mDestroyCounter)) mDestroyCounter->kill();
}

PurpleCoinCounter* GameSceneLayoutHolder::getPurpleCoinCounter() const {
    return mPurpleCoinCounter;
}

namespace MR {
    GameSceneLayoutHolder* getGameSceneLayoutHolder() {
        return MR::getSceneObj< GameSceneLayoutHolder >(SceneObj_GameSceneLayoutHolder);
    }
};  // namespace MR
