#pragma auto_inline off
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <cstdio>
#include <new>

MiniStarWorldPoint::MiniStarWorldPoint(WorldMapResultWorld* pWorld, s32 pointIndex, s32 targetWorld)
    : LiveActor("グランドギャラクシーマップ用ワープポイント") {
    mWorld = pWorld;
    mPointIndex = pointIndex;
    mTargetWorld = targetWorld;
    mLayout = nullptr;
}

static void setupCrownLayout(MiniStarWorldPoint* pPoint);

void MiniStarWorldPoint::init(const JMapInfoIter& rIter) {
    char name[0x80];
    snprintf(name, sizeof(name), "MiniWorld%02d", mTargetWorld);
    s32 result = MR::initActor(this, name, false);
    if (result == 0 && mModelManager != nullptr) {
        MR::connectToSceneMapObjDecoration(this);
        initEffectKeeper(8, nullptr, false);
    }

    setupCrownLayout(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 1000.0f);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}

#pragma dont_inline on
// NONMATCHING: stack temporaries (iterator/accessor slots, makeAccessor result copied to a second slot) differ; instruction set otherwise matches
// Builds the crown/completion layout of the star road node and shows the crown matching the world's progress.
static void setupCrownLayout(MiniStarWorldPoint* pPoint) {
    LayoutActor* pLayout = new SimpleFollowActorLayout(pPoint, "コンプリートアイコン", "MapGalaxyState", TVec3f(0.0f), 1, -1);
    pPoint->mLayout = pLayout;
    MR::registerDemoSimpleCastAll(pLayout);
    MR::hidePaneRecursive(pPoint->mLayout, "MapGalaxyState");
    MR::showPane(pPoint->mLayout, "MapGalaxyState");

    s32 flagA = 1;
    s32 flagB = 1;
    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();
    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        if (accessor.getWorldNo() == pPoint->mTargetWorld) {
            if (!accessor.isCompletedWithGreen()) {
                flagA = 0;
            }
            if (!accessor.isCompletedNoGreen()) {
                flagB = 0;
            }
            if (!flagA && !flagB) {
                break;
            }
        }
        iter.goNext();
    }

    if (flagB) {
        MR::showPaneRecursive(pPoint->mLayout, "Crown2");
    } else if (flagA) {
        MR::showPaneRecursive(pPoint->mLayout, "Crown1");
    }

    if (flagA || flagB) {
        pPoint->mLayout->appear();
        return;
    }

    pPoint->mLayout->kill();
}

#pragma dont_inline reset

void MiniStarWorldPoint::makeActorAppeared() {
    if (mWorld->hasOpenLineNoAlpha(mPointIndex)) {
        LiveActor::makeActorAppeared();
    }
}

// NONMATCHING: target keeps the 0.0f additions/subtractions (fsubs/fadds with a loaded 0.0f); folded here
void MiniStarWorldPoint::control() {
    mRotation.y += 0.1f;
    f32 angle = 360.0f + (mRotation.y - 0.0f);
    mRotation.y = 0.0f + (f32)fmod(angle, 360.0);
}

MiniStarWorldPoint::~MiniStarWorldPoint() {
}
