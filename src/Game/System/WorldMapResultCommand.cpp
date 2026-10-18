#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <cstdio>

extern "C" {
    bool fn_804D6E10();
    bool fn_80059100();
    void fn_8005BC20(s32);
}

class WorldMapResultCommand;
extern "C" {
    void fn_804F0700(WorldMapResultCommand*);
    void fn_804F0810(WorldMapResultCommand*);
    void fn_804F08C0(WorldMapResultCommand*);
    void fn_804F0920(WorldMapResultCommand*);
    void fn_804F0BA0(WorldMapResultCommand*, const char*, s32);
    void fn_804F0C00(WorldMapResultCommand*, const char*);
    void fn_804F0C60(WorldMapResultCommand*, const char*, LiveActor*);
    void fn_804F0D00(WorldMapResultCommand*, const char*);
    bool fn_804F0D90(WorldMapResultCommand*, const char*, TVec3f*, TVec3f*, f32);
    s32 fn_804E8960(const char*);
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

class WorldMapResultCommand;
class LayoutActor;

/// Layout used by the comet-appear state (0x44 bytes).
class WorldMapResultCometLayout : public LayoutActor {
public:
    WorldMapResultCometLayout(s32);

    void setComet(const char*, s32);

    u8 _2C[0x14];
    u8 mFlag;
    u8 _41[3];
};

/// Demo comet shown on the map (0x94 bytes).
class MiniComet : public LiveActor {
public:
    MiniComet(const char*);

    u8 _90[4];
};

/// Opaque helper object (0x404 bytes) of the route-open state; its count lives at 0x400.
class WorldMapResultRoute {
public:
    WorldMapResultRoute();

    u8 _0[0x400];
    s32 mCount;
};

extern "C" {
    void fn_804F0970(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    void fn_804F09C0(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    void fn_804F0A40(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    bool fn_804F0B00(WorldMapResultCommand*, s32, WorldMapResultRoute*, s32);
    bool fn_804F0B20(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    void fn_804F0EE0(WorldMapResultCommand*, s32);
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

namespace NrvWorldMapResultStateStart {
    WMR_NERVE_DECL(WorldMapResultStateStartWait);
    WMR_NERVE_DECL(WorldMapResultStateStartEnd);
};  // namespace NrvWorldMapResultStateStart

namespace NrvWorldMapResultStateRouteOpen {
    WMR_NERVE_DECL(WorldMapResultStateRouteOpenStart);
    WMR_NERVE_DECL(WorldMapResultStateRouteOpenExtend);
    WMR_NERVE_DECL(WorldMapResultStateRouteOpenWaitNext);
    WMR_NERVE_DECL(WorldMapResultStateRouteOpenMessage);
    WMR_NERVE_DECL(WorldMapResultStateRouteOpenEnd);
};  // namespace NrvWorldMapResultStateRouteOpen

namespace NrvWorldMapResultStateCreateTico {
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoClose);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoOpen);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoExtend);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoWhiteClose);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoWhiteWait);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoWhiteOpen);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoMessage);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoCloseEnd);
    WMR_NERVE_DECL(WorldMapResultStateCreateTicoEnd);
};  // namespace NrvWorldMapResultStateCreateTico

namespace NrvWorldMapResultStateCometAppear {
    WMR_NERVE_DECL(WorldMapResultStateCometAppearWait);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearClose);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearOpen);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearAppearComet);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearMove);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearZoom);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearMessage);
    WMR_NERVE_DECL(WorldMapResultStateCometAppearEnd);
};  // namespace NrvWorldMapResultStateCometAppear

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

WMR_STATE_DECL(WorldMapResultStateStart, void exeWait(); void exeEnd(););
WMR_STATE_DECL(WorldMapResultStateEnd, void exeWait(); void exeEnd(););
WMR_STATE_DECL(WorldMapResultStateRouteOpen, virtual void control(); bool isXXX() const;
               void exeStart() {
                   MR::isFirstStep(this);

                   if (!MR::isSystemWipeActive()) {
                       setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenExtend));
                   }
               } void exeExtend(); void exeWaitNext(); void exeMessage(); void exeEnd();
               WorldMapResultRoute* mRoute; s32 mCount; s32 mWorldId;);
WMR_STATE_DECL(WorldMapResultStateCometAppear, bool isXXX() const NO_INLINE; const char* getCurrentComet() const NO_INLINE; void setupComet();
               void exeWait() {
                   if (!MR::isSystemWipeActive()) {
                       setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearAppearComet));
                   }
               } void exeClose(); void exeOpen(); void exeAppearComet(); void exeMove(); void exeZoom(); void exeMessage(); void exeEnd();
               bool mDone; WorldMapResultCometLayout* mLayout; s32 _18; MiniComet* mComet; MR::AssignableArray< const char* > mNames; s32 mNum;
               u32 mCurrent; TVec3f mFrom; TVec3f mTo;);
