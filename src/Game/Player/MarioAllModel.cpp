#include "Game/Player/MarioAllModel.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>

const char* sMarioEyeLidMaterial = "EyeLid_v";
const char* sMarioEyeLidTexture = "mario_eyeLid.0";

MarioAllModel::MarioAllModel(MarioActor* actor) : mActor(actor), mCurrentModel(0), mPreviousModel(0), mEyeLidTextureIndex(0) {
    for (s32 index = 0; index < 9; ++index) {
        mModels[index] = nullptr;
        mModelDLs[index] = nullptr;
    }
}

void MarioAllModel::registerModel(s32 index, LiveActor* actor, bool shareAnimation, bool createDL) {
    J3DModelX* model = static_cast<J3DModelX*>(MR::getJ3DModel(actor));
    if (index == 0) {
        model->_DD = 8;
        for (u32 buffer = 0; buffer < model->_DD; ++buffer) {
            model->mExtraMtxBuffer[buffer] = new (32) Mtx[96];
        }
    } else {
        J3DModelX* baseModel = mModels[0];
        model->_DD = baseModel->_DD;
        for (u32 buffer = 0; buffer < model->_DD; ++buffer) {
            model->mExtraMtxBuffer[buffer] = baseModel->mExtraMtxBuffer[buffer];
        }
    }
    if (shareAnimation) model->copyAnmMtxBuffer(mModels[0]);
    MR::initDLMakerFog(actor, true);
    MR::newDifferedDLBuffer(actor);
    mModels[index] = model;
    if (createDL) fn_803E7C10(index, actor);
}

void MarioAllModel::fn_803E7880(s32 index) {
    J3DModelX* previous = mModels[mCurrentModel];
    mCurrentModel = index;
    mActor->changeDisplayMode();
    mModels[mCurrentModel]->_DC = previous->_DC;
}

// NONMATCHING: redundant model-index reload and first joint-copy register allocation.
void MarioAllModel::fn_803E78F0() {
    if (mCurrentModel == 0) return;
    {
        MtxPtr base;
        J3DModelX* model = mModels[mCurrentModel];
        base = MR::getJointMtx(mModels[0], "HandL0");
        PSMTXCopy(MR::getJointMtx(model, "HandL0"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "HandR0");
        PSMTXCopy(MR::getJointMtx(model, "HandR0"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "FootL");
        PSMTXCopy(MR::getJointMtx(model, "FootL"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "FootR");
        PSMTXCopy(MR::getJointMtx(model, "FootR"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "Center");
        PSMTXCopy(MR::getJointMtx(model, "Center"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "All_Root");
        PSMTXCopy(MR::getJointMtx(model, "All_Root"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "Spine1");
        PSMTXCopy(MR::getJointMtx(model, "Spine1"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "Hip");
        PSMTXCopy(MR::getJointMtx(model, "Hip"), base);
    }
    {
        J3DModelX* model = mModels[mCurrentModel];
        MtxPtr base = MR::getJointMtx(mModels[0], "Face0");
        PSMTXCopy(MR::getJointMtx(model, "Face0"), base);
    }
    PSMTXCopy(mModels[mCurrentModel]->getBaseTRMtx(), mModels[0]->getBaseTRMtx());
}

u16 MarioAllModel::fn_803E7B30() {
    u16 maximum = 0;
    for (s32 index = 0; index < 9; ++index) {
        J3DModelX* model = mModels[index];
        if (model != nullptr) {
            u16 count = model->getModelData()->getDrawMtxNum();
            if (count > maximum) maximum = count;
        }
    }
    return maximum;
}

void MarioAllModel::fn_803E7B80() { fn_803E7880(mPreviousModel); }

J3DModel* MarioAllModel::getJ3DModel() const { return mModels[mCurrentModel]; }

void MarioAllModel::fn_803E7BB0(MarioActor* actor) { mEyeLidTextureIndex = MR::getTextureIndex(actor, sMarioEyeLidTexture); }

void MarioAllModel::fn_803E7BF0(u32 index) {
    if (mModelDLs[mCurrentModel] != nullptr) mModelDLs[mCurrentModel]->setDisplayList(index);
}

void MarioAllModel::fn_803E7C10(s32 index, LiveActor* actor) {
    mModelDLs[index] = new MarioModelDL(actor, sMarioEyeLidMaterial, sMarioEyeLidTexture);
}
