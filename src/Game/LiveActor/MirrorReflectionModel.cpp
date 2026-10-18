#include "Game/LiveActor/MirrorReflectionModel.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

MirrorReflectionModel::MirrorReflectionModel(const LiveActor* pActor, const char* pName, const char* pInternalName, MtxPtr mtx) : LiveActor(pName) {
    _90 = pActor;
    _94 = mtx;
    _98 = 0;

    if (mtx) {
        TPos3f pos;
        pos.set(mtx);
        pos.getTrans(mPosition);
    } else {
        MR::setPosition(this, pActor->mPosition);
    }

    MR::setRotation(this, _90->mRotation);
    MR::setScale(this, _90->mScale);
    MR::initActor(this, pInternalName, false);
    MR::connectToSceneMirrorMapObjDecoration(this);
}

void MirrorReflectionModel::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
    _98 = 0;
}

void MirrorReflectionModel::movement() {
    if (MR::isDead(_90) || MR::isClipped(_90) || MR::isHiddenModel(_90)) {
        if (!_98) {
            _98 = 1;

            if (!MR::isHiddenModel(this)) {
                MR::disconnectToDrawTemporarily(this);
            }
        }
    } else {
        if (_98) {
            _98 = 0;
            if (!MR::isHiddenModel(this)) {
                MR::connectToDrawTemporarily(this);
            }
        }

        LiveActor::movement();
    }
}

void MirrorReflectionModel::calcAnim() {
    if (!_98) {
        if (!MR::isDead(this)) {
            if (MR::isNoCalcAnim(this) || MR::isClipped(this)) {
                return;
            }

            LiveActor::calcAnim();
        }
    }
}

void MirrorReflectionModel::calcViewAndEntry() {
    if (!_98) {
        LiveActor::calcViewAndEntry();
    }
}

void MirrorReflectionModel::calcAndSetBaseMtx() {
    if (!_94) {
        LiveActor::calcAndSetBaseMtx();
    } else {
        TPos3f pos;
        pos.set(_94);
        pos.getTrans(mPosition);
        MR::setBaseTRMtx(this, _94);
    }
}

MirrorReflectionModel::~MirrorReflectionModel() {
}
