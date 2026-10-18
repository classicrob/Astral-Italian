#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

ModelObj::ModelObj(const char* pName, const char* pModelName, MtxPtr pMtx, int drawBufferType, int movementType, int calcAnimType, bool a7)
    : LiveActor(pName), mBaseMtx(pMtx) {
    if (drawBufferType < MR::DrawBufferType_None) {
        drawBufferType = MR::DrawBufferType_MapObj;
    }

    if (movementType < MR::MovementType_None) {
        movementType = MR::MovementType_MapObjDecoration;
    }

    if (calcAnimType < MR::CalcAnimType_None) {
        calcAnimType = MR::CalcAnimType_MapObjDecoration;
    }

    initModelManagerWithAnm(pModelName, nullptr, a7);
    MR::connectToScene(this, movementType, calcAnimType, drawBufferType, MR::DrawType_None);
    initSound(8, "ModelObj", nullptr, TVec3f(0.0f));
    initEffectKeeper(8, nullptr, false);

    if (mBaseMtx != nullptr) {
        mPosition.set< f32 >(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
    }
}

void ModelObj::init(const JMapInfoIter& rIter) {
    makeActorAppeared();
}

void ModelObj::calcAndSetBaseMtx() {
    if (mBaseMtx != nullptr) {
        mPosition.set< f32 >(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
        MR::setBaseTRMtx(this, mBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}



