#include "Game/LiveActor/ShadowVolumeFlatModel.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>

ShadowVolumeFlatModel::ShadowVolumeFlatModel(const char* pModelName)
    : ShadowVolumeModel("板状シャドウボリューム描画"), mRootDrawMtxIndex(-1), mDropDrawMtxIndex(-1) {
    initVolumeModel(pModelName);
    initJointDrawMtxResister();
}

ShadowVolumeFlatModel::ShadowVolumeFlatModel()
    : ShadowVolumeModel("板状シャドウボリューム描画"), mRootDrawMtxIndex(-1), mDropDrawMtxIndex(-1) {
}

void ShadowVolumeFlatModel::initModel(const char* pModelName) {
    initVolumeModel(pModelName);
    initJointDrawMtxResister();
}

void ShadowVolumeFlatModel::initJointDrawMtxResister() {
    J3DJoint* root = mModelData->getJointTree().getRootNode();
    u16 rootIndex = root->getJntNo();
    u16 dropIndex = root->getChild()->getJntNo();
    mRootDrawMtxIndex = MR::findPosNrmMtxIndexFromModel(mModelData, rootIndex);
    mDropDrawMtxIndex = MR::findPosNrmMtxIndexFromModel(mModelData, dropIndex);
}

void ShadowVolumeFlatModel::draw() const {
    if (getController()->isDraw()) {
        ShadowVolumeDrawer::draw();
    }
}

void ShadowVolumeFlatModel::loadModelDrawMtx() const {
    ShadowController* controller = getController();
    TVec3f displacement;
    controller->getDropDir(&displacement);
    displacement.scale(calcBaseDropLength());

    TPos3f rootMtx;
    controller->calcBaseMtx(&rootMtx);
    TVec3f dropPos;
    calcBaseDropPosition(&dropPos);
    rootMtx.setTrans(dropPos);
    TPos3f dropMtx;
    dropMtx.set(rootMtx);

    TVec3f rootUp;
    rootMtx.getYDir(rootUp);
    if (rootUp.dot(displacement) >= 0.0f) {
        MR::addTransMtx(rootMtx, displacement);
    }

    if (controller->isFollowHostScale()) {
        MR::preScaleMtx(rootMtx, controller->getHost()->mScale);
    }

    PSMTXConcat(MR::getCameraViewMtx(), rootMtx, rootMtx);
    GXLoadPosMtxImm(rootMtx, mRootDrawMtxIndex);

    TVec3f dropUp;
    dropMtx.getYDir(dropUp);
    if (dropUp.dot(displacement) < 0.0f) {
        MR::addTransMtx(dropMtx, displacement);
    }

    if (controller->isFollowHostScale()) {
        MR::preScaleMtx(dropMtx, controller->getHost()->mScale);
    }

    PSMTXConcat(MR::getCameraViewMtx(), dropMtx, dropMtx);
    GXLoadPosMtxImm(dropMtx, mDropDrawMtxIndex);
}

ShadowVolumeFlatModel::~ShadowVolumeFlatModel() {
}
