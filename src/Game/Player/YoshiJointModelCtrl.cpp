#include "Game/Player/YoshiAnimationModelCtrl.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JointUtil.hpp"

YoshiJointModelCtrl::YoshiJointModelCtrl(Yoshi* actor, const char* joint, PartsModel* model, u8 count)
    : mActor(actor), mJoint(joint), mModel(model), mJointCount(count), mSelected(0xFF) {
    fn_80386490();
}

void YoshiJointModelCtrl::fn_80386400(u8 selected) {
    PartsModel* model = mModel;
    if (selected != mSelected) {
        if (mSelected != 0xFF)
            MR::hideJoint(MR::getJoint(model, static_cast<u16>(mSelected + 1)));
        MR::showJoint(MR::getJoint(model, static_cast<u16>(selected + 1)));
    }
    mSelected = selected;
}

void YoshiJointModelCtrl::fn_80386480() {
    MR::hideJoint(mActor, mJoint);
}

void YoshiJointModelCtrl::fn_80386490() {
    for (s32 i = 0; i < mJointCount; ++i)
        MR::hideJoint(MR::getJoint(mModel, static_cast<u16>(i + 1)));
    mSelected = 0xFF;
    MR::showJoint(mActor, mJoint);
}
