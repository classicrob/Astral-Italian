#include "Game/Screen/ScenarioSelectLayout.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Camera/CameraContext.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/Map/ScenarioSelectStar.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Util/NativeUiObserve.hpp"
#include <petari/progress_hook.hpp>
#endif

namespace {
    const char* const cStarPaneName[] = {"Star1", "Star2", "Star3", "Star4", "Star5", "Star6"};
    const char* const cNewPaneName = "New";
    const char* const cNewGreenPaneName = "NewStar";
    const char* const cNewCometPaneName = "NewComet";
    const char* const cQuestionPaneName[] = {
        "NewGreenComet1",
        "NewGreenComet2",
        "NewGreenComet3",
    };
    const Vec cSkyTrans = {0.0f, 0.0f, -10000.0f};
    const f32 cPointingTargetRadius = 60.0f;
    const s32 cDecideFrame = 40;
    const s32 cSelectedAccelStartStep = 60;
    const s32 cSelectedAccelFrame = 150;
    const f32 cSelectedEffectRateMin = 2.0f;
    const f32 cSelectedEffectRateMax = 15.0f;
    const f32 cSelectedEffectSpeedMin = 9.0f;
    const f32 cSelectedEffectSpeedMax = 15.0f;
    const s32 cSelectedEndAnimStartStep = 60;
    const s32 cSelectedFadeOutStartStep = 180;
    const s32 cSelectedFadeOutFrame = 30;
    const s32 cSelectedFadeInFrame = 180;
    const s32 cCancelFadeOutFrame = 90;
    const f32 cSkyScale = 10.0f;
    const s32 cCometWarningWaitFrame = 15;
    const s32 cCometWarningFrame = 120;
};  // namespace

namespace {
    inline bool hasScenarioClearTime(const char* stage, s32 scenario) {
        return GameDataFunction::getScenarioClearTime(stage, scenario) != 0;
    }

    inline s32 getHiddenScenarioNum() {
        s32 normalNum = MR::getCurrentStageNormalScenarioNum();
        return MR::getCurrentStagePowerStarNum() - normalNum;
    }

    bool isCurrentStageRaceComplete(s32 scenarioNo) {
        GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
        if (!accessor.hasPowerStar(scenarioNo)) return false;
        if (accessor.isGreenStar(scenarioNo)) return false;
        return RaceManagerFunction::getRaceId(accessor.getName()) >= 0;
    }

    struct ScoreAttackScenario { const char* stage; s32 scenario; };
    const ScoreAttackScenario cScoreAttackScenario[] = {
        {"MokumokuValleyGalaxy", 2}, {"HoneyBeeVillageGalaxy", 2},
        {"UnderGroundDangeonGalaxy", 2}, {"TwisterTowerGalaxy", 3},
        {"KachikochiLavaGalaxy", 3}, {"WhiteSnowGalaxy", 3}
    };

    bool isCurrentStageScoreAttackComplete(s32 scenarioNo) {
        GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
        const char* stage = accessor.getName();
        if (!accessor.hasPowerStar(scenarioNo)) return false;
        for (u32 i = 0; i < ARRAY_SIZE(cScoreAttackScenario); i++) {
            if (scenarioNo == cScoreAttackScenario[i].scenario && MR::isEqualString(stage, cScoreAttackScenario[i].stage))
                return GameDataFunction::getBestScoreAttackCurrentStage(stage) != 0;
        }
        return false;
    }

    bool isNewGreenStarScenarioOpen() {
        if (!GameDataFunction::isOnGameEventFlagEnding120Stars()) return false;
        GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
        for (s32 i = 1; i <= accessor.getPowerStarNum(); i++) {
            if (accessor.isGreenStar(i) && !accessor.hasPowerStar(i)) return true;
        }
        return false;
    }

};  // namespace

namespace NrvScenarioSelectLayout {
    NEW_NERVE(ScenarioSelectLayoutNrvAppearStar, ScenarioSelectLayout, AppearStar);
    NEW_NERVE(ScenarioSelectLayoutNrvAppear, ScenarioSelectLayout, Appear);
    NEW_NERVE(ScenarioSelectLayoutNrvWaitScenarioSelect, ScenarioSelectLayout, WaitScenarioSelect);
    NEW_NERVE(ScenarioSelectLayoutNrvDecide, ScenarioSelectLayout, Decide);
    NEW_NERVE(ScenarioSelectLayoutNrvAfterScenarioSelected, ScenarioSelectLayout, AfterScenarioSelected);
    NEW_NERVE(ScenarioSelectLayoutNrvCancel, ScenarioSelectLayout, Cancel);
    NEW_NERVE(ScenarioSelectLayoutNrvCancelFadeOut, ScenarioSelectLayout, CancelFadeOut);
    NEW_NERVE(ScenarioSelectLayoutNrvAppearCometWarning, ScenarioSelectLayout, AppearCometWarning);
    NEW_NERVE(ScenarioSelectLayoutNrvWaitCometWarning, ScenarioSelectLayout, WaitCometWarning);
    NEW_NERVE(ScenarioSelectLayoutNrvDisappearCometWarning, ScenarioSelectLayout, DisappearCometWarning);
};  // namespace NrvScenarioSelectLayout

