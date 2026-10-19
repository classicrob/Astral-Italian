#include "Game/Player/JetTurtleShadow.hpp"
#include "Game/Util.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MutexHolder.hpp"

JetTurtleShadow::JetTurtleShadow(const char* pName) : LiveActor(pName) {
}

void JetTurtleShadow::calcType0(MtxPtr pMtx) {
    J3DModel* pModel = MR::getJ3DModel(this);
    MR::setBaseTRMtx(this, pMtx);

    OSLockMutex(&MR::MutexHolder< 0 >::sMutex);

    pModel->calc();
    pModel->viewCalc();

    OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
}

void JetTurtleShadow::drawType0() const {
    J3DModelX* pModel = static_cast< J3DModelX* >(MR::getJ3DModel(this));

    pModel->mDisplayListMask = 0;
    pModel->mDisplayListMask |= (1U << 11);

    GXSetZScaleOffset(1.0f, 0.0f);

    pModel->directDraw(nullptr);
}

void JetTurtleShadow::drawType1() const {
    J3DModelX* pModel = static_cast< J3DModelX* >(MR::getJ3DModel(this));

    pModel->mDisplayListMask = 0;
    pModel->mDisplayListMask |= (1U << 6);

    pModel->directDraw(nullptr);
}

void JetTurtleShadow::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Koura", false);

    makeActorDead();
}