WMR_STATE_DECL(WorldMapResultStateCreateTico,
               void exeWhiteWait() {
                   if (MR::isStep(this, 60)) {
                       setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoWhiteOpen));
                   }
               } void exeClose(); void exeOpen(); void exeExtend(); void exeWhiteClose(); void exeWhiteOpen(); void exeMessage(); void exeCloseEnd();
               void exeEnd(););

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

WorldMapResultStateStart::WorldMapResultStateStart(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[リザルトマップ開始]", pHost) {
}

void WorldMapResultStateStart::init() {
    initNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartWait), 0);
    mIsDead = true;
}

void WorldMapResultStateStart::appear() {
    MR::closeSystemWipeMario(-1);
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartWait));
    fn_8005BC20(0x32);
}

void WorldMapResultStateStart::kill() {
    MR::openSystemWipeMario(-1);
    mIsDead = true;
}

WorldMapResultStateStart::~WorldMapResultStateStart() {
}

inline void WorldMapResultStateStart::exeWait() {
    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartEnd));
    } else {
        fn_8005AC70(12, 30);
    }
}

inline void WorldMapResultStateStart::exeEnd() {
    if (MR::isStep(this, getResultEndStep())) {
        kill();
    } else {
        fn_8005AC70(12, 30);
    }
}

namespace NrvWorldMapResultStateStart {
    WMR_NERVE_DEF(WorldMapResultStateStartWait, WorldMapResultStateStart, Wait);
    WMR_NERVE_DEF(WorldMapResultStateStartEnd, WorldMapResultStateStart, End);
};  // namespace NrvWorldMapResultStateStart

WorldMapResultStateRouteOpen::WorldMapResultStateRouteOpen(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[ルートオープン]", pHost) {
    mRoute = nullptr;
    mCount = 0;
    mWorldId = 0;
}

void WorldMapResultStateRouteOpen::init() {
    mRoute = new WorldMapResultRoute();
    initNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenStart), 0);

    if (GameSequenceFunction::hasStageResultSequence() && fn_804D6E10()) {
        mWorldId = 7;
        fn_804F09C0(mHost, 7, mRoute);
    } else {
        mWorldId = GameDataFunction::getCurrentWorldId();
        fn_804F0970(mHost, mWorldId, mRoute);
    }

    kill();
}

void WorldMapResultStateRouteOpen::appear() {
    mCount = 0;
    mIsDead = false;
    mHost->startResult(mWorldId);
    fn_804F0A40(mHost, mWorldId, mRoute);
    setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenStart));
}

void WorldMapResultStateRouteOpen::control() {
    fn_804F0EE0(mHost, mWorldId);
}

bool WorldMapResultStateRouteOpen::isXXX() const {
    return mRoute->mCount > 0;
}

void WorldMapResultStateRouteOpen::exeExtend() {
    WorldMapResultCommand* pHost = mHost;
    MR::calcNerveRate(this, 30);
    bool done = fn_804F0B00(pHost, mWorldId, mRoute, mCount);
    MR::startActionSound(mHost->mRocket, "RoadExtend", -1, -1, -1);

    if (MR::isGreaterStep(this, 30)) {
        if (done) {
            setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenMessage));
        } else {
            mCount++;
            setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenExtend));
        }
    }
}

void WorldMapResultStateRouteOpen::exeWaitNext() {
    if (fn_804F0B20(mHost, mWorldId, mRoute)) {
        setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenMessage));
    }
}

void WorldMapResultStateRouteOpen::exeMessage() {
    if (MR::isStep(this, 12)) {
        MR::appearInformationMessageDown("System_ResultWorldMapRouteOpen", true);
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
    }

    if (!MR::isLessEqualStep(this, 12) && MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenEnd));
    }
}

inline void WorldMapResultStateRouteOpen::exeEnd() {
    if (MR::isStep(this, 12)) {
        kill();
    }
}

WorldMapResultStateRouteOpen::~WorldMapResultStateRouteOpen() {
}

namespace NrvWorldMapResultStateRouteOpen {
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenStart, WorldMapResultStateRouteOpen, Start);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenExtend, WorldMapResultStateRouteOpen, Extend);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenWaitNext, WorldMapResultStateRouteOpen, WaitNext);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenMessage, WorldMapResultStateRouteOpen, Message);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenEnd, WorldMapResultStateRouteOpen, End);
};  // namespace NrvWorldMapResultStateRouteOpen