ScenarioSelectLayout::ScenarioSelectLayout(EffectSystem* pEffectSystem, const CameraContext* pCameraContext)
    : LayoutActor("シナリオ選択レイアウト", true), mSelectedScenarioNo(-1), mSelectionTimeout(), mSelectionLocked(), mStar(), mScenarioSky(), mEffectSystem(pEffectSystem),
      mCameraContext(pCameraContext), mNewTextFollowPos(0.0f, 0.0f), mNewGreenTextFollowPos(0.0f, 0.0f), mNewCometFollowPos(0.0f, 0.0f), mStarTopFollowPos(0.0f, 0.0f), mRaceTimeFollowPos(0.0f, 0.0f), mBackButton(), mCanCancel(true),
      mMarioPaneFollowPos(0.0f, 0.0f), mMarioPaneName(), mScenarioTextFadingOut(), mDisplayedScenarioNo(-1) {
    mEffectHostMtx.identity();

    for (s32 i = 0; i < ARRAY_SIZE(mQuestionPane); i++) {
        mQuestionPane[i].zero();
    }
}

void ScenarioSelectLayout::init(const JMapInfoIter& rIter) {
    initLayoutManager("ScenarioSelect", 1);
    MR::createAndAddPaneCtrl(this, "ScenarioSelect", 1);
    MR::createAndAddPaneCtrl(this, "ScenarioFrame", 1);
    MR::createAndAddPaneCtrl(this, "Scenario", 1);
    MR::createAndAddPaneCtrl(this, "StarTop", 1);
    MR::createAndAddPaneCtrl(this, "StarDown", 1);
    MR::createAndAddPaneCtrl(this, "BestTime", 1);
    MR::createAndAddPaneCtrl(this, "RaceTime", 1);
    MR::setFollowPos(&mRaceTimeFollowPos, this, "RaceTime");
    MR::setFollowTypeAdd(this, "RaceTime");
    MR::createAndAddPaneCtrl(this, cNewPaneName, 1);
    MR::createAndAddPaneCtrl(this, cNewGreenPaneName, 1);
    MR::createAndAddPaneCtrl(this, cNewCometPaneName, 1);
    for (s32 i = 0; i < ARRAY_SIZE(mQuestionPane); i++) MR::createAndAddPaneCtrl(this, cQuestionPaneName[i], 1);
    MR::createAndAddPaneCtrl(this, "Mario", 1);
    MR::createAndAddPaneCtrl(this, "CometAppear", 2);
    MR::setFollowPos(&mStarTopFollowPos, this, "StarTop");
    MR::setFollowTypeAdd(this, "StarTop");
    MR::setFollowPos(&mNewTextFollowPos, this, cNewPaneName);
    MR::setFollowPos(&mNewGreenTextFollowPos, this, cNewGreenPaneName);
    MR::setFollowPos(&mNewCometFollowPos, this, cNewCometPaneName);
    for (s32 i = 0; i < ARRAY_SIZE(mQuestionPane); i++) MR::setFollowPos(&mQuestionPane[i], this, cQuestionPaneName[i]);
    MR::setFollowPos(&mMarioPaneFollowPos, this, "Mario");
    initPointingTarget(6);
    initEffectKeeper(0, "ScenarioSelect", mEffectSystem);
    MR::setEffectHostMtx(this, "ScenarioSelectEffect", mEffectHostMtx);
    mStar = new ScenarioSelectStar*[6];
    for (s32 i = 0; i < 6; i++) {
        mStar[i] = new ScenarioSelectStar(mEffectSystem, mCameraContext->mProjection);
        mStar[i]->initWithoutIter();
        MR::addStarPointerTargetCircle(this, cStarPaneName[i], cPointingTargetRadius, TVec2f(0.0f, 0.0f), nullptr);
    }
    mScenarioSky = new MultiSceneActor("シナリオ選択の空", "ScenarioSelectSky", mCameraContext->mProjection, false);
    mScenarioSky->initWithoutIter();
    mBackButton = new BackButtonCancelB("戻るボタン", false);
    mBackButton->initWithoutIter();
    initNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppearStar));
}

