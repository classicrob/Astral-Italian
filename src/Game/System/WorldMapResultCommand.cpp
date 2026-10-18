#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <cstdio>

// Declarations that are not in the shared headers yet (all symbols.txt-verified).
namespace GameDataFunction {
    s32 getCurrentWorldId();
}

namespace GameSequenceFunction {
    bool hasStageResultSequence();
    bool isGalaxyCometLand(const char*);
}

namespace MR {
    bool isActorStateDead(const NerveExecutor*);
    void endActorState(NerveExecutor*);
    bool updateActorState(NerveExecutor*);
    void closeSystemWipeMario(s32);
    void openSystemWipeMario(s32);
    void forceOpenSystemWipeMario();
}

extern "C" {
    bool fn_804D6E10();
    bool fn_80059100();
    bool fn_804D8320();
    void fn_804EAB40(void*);
    void fn_80458060(NameObj*);
    void fn_804581E0(NameObj*);
    void fn_80457F80();
    void fn_80458860();
    void fn_80458890();
    void fn_804588D0();
    void fn_8005AC70(s32, s32);
}

// ---------------------------------------------------------------------------------------------------------------
// Classes of this translation unit
// ---------------------------------------------------------------------------------------------------------------

/// One world of the result map ("WorldMapXX.arc"). Methods are defined in a later part of the unit.
class WorldMapResultWorld {
public:
    WorldMapResultWorld(s32);

    void initWorld(s32);
    void setNum(s32);
    void prepare();
    void finish();
    void disappear();
    void fn_EF190() NO_INLINE {
        fn_804EAB40(_C);
    }

    /* 0x00 */ s32 mWorldId;
    /* 0x04 */ s32 _4;
    /* 0x08 */ s32 _8;
    /* 0x0C */ void* _C;
    /* 0x10 */ s32 _10[4];
};

class WorldMapResultDraw : public NameObj {
public:
    WorldMapResultDraw();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void draw() const;
    virtual void calcAnim();
    virtual void calcViewAndEntry();

    void start(WorldMapResultWorld*);
    void end();

    /* 0x14 */ WorldMapResultWorld* mWorld;
    /* 0x18 */ TVec3f mEye;
    /* 0x24 */ TVec3f mTarget;
    /* 0x30 */ u8 _30[0x30];
    /* 0x60 */ SimpleLayout* mLayoutBG;
    /* 0x64 */ SimpleLayout* mLayoutName;
};

#define WMR_NERVE_DECL(name)                                                                                                                         \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine*) const;                                                                                                          \
        static name sInstance;                                                                                                                       \
    }

#define WMR_NERVE_DEF(name, parent_class, executor_name)                                                                                             \
    void name::execute(Spine* pSpine) const {                                                                                                        \
        reinterpret_cast< parent_class* >(pSpine->mExecutor)->exe##executor_name();                                                                  \
    }                                                                                                                                                \
    name name::sInstance ATTRIBUTE_WEAK

namespace NrvWorldMapResultStateEnd {
    WMR_NERVE_DECL(WorldMapResultStateEndWait);
    WMR_NERVE_DECL(WorldMapResultStateEndEnd);
};  // namespace NrvWorldMapResultStateEnd

#define WMR_STATE_DECL(name, extra)                                                                                                                 \
    class name : public ActorStateBase< WorldMapResultCommand > {                                                                                    \
    public:                                                                                                                                          \
        name(WorldMapResultCommand*);                                                                                                                \
        virtual ~name();                                                                                                                             \
        virtual void init();                                                                                                                         \
        virtual void appear();                                                                                                                       \
        virtual void kill();                                                                                                                         \
        extra                                                                                                                                        \
    }

WMR_STATE_DECL(WorldMapResultStateStart, );
WMR_STATE_DECL(WorldMapResultStateEnd, void exeWait(); void exeEnd(););
WMR_STATE_DECL(WorldMapResultStateRouteOpen, bool isXXX() const; u8 _10[0xC];);
WMR_STATE_DECL(WorldMapResultStateCometAppear, bool isXXX() const; u8 _10[0x38];);
WMR_STATE_DECL(WorldMapResultStateCreateTico, );

