#pragma once

// Shared declarations of the WorldMapResult* translation units (SMG2-only; the target splits
// WorldMapResultCommand into ~25 TUs, see splits.txt).

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

s32 getResultEndStep() NO_INLINE;