void ScenarioSelectLayout::appear() {
    MR::showLayout(this);

    s32 normalScenarioCompleteNum;
    s32 extraScenarioCompleteNum;
    calcDisplayScenarioNum(&normalScenarioCompleteNum, &extraScenarioCompleteNum);

    MR::startPaneAnim(this, "ScenarioSelect", "Appear", 0);
    MR::setPaneAnimFrameAndStop(this, "ScenarioSelect", 0.0f, 0);
    startAnimAllNewPane("Appear");
    setAnimRateAllNewPane(0.0f);
    MR::startPaneAnim(this, "ScenarioFrame", "Wait", 0);
    MR::startPaneAnim(this, "StarTop", "StarPositionTop", 0);
    MR::setPaneAnimFrameAndStop(this, "StarTop", normalScenarioCompleteNum - 1, 0);

    if (extraScenarioCompleteNum > 0) {
        MR::startPaneAnim(this, "StarDown", "StarPositionDown", 0);
        MR::setPaneAnimFrameAndStop(this, "StarDown", extraScenarioCompleteNum - 1, 0);
        mStarTopFollowPos.zero();
    } else {
        mStarTopFollowPos.set(0.0f, 50.0f);
    }

    MR::setTextBoxMessageRecursive(this, "Galaxy", MR::getCurrentGalaxyNameOnCurrentLanguage());
    MR::hidePaneRecursive(this, "Scenario");
    setPlayerLeft();
    LayoutActor::appear();
    MR::hidePaneRecursive(this, cNewPaneName);
    MR::hidePaneRecursive(this, cNewGreenPaneName);
    MR::hidePaneRecursive(this, cNewCometPaneName);
    for (s32 i = 0; i < ARRAY_SIZE(::cQuestionPaneName); i++) {
        MR::hidePaneRecursive(this, ::cQuestionPaneName[i]);
    }
    appearAllStar(normalScenarioCompleteNum, extraScenarioCompleteNum);

    MR::startPaneAnim(this, "BestTime", "SelectIn", 0);
    MR::startPaneAnim(this, "RaceTime", "SelectIn", 0);
    MR::setPaneAnimFrameAndStop(this, "BestTime", 0.0f, 0);
    MR::setPaneAnimFrameAndStop(this, "RaceTime", 0.0f, 0);

    mScenarioSky->mTranslation.set(::cSkyTrans);
    mScenarioSky->mScale.set(::cSkyScale);
    mScenarioSky->appear();
    MultiScene::startBtk(mScenarioSky, "ScenarioSelectSky");

    MR::hidePaneRecursive(this, "CometAppear");

    mSelectedScenarioNo = -1;
    mSelectionTimeout = 0;
    mSelectionLocked = 0;

    mCanCancel = GameDataFunction::calcCurrentPowerStarNum() > 0;
    setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppearStar));
}

void ScenarioSelectLayout::kill() {
    MR::getEffect(this, "ScenarioSelectEffect")->forceDelete(mEffectSystem);
    for (s32 i = 0; i < 6; i++) {
        ScenarioSelectStar* pStar = mStar[i];

        if (pStar->_30) {
            continue;
        }

        pStar->kill();
    }

    mScenarioSky->kill();
    mBackButton->kill();
    LayoutActor::kill();
}

void ScenarioSelectLayout::movement() {
    LayoutActor::movement();

    for (s32 i = 0; i < 6; i++) {
        mStar[i]->movement();
    }

    mScenarioSky->movement();
    mBackButton->movement();
}

void ScenarioSelectLayout::calcAnim() {
    LayoutActor::calcAnim();

    for (s32 i = 0; i < 6; i++) {
        mStar[i]->calcAnim();
    }

    mScenarioSky->calcAnim();
    mBackButton->calcAnim();
}

void ScenarioSelectLayout::draw() const {
    LayoutActor::draw();
    mBackButton->draw();
}

void ScenarioSelectLayout::calcViewAndEntryStarModel() {
    for (s32 i = 0; i < 6; i++) {
        mStar[i]->calcViewAndEntry();
    }

    mScenarioSky->calcViewAndEntry();
}

s32 ScenarioSelectLayout::getSelectedScenarioNo() const {
    return mSelectedScenarioNo;
}

bool ScenarioSelectLayout::isReadyToDisappear() const {
    if (isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAfterScenarioSelected))) {
        return MR::isGreaterStep(this, 210);
    }

    return false;
}


bool ScenarioSelectLayout::isAppearStarEnd() const {
    return !MR::isDead(this) && !isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppearStar)) &&
           !isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppearCometWarning)) &&
           !isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvWaitCometWarning)) &&
           !isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvDisappearCometWarning));
}

bool ScenarioSelectLayout::isEndAnimStartStep() const {
    return isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAfterScenarioSelected)) && MR::isStep(this, ::cSelectedEndAnimStartStep);
}

bool ScenarioSelectLayout::isCanceled() const {
    return isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvCancel)) ||
           isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvCancelFadeOut));
}

void ScenarioSelectLayout::control() {
    if (!mSelectionLocked && mSelectedScenarioNo > 0) {
        getSelectedStar()->tryPointing();
    }

    MR::copyPaneTrans(&mMarioPaneFollowPos, this, mMarioPaneName);
}

