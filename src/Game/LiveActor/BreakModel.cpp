#pragma auto_inline off
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

BreakModel::BreakModel(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx, const char* pAnimName)
    : LiveActor(pName) {
    mHost = pHost;
    mAnimName = pAnimName;
    mMtx = pMtx;
    if (mMtx != nullptr) {
        MR::setPosition(this, mMtx[0][3], mMtx[1][3], mMtx[2][3]);
    } else {
        MR::copyTransRotateScale(pHost, this);
    }
    MR::initActor(this, pModelName, false);
}

void BreakModel::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(&NrvBreakModel::BreakModelNrvWait::sInstance, 0);
    makeActorDead();
}

void BreakModel::appear() {
    if (mMtx != nullptr) {
        MR::setPosition(this, mMtx[0][3], mMtx[1][3], mMtx[2][3]);
    } else {
        MR::copyTransRotateScale(mHost, this);
    }
    LiveActor::appear();
    setNerve(&NrvBreakModel::BreakModelNrvBreak::sInstance);
}

void BreakModel::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, mAnimName);
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void BreakModel::calcAndSetBaseMtx() {
    if (mMtx != nullptr) {
        mPosition.set<f32>(mMtx[0][3], mMtx[1][3], mMtx[2][3]);
        MR::setBaseTRMtx(this, mMtx);
        return;
    }

    LiveActor::calcAndSetBaseMtx();
}

BreakModel::~BreakModel() {
}

namespace NrvBreakModel {
    BreakModelNrvWait BreakModelNrvWait::sInstance ATTRIBUTE_WEAK;
    BreakModelNrvBreak BreakModelNrvBreak::sInstance ATTRIBUTE_WEAK;

    void BreakModelNrvWait::execute(Spine* pSpine) const {
    }

    void BreakModelNrvBreak::execute(Spine* pSpine) const {
        BreakModel* pBreakModel = reinterpret_cast< BreakModel* >(pSpine->mExecutor);
        pBreakModel->exeBreak();
    }
};  // namespace NrvBreakModel
