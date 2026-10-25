#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

static const char* sResultCameraName = "リザルトマップカメラ";

WorldMapResultDraw::WorldMapResultDraw() : NameObj("リザルト描画（ワールドマップ）"), mWorld(nullptr), mEye(0.0f, 1000.0f, 1000.0f), mTarget(0.0f, 0.0f, 0.0f) {
    mLayoutBG = nullptr;
}

void WorldMapResultDraw::init(const JMapInfoIter&) {
    mLayoutBG = new SimpleLayout("背景", "ResultMapBG", 1, 0x45);
    mLayoutBG->initWithoutIter();
    mLayoutBG->kill();
    mLayoutName = new SimpleLayout("ワールド番号", "MapGalaxyWorld", 1, 0x4C);
    mLayoutName->initWithoutIter();
    mLayoutName->kill();
    MR::declareEventCameraProgrammable(sResultCameraName);
}

void WorldMapResultDraw::movement() {
    if (mWorld != nullptr) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::setProgrammableCameraParam(sResultCameraName, mTarget, mEye, up, true);
        mWorld->startAllPointingAnim();
        MR::movementWorldMapBig();
    }
}

void WorldMapResultDraw::calcAnim() {
    if (mWorld != nullptr) {
        MR::calcAnimWorldMapBig();
    }
}

void WorldMapResultDraw::calcViewAndEntry() {
    if (mWorld != nullptr) {
        MR::entryWorldMapBig();
    }
}

void WorldMapResultDraw::draw() const {
    if (mWorld != nullptr) {
        SceneFunction::executeDrawAfterImageEffect();
        MR::reinitGX();
        MR::clearZBuffer();
        MR::loadProjectionMtx();
        MR::loadViewMtx();
        MR::drawWorldMapBig();
        MR::drawWorldMapMiniEffects();
    }
}

void WorldMapResultDraw::start(WorldMapResultWorld* pWorld) {
    mWorld = pWorld;
    mLayoutBG->appear();
    MR::startAnim(mLayoutBG, "BG", 0);
    MR::setAnimFrameAndStop(mLayoutBG, mWorld->mWorldId - 1, 0);
    char buf[0x20];
    snprintf(buf, 0x20, "WorldMap%02dGalaxy", mWorld->mWorldId);
    mLayoutName->appear();
    MR::hidePane(mLayoutName, "Frame");
    MR::setTextBoxMessageRecursive(mLayoutName, "MapGalaxyWorld", MR::getGalaxyNameOnCurrentLanguage(buf));
    MR::startGlobalEventCameraNoTarget(sResultCameraName, 0);
    MR::setProgrammableCameraParamFovy(sResultCameraName, 26.0f);
}

void WorldMapResultDraw::end() {
    mWorld = nullptr;
    mLayoutBG->kill();
    mLayoutName->kill();
    MR::endGlobalEventCamera(sResultCameraName, 0, true);
}

void WorldMapResultCommand::startResult(s32 worldId) {
    if (worldId < 0) {
        worldId = GameDataFunction::getCurrentWorldId();
    }

    if (mDraw->mWorld != nullptr) {
        endResult();
    }

    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    pWorld->prepare();
    mDraw->start(pWorld);

    LiveActor* pActor = mActor;

    if (pActor != nullptr) {
        MR::forceDeleteEffectAll(pActor);
        pActor->kill();
    }
}

bool WorldMapResultCommand::isResultActive() const {
    return mDraw->mWorld != nullptr;
}

s32 WorldMapResultCommand::getResultWorldId() const {
    if (mDraw->mWorld == nullptr) {
        return -1;
    }

    return mDraw->mWorld->mWorldId;
}

void WorldMapResultCommand::endResult() {
    if (mDraw->mWorld != nullptr) {
        mDraw->mWorld->finish();
        mDraw->end();

        if (mActor != nullptr) {
            mActor->appear();
        }
    }
}

void WorldMapResultCommand::setCameraEye(const TVec3f& rEye) {
    mDraw->mEye.set(rEye);
}

void WorldMapResultCommand::setCameraTarget(const TVec3f& rTarget) {
    mDraw->mTarget.set(rTarget);
}

const TVec3f& WorldMapResultCommand::getCameraEye() const {
    return mDraw->mEye;
}

const TVec3f& WorldMapResultCommand::getCameraTarget() const {
    return mDraw->mTarget;
}

void WorldMapResultCommand::movementDraw() {
    mDraw->movement();
}

void WorldMapResultCommand::calcAnimDraw() {
    mDraw->calcAnim();
}

void WorldMapResultCommand::calcViewAndEntryDraw() {
    mDraw->calcViewAndEntry();
}

void WorldMapResultCommand::drawDraw() {
    mDraw->draw();
}

bool WorldMapResultCommand::isActive() const {
    return mDraw->mWorld != nullptr;
}

bool WorldMapResultCommand::isStateDead() const {
    return MR::isActorStateDead(mController);
}

static const char* getCometLandName(s32 index) NO_INLINE;

static const char* getCometLandName(s32 index) {
    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();

    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        const char* pName = accessor.getName();

        if (GameSequenceFunction::isNewlyOpenedGalaxyAtResult(pName)) {
            if (index <= 0) {
                return pName;
            }

            index--;
        }

        iter.goNext();
    }

    return nullptr;
}

bool WorldMapResultCommand::tryStartResultMapStart() {
    if (isAnyResultReady()) {
        mController->startResultMapStart();
        return true;
    }

    return false;
}

bool WorldMapResultCommand::tryStartResultMapEnd() {
    if (isAnyResultReady()) {
        mController->startResultMapEnd();
        return true;
    }

    return false;
}

bool WorldMapResultCommand::tryRouteOpen() {
    return mController->tryRouteOpen();
}

void WorldMapResultCommand::startRouteCreateTico() {
    mController->startRouteCreateTico();
}

bool WorldMapResultCommand::updateController() {
    mController->update();
    return MR::isActorStateDead(mController);
}

bool WorldMapResultCommand::isAnyResultReady() {
    if (mController->isRouteOpenReady()) {
        return true;
    }

    if (getCometLandName(0) != nullptr) {
        return true;
    }

    return GameSequenceFunction::isClearedAllPowerStarAtResult();
}

bool WorldMapResultCommand::isWorldEnabled(s32 world) const {
    if (world == GameDataFunction::getCurrentWorldId()) {
        return true;
    }

    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }

    if (world == 7 && GameSequenceFunction::isClearedAllPowerStarAtResult()) {
        return true;
    }

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();

    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();

        if (GameSequenceFunction::isNewlyOpenedGalaxyAtResult(accessor.getName()) && world == accessor.getWorldNo()) {
            return true;
        }

        iter.goNext();
    }

    return false;
}

bool WorldMapResultCommand::tryCometAppear() {
    return mController->tryCometAppear();
}

void WorldMapResultCommand::endCurrent() {
    MR::endActorState(mController);
    s32 idx = GameDataFunction::getCurrentWorldId() - 1;
    mWorlds[idx]->disappear();
}

bool setWorldMapResultActor(LiveActor* pActor) {
    if (!MR::isExistSceneObj(SceneObj_WorldMapResultCommand)) {
        return false;
    }

    MR::getSceneObj< WorldMapResultCommand* >(SceneObj_WorldMapResultCommand)->mActor = pActor;
    return true;
}