class WorldMapResultController : public NerveExecutor {
public:
    WorldMapResultController(WorldMapResultCommand*);

    virtual ~WorldMapResultController() {
    }

    void init();
    void update();
    bool isRouteOpenReady() const NO_INLINE;
    void startResultMapStart() NO_INLINE;
    void startResultMapEnd() NO_INLINE;
    bool tryRouteOpen() NO_INLINE;
    bool tryCometAppear() NO_INLINE;
    void startRouteCreateTico() NO_INLINE;

    void exeWait() {
    }

    void exeResultMapStart() {
        MR::updateActorState(this);
    }

    void exeResultMapEnd() {
        MR::updateActorState(this);
    }

    void exeRouteOpen() {
        MR::updateActorState(this);
    }

    void exeCometAppear() {
        MR::updateActorState(this);
    }

    void exeRouteCreateTico() {
        MR::updateActorState(this);
    }

    /* 0x08 */ WorldMapResultCommand* mCommand;
    /* 0x0C */ WorldMapResultStateCometAppear* mCometAppear;
    /* 0x10 */ WorldMapResultStateRouteOpen* mRouteOpen;
};

namespace NrvWorldMapResultController {
    NEW_NERVE(WorldMapResultControllerWait, WorldMapResultController, Wait);
    NEW_NERVE(WorldMapResultControllerResultMapStart, WorldMapResultController, ResultMapStart);
    NEW_NERVE(WorldMapResultControllerResultMapEnd, WorldMapResultController, ResultMapEnd);
    NEW_NERVE(WorldMapResultControllerRouteOpen, WorldMapResultController, RouteOpen);
    NEW_NERVE(WorldMapResultControllerCometAppear, WorldMapResultController, CometAppear);
    NEW_NERVE(WorldMapResultControllerRouteCreateTico, WorldMapResultController, RouteCreateTico);
};  // namespace NrvWorldMapResultController

// ---------------------------------------------------------------------------------------------------------------
// WorldMapResultCommand
// ---------------------------------------------------------------------------------------------------------------

WorldMapResultCommand::WorldMapResultCommand() : NameObj("ワールドマップ（リザルト指揮）") {
    mWorldNum = 7;
    mWorlds = nullptr;
    mController = nullptr;
    mDraw = nullptr;
    mRocket = nullptr;
    mActor = nullptr;
}

void WorldMapResultCommand::init(const JMapInfoIter&) {
    mWorlds = new WorldMapResultWorld*[mWorldNum];
    for (s32 i = 0; i < mWorldNum; i++) {
        s32 world = i + 1;
        if (isWorldEnabled(world)) {
            mWorlds[i] = new WorldMapResultWorld(world);
            mWorlds[i]->initWorld(0);
            mWorlds[i]->setNum(9);
        } else {
            mWorlds[i] = nullptr;
        }
    }
    mDraw = new WorldMapResultDraw();
    mDraw->initWithoutIter();
    mController = new WorldMapResultController(this);
    mController->init();
    mRocket = new LiveActor("顔惑星");
    MR::initActor(mRocket, "MiniPlayerRocket", "ResultMap", false);
    mRocket->initWithoutIter();
    mRocket->makeActorDead();
}

WorldMapResultCommand::~WorldMapResultCommand() {
}

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
        mWorld->fn_EF190();
        fn_80457F80();
    }
}

void WorldMapResultDraw::calcAnim() {
    if (mWorld != nullptr) {
        fn_80458060(this);
    }
}

void WorldMapResultDraw::calcViewAndEntry() {
    if (mWorld != nullptr) {
        fn_804581E0(this);
    }
}

