#include "Game/NameObj/BombHeiRed.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/MtxUtil.hpp"

void BombHeiRed::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("BombHeiRed");
    caps.setDefault();

    mParam._14 = "Wait";
    mParam._18 = "Turn";
    mParam._1C = "Talk";
    mParam._20 = "Turn";
    _130 = cNPCSpinDefault;
    _134 = cNPCTrampledDefault;
    _138 = cNPCPointingDefault;
    _13C = cNPCReactionDefault;
    _12C = 450.0f;

    initialize(rIter, caps, "BombHeiRed", nullptr, nullptr);

    bool (BombHeiRed::*pCalcFunc)(TPos3f*, const JointControllerInfo&) = &BombHeiRed::calcZenmaiRotation;
    JointControlDelegator< BombHeiRed >* pController = new JointControlDelegator< BombHeiRed >(this, pCalcFunc, nullptr);
    MR::setJointControllerParam(pController, this, "Zenmai");
    mJointController = pController;

    makeActorAppeared();
}

void BombHeiRed::calcAndSetBaseMtx() {
    NPCActor::calcAndSetBaseMtx();
    if (mJointController != nullptr) {
        mJointController->registerCallBack();
    }
}

void BombHeiRed::control() {
    NPCActor::control();
    mZenmaiAngle += 4.0f;
}

bool BombHeiRed::calcZenmaiRotation(TPos3f* pMtx, const JointControllerInfo&) {
    MR::rotateMtxLocalZDegree(pMtx->mMtx, mZenmaiAngle);
    return true;
}
