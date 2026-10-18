#pragma once

// Shared declarations of the WorldMapResult* translation units (SMG2-only; the target splits
// WorldMapResultCommand into ~25 TUs, see splits.txt).

#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/MiniRouteLine.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/System/WorldMapPointMarks.hpp"
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
    bool fn_80059100();
}

class WorldMapResultCommand;
class WorldMapOwner;
extern "C" {
    void fn_804F0700(WorldMapResultCommand*);
    void fn_804F08C0(WorldMapResultCommand*);
    void fn_804F0920(WorldMapResultCommand*);
    void fn_804F0BA0(WorldMapResultCommand*, const char*, s32);
    void fn_804F0C00(WorldMapResultCommand*, const char*);
    void fn_804F0C60(WorldMapResultCommand*, const char*, LiveActor*);
    void fn_804F0D00(WorldMapResultCommand*, const char*);
    bool fn_804F0D90(WorldMapResultCommand*, const char*, TVec3f*, TVec3f*, f32);
    s32 fn_804E8960(const char*);
    void fn_80458060(NameObj*);
    void fn_804581E0(NameObj*);
    void fn_80457F80();
    void fn_80458860();
    void fn_80458890();
    void fn_804588D0();
}

class WorldMapResultCommand;
class LayoutActor;
class WorldMapResultWorld;

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

    virtual ~MiniComet();
    virtual void init(const JMapInfoIter&);
    void appear();

    /* 0x90 */ const char* mName;
};

/// One step of a route: two map nodes, the step in which it opens and the direction.
struct WorldMapResultEdge {
    s32 mFrom;
    s32 mTo;
    s32 mStep;
    u8 mReverse;
    u8 _D[3];
};

/// Helper object (0x404 bytes) of the route-open state; its count lives at 0x400.
class WorldMapResultRoute {
public:
    WorldMapResultRoute();

    void collect(WorldMapResultWorld*);
    void addEdge(s32, s32);
    void subtract(WorldMapResultRoute*);

    WorldMapResultEdge mEdges[64];
    s32 mCount;
};