void WorldMapResultDraw::draw() const {
    if (mWorld != nullptr) {
        fn_80458860();
        MR::reinitGX();
        MR::clearZBuffer();
        MR::loadProjectionMtx();
        MR::loadViewMtx();
        fn_80458890();
        fn_804588D0();
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

        if (GameSequenceFunction::isGalaxyCometLand(pName)) {
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

    return fn_804D6E10();
}

bool WorldMapResultCommand::isWorldEnabled(s32 world) const {
    if (world == GameDataFunction::getCurrentWorldId()) {
        return true;
    }

    if (!GameSequenceFunction::hasStageResultSequence()) {
        return false;
    }

    if (world == 7 && fn_804D6E10()) {
        return true;
    }

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();

    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();

        if (GameSequenceFunction::isGalaxyCometLand(accessor.getName()) && world == accessor.getWorldNo()) {
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

WorldMapResultController::WorldMapResultController(WorldMapResultCommand* pCommand)
    : NerveExecutor("シーケンス管理"), mCommand(pCommand), mCometAppear(nullptr), mRouteOpen(nullptr) {
}

void WorldMapResultController::init() {
    initNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerWait), 8);
    mCometAppear = new WorldMapResultStateCometAppear(mCommand);
    mRouteOpen = new WorldMapResultStateRouteOpen(mCommand);
    MR::initActorState(this, new WorldMapResultStateStart(mCommand), GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapStart),
                       "ResultMapStart");
    MR::initActorState(this, new WorldMapResultStateEnd(mCommand), GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapEnd),
                       "ResultMapEnd");
    MR::initActorState(this, mRouteOpen, GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteOpen), "RouteOpen");
    MR::initActorState(this, mCometAppear, GET_NERVE(WorldMapResultController, WorldMapResultControllerCometAppear), "CometAppear");
    MR::initActorState(this, new WorldMapResultStateCreateTico(mCommand),
                       GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteCreateTico), "RouteCreateTico");
}

void WorldMapResultController::update() {
    updateNerve();
}

bool WorldMapResultController::isRouteOpenReady() const {
    return mRouteOpen->isXXX();
}

void WorldMapResultController::startResultMapStart() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapStart));
}

void WorldMapResultController::startResultMapEnd() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapEnd));
}

bool WorldMapResultController::tryRouteOpen() {
    if (mRouteOpen->isXXX()) {
        setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteOpen));
        return true;
    }

    return false;
}

bool WorldMapResultController::tryCometAppear() {
    if (mCometAppear->isXXX()) {
        setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerCometAppear));
        return true;
    }

    return false;
}

void WorldMapResultController::startRouteCreateTico() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteCreateTico));
}


s32 getResultEndStep() NO_INLINE;

WorldMapResultStateEnd::WorldMapResultStateEnd(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[リザルトマップ終了]", pHost) {
}

void WorldMapResultStateEnd::init() {
    initNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndWait), 0);
    mIsDead = true;
}

void WorldMapResultStateEnd::appear() {
    MR::closeSystemWipeMario(-1);
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndWait));
}

void WorldMapResultStateEnd::kill() {
    mHost->endResult();

    if (fn_804D8320()) {
        MR::forceOpenSystemWipeMario();
        MR::forceCloseWipeFade();
    } else {
        MR::openSystemWipeMario(-1);
    }

    mIsDead = true;
}

WorldMapResultStateEnd::~WorldMapResultStateEnd() {
}


inline void WorldMapResultStateEnd::exeWait() {
    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndEnd));
    } else {
        fn_8005AC70(12, 30);
    }
}

inline void WorldMapResultStateEnd::exeEnd() {
    if (MR::isStep(this, getResultEndStep())) {
        kill();
    } else {
        fn_8005AC70(12, 30);
    }
}

namespace NrvWorldMapResultStateEnd {
    WMR_NERVE_DEF(WorldMapResultStateEndWait, WorldMapResultStateEnd, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEndEnd, WorldMapResultStateEnd, End);
};  // namespace NrvWorldMapResultStateEnd

s32 getResultEndStep() {
    return 5;
}
