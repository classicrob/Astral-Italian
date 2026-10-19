#include "Game/Scene/ScenarioSelectScene.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraContext.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/CinemaFrame.hpp"
#include "Game/Screen/ScenarioSelectLayout.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <revolution/gx/GXEnum.h>
#include <JSystem/J3DGraphBase/J3DDrawBuffer.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>

namespace NrvScenarioSelectScene {
    NEW_NERVE(ScenarioSelectSceneNrvDeactive, ScenarioSelectScene, Deactive);
    // Target execute is empty (blr): no exeInvalidScenarioSelect exists in the binary
    class ScenarioSelectSceneNrvInvalidScenarioSelect : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static ScenarioSelectSceneNrvInvalidScenarioSelect sInstance;
    };
    ScenarioSelectSceneNrvInvalidScenarioSelect ScenarioSelectSceneNrvInvalidScenarioSelect::sInstance ATTRIBUTE_WEAK;


    // Target inlines exeWaitStartScenarioSelect into execute
    class ScenarioSelectSceneNrvWaitStartScenarioSelect : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ScenarioSelectSceneNrvWaitStartScenarioSelect sInstance;
    };
    ScenarioSelectSceneNrvWaitStartScenarioSelect ScenarioSelectSceneNrvWaitStartScenarioSelect::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(ScenarioSelectSceneNrvStartScenarioSelect, ScenarioSelectScene, StartScenarioSelect);
    void ScenarioSelectSceneNrvWaitStartScenarioSelect::execute(Spine* pSpine) const {
            ScenarioSelectScene* actor = reinterpret_cast< ScenarioSelectScene* >(pSpine->mExecutor);
            if (!MR::isSystemWipeActive()) {
                actor->setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvStartScenarioSelect));
            }
        }

    NEW_NERVE(ScenarioSelectSceneNrvWaitScenarioSelect, ScenarioSelectScene, WaitScenarioSelect);
    NEW_NERVE(ScenarioSelectSceneNrvWaitResumeInitializeThread, ScenarioSelectScene, WaitResumeInitializeThread);
    NEW_NERVE(ScenarioSelectSceneNrvWaitInitializeEnd, ScenarioSelectScene, WaitInitializeEnd);
    NEW_NERVE(ScenarioSelectSceneNrvWaitResumeInitializeThreadIfRequestedReset, ScenarioSelectScene, WaitResumeInitializeThreadIfRequestedReset);
    NEW_NERVE(ScenarioSelectSceneNrvWaitResumeInitializeThreadIfCanceledSelect, ScenarioSelectScene, WaitResumeInitializeThreadIfCanceledSelect);
};  // namespace NrvScenarioSelectScene

namespace {
    J3DDrawBuffer* createDrawBuffer() {
        J3DDrawBuffer* buffer = new J3DDrawBuffer(1);
        buffer->mSortMode = 5;
        return buffer;
    }

    bool tryResumeInitializeThread() {
        const char* thread = "シーン初期化";
        if (MR::isSuspendedAsyncExecuteThread(thread)) {
            MR::resumeAsyncExecuteThread(thread);
            return true;
        }

        return false;
    }
};  // namespace

ScenarioSelectScene::ScenarioSelectScene()
    : Scene("シナリオ選択シーン"), _14(), _15(), _16(), mScenarioLayout(), mCinemaFrame(), _28(), mEffectSystem(), mCameraContext() {
}

