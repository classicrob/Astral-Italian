#pragma once
// SCRATCH ONLY. Included only by GameScene.cpp under GALAXY_NATIVE.
#include <petari/wm_diag.hpp>
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraViewInterpolator.hpp"
#include "Game/Camera/CameraPoseParam.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace PetariWmDiagScene {
inline unsigned frame{};
inline void reset() { frame = 0; PetariWmDiag::worldFrame = false; PetariWmDiag::nodeCount = 0; }
struct Counts { unsigned total{}, alive{}, hidden{}, clipped{}; };
inline Counts parts, marks, galaxies;
inline void actor(const LiveActor* p, int point, Counts& count) {
    if (!p) return;
    ++count.total;
    count.alive += !p->mFlags.mIsDead;
    count.hidden += p->mFlags.mIsNoEntryDrawBuffer;
    count.clipped += p->mFlags.mIsClipped;
    if (p->mModelManager) PetariWmDiag::addNode(p->mModelManager->getJ3DModel(), point);
}
inline void snapshot(const char* phase) {
    auto* owner = MR::getSceneObj<WorldMapOwner>(0x33);
    const TVec3f eye = MR::getCamPos();
    const TVec3f up = MR::getCamYdir();
    const TVec3f forward = MR::getCamZdir();
    TVec3f target(0.0f), holderTarget(0.0f);
    auto* director = MR::getCameraDirector();
    const bool hasTarget = director && director->mViewInterpolator;
    if (hasTarget) target = director->mViewInterpolator->mTargetPosition;
    const bool hasHolder = owner && owner->mContext && owner->mContext->mCamera && owner->mContext->mCamera->mCameraTarget;
    if (hasHolder) holderTarget = owner->mContext->mCamera->mCameraTarget->mPosition;
    std::fprintf(stderr, "[wm-scene] %s stage=%s frame=%u eye=(%.5g,%.5g,%.5g) target=(%.5g,%.5g,%.5g) target-valid=%d target-source=CameraViewInterpolator up=(%.5g,%.5g,%.5g) forward=(%.5g,%.5g,%.5g) fovy=%.5g near=%.5g far=%.5g holder-target=(%.5g,%.5g,%.5g) holder-valid=%d\n",
        phase, MR::getCurrentStageName(), frame, eye.x, eye.y, eye.z, target.x, target.y, target.z, hasTarget,
        up.x, up.y, up.z, forward.x, forward.y, forward.z, MR::getFovy(), MR::getNearZ(), MR::getFarZ(),
        holderTarget.x, holderTarget.y, holderTarget.z, hasHolder);
    std::fprintf(stderr, "[wm-count] frame=%u owner-present=%d (NameObj; no alive flag) PointParts=%u/alive%u/hidden%u/clipped%u MiniRoutePoint=%u/alive%u/hidden%u/clipped%u galaxy=%u/alive%u/hidden%u/clipped%u drawn-unique-nodes=%u tracked-models=%u overflow=%u\n",
        frame, owner != nullptr, parts.total, parts.alive, parts.hidden, parts.clipped,
        marks.total, marks.alive, marks.hidden, marks.clipped, galaxies.total, galaxies.alive, galaxies.hidden, galaxies.clipped,
        PetariWmDiag::drawnNodes(), PetariWmDiag::nodeCount, PetariWmDiag::nodeOverflow);
    std::fflush(stderr);
}
inline void beginFrame() {
    if (!PetariWmDiag::enabled()) return;
    PetariWmDiag::worldFrame = MR::isStageWorldMap();
    PetariWmDiag::nodeCount = PetariWmDiag::nodeOverflow = 0;
    if (!PetariWmDiag::worldFrame) return;
    parts = {}; marks = {}; galaxies = {};
    auto* owner = MR::getSceneObj<WorldMapOwner>(0x33);
    if (owner && owner->mContext && owner->mContext->mWorld) {
        auto* p = owner->getPointParts();
        if (p) for (int i = 0; i < p->mNum; ++i) actor(p->mParts[i].mActor, p->mParts[i].mPointIndex, parts);
        auto* m = owner->getPointMarks();
        if (m) for (int i = 0; i < m->mNum; ++i) if (m->mMarks[i]) {
            auto* mark = m->mMarks[i]; actor(mark, mark->mPointIndex, marks);
            Counts attached; actor(mark->mShadow, mark->mPointIndex, attached);
        }
        auto* g = owner->getInfoHolder();
        if (g) for (int i = 0; i < g->mNum; ++i) if (g->mInfos[i]) {
            auto* info = g->mInfos[i]; actor(info, info->mPointIndex, galaxies);
            Counts attached; actor(info->mShadow, info->mPointIndex, attached);
            actor(info->mBack, info->mPointIndex, attached); actor(info->mFlag, info->mPointIndex, attached);
        }
    }
    // Capture before GX work as well, so a first-frame FIFO failure leaves a camera sample.
    if (frame == 0) snapshot("stage-start-before-draw");
}
inline void endFrame() {
    if (!PetariWmDiag::enabled() || !PetariWmDiag::worldFrame) return;
    if (frame % 60 == 0) snapshot("after-draw");
    ++frame;
}
}
