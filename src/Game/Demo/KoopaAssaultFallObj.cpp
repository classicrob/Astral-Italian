#include "Game/Demo/KoopaAssaultFallObj.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaAssaultFallObj {
    NEW_NERVE(KoopaAssaultFallObjNrvFall, KoopaAssaultFallObj, Fall);
    NEW_NERVE(KoopaAssaultFallObjNrvBreak, KoopaAssaultFallObj, Break);
};  // namespace NrvKoopaAssaultFallObj

KoopaAssaultFallObj::KoopaAssaultFallObj(const char* pName) : LiveActor(pName) {
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mModel = nullptr;
}

void KoopaAssaultFallObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "CrystalCageS", "KoopaAssault", false);
    mScale.set(2.0f, 2.0f, 2.0f);
    mBasePos.set(mPosition);

    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &mPosition, 6000.0f);

    mModel = MR::createModelObjMapObjStrongLight("クッパ襲撃落下オブジェ壊れモデル", "CrystalCageSBreak", nullptr);
    mModel->mPosition.set(mBasePos);
    mModel->mRotation.set(mRotation);

    initNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvFall), 0);
    makeActorDead();
}

void KoopaAssaultFallObj::appear() {
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &mPosition, 6000.0f);

    LiveActor::appear();
    MR::showModel(this);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvFall));
}

void KoopaAssaultFallObj::makeActorDead() {
    mModel->makeActorDead();
    LiveActor::makeActorDead();
}

void KoopaAssaultFallObj::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "FallStart", -1, -1, -1);
        MR::emitEffect(this, "Blur");
    }

    TVec3f zero(0.0f, 0.0f, 0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &zero, 6000.0f);
    MR::vecBlend(&mPosition, zero, mBasePos, MR::calcNerveEaseInRate(this, 90));

    if (MR::isStep(this, 90)) {
        MR::deleteEffect(this, "Blur");
        MR::tryHitReaction(this, "砕ける");
        setNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvBreak));
    }
}

void KoopaAssaultFallObj::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        mModel->appear();
        MR::invalidateClipping(mModel);
        MR::startAction(mModel, "Break");
    }

    if (MR::isActionEnd(mModel)) {
        MR::validateClipping(this);
        kill();
    }
}

KoopaAssaultFallObj::~KoopaAssaultFallObj() {
}
