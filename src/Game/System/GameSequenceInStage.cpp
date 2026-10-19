#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceInStage.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/Screen/PlayerMissLeft.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/PlayResultInStageHolder.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace NrvGameSequenceInStage {
    void GameSequenceInStageNrvMissLeft::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceInStage* >(pSpine->mExecutor)->exeMissLeft();
    }
    void GameSequenceInStageNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceInStage* >(pSpine->mExecutor)->exeWait();
    }
    void GameSequenceInStageNrvStart::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceInStage* >(pSpine->mExecutor)->exeStart();
    }
    void GameSequenceInStageNrvEnd::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceInStage* >(pSpine->mExecutor)->exeEnd();
    }

    GameSequenceInStageNrvMissLeft GameSequenceInStageNrvMissLeft::sInstance ATTRIBUTE_WEAK;
    GameSequenceInStageNrvWait GameSequenceInStageNrvWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceInStageNrvStart GameSequenceInStageNrvStart::sInstance ATTRIBUTE_WEAK;
    GameSequenceInStageNrvEnd GameSequenceInStageNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGameSequenceInStage

GameSequenceInStage::GameSequenceInStage(GameSequenceInGame* pHost) : ActorStateBase< GameSequenceInGame >("[シーケンス]ステージ中", pHost),
    mPlayResultHolder(nullptr), mPlayerMissLeft(nullptr), mIsRetry(false), mIsKillReady(false) {
    mPlayResultHolder = new PlayResultInStageHolder();
}

GameSequenceInStage::~GameSequenceInStage() {
}

void GameSequenceInStage::init() {
    initNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvWait), 0);
}

void GameSequenceInStage::appear() {
    mIsDead = false;
    if (mIsRetry) {
        mIsRetry = false;
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvMissLeft));
    } else {
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvWait));
    }
    mPlayResultHolder->clearStageResult();
    mIsKillReady = false;
}

PlayResultInStageHolder* GameSequenceInStage::getPlayResultHolder() const {
    return mPlayResultHolder;
}

void GameSequenceInStage::initPlayerMissLeft() {
    mPlayerMissLeft = new PlayerMissLeft("\x83~\x83X\x8e\x9e\x82\xcc\x83v\x83\x8c\x83""C\x83\x84\x81[\x8e""c\x8b@\x95\x5c\x8e\xa6");
    mPlayerMissLeft->initWithoutIter();
}

void GameSequenceInStage::exeMissLeft() {
    if (MR::isFirstStep(this)) {
        mPlayerMissLeft->appear();
    }
    mPlayerMissLeft->movement();
    mPlayerMissLeft->calcAnim();
    if (MR::isDead(mPlayerMissLeft)) {
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvWait));
    }
}

void GameSequenceInStage::exeWait() {
    if (GameSequenceFunction::isReadyToStartScene() != 0) {
        if (mIsKillReady) {
            kill();
        } else {
            setNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvStart));
        }
    }
}

void GameSequenceInStage::exeStart() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startScene();
        if (mPlayResultHolder->mHasRetryGalaxy) {
            GameSceneFunction::startStagePlayFirst();
            MR::forceOpenWipeCircle();
            MR::openSystemWipeWhiteFade(0x5A);
        } else {
            GameSceneFunction::startStagePlayRetry();
            MR::openSystemWipeCircleOnPlayer(-1);
        }
        if (mPlayResultHolder->mPadMode != 0) {
            MR::setStarPointerModeBase();
        }
    }
    if (GameSceneFunction::isExecAction()) {
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceInStage, GameSequenceInStageNrvEnd));
    }
}

void GameSequenceInStage::exeEnd() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidStageName(MR::getCurrentStageName())) {
            s32 scenarioNo = MR::getCurrentOrSelectedScenarioNo();
            GameDataFunction::onGalaxyScenarioFlagAlreadyVisited(MR::getCurrentStageName(), scenarioNo);
        }
        if (mPlayResultHolder->mHasRetryGalaxy) {
            mPlayResultHolder->mPlayTime = 0;
        }
    }
    if (!GameSceneFunction::isExecPauseMenu() && !MR::isPowerStarGetDemoActive()) {
        GameSequenceFunction::incPlayTimeInStage();
    }
}

void GameSequenceInStage::killPlayerMissLeft() {  // was fn_804B4360
    mIsRetry = false;
    if (mPlayerMissLeft != nullptr) {
        mPlayerMissLeft->kill();
    }
}