void ScenarioSelectLayout::updateSelectedScenario() {
    ScenarioSelectStar* star;

    if (isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvWaitScenarioSelect))) {
        for (s32 i = 0; i < 6; i++) {
            star = mStar[i];

            if (!star->_30 && MR::isStarPointerPointingTarget(this, ::cStarPaneName[i], 0, true, "弱")) {
                mSelectedScenarioNo = star->mScenarioNo;
                mSelectionTimeout = 0;
                return;
            }
        }
    }

    if (mSelectionTimeout < 0) {
        mSelectionTimeout++;
    }

    if (mSelectionTimeout >= 0) {
        mSelectedScenarioNo = -1;
    }
}

void ScenarioSelectLayout::updateScenarioText() {
    bool isEndAnimStep = isNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAfterScenarioSelected)) &&
                         MR::isGreaterEqualStep(this, ::cSelectedEndAnimStartStep);

    if (MR::isHiddenPane(this, "Scenario") || (mSelectedScenarioNo > 0 && getSelectedScenarioNo() != mDisplayedScenarioNo)) {
        if (!isEndAnimStep && mSelectedScenarioNo > 0) {
            fadeInText();
        }
    } else if (mScenarioTextFadingOut == 0) {
        if (MR::isPaneAnimStopped(this, "Scenario", 0) && mSelectedScenarioNo <= 0) {
            fadeOutText();
        }
    } else if (MR::isPaneAnimStopped(this, "Scenario", 0)) {
        MR::hidePaneRecursive(this, "Scenario");
    }
}

void ScenarioSelectLayout::fadeInText() {
    const char* stage = MR::getCurrentStageName();
    mScenarioTextFadingOut = 0;
    mDisplayedScenarioNo = mSelectedScenarioNo;
    MR::setTextBoxMessageRecursive(this, "Scenario", MR::getScenarioNameOnCurrentLanguage(stage, mSelectedScenarioNo));
    MR::showPaneRecursive(this, "Scenario");
    MR::startPaneAnim(this, "Scenario", "SelectIn", 0);
    MR::startPaneAnim(this, "BestTime", "SelectIn", 0);
    if (hasScenarioClearTime(stage, mSelectedScenarioNo)) {
        wchar_t clearTime[16];
        MR::makeClearTimeString(clearTime, GameDataFunction::getScenarioClearTime(stage, mSelectedScenarioNo));
        MR::setTextBoxMessageRecursive(this, "ShaBCounter", clearTime);
    } else {
        MR::setPaneAnimFrameAndStop(this, "BestTime", 0.0f, 0);
        MR::clearTextBoxMessageRecursive(this, "ShaBCounter");
    }
    MR::startPaneAnim(this, "RaceTime", "SelectIn", 0);
    bool race = isCurrentStageRaceComplete(mSelectedScenarioNo);
    bool score = isCurrentStageScoreAttackComplete(mSelectedScenarioNo);
    if (race || score) {
        if (race) {
            wchar_t raceTime[16];
            MR::makeRaceBestTimeString(raceTime, RaceManagerFunction::getRaceId(stage));
            MR::setTextBoxMessageRecursive(this, "ShaRCounter", raceTime);
            MR::setTextBoxGameMessageRecursive(this, "RTime", "ScenarioSelect_RaceTime");
        } else {
            MR::setTextBoxNumberRecursive(this, "ShaRCounter", GameDataFunction::getBestScoreAttackCurrentStage(stage));
            MR::setTextBoxGameMessageRecursive(this, "RTime", "ScenarioSelect_HighScore");
        }
        if (hasScenarioClearTime(stage, mSelectedScenarioNo)) mRaceTimeFollowPos.zero();
        else mRaceTimeFollowPos.set(0.0f, 27.0f);
        return;
    }
    MR::setPaneAnimFrameAndStop(this, "RaceTime", 0.0f, 0);
    MR::clearTextBoxMessageRecursive(this, "ShaRCounter");
}

void ScenarioSelectLayout::fadeOutText() {
    const char* stage = MR::getCurrentStageName();
    mScenarioTextFadingOut = 1;
    MR::startPaneAnim(this, "Scenario", "SelectOut", 0);
    if (hasScenarioClearTime(stage, mDisplayedScenarioNo)) MR::startPaneAnim(this, "BestTime", "SelectOut", 0);
    if (isCurrentStageRaceComplete(mDisplayedScenarioNo) || isCurrentStageScoreAttackComplete(mDisplayedScenarioNo)) MR::startPaneAnim(this, "RaceTime", "SelectOut", 0);
}

bool ScenarioSelectLayout::trySelect() {
    if (MR::testDPDMenuPadDecideTrigger()) {
        if (mSelectedScenarioNo > 0) {
            return true;
        }

        if (!MR::isStarPointerInScreen(WPAD_CHAN0)) {
            for (s32 i = 0; i < 6; i++) {
                if (mStar[i]->_30) {
                    continue;
                }

                if (mStar[i]->mStarCollectedStatus) {
                    continue;
                }

                mSelectedScenarioNo = mStar[i]->mScenarioNo;

                return true;
            }
        }
    }

    return false;
}