void ScenarioSelectScene::init() {
    _20 = ::createDrawBuffer();
    _24 = ::createDrawBuffer();
    mEffectSystem = new EffectSystem("エフェクトシステム", false);
    mEffectSystem->initWithoutIter();
    mEffectSystem->entry(MR::getParticleResourceHolder(), 0x280, 0x20);
    mCameraContext = new CameraContext();

    TPos3f view;
    view.setPositionFromLookAt(TVec3f(0.0f, 0.0f, 1000.0f), TVec3f(0.0f, 1.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mCameraContext->setViewMtx(view, false, false, TVec3f(0.0f, 0.0f, 0.0f));

    mScenarioLayout = new ScenarioSelectLayout(mEffectSystem, mCameraContext);
    mScenarioLayout->initWithoutIter();
    mCinemaFrame = new CinemaFrame(false);
    mCinemaFrame->initWithoutIter();
    initNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
    _14 = 1;
}

void ScenarioSelectScene::start() {
    _15 = 0;
    _16 = 0;
    setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvInvalidScenarioSelect));
}

void ScenarioSelectScene::startBackground() {
    start();
    _15 = 1;
}

void ScenarioSelectScene::update() {
    if (!_14) {
        return;
    }

    updateNerve();

    bool res = isExecForeground() && _28 == 0;

    if (res) {
        mScenarioLayout->movement();
        mCinemaFrame->movement();
        MR::Effect::checkEffectSceneUpdate(mEffectSystem);
    }
}

void ScenarioSelectScene::calcAnim() {
    bool res = isExecForeground() && _28 == 0;

    if (res) {
        mScenarioLayout->calcAnim();
        mCinemaFrame->calcAnim();
        MR::Effect::movementEffectNormal(mEffectSystem);
        calcViewAndEntry();
    }
}

void ScenarioSelectScene::calcViewAndEntry() {
    bool res = isExecForeground() && _28 == 0;

    if (res) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        setupCameraMtx();
        j3dSys.mDrawBuffer[0] = _20;
        j3dSys.mDrawBuffer[1] = _24;
        mScenarioLayout->calcViewAndEntryStarModel();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }
}

void ScenarioSelectScene::draw() const {
    bool res = isExecForeground() && _28 == 0;

    if (res) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        setupCameraMtx();
        j3dSys.drawInit();
        j3dSys.mDrawMode = 3;
        _20->draw();
        _20->frameInit();
        j3dSys.mDrawMode = 4;
        _24->draw();
        _24->frameInit();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        MR::Effect::drawEffect3D(mEffectSystem, mCameraContext->getViewMtx());
        mScenarioLayout->draw();
        MR::Effect::drawEffect2D(mEffectSystem);
        mCinemaFrame->draw();
    }
}

bool ScenarioSelectScene::isActive() const {
    return _14 && !isNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
}

bool ScenarioSelectScene::isExecForeground() const {
    return _14 && !isNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive)) && _15 == 0;
}

bool ScenarioSelectScene::isScenarioSelecting() const {
    return isExecForeground();
}

void ScenarioSelectScene::validateScenarioSelect() {
    if (!_28) {
        if (_15) {
            setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitResumeInitializeThread));
            return;
        }

        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitStartScenarioSelect));
    } else {
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitResumeInitializeThreadIfRequestedReset));
    }
}

void ScenarioSelectScene::requestReset(bool waitForInitialize) {
    if (!_14) {
        _28 = 0;
        return;
    }

    _28 = 1;
    suspend();
    if (isNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive)) ||
        isNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitInitializeEnd))) {
        _28 = 0;
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
    } else if (isNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvInvalidScenarioSelect))) {
        if (!waitForInitialize) {
            _28 = 0;
            setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
        }
    } else {
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitResumeInitializeThreadIfRequestedReset));
    }
}

bool ScenarioSelectScene::isResetEnd() const {
    return _28 == 0;
}

void ScenarioSelectScene::setupCameraMtx() const {
    PSMTXCopy(mCameraContext->getViewMtx(), j3dSys.mViewMtx);
    GXSetProjection(mCameraContext->mProjection, GX_PERSPECTIVE);
}


void ScenarioSelectScene::suspend() {
    _16 = 0;

    if (!MR::isDead(mScenarioLayout)) {
        mScenarioLayout->kill();
    }

    if (!MR::isDead(mCinemaFrame)) {
        mCinemaFrame->kill();
    }

    _20->frameInit();
    _24->frameInit();
    MR::Effect::forceDeleteAllEmitters(mEffectSystem);
}

