#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"

/// Layout of the enter-home confirm screen (0x3C bytes).
class GalaxyConfirmLayout : public LayoutActor {
public:
    GalaxyConfirmLayout(s32);

    bool isSelected() const;
};

/// Stage-info block returned by fn_804E8300: the camera actor sits at 0x54.
struct WorldMapResultCameraInfo {
    /* 0x00 */ u8 _0[0x54];
    /* 0x54 */ LiveActor* mActor;
};

extern "C" {
    void fn_804F8380(WorldMapResultCommand*, const char*);
    void fn_804F83E0(WorldMapResultCommand*, const char*, s32);
    void fn_804F8540(WorldMapResultCommand*, const char*, s32, s32);
    void fn_804F77A0(WorldMapResultCommand*, f32);
    void fn_804F7650(WorldMapResultCommand*, TVec3f*);
    void fn_804F6B10(WorldMapResultCommand*);
    void fn_804F6FE0(WorldMapResultCommand*);
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    void fn_804F7580(WorldMapResultCommand*, const char*);
    void fn_804E8780(WorldMapResultCommand*);
    WorldMapResultCameraInfo* fn_804E8300(WorldMapResultCommand*);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
    void* fn_804E8950(WorldMapResultCommand*);
    bool fn_804E8E90(void*);
    void fn_800592A0();
    void fn_800591E0();
    void fn_8001C030(s32);
    bool fn_8046F870(GalaxyConfirmLayout*);
    bool fn_8046F8D0(GalaxyConfirmLayout*);
    void fn_804F4CB0(s32);
}

namespace MR {
    void setImageEffectControlAuto();
    void startAnimCameraTargetSelf(const LiveActor*, const ActorCameraInfo*, const char*, s32, bool, f32);
    void initAnimCamera(const LiveActor*, const ActorCameraInfo*, const char*, ResourceHolder*);
    bool isAnimCameraEnd(const LiveActor*, const ActorCameraInfo*, const char*);
    void setScale(LiveActor*, f32);
    void setDepthOfFieldBlurDistance(s32, s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
    void onDestroySceneKeepAllSound();
}

extern const char* lbl_807D0570;
extern f32 lbl_807E3140;

/// Result-map step that enters the home galaxy ("State[ホームに入る]").
WMR_STATE_DECL(WorldMapResultStateEnterHome, virtual void control(); void exeApproach(); void exeWait(); void exeEnter(); void exeExit();
               /* 0x10 */ GalaxyConfirmLayout* mLayout; /* 0x14 */ ActorCameraInfo* mCamInfo;);

namespace NrvWorldMapResultStateEnterHome {
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeApproach);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeWait);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeEnter);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeExit);
};  // namespace NrvWorldMapResultStateEnterHome

static void setEnterHomeCamera(WorldMapResultCommand* pHost) {
    fn_804F77A0(pHost, 0.1f);
    TVec3f pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = -1.0f;
    fn_804F7650(pHost, &pos);
}

WorldMapResultStateEnterHome::WorldMapResultStateEnterHome(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ホームに入る]", pHost) {
    mLayout = nullptr;
}

// NONMATCHING: target builds a 0x20 frame and keeps host/actor in r29/r30 across calls (savegpr_29)
void WorldMapResultStateEnterHome::init() {
    fn_804F8380(mHost, "接近（ホーム）");
    fn_804F8380(mHost, "突入（ホーム）");
    mLayout = new GalaxyConfirmLayout(2);
    mLayout->initWithoutIter();
    MR::createCenterScreenBlur();
    initNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeApproach), 0);
    mCamInfo = new ActorCameraInfo(-1, 0);
    LiveActor* pActor = fn_804E8300(mHost)->mActor;
    MR::initAnimCamera(pActor, mCamInfo, lbl_807D0570, MR::getResourceHolder(fn_804E8470(mHost)));
    MR::createDepthOfFieldBlur();
    mIsDead = false;
}