ScenarioSelectStar* ScenarioSelectLayout::getSelectedStar() const {
    for (s32 i = 0; i < 6; i++) {
        if (mStar[i]->_30) {
            continue;
        }

        if (mSelectedScenarioNo != mStar[i]->mScenarioNo) {
            continue;
        }

        return mStar[i];
    }

    return nullptr;
}

// NONMATCHING: register allocation and floating-point instruction scheduling differ.
void ScenarioSelectLayout::calcWorldPositionFromScreenPos(TVec3f* pWorldPos, const TVec2f& rScreenPos, f32 depth) const {
    f32 height = static_cast< s32 >(JUTVideo::getManager()->getEfbHeight());
    f32 fovy = mCameraContext->getFovy() * (PI / 180.0f);
    f32 focalLength = (height * 0.5f) / MR::tan(fovy * 0.5f);
    f32 distance;

    if (depth >= 0.0f) {
        distance = depth;
    } else {
        f32 defaultDistance = focalLength;
        distance = defaultDistance;
    }

    f32 scale = distance / focalLength;
    TVec3f pos(scale * (rScreenPos.x - MR::getScreenWidth() * 0.5f), scale * -(rScreenPos.y - height * 0.5f), -distance);
    TPos3f invView(mCameraContext->getInvViewMtx());
    invView.mult(pos, *pWorldPos);
}

void ScenarioSelectLayout::calcDisplayScenarioNum(s32* pNormalScenarioCompleteNum, s32* pExtraScenarioCompleteNum) const {
    GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
    bool isNewScenario = false;
    *pNormalScenarioCompleteNum = 0;
    *pExtraScenarioCompleteNum = 0;

    s32 greenNum;
    for (s32 i = 1; (greenNum = MR::makeCurrentGalaxyStatusAccessor().getGreenStarNum(), i <= MR::getCurrentStagePowerStarNum() - greenNum); i++) {
        if (MR::hasPowerStarInCurrentStage(i)) {
            *pNormalScenarioCompleteNum += 1;
        } else if (!isNewScenario) {
            if (!MR::makeCurrentGalaxyStatusAccessor().isNotNormalStar(i)) {
                if (accessor.isOpenScenario(i)) {
                    *pNormalScenarioCompleteNum += 1;
                    isNewScenario = true;
                }
            }
        }
    }

    if (GameDataFunction::isOnGameEventFlagEnding120Stars()) {
        *pExtraScenarioCompleteNum = MR::makeCurrentGalaxyStatusAccessor().getGreenStarNum();
    }
}

// NONMATCHING: remaining stack slots, registers, and instruction scheduling differ.
void ScenarioSelectLayout::appearAllStar(s32 normalScenarioCompleteNum, s32 extraScenarioCompleteNum) {
    GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
    s32 starIndex = 0;
    bool completed;
    if (GameDataFunction::isOnGameEventFlagEnding120Stars()) {
        completed = MR::makeCurrentGalaxyStatusAccessor().isCompletedNoGreen();
    } else {
        completed = MR::makeCurrentGalaxyStatusAccessor().isCompletedWithGreen();
    }
    s32 starState = 1;
    if (completed) {
        starState = 2;
    }
    bool appearedNew = false;
    s32 greenNum;
    for (s32 i = 1; (greenNum = MR::makeCurrentGalaxyStatusAccessor().getGreenStarNum(), i <= MR::getCurrentStagePowerStarNum() - greenNum); i++) {
        const char* stageName = MR::getCurrentStageName();
        if (MR::hasPowerStarInCurrentStage(i)) {
            appearStar(starIndex, i, starState);
            starIndex++;
        } else if (!MR::makeCurrentGalaxyStatusAccessor().isNotNormalStar(i)) {
            if (!appearedNew && accessor.isOpenScenario(i)) {
                appearStar(starIndex, i, 0);
                appearedNew = true;
                if (MR::makeCurrentGalaxyStatusAccessor().isValidComet(i)) {
                    MR::showPaneRecursive(this, cNewCometPaneName);
                    MR::copyPaneTrans(&mNewCometFollowPos, this, ::cStarPaneName[starIndex]);
                } else {
                    MR::showPaneRecursive(this, cNewPaneName);
                    MR::copyPaneTrans(&mNewTextFollowPos, this, ::cStarPaneName[starIndex]);
                }
                starIndex++;
            }
        } else if (MR::hasAllNormalPowerStarWithoutComet(stageName) && accessor.isOpenScenario(i)) {
            s32 hidden = MR::getPlacedHiddenStarScenarioNo(stageName, i) - 1;
            MR::showPaneRecursive(this, cNewGreenPaneName);
            MR::copyPaneTrans(&mNewGreenTextFollowPos, this, ::cStarPaneName[hidden]);
        }
    }
    for (s32 i = 0; i < extraScenarioCompleteNum; i++) {
        s32 state;
        s32 scenarioNo;
        s32 slot = i + 3;
        s32 greenNum = MR::makeCurrentGalaxyStatusAccessor().getGreenStarNum();
        scenarioNo = i + (MR::getCurrentStagePowerStarNum() - greenNum) + 1;
        if (MR::hasPowerStarInCurrentStage(scenarioNo)) {
            state = starState;
        } else {
            MR::showPaneRecursive(this, ::cQuestionPaneName[i]);
            state = 0;
            MR::copyPaneTrans(&mQuestionPane[i], this, ::cStarPaneName[i + 3]);
        }
        appearStar(slot, scenarioNo, state);
    }
}