void ScenarioSelectScene::tryStartScreenToFrame() {
    if (!_15 && mScenarioLayout->isEndAnimStartStep()) {
        mCinemaFrame->tryScreenToFrame();
    }
}

void ScenarioSelectScene::exeDeactive() {
    if (MR::isFirstStep(this)) {
        if (!MR::isDead(mCinemaFrame)) {
            mCinemaFrame->kill();
        }
    }
}


void ScenarioSelectScene::exeStartScenarioSelect() {
    if (MR::isFirstStep(this)) {
        MR::openSystemWipeWhiteFade();

        mCinemaFrame->forceToFrame();
        mScenarioLayout->appear();
    }

    if (mScenarioLayout->isAppearStarEnd()) {
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitScenarioSelect));
    }
}


void ScenarioSelectScene::exeWaitScenarioSelect() {
    if (MR::isFirstStep(this)) {
        mCinemaFrame->tryFrameToScreen();
        MR::startStarPointerModeScenarioSelectScene(this);
    }

    if (trySetCurrentScenarioNo()) {
        MR::endStarPointerMode(this);
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitResumeInitializeThread));
    } else if (mScenarioLayout->isCanceled()) {
        MR::endStarPointerMode(this);
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitResumeInitializeThreadIfCanceledSelect));
    }
}

void ScenarioSelectScene::exeWaitResumeInitializeThread() {
    tryStartScreenToFrame();

    if (::tryResumeInitializeThread()) {
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvWaitInitializeEnd));
    }
}

void ScenarioSelectScene::exeWaitInitializeEnd() {
    tryStartScreenToFrame();

    if (MR::isInitializeStateEnd()) {
        if (MR::isDead(mScenarioLayout)) {
            MR::Effect::forceDeleteAllEmitters(mEffectSystem);
            setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
        } else if (mScenarioLayout->isReadyToDisappear()) {
            mCinemaFrame->forceToScreen();
            mScenarioLayout->kill();
            MR::Effect::forceDeleteAllEmitters(mEffectSystem);
            setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
        }
    }
}


void ScenarioSelectScene::exeWaitResumeInitializeThreadIfRequestedReset() {
    if (MR::isFirstStep(this)) {
        GameSystemFunction::resetCurrentScenarioNo();
    }

    if (::tryResumeInitializeThread()) {
        _28 = 0;
        setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
    }
}

bool ScenarioSelectScene::trySetCurrentScenarioNo() const {
    if (mScenarioLayout->mSelectionLocked) {
        if (!MR::isScenarioDecided()) {
            s32 selected = mScenarioLayout->getSelectedScenarioNo();
            s32 placed = selected;
            GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
            if (accessor.isNotNormalStar(selected)) {
                placed = MR::getPlacedHiddenStarScenarioNo(MR::getCurrentStageName(), selected);
            }
            MR::setCurrentScenarioNo(placed, selected);
        }
        return true;
    }
    return false;
}

// Local prototypes: defined in src/System, not declared in their headers
namespace GameSequenceFunction {
    void requestCancelScenarioSelect();
};  // namespace GameSequenceFunction
namespace GameSystemFunction {
    void resetCurrentScenarioNo();
};  // namespace GameSystemFunction

void ScenarioSelectScene::exeWaitResumeInitializeThreadIfCanceledSelect() {
    if (MR::isDead(mScenarioLayout)) {
        if (::tryResumeInitializeThread()) {
            GameSystemFunction::resetCurrentScenarioNo();
            GameSequenceFunction::setInStageKillReady();
            suspend();
            _16 = 1;
            MR::Effect::forceDeleteAllEmitters(mEffectSystem);
            setNerve(GET_NERVE(ScenarioSelectScene, ScenarioSelectSceneNrvDeactive));
        }
    }
}
