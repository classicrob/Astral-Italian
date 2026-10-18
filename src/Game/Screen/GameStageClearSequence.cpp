#include "Game/Screen/GameStageClearSequence.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sClearDemoStopFrame = 10;
    static const s32 sLayoutStartStepPowerStar = 247;
    static const s32 sClearDemoWipeStepPowerStar = 340;
    static const s32 sClearDemoWipeFramePowerStar = 110;
    static const s32 sFanfareStartStepPowerStar = 49;
    static const f32 sLayoutOffsetYPowerStar = 80.0f;
    static const s32 sLayoutStartStepGrandStar = 510;
    static const s32 sClearDemoWipeStepGrandStar = 670;
    static const s32 sFanfareStartStepGrandStar = 55;
    static const f32 sLayoutOffsetYGrandStar = 20.0f;
    static const s32 sClearDemoWipeStepKoopaVs3 = 840;
    static const s32 sClearDemoWipeFrameKoopaVs3 = 60;
};  // namespace

namespace NrvGameStageClearSequence {
    NEW_NERVE(GameStageClearSequencePowerStarGetDemo, GameStageClearSequence, PowerStarGetDemo);
    NEW_NERVE(GameStageClearSequenceGrandStarGetDemo, GameStageClearSequence, GrandStarGetDemo);
};  // namespace NrvGameStageClearSequence

GameStageClearSequence::GameStageClearSequence()
    : LayoutActor("ステージクリアシーケンス", true), mEffectLayout(nullptr), mEffectFollowPos(0.0f, 0.0f) {
}

void GameStageClearSequence::init(const JMapInfoIter& rIter) {
    mEffectLayout = new SimpleEffectLayout("スター取得", "GetStar", 1, -1);
    mEffectLayout->initWithoutIter();

    MR::setFollowPos(&mEffectFollowPos, mEffectLayout, nullptr);
    MR::setFollowTypeAdd(mEffectLayout, nullptr);
    mEffectLayout->kill();

    initNerve(GET_NERVE(GameStageClearSequence, GameStageClearSequencePowerStarGetDemo));
    kill();
}

void GameStageClearSequence::startPowerStarGetDemo() {
    LayoutActor::appear();
    mEffectFollowPos.set(TVec2f(0.0f, ::sLayoutOffsetYPowerStar));
    MR::hidePane(mEffectLayout, "GetGrandStar");
    MR::showPane(mEffectLayout, "GetStar");
    setNerve(GET_NERVE(GameStageClearSequence, GameStageClearSequencePowerStarGetDemo));
}

void GameStageClearSequence::startGrandStarGetDemo() {
    LayoutActor::appear();
    mEffectFollowPos.set(TVec2f(0.0f, ::sLayoutOffsetYGrandStar));
    MR::hidePane(mEffectLayout, "GetStar");
    MR::showPane(mEffectLayout, "GetGrandStar");
    setNerve(GET_NERVE(GameStageClearSequence, GameStageClearSequenceGrandStarGetDemo));
}

void GameStageClearSequence::appearGetStarLayout() {
    mEffectLayout->appear();
    MR::startAnim(mEffectLayout, "GetStar", 0);
    MR::emitEffect(mEffectLayout, "GetStar");
}

void GameStageClearSequence::exePowerStarGetDemo() {
    AudSystem::setField834(2);
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(2);
        MR::stopScene(::sClearDemoStopFrame);
        MR::deactivateDefaultGameLayout();
    }

    if (MR::isStep(this, ::sFanfareStartStepPowerStar)) {
        MR::startStarGetJingle();
    }

    if (MR::isStep(this, ::sLayoutStartStepPowerStar)) {
        appearGetStarLayout();
    }

    if (MR::isStep(this, ::sClearDemoWipeStepPowerStar)) {
        MR::closeWipeCircle(::sClearDemoWipeFramePowerStar);
    }

    bool isWipeActive = MR::isWipeActive();

    if (MR::isGreaterStep(this, ::sClearDemoWipeStepPowerStar) && !isWipeActive) {
        GameSequenceFunction::requestChangeStageAfterStageClear();
        kill();
    }
}

void GameStageClearSequence::exeGrandStarGetDemo() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(2);
        MR::stopScene(::sClearDemoStopFrame);
        MR::deactivateDefaultGameLayout();
    }

    AudSystem::setField834(2);

    if (MR::isStep(this, ::sFanfareStartStepGrandStar)) {
        MR::startGrandStarGetJingle();
    }

    if (MR::isStep(this, ::sLayoutStartStepGrandStar)) {
        appearGetStarLayout();
    }

    if (PowerStar::isCurrentStageKoopaVs3()) {
        if (MR::isStep(this, ::sClearDemoWipeStepKoopaVs3)) {
            MR::closeSystemWipeWhiteFade(::sClearDemoWipeFrameKoopaVs3);
        }

        bool isWipeActive = MR::isSystemWipeActive();

        if (MR::isGreaterStep(this, ::sClearDemoWipeStepKoopaVs3) && !isWipeActive) {
            GameSequenceFunction::requestChangeStageAfterStageClear();
            kill();
        }
    } else {
        if (MR::isStep(this, ::sClearDemoWipeStepGrandStar)) {
            MR::closeWipeCircle();
        }

        bool isWipeActive = MR::isWipeActive();

        if (MR::isGreaterStep(this, ::sClearDemoWipeStepGrandStar) && !isWipeActive) {
            GameSequenceFunction::requestChangeStageAfterStageClear();
            kill();
        }
    }
}