void ScenarioSelectLayout::appearStar(s32 starId, s32 scenarioNo, int a3) {
    ScenarioSelectStar* pStar = mStar[starId];

    TVec2f starPaneTrans;
    MR::copyPaneTrans(&starPaneTrans, this, ::cStarPaneName[starId]);

    TVec3f worldPos;
    calcWorldPositionFromScreenPos(&worldPos, starPaneTrans, 1000.0f);

    pStar->setup(scenarioNo, a3, worldPos, starId);
    pStar->appear();
}

bool ScenarioSelectLayout::isAppearStarEndAll() const {
    for (s32 i = 0; i < 6; i++) {
        if (mStar[i]->_30) {
            continue;
        }

        if (mStar[i]->isAppearEnd()) {
            continue;
        }

        return false;
    }

    return true;
}

bool ScenarioSelectLayout::tryCancel() {
    if (!mCanCancel) return false;
    if (mBackButton->isSelected()) {
        setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvCancel));
        return true;
    }
    return false;
}

void ScenarioSelectLayout::setPlayerLeft() {
    MR::setTextBoxGameMessageRecursive(this, "ShaMario", "PlayerLeft");
    MR::setTextBoxNumberRecursive(this, "ShaMarioNum", MR::getPlayerLeft());

    if (MR::getPlayerLeft() < 10) {
        mMarioPaneName = "MarioPosition01";
    } else {
        mMarioPaneName = "MarioPosition10";
    }
}

void ScenarioSelectLayout::startAnimAllNewPane(const char* pAnimName) {
    MR::startPaneAnim(this, cNewPaneName, pAnimName, 0);
    MR::startPaneAnim(this, cNewGreenPaneName, pAnimName, 0);
    MR::startPaneAnim(this, cNewCometPaneName, pAnimName, 0);

    for (s32 i = 0; i < ARRAY_SIZE(::cQuestionPaneName); i++) {
        MR::startPaneAnim(this, ::cQuestionPaneName[i], pAnimName, 0);
    }
}

void ScenarioSelectLayout::setAnimRateAllNewPane(f32 rate) {
    MR::setPaneAnimRate(this, cNewPaneName, rate, 0);
    MR::setPaneAnimRate(this, cNewGreenPaneName, rate, 0);
    MR::setPaneAnimRate(this, cNewCometPaneName, rate, 0);

    for (s32 i = 0; i < ARRAY_SIZE(::cQuestionPaneName); i++) {
        MR::setPaneAnimRate(this, ::cQuestionPaneName[i], rate, 0);
    }
}

void ScenarioSelectLayout::exeCancelFadeOut() {
    kill();
}

void ScenarioSelectLayout::exeAppearStar() {
    if (MR::isFirstStep(this)) {
        MR::submitTrigSE();
        MR::getEffect(this, "ScenarioSelectEffect")->create(mEffectSystem);
        MR::setEffectRate(this, "ScenarioSelectEffect", ::cSelectedEffectRateMin);
        MR::setEffectDirectionalSpeed(this, "ScenarioSelectEffect", ::cSelectedEffectSpeedMin);

        if (MR::isUseScenarioSelectMusicZeroStars() || MR::isUseScenarioSelectMusicOneStar()) MR::startDepartBGM();
        else MR::startStageBGM("BGM_SMG2_COURSESELECT02", false);
    }

    MR::startSystemLevelSE("SE_DM_LV_SENARIO_SEL_FLY");

    if (!isAppearStarEndAll()) {
        return;
    }

    bool warning = false;
    if (MR::getGalaxyStarTotalNum(MR::getCurrentStageName()) != 0 || isNewGreenStarScenarioOpen()) warning = true;
    if (warning) {
        setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppearCometWarning));
    } else {
        setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppear));
    }
}

void ScenarioSelectLayout::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::setPaneAnimRate(this, "ScenarioSelect", 1.0f, 0);
        setAnimRateAllNewPane(1.0f);
    }

    MR::startSystemLevelSE("SE_DM_LV_SENARIO_SEL_FLY");
    MR::setNerveAtPaneAnimStopped(this, "ScenarioSelect", GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvWaitScenarioSelect), 0);
}

