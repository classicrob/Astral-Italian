#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_MUL_NOINLINE
#define JGEOMETRY_VEC3_MI_NOINLINE
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"

#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/System/WorldMapResultStateTicoRouteCreate.hpp"


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

WorldMapResultStateTicoRouteCreate::WorldMapResultStateTicoRouteCreate(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[チコのルート作り]", pHost) {
    mTicoCreator = nullptr;
}

void WorldMapResultStateTicoRouteCreate::init() {
    mHost->declareCamera("接近（ルートチコ）");
    mLayout = new GalaxyConfirmLayout(1);
    mLayout->initWithoutIter();
    initNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateWait), 0);
    mIsDead = true;
}

void WorldMapResultStateTicoRouteCreate::appear() {
    mIsDead = false;
    mHost->changeRocketAction("Wait");
    mHost->startCameraAtCurrent("接近（ルートチコ）", -1);
    const TVec3f* pPos = mHost->getRocketPosition();
    mPos.x = pPos->x;
    mPos.y = pPos->y;
    mPos.z = pPos->z;
    setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateZoomIn));
}

void WorldMapResultStateTicoRouteCreate::kill() {
    if (mTicoCreator != nullptr) {
        mTicoCreator = nullptr;
    }
    mIsDead = true;
}

void WorldMapResultStateTicoRouteCreate::control() {
    mHost->updateParts();
    MR::setDepthOfFieldBlurIntensity(1.0f);
    MR::setDepthOfFieldBlurDistance(0x22C4, 0x23F0);
}

void WorldMapResultStateTicoRouteCreate::updateRoutePosition(f32 pT) {
    TVec3f base(mPos + lbl_8072BB20);
    TVec3f scaled(base * (1.0f - pT));
    TVec3f moved(mPos * pT);
    TVec3f pos(moved + scaled);
    MR::setPosition(mHost->getRocket(), pos);
}

// NONMATCHING: one extra scheduling slot in the scale expression and the position update order (1 insn)
void WorldMapResultStateTicoRouteCreate::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mHost->getRocket(), "ZoomInTico", -1, -1, -1);
    }

    f32 rate = MR::calcNerveRate(this, 30);
    f32 eased = MR::getEaseInOutValue(1.0f - rate, 0.0f, 1.0f, 1.0f);
    updateRoutePosition(eased);

    f32 inRate = MR::calcNerveEaseInRate(this, 30);
    f32 scale = (1.0f - 0.5f) + (1.0f - inRate) * 0.5f;
    MR::setScale(mHost->getRocket(), scale);

    if (MR::isStep(this, 20)) {
        MR::turnOnDepthOfField(false);
    }

    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateTicoRouteCreate, WorldMapResultStateTicoRouteCreateWait));
    }
}

void WorldMapResultStateTicoRouteCreate::exeWait() {
    if (MR::isFirstStep(this)) {
        mLayout->appear();
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
        mHost->startDemo();
        mTicoCreator->startGoMarioPlanet();
        MR::hidePlayer();
    }

    if (MR::isStep(this, 80)) {
        mHost->startRocketBounce();
    }

    if (MR::isGreaterStep(this, 100)) {
        MiniTicoMaster* pCreator = mTicoCreator;
        GameSequenceFunction::setTicoFatStarPieceId(pCreator->mUnkA0);
        GameSequenceFunction::setTicoFatStarPieceRequirement(*pCreator->mUnk94);
        mHost->enterCurrentGalaxy();
        MR::startStarPointerModeDemo(mHost);
        MR::startActionSound(mHost->getRocket(), "TicoReturnHomeWipeIn", -1, -1, -1);
        MR::stopStageBGM(15);
        MR::onDestroySceneKeepAllSound();
    }
}

// NONMATCHING: scale expression scheduled with the constant loads in a different order (1 insn)
void WorldMapResultStateTicoRouteCreate::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        mHost->startNodeCamera(-1, -1);
        MR::startActionSound(mHost->getRocket(), "ZoomOutTico", -1, -1, -1);
    }

    f32 rate = MR::calcNerveRate(this, 30);
    f32 eased = MR::getEaseInOutValue(rate, 0.0f, 1.0f, 1.0f);
    updateRoutePosition(eased);

    f32 outRate = MR::calcNerveEaseInOutRate(this, 30);
    f32 scale = (1.0f - 0.5f) + (1.0f - (1.0f - outRate)) * 0.5f;
    MR::setScale(mHost->getRocket(), scale);

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