extern "C" {
    void fn_804F0970(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    void fn_804F09C0(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    void fn_804F0A40(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    bool fn_804F0B20(WorldMapResultCommand*, s32, WorldMapResultRoute*);
    bool fn_804F0EE0(WorldMapResultCommand*, s32);
}

// ---------------------------------------------------------------------------------------------------------------
// Classes of this translation unit
// ---------------------------------------------------------------------------------------------------------------

/// One node (island/point) of the result map (0x14 bytes).
struct WorldMapResultNode {
    WorldMapResultNode();

    TVec3f mPos;
    union {
        u8 _C[4];
        s32 mLayer;
    };
    u8 mIsHidden;
    u8 mFlag11;
    u8 mIsValid;
    u8 _13;
};

/// Node table of a world: count at 4, nodes at 8.
class WorldMapResultMapData {
public:
    WorldMapResultMapData(WorldMapResultWorld* pWorld);

    void load();
    void getLayerRange(s32*, s32*) const;
    bool isNodeValid(s32) const NO_INLINE;
    bool isNodeHidden(s32) const NO_INLINE;

    /* 0x00 */ WorldMapResultWorld* mWorld;
    /* 0x04 */ s32 mNum;
    /* 0x08 */ WorldMapResultNode* mNodes;
};

class WorldMapResultWorld;
class WorldMapResultBox;

/// Breadth-first path table between the nodes of a world.
class WorldMapResultPathFinder {
public:
    struct Entry {
        Entry();

        s32 mDistance;
        s32* mPath;
    };

    WorldMapResultPathFinder(WorldMapResultWorld*) NO_INLINE;

    void update(s32);
    bool isReachable(s32) const NO_INLINE;
    s32 getDistance(s32) const NO_INLINE;
    s32 getPath(s32, s32) const NO_INLINE;
    void allocate();

    /* 0x00 */ WorldMapResultWorld* mWorld;
    /* 0x04 */ Entry* mEntries;
};

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
        mWorldData->startAllPointingAnim();
    }

    JMapInfo* createParser(const char*) const;
    s32 getNodeNum() const NO_INLINE;
    WorldMapResultNode* getNode(s32) const;
    bool isNodeValid(s32) const NO_INLINE;
    bool isNodeHidden(s32) const;
    bool isNodeFlag11(s32) const;
    bool isLinked(s32, s32) const NO_INLINE;
    bool isLinked14(s32, s32) const NO_INLINE;
    bool isLinkedDirect(s32, s32) const;
    bool isPathReachable(s32) const;
    bool hasOpenLine(s32);
    bool hasOpenLineNoAlpha(s32);
    bool isNodeOpened(s32) const;
    void updatePaths(s32);
    s32 getPathDistance(s32) const;
    s32 getPathNode(s32, s32) const;
    bool setupNode(s32, s32);
    void refreshNode(s32);
    void refreshNodeFull(s32);
    bool isNodeNotReady(s32) const;
    bool hasWorldData(s32) const NO_INLINE;
    bool hasWorldDataAndComet(s32) const NO_INLINE;
    bool isWorldDataType(s32, s32) const NO_INLINE;
    bool isWorldDataType3(s32) const;
    bool isWorldDataType4(s32) const;
    bool isWorldDataType5(s32) const;
    bool callU14_6D0(s32) const;
    bool callU14_450(s32);
    bool callU14_860(s32);
    bool callU14_8A0(s32);
    bool callU14_620(s32);
    s32 callU14_2B0(s32);
    s32 callU14_310(s32);
    void setLinkProgress(s32, s32, f32);
    void setLinkProgressFull(s32, s32);
    s32 getWorldId() const;
    void setupRouteSteps(WorldMapResultRoute*, s32);
    void refreshRouteComet();
    void startRoutePointOpenSound(s32);
    void startRoutePointCloseSound(s32);
    void setupRoute(WorldMapResultRoute*);
    void setupRouteFromNode(WorldMapResultRoute*, s32);
    bool updateRouteStep(WorldMapResultRoute*, s32, f32);
    bool isRouteNotHeld(WorldMapResultRoute*) const;

    /* 0x00 */ s32 mWorldId;
    /* 0x04 */ WorldMapResultMapData* mMapData;
    /* 0x08 */ MiniRouteLineList* mLinks;
    /* 0x0C */ GalaxyWorldMapInfoHolder* mWorldData;
    /* 0x10 */ s32 _10;
    /* 0x14 */ PointPartsHolder* mUnit14;
    /* 0x18 */ WorldMapPointMarks* mUnit18;
    /* 0x1C */ WorldMapResultPathFinder* mPathFinder;
};

/// Axis aligned bounding box of the map: min at 0, max at 0xC.
class WorldMapResultBox {
public:
    WorldMapResultBox();

    void extend(const TVec3f&) NO_INLINE;
    void getCenter(TVec3f*) const NO_INLINE;
    void getCameraPos(TVec3f*) const NO_INLINE;
    f32 calcCameraDistance() const NO_INLINE;
    void extendByRoute(WorldMapResultWorld*, WorldMapResultRoute*);
    void extendByWorld(WorldMapResultWorld*);

    TVec3f mMin;
    TVec3f mMax;
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

#define WMR_STATE_DECL_HOST(host, name, extra)                                                                                                       \
    class name : public ActorStateBase< host > {                                                                                                     \
    public:                                                                                                                                          \
        name(host*);                                                                                                                                 \
        virtual ~name();                                                                                                                             \
        virtual void init();                                                                                                                         \
        virtual void appear();                                                                                                                       \
        virtual void kill();                                                                                                                         \
        extra                                                                                                                                        \
    }

/// State of the result cutscenes (host: WorldMapResultCommand).
#define WMR_STATE_DECL(name, extra) WMR_STATE_DECL_HOST(WorldMapResultCommand, name, extra)

/// State of the world-map sequence (WorldMapSequence; host: WorldMapOwner).
#define WMR_OWNER_STATE_DECL(name, extra) WMR_STATE_DECL_HOST(WorldMapOwner, name, extra)

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

s32 getResultEndStep() NO_INLINE;

// TU02 helpers (take the nerve rate in f1)
bool updateTicoRoad(WorldMapResultCommand*, f32);
bool updateRouteExtend(WorldMapResultCommand*, s32, WorldMapResultRoute*, s32, f32);
