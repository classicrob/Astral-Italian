#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ClipArea.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

namespace {
    static const s32 sClipAreaMax = 0x200;
};  // namespace

namespace {
    ClipAreaHolder* getClipAreaHolder() {
        return MR::getSceneObj< ClipAreaHolder >(SceneObj_ClipAreaHolder);
    }
};  // namespace

ClipAreaHolder::ClipAreaHolder(const char* pName) : DeriveActorGroup< ClipArea >(pName, ::sClipAreaMax), mIsActive(true) {
    MR::FunctorV0F functor(&MR::setupShadowVolumeDraw);
    MR::registerPreDrawFunction(functor, 0x2E /* DrawType_ClipArea */);
}

void ClipAreaHolder::init(const JMapInfoIter& rIter) {
}

bool ClipAreaHolder::isInArea(const TVec3f& rParam1) const {
    if (!mIsActive) {
        return false;
    }

    s32 memberNum = getObjNum();

    for (s32 i = 0; i < memberNum; i++) {
        if (MR::isDead(getActor(i))) {
            continue;
        }

        if (!getMember(i)->isInArea(rParam1)) {
            continue;
        }

        return true;
    }
    return false;
}

namespace MR {
    void createClipAreaHolder() {
        createSceneObj(SceneObj_ClipAreaHolder);
    }

    bool isExistClipAreaHolder() {
        return isExistSceneObj(SceneObj_ClipAreaHolder);
    }

    void addClipArea(ClipArea* pClipArea) {
        ::getClipAreaHolder()->registerActor(pClipArea);
    }

    bool isInClipArea(const TVec3f& rParam1) {
        return ::getClipAreaHolder()->isInArea(rParam1);
    }

    bool isActiveClipArea() {
        return ::getClipAreaHolder()->mIsActive != false;
    }

    void activateClipArea() {
        ::getClipAreaHolder()->mIsActive = true;
    }

    void deactivateClipArea() {
        ::getClipAreaHolder()->mIsActive = false;
    }
};  // namespace MR