WorldMapResultStateCreateTico::WorldMapResultStateCreateTico(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[チコによるルート作成]", pHost) {
}

void WorldMapResultStateCreateTico::init() {
    initNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoClose), 0);
    mIsDead = true;
}

void WorldMapResultStateCreateTico::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoClose));
}

void WorldMapResultStateCreateTico::kill() {
    mIsDead = true;
    mHost->endResult();
}

void WorldMapResultStateCreateTico::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::closeSystemWipeMario(-1);
    }

    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoOpen));
    }
}

void WorldMapResultStateCreateTico::exeOpen() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            mHost->startResult(-1);
            fn_804F0700(mHost);
            MR::openSystemWipeMario(-1);
            return;
        }

        if (!MR::isSystemWipeActive()) {
            setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoExtend));
        }
    }
}

void WorldMapResultStateCreateTico::exeExtend() {
    WorldMapResultCommand* pHost = mHost;
    MR::calcNerveRate(this, 90);
    fn_804F0810(pHost);
    MR::startActionSound(mHost->mRocket, "RoadExtend", -1, -1, -1);

    if (MR::isStep(this, 90)) {
        fn_804F08C0(mHost);
    }

    if (MR::isGreaterEqualStep(this, 135)) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoWhiteClose));
    }
}

void WorldMapResultStateCreateTico::exeWhiteClose() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(60);
    }

    if (!MR::isWipeActive()) {
        fn_804F0920(mHost);
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoWhiteWait));
    }
}

void WorldMapResultStateCreateTico::exeWhiteOpen() {
    if (MR::isFirstStep(this)) {
        MR::openWipeWhiteFade(60);
    }

    if (!MR::isWipeActive()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoMessage));
    }
}

void WorldMapResultStateCreateTico::exeMessage() {
    if (MR::isFirstStep(this)) {
        MR::appearInformationMessageDown("InformationRouteCreateTico", true);
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
    }

    if (MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoCloseEnd));
    }
}

void WorldMapResultStateCreateTico::exeCloseEnd() {
    if (MR::isFirstStep(this)) {
        MR::closeSystemWipeMario(-1);
    }

    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoEnd));
    }
}

void WorldMapResultStateCreateTico::exeEnd() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            MR::openSystemWipeMario(-1);
            kill();
        }
    }
}

WorldMapResultStateCreateTico::~WorldMapResultStateCreateTico() {
}

namespace NrvWorldMapResultStateCreateTico {
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoClose, WorldMapResultStateCreateTico, Close);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoOpen, WorldMapResultStateCreateTico, Open);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoExtend, WorldMapResultStateCreateTico, Extend);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteClose, WorldMapResultStateCreateTico, WhiteClose);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteWait, WorldMapResultStateCreateTico, WhiteWait);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteOpen, WorldMapResultStateCreateTico, WhiteOpen);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoMessage, WorldMapResultStateCreateTico, Message);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoCloseEnd, WorldMapResultStateCreateTico, CloseEnd);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoEnd, WorldMapResultStateCreateTico, End);
};  // namespace NrvWorldMapResultStateCreateTico

WorldMapResultStateCometAppear::WorldMapResultStateCometAppear(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[コメット飛来]", pHost), mDone(false), mLayout(nullptr), mComet(nullptr), mNames(), mNum(0),
      mCurrent(0), mFrom(0.0f, 0.0f, 0.0f), mTo(0.0f, 0.0f, 100.0f) {
}

WorldMapResultStateCometAppear::~WorldMapResultStateCometAppear() {
}

void WorldMapResultStateCometAppear::init() {
    mLayout = new WorldMapResultCometLayout(0x50);
    mLayout->initWithoutIter();
    mLayout->mFlag = 0;
    mLayout->kill();
    mComet = new MiniComet("デモ用コメット");
    mComet->initWithoutIter();
    mComet->makeActorDead();
    initNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearWait), 0);

    if (GameSequenceFunction::hasStageResultSequence()) {
        s32 count = 0;
        ScenarioDataIter iter = MR::makeBeginScenarioDataIter();

        while (!iter.isEnd()) {
            GalaxyStatusAccessor accessor = iter.makeAccessor();

            if (GameSequenceFunction::isGalaxyCometLand(accessor.getName())) {
                count++;
            }

            iter.goNext();
        }

        mNames.init(count);

        if (count > 0) {
            ScenarioDataIter iter2 = MR::makeBeginScenarioDataIter();

            while (!iter2.isEnd()) {
                GalaxyStatusAccessor accessor = iter2.makeAccessor();
                const char* pName = accessor.getName();

                if (GameSequenceFunction::isGalaxyCometLand(pName)) {
                    s32 idx = mNum;
                    mNum = idx + 1;
                    mNames[idx] = pName;
                    fn_804F0BA0(mHost, pName, idx);
                }

                iter2.goNext();
            }
        }
    }

    mIsDead = true;
}