const char* ScenarioSelectLayout::getBackButtonMovePositionName(s32 index) {
    s32 column = index % 3;
    if (column == 0) return "BackButton0";
    if (column == 1) return "BackButton1";
    return "BackButton2";
}

void ScenarioSelectLayout::connectStarPointerMovePosition(s32 index) {
    if (mStar[index]->_30) return;
    s32 right = index + 1;
    if (right == 3) right = 0;
    if (right == 6) right = 3;
    s32 left = index - 1;
    if (left == -1) left = 2;
    if (left == 2) left = 5;
    if (!mStar[right]->_30) {
        StarPointerUtil::setConnectionMovePositionRight2Way(cStarPaneName[index], cStarPaneName[right]);
    } else if (!mStar[left]->_30) {
        StarPointerUtil::setConnectionMovePositionRight2Way(cStarPaneName[index], cStarPaneName[left]);
    }
    if (mCanCancel) {
        if (index < 3) {
            s32 below = index + 3;
            if (!mStar[below]->_30) {
                StarPointerUtil::setConnectionMovePositionDown2Way(cStarPaneName[index], cStarPaneName[below]);
            } else {
                StarPointerUtil::setConnectionMovePositionDown2Way(cStarPaneName[index], getBackButtonMovePositionName(index));
            }
            return;
        }
        StarPointerUtil::setConnectionMovePositionDown2Way(cStarPaneName[index], getBackButtonMovePositionName(index));
    }
}

// NONMATCHING: temporary vector stack slots and register allocation differ.
void ScenarioSelectLayout::updateStarPointerMovePositions() {
    if (mStar[0]->_30) return;
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        if (mCanCancel) {
            TVec2f offset0(0.0f, 0.0f);
            TVec2f* position0 = StarPointerUtil::getDefaultButtonOffsetVec2();
            StarPointerUtil::addStarPointerMovePosition(getBackButtonMovePositionName(0), position0, offset0);
            TVec2f offset1(0.0f, 0.0f);
            TVec2f* position1 = StarPointerUtil::getDefaultButtonOffsetVec2();
            StarPointerUtil::addStarPointerMovePosition(getBackButtonMovePositionName(1), position1, offset1);
            TVec2f offset2(0.0f, 0.0f);
            TVec2f* position2 = StarPointerUtil::getDefaultButtonOffsetVec2();
            StarPointerUtil::addStarPointerMovePosition(getBackButtonMovePositionName(2), position2, offset2);
        }
        for (s32 i = 0; i < 6; i++) {
            if (!mStar[i]->_30) {
                TVec2f offset(0.0f, 0.0f);
                StarPointerUtil::addStarPointerMovePositionFromPane(this, cStarPaneName[i], &offset);
            }
        }
        for (s32 i = 0; i < 6; i++) connectStarPointerMovePosition(i);
        StarPointerUtil::setDefaultAllMovePosition(cStarPaneName[0]);
    }
    if (mCanCancel) {
        TVec2f position;
        MR::copyPaneTrans(&position, mBackButton, "BoxButton");
        TVec2f copy(position);
        TVec2f buttonPosition(copy);
        StarPointerUtil::setStarPointerMovePosition(getBackButtonMovePositionName(0), &buttonPosition);
        StarPointerUtil::setStarPointerMovePosition(getBackButtonMovePositionName(1), &buttonPosition);
        StarPointerUtil::setStarPointerMovePosition(getBackButtonMovePositionName(2), &buttonPosition);
    }
    StarPointerUtil::refreshMovePositionsFromLayout(this);
    StarPointerUtil::endMovePositionRegistration(this);
}

void ScenarioSelectLayout::exeWaitScenarioSelect() {
    if (MR::isFirstStep(this)) {
        startAnimAllNewPane("NewWait");
        if (mCanCancel) mBackButton->appear();
    }

    updateStarPointerMovePositions();
    updateSelectedScenario();
#ifdef GALAXY_NATIVE
    for (s32 i = 0; i < ARRAY_SIZE(cStarPaneName); ++i) {
        if (!mStar[i]->_30 && mStar[i]->mScenarioNo == 1) {
            MR::Native::publishUiPaneTarget(this, ::cStarPaneName[i], "Scenario.First", 1,
                PETARI_UI_SELECTABLE | (mSelectedScenarioNo == 1 ? PETARI_UI_POINTING : 0));
        }
    }
    // The personal progress badges (native/PROGRESS.md): where each shown mission star is, every frame.
    for (s32 i = 0; i < ARRAY_SIZE(cStarPaneName); ++i) {
        TVec2f badgePos;
        if (!mStar[i]->_30 && MR::Native::paneCenterOnScreen(this, ::cStarPaneName[i], &badgePos)) {
            petari_progress_badge(MR::getCurrentStageName(), mStar[i]->mScenarioNo, badgePos.x / MR::getScreenWidth(),
                                  badgePos.y / MR::getScreenHeight());
        }
    }
    // Every shown mission star, indexed by its scenario number (comet and
    // hidden stars included once the file owns them).
    for (s32 i = 0; i < ARRAY_SIZE(cStarPaneName); ++i) {
        if (!mStar[i]->_30) {
            const s32 scenarioNo = mStar[i]->mScenarioNo;
            MR::Native::publishUiPaneTarget(this, ::cStarPaneName[i], "Scenario.Star", scenarioNo,
                PETARI_UI_SELECTABLE | (mSelectedScenarioNo == scenarioNo ? PETARI_UI_POINTING : 0));
        }
    }
#endif
    updateScenarioText();
    MR::startSystemLevelSE("SE_DM_LV_SENARIO_SEL_FLY");

    if (tryCancel()) {
        return;
    }

    if (trySelect()) {
        mSelectionLocked = true;
        MR::startSystemSE("SE_SY_DECIDE_1");
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        if (mCanCancel) mBackButton->disappear();
        setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvDecide));
    }
}

