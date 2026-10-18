#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/Util/LiveActorUtil.hpp"

PartsModelLodCtrl::PartsModelLodCtrl(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx, s32 drawBufferType, bool isShadow)
    : PartsModel(pHost, pName, pModelName, pMtx, drawBufferType, isShadow) {
    mLodCtrl = nullptr;
}

PartsModelLodCtrl::~PartsModelLodCtrl() {
}

void PartsModelLodCtrl::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);
    mLodCtrl = MR::createLodCtrlNPC(this, rIter);
}

void PartsModelLodCtrl::control() {
    mLodCtrl->update();
}

LodCtrl* PartsModelLodCtrl::setLodCtrl(LodCtrl*) {
    return mLodCtrl;
}

void PartsModelLodCtrl::initLightCtrl() {
    MR::initLightCtrl(this);
    if (mLodCtrl != nullptr) {
        mLodCtrl->initLightCtrl();
    }
}

void PartsModelLodCtrl::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    if (mLodCtrl != nullptr) {
        mLodCtrl->syncJointAnimation();
    }
}
