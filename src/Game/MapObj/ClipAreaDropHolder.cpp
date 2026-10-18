#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/MapObj/ClipAreaDrop.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/LiveActorUtil.hpp"

namespace {
    static const s32 sMaxClipAreaDrop = 256;
};  // namespace

ClipAreaDropHolder::ClipAreaDropHolder() : DeriveActorGroup< ClipAreaDrop >("クリップエリアのしずく管理", ::sMaxClipAreaDrop) {
    ClipAreaDrop* pClipAreaDrop = nullptr;

    for (int i = 0; i < ::sMaxClipAreaDrop; i++) {
        pClipAreaDrop = new ClipAreaDrop("クリップエリアのしずく", "VolumeSphereLow");
        pClipAreaDrop->initWithoutIter();
        registerActor(pClipAreaDrop);
    }
}

namespace MR {
    NameObj* createClipAreaDropHolder() {
        return MR::createSceneObj(SceneObj_ClipAreaDropHolder);
    }

    ClipAreaDrop* getDeadClipAreaDrop() {
        return MR::getSceneObj< ClipAreaDropHolder >(SceneObj_ClipAreaDropHolder)->getDeadMember();
    }

    ClipAreaDrop* appearClipAreaDrop(const TVec3f& rPos, f32 baseSize) {
        ClipAreaDrop* pClipAreaDrop = getDeadClipAreaDrop();

        if (pClipAreaDrop == nullptr) {
            return nullptr;
        }

        MR::setPosition(pClipAreaDrop, rPos);
        pClipAreaDrop->setBaseSize(baseSize);
        pClipAreaDrop->appear();
        pClipAreaDrop->startHold();

        return pClipAreaDrop;
    }
};  // namespace MR