void ScenarioSelectLayout::exeDecide() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < 6; i++) {
            if (mStar[i]->_30) {
                continue;
            }

            if (mStar[i] == getSelectedStar()) {
                mStar[i]->select();
            } else {
                mStar[i]->notSelect();
            }
        }

        startAnimAllNewPane("End");
    }

    updateScenarioText();
    MR::startSystemLevelSE("SE_DM_LV_SENARIO_SEL_FLY");
    MR::setNerveAtStep(this, GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAfterScenarioSelected), ::cDecideFrame);
}

void ScenarioSelectLayout::exeAfterScenarioSelected() {
    MR::startPaneAnimAtStep(this, "ScenarioFrame", "End", ::cSelectedEndAnimStartStep, 0);
    updateScenarioText();
    MR::startSystemLevelSE("SE_DM_LV_SENARIO_SEL_FLY");

    f32 rate = MR::calcNerveEaseInValue(this, ::cSelectedAccelStartStep, ::cSelectedAccelFrame, ::cSelectedEffectRateMin, ::cSelectedEffectRateMax);
    f32 directionalSpeed =
        MR::calcNerveEaseInValue(this, ::cSelectedAccelStartStep, ::cSelectedAccelFrame, ::cSelectedEffectSpeedMin, ::cSelectedEffectSpeedMax);

    MR::setEffectRate(this, "ScenarioSelectEffect", rate);
    MR::setEffectDirectionalSpeed(this, "ScenarioSelectEffect", directionalSpeed);

    if (MR::isStep(this, ::cSelectedFadeOutStartStep)) {
        MR::stopStageBGM(::cSelectedFadeOutFrame);
        MR::closeSystemWipeWhiteFade(::cSelectedFadeOutFrame);
    }
}


void ScenarioSelectLayout::exeCancel() {
    if (MR::isFirstStep(this)) {
        mSelectedScenarioNo = -1;
        MR::stopStageBGM(30);
    }

    updateScenarioText();

    if (mBackButton->isHidden()) {
        MR::startSystemWipeCircleWithCaptureScreen(::cCancelFadeOutFrame);
        MR::stopStageBGM(1);
        setNerve(GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvCancelFadeOut));
    }
}

void ScenarioSelectLayout::exeAppearCometWarning() {
    if (MR::isStep(this, cCometWarningWaitFrame)) {
        MR::showPaneRecursive(this, "CometAppear");
        MR::startPaneAnim(this, "CometAppear", "CometAppear", 0);
        MR::startPaneAnim(this, "CometAppear", "Color", 1);
        MR::setPaneAnimFrameAndStop(this, "CometAppear", isNewGreenStarScenarioOpen() ? 1.0f : 0.0f, 1);
        MR::startSystemSE("SE_SY_COMET_WARNING_DISP");
    }
    if (MR::isGreaterStep(this, cCometWarningWaitFrame)) {
        MR::setNerveAtPaneAnimStopped(this, "CometAppear", GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvWaitCometWarning), 0);
    }
}

void ScenarioSelectLayout::exeWaitCometWarning() {
    if (getNerveStep() % 80 == 25) {
        MR::startSystemSE("SE_SY_COMET_WARNING");
    }

    MR::startPaneAnimAtFirstStep(this, "CometAppear", "CometWait", 0);
    MR::setNerveAtStep(this, GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvDisappearCometWarning), ::cCometWarningFrame);
}

void ScenarioSelectLayout::exeDisappearCometWarning() {
    MR::startPaneAnimAtFirstStep(this, "CometAppear", "CometEnd", 0);
    MR::setNerveAtPaneAnimStopped(this, "CometAppear", GET_NERVE(ScenarioSelectLayout, ScenarioSelectLayoutNrvAppear), 0);
}