void WorldMapResultStateEnterHome::appear() {
    mIsDead = false;
    fn_804F83E0(mHost, "接近（ホーム）", -1);
    fn_804F7580(mHost, "Wait");
    setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeApproach));
}

void WorldMapResultStateEnterHome::kill() {
    fn_804F8540(mHost, "接近（ホーム）", 0, -1);
    mIsDead = true;
}

void WorldMapResultStateEnterHome::control() {
    fn_804F6B10(mHost);
    MR::setDepthOfFieldBlurIntensity(1.0f);
    MR::setDepthOfFieldBlurDistance(0x22C4, 0x23F0);
}

// NONMATCHING: scale expression scheduled with the constant loads in a different order (few insns)
void WorldMapResultStateEnterHome::exeApproach() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(fn_804E8470(mHost), "ZoomInShip", -1, -1, -1);
    }

    f32 rate = MR::calcNerveEaseInRate(this, 30);
    f32 inv = 1.0f - rate;
    f32 scale = (1.0f - lbl_807E3140) + inv * lbl_807E3140;
    MR::setScale(fn_804E8470(mHost), scale);

    if (MR::isStep(this, 16)) {
        MR::turnOnDepthOfField(false);
    }

    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeWait));
    }
}

// NONMATCHING: NameObj vtable slot: target calls the 0x2C entry, our header maps endMovement to 0x28
void WorldMapResultStateEnterHome::exeWait() {
    if (MR::isFirstStep(this)) {
        mLayout->endMovement();
    }

    if (fn_8046F870(mLayout) && fn_8046F8D0(mLayout)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeEnter));
    } else if (fn_8046F870(mLayout) && !fn_8046F8D0(mLayout)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeExit));
    } else if (mLayout->isSelected() && fn_8046F8D0(mLayout)) {
        setEnterHomeCamera(mHost);
    }
}

void WorldMapResultStateEnterHome::exeEnter() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        LiveActor* pActor = fn_804E8300(mHost)->mActor;
        MR::startAnimCameraTargetSelf(pActor, mCamInfo, lbl_807D0570, -1, false, 1.0f);
        fn_8001C030(30);
        fn_804E8780(mHost);
    }

    if (MR::isStep(this, 15)) {
        MR::startSystemSE("SE_SY_WMAP_TO_SHIP", -1, -1);
        MR::onDestroySceneKeepAllSound();
    }

    setEnterHomeCamera(mHost);

    if (MR::isStep(this, 38)) {
        if (fn_804E8E90(fn_804E8950(mHost))) {
            fn_800592A0();
        } else {
            fn_800591E0();
        }
    }

    LiveActor* pActor = fn_804E8300(mHost)->mActor;
    if (MR::isAnimCameraEnd(pActor, mCamInfo, lbl_807D0570)) {
        fn_804F6FE0(mHost);
    }
}

// NONMATCHING: scale expression scheduled with the constant loads in a different order (few insns)
void WorldMapResultStateEnterHome::exeExit() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        fn_804F8540(mHost, "接近（ホーム）", 0, -1);
        fn_804F82C0(mHost, -1, -1);
        MR::startActionSound(fn_804E8470(mHost), "ZoomOutShip", -1, -1, -1);
    }

    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    f32 inv = 1.0f - rate;
    f32 scale = (1.0f - lbl_807E3140) + (1.0f - inv) * lbl_807E3140;
    MR::setScale(fn_804E8470(mHost), scale);

    if (MR::isGreaterEqualStep(this, 30)) {
        kill();
    }
}

WorldMapResultStateEnterHome::~WorldMapResultStateEnterHome() {
}

namespace NrvWorldMapResultStateEnterHome {
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeApproach, WorldMapResultStateEnterHome, Approach);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeWait, WorldMapResultStateEnterHome, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeEnter, WorldMapResultStateEnterHome, Enter);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeExit, WorldMapResultStateEnterHome, Exit);
};  // namespace NrvWorldMapResultStateEnterHome
