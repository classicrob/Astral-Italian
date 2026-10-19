#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

DemoObj::DemoObj(const char* pName, MtxPtr pMtx) : LiveActor(pName), mMtx(pMtx) {
    if (pMtx != nullptr) {
        mPosition.set<f32>(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    }
}

DemoObj::~DemoObj() {
}

void DemoObj::init(const JMapInfoIter&) {
    makeActorAppeared();
}

void DemoObj::calcAndSetBaseMtx() {
    if (mMtx != nullptr) {
        mPosition.set<f32>(mMtx[0][3], mMtx[1][3], mMtx[2][3]);
        MR::setBaseTRMtx(this, mMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}