void WorldMapResultStateCometAppear::appear() {
    mComet->kill();
    mIsDead = false;

    if (mHost->isResultActive()) {
        WorldMapResultCommand* pHost = mHost;
        s32 world = fn_804E8960(getCurrentComet());

        s32 id = pHost->getResultWorldId();

        if (id == world) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearZoom));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearClose));
        return;
    }

    setupComet();
    setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearWait));
}

void WorldMapResultStateCometAppear::kill() {
    mLayout->kill();
    mIsDead = true;
}

bool WorldMapResultStateCometAppear::isXXX() const {
    return mNum > 0;
}

const char* WorldMapResultStateCometAppear::getCurrentComet() const {
    return mNames[mCurrent];
}

void WorldMapResultStateCometAppear::setupComet() {
    WorldMapResultCommand* pHost = mHost;
    fn_804F0D00(pHost, getCurrentComet());
    WorldMapResultCommand* pHost2 = mHost;
    pHost2->startResult(fn_804E8960(getCurrentComet()));
    WorldMapResultCommand* pHost3 = mHost;
    fn_804F0D90(pHost3, getCurrentComet(), &mFrom, &mTo, 1.0f);
}

void WorldMapResultStateCometAppear::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }

    if (MR::isWipeBlank()) {
        setupComet();
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearOpen));
    }
}

void WorldMapResultStateCometAppear::exeOpen() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            MR::openWipeFade(-1);
        }

        if (MR::isWipeOpen()) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearAppearComet));
        }
    }
}

void WorldMapResultStateCometAppear::exeZoom() {
    if (MR::isFirstStep(this)) {
        mFrom.set(mHost->getCameraTarget());
        mTo.set(mHost->getCameraEye());
    }

    WorldMapResultCommand* pHost = mHost;
    f32 rate = MR::calcNerveRate(this, 20);

    if (fn_804F0D90(pHost, getCurrentComet(), &mFrom, &mTo, rate)) {
        if (mDone) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMove));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearAppearComet));
    }
}

void WorldMapResultStateCometAppear::exeAppearComet() {
    if (MR::isFirstStep(this)) {
        WorldMapResultCommand* pHost = mHost;
        fn_804F0C60(pHost, getCurrentComet(), mComet);
    }

    if (MR::isStep(this, 19)) {
        MR::startStageBGM("STM_SMG2_JG_COMETSE", false);
    }

    if (MR::isStep(this, 50)) {
        mDone = true;
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMove));
    }
}

void WorldMapResultStateCometAppear::exeMove() {
    if (MR::isFirstStep(this)) {
        WorldMapResultCommand* pHost = mHost;
        fn_804F0C00(pHost, getCurrentComet());
    }

    if (MR::isGreaterEqualStep(this, 90)) {
        mLayout->setComet(getCurrentComet(), 4);
    }

    if (MR::isGreaterEqualStep(this, 150)) {
        u32 next = mCurrent + 1;

        if (next < mNum) {
            mCurrent = next;
            WorldMapResultCommand* pHost = mHost;
            s32 world = fn_804E8960(getCurrentComet());
            s32 id = pHost->getResultWorldId();

            if (id == world) {
                setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearZoom));
                return;
            }

            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearClose));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMessage));
    }
}

void WorldMapResultStateCometAppear::exeMessage() {
    if (MR::isStep(this, 12)) {
        MR::appearInformationMessageDown("System_ResultWorldMapCometAppear", true);
        MR::startSubBGM("BGM_SMG2_JG_COMET", false);
    }

    mLayout->setComet(getCurrentComet(), 4);

    if (!MR::isLessEqualStep(this, 12) && MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearEnd));
    }
}

void WorldMapResultStateCometAppear::exeEnd() {
    mLayout->setComet(getCurrentComet(), 30);

    if (MR::isStep(this, 12)) {
        mIsDead = true;
    }
}

namespace NrvWorldMapResultStateCometAppear {
    WMR_NERVE_DEF(WorldMapResultStateCometAppearWait, WorldMapResultStateCometAppear, Wait);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearClose, WorldMapResultStateCometAppear, Close);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearOpen, WorldMapResultStateCometAppear, Open);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearAppearComet, WorldMapResultStateCometAppear, AppearComet);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearMove, WorldMapResultStateCometAppear, Move);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearZoom, WorldMapResultStateCometAppear, Zoom);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearMessage, WorldMapResultStateCometAppear, Message);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearEnd, WorldMapResultStateCometAppear, End);
};  // namespace NrvWorldMapResultStateCometAppear
