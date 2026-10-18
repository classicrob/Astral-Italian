#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"

#include "Game/Screen/GalaxyConfirmLayout.hpp"

extern "C" {
    void fn_804F8380(WorldMapResultCommand*, const char*);
    void fn_804F8470(WorldMapResultCommand*, const char*, s32);
    void fn_804F7580(WorldMapResultCommand*, const char*);
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    void fn_804F6B10(WorldMapResultCommand*);
    void fn_804F6FE0(WorldMapResultCommand*);
    void fn_804F77E0(WorldMapResultCommand*);
    void fn_804E8780(WorldMapResultCommand*);
    void fn_804E7170(void*);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
    TVec3f* fn_804E8510(WorldMapResultCommand*);
    void fn_804F4CB0(s32);
}

namespace MR {
    void setImageEffectControlAuto();
    void hidePlayer();
    void stopStageBGM(u32);
    void onDestroySceneKeepAllSound();
    void setPosition(LiveActor*, const TVec3f&);
    void setDepthOfFieldBlurDistance(s32, s32);
    f32 calcNerveRate(const NerveExecutor*, s32);
    f32 calcNerveEaseInRate(const NerveExecutor*, s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
    f32 getEaseInOutValue(f32, f32, f32, f32);
}

namespace MR {
    void startStarPointerModeDemo(void*);
}

extern TVec3f lbl_8072BB20;

/// Step that creates the tico route ("State[チコのルート作り]").
WMR_STATE_DECL(WorldMapResultStateTicoRouteCreate, virtual void control(); void exeZoomIn(); void exeWait(); void exeReturn(); void exeZoomOut(); void updateRoutePosition(f32);
               /* 0x10 */ TVec3f mPos; /* 0x1C */ GalaxyConfirmLayout* mLayout; /* 0x20 */ void* mStarInfo;);

namespace NrvWorldMapResultStateTicoRouteCreate {
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateWait);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateReturn);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateZoomOut);
};  // namespace NrvWorldMapResultStateTicoRouteCreate

WorldMapResultStateTicoRouteCreate::WorldMapResultStateTicoRouteCreate(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[チコのルート作り]", pHost) {
    mStarInfo = nullptr;
}

void WorldMapResultStateTicoRouteCreate::init() {
    fn_804F8380(mHost, "接近（ルートチコ）");
    mLayout = new GalaxyConfirmLayout(1);
    mLayout->initWithoutIter();
    initNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateWait), 0);
    mIsDead = true;
}

void WorldMapResultStateTicoRouteCreate::appear() {
    mIsDead = false;
    fn_804F7580(mHost, "Wait");
    fn_804F8470(mHost, "接近（ルートチコ）", -1);
    TVec3f* pPos = fn_804E8510(mHost);
    mPos.x = pPos->x;
    mPos.y = pPos->y;
    mPos.z = pPos->z;
    setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateZoomIn));
}

void WorldMapResultStateTicoRouteCreate::kill() {
    if (mStarInfo != nullptr) {
        mStarInfo = nullptr;
    }
    mIsDead = true;
}

void WorldMapResultStateTicoRouteCreate::control() {
    fn_804F6B10(mHost);
    MR::setDepthOfFieldBlurIntensity(1.0f);
    MR::setDepthOfFieldBlurDistance(0x22C4, 0x23F0);
}

void WorldMapResultStateTicoRouteCreate::updateRoutePosition(f32 pT) {
    TVec3f base(mPos + lbl_8072BB20);
    TVec3f scaled(base * (1.0f - pT));
    TVec3f moved(mPos * pT);
    TVec3f pos(moved + scaled);
    MR::setPosition(fn_804E8470(mHost), pos);
}

