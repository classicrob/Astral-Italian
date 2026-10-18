#include "Game/System/WorldMapResultLocal.hpp"

extern "C" {
    bool fn_804F71E0(WorldMapResultCommand*);
    void fn_804E8780(WorldMapResultCommand*);
    void fn_804E8850(WorldMapResultCommand*);
    void fn_804F7580(WorldMapResultCommand*, const char*);
    void fn_804F4CB0(s32);
}

namespace MR {
    void hidePlayer();
    void showPlayer();
}

extern s32 sOpenGalaxyStep;

namespace NrvWorldMapResultStateOpenGalaxy {
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyOpen);
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyWait);
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyClose);
};  // namespace NrvWorldMapResultStateOpenGalaxy

/// Result-map step that opens the galaxy ("State[ギャラクシーをオープンする]").
class WorldMapResultStateOpenGalaxy : public NerveExecutor {
public:
    WorldMapResultStateOpenGalaxy(WorldMapResultCommand*);
    virtual ~WorldMapResultStateOpenGalaxy();

    void init();
    void appear();
    void kill();
    void exeOpen();
    void exeWait() {
        if (MR::isGreaterEqualStep(this, 0x96)) {
            setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyClose));
        }
    }
    void exeClose() {
        if (MR::isGreaterEqualStep(this, 0x1E)) {
            setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyWait));
        }
    }

    // vtable slots 0x10 and 0x14 (the second is called from exeOpen); both defined by the owning TU.
    virtual void onStart();
    virtual void onOpenEnd();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ WorldMapResultCommand* mHost;
    /* 0x10 */ f32 _10;
    /* 0x14 */ f32 _14;
    /* 0x18 */ f32 _18;
};


WorldMapResultStateOpenGalaxy::WorldMapResultStateOpenGalaxy(WorldMapResultCommand* pHost) : NerveExecutor("State[ギャラクシーをオープンする]") {
    mIsDead = true;
    mHost = pHost;
    _10 = 0.0f;
    _14 = 0.0f;
    _18 = 0.0f;
}

void WorldMapResultStateOpenGalaxy::init() {
    initNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyOpen), 0);
    mIsDead = true;
}

void WorldMapResultStateOpenGalaxy::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyOpen));
    if (fn_804F71E0(mHost)) {
        fn_804E8780(mHost);
        fn_804F7580(mHost, "Wait");
        MR::hidePlayer();
    } else {
        mIsDead = true;
    }
}

void WorldMapResultStateOpenGalaxy::kill() {
    mIsDead = true;
}

void WorldMapResultStateOpenGalaxy::exeOpen() {
    MR::isFirstStep(this);
    if (MR::isGreaterEqualStep(this, sOpenGalaxyStep)) {
        fn_804E8850(mHost);
        MR::showPlayer();
        onOpenEnd();
    }
}

WorldMapResultStateOpenGalaxy::~WorldMapResultStateOpenGalaxy() {
}

namespace NrvWorldMapResultStateOpenGalaxy {
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyOpen, WorldMapResultStateOpenGalaxy, Open);
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyWait, WorldMapResultStateOpenGalaxy, Wait);
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyClose, WorldMapResultStateOpenGalaxy, Close);
};  // namespace NrvWorldMapResultStateOpenGalaxy
