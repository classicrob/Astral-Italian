#include "Game/NameObj/BombHeiRed.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/MtxUtil.hpp"

void BombHeiRed::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("BombHeiRed");
    caps.setDefault();

    // PLACEHOLDER(core): NPCActor's header types 0x100-0x140 differently (the target stores motion names and 450.0f here); resolve when the NPCActor layout is fixed
    u8* pSelf = reinterpret_cast< u8* >(this);
    *reinterpret_cast< const char** >(pSelf + 0x100) = "Wait";
    *reinterpret_cast< const char** >(pSelf + 0x104) = "Turn";
    *reinterpret_cast< const char** >(pSelf + 0x108) = "Talk";
    *reinterpret_cast< const char** >(pSelf + 0x10C) = "Turn";
    *reinterpret_cast< const char** >(pSelf + 0x134) = "Spin";
    *reinterpret_cast< const char** >(pSelf + 0x138) = "Trampled";
    *reinterpret_cast< const char** >(pSelf + 0x13C) = "Pointing";
    *reinterpret_cast< const char** >(pSelf + 0x140) = "Reaction";
    *reinterpret_cast< f32* >(pSelf + 0x130) = 450.0f;

    initialize(rIter, caps, "BombHeiRed", nullptr, nullptr);

    JointControlDelegator< BombHeiRed >* pController = new JointControlDelegator< BombHeiRed >(this, &BombHeiRed::calcZenmaiRotation, nullptr);
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