// NONMATCHING: one extra scheduling slot in the scale expression and the position update order (1 insn)
void WorldMapResultStateTicoRouteCreate::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(fn_804E8470(mHost), "ZoomInTico", -1, -1, -1);
    }

    f32 rate = MR::calcNerveRate(this, 30);
    f32 eased = MR::getEaseInOutValue(1.0f - rate, 0.0f, 1.0f, 1.0f);
    updateRoutePosition(eased);

    f32 inRate = MR::calcNerveEaseInRate(this, 30);
    f32 scale = (1.0f - 0.5f) + (1.0f - inRate) * 0.5f;
    MR::setScale(fn_804E8470(mHost), scale);

    if (MR::isStep(this, 20)) {
        MR::turnOnDepthOfField(false);
    }

    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateWait));
    }
}

// NONMATCHING: NameObj vtable slot: target calls the 0x2C entry, our header maps endMovement to 0x28
void WorldMapResultStateTicoRouteCreate::exeWait() {
    if (MR::isFirstStep(this)) {
        mLayout->endMovement();
    }

    if (mLayout->isSelectionComplete() && mLayout->isSelectedYes()) {
        setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateReturn));
    } else if (mLayout->isSelectionComplete() && !mLayout->isSelectedYes()) {
        setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateZoomOut));
    }
}

// NONMATCHING: the comet-name pointer loads are scheduled differently (target keeps the 0x94 chain first)
void WorldMapResultStateTicoRouteCreate::exeReturn() {
    if (MR::isFirstStep(this)) {
        fn_804E8780(mHost);
        fn_804E7170(mStarInfo);
        MR::hidePlayer();
    }

    if (MR::isStep(this, 80)) {
        fn_804F77E0(mHost);
    }

    if (MR::isGreaterStep(this, 100)) {
        u8* pInfo = static_cast< u8* >(mStarInfo);
        GameSequenceFunction::setTicoFatStarPieceId(*reinterpret_cast< s32* >(pInfo + 0xA0));
        GameSequenceFunction::setTicoFatStarPieceRequirement(**reinterpret_cast< s32** >(pInfo + 0x94));
        fn_804F6FE0(mHost);
        MR::startStarPointerModeDemo(mHost);
        MR::startActionSound(fn_804E8470(mHost), "TicoReturnHomeWipeIn", -1, -1, -1);
        MR::stopStageBGM(15);
        MR::onDestroySceneKeepAllSound();
    }
}

// NONMATCHING: scale expression scheduled with the constant loads in a different order (1 insn)
void WorldMapResultStateTicoRouteCreate::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        fn_804F82C0(mHost, -1, -1);
        MR::startActionSound(fn_804E8470(mHost), "ZoomOutTico", -1, -1, -1);
    }

    f32 rate = MR::calcNerveRate(this, 30);
    f32 eased = MR::getEaseInOutValue(rate, 0.0f, 1.0f, 1.0f);
    updateRoutePosition(eased);

    f32 outRate = MR::calcNerveEaseInOutRate(this, 30);
    f32 scale = (1.0f - 0.5f) + (1.0f - (1.0f - outRate)) * 0.5f;
    MR::setScale(fn_804E8470(mHost), scale);

    if (MR::isGreaterStep(this, 30)) {
        kill();
    }
}

WorldMapResultStateTicoRouteCreate::~WorldMapResultStateTicoRouteCreate() {
}

namespace NrvWorldMapResultStateTicoRouteCreate {
    WMR_NERVE_DEF(WorldMapResultStateTicoRouteCreateZoomIn, WorldMapResultStateTicoRouteCreate, ZoomIn);
    WMR_NERVE_DEF(WorldMapResultStateTicoRouteCreateWait, WorldMapResultStateTicoRouteCreate, Wait);
    WMR_NERVE_DEF(WorldMapResultStateTicoRouteCreateReturn, WorldMapResultStateTicoRouteCreate, Return);
    WMR_NERVE_DEF(WorldMapResultStateTicoRouteCreateZoomOut, WorldMapResultStateTicoRouteCreate, ZoomOut);
};  // namespace NrvWorldMapResultStateTicoRouteCreate

TVec3f lbl_8072BB20(-770.0f, -1500.0f, 290.0f);
