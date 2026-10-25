#include "Game/NameObj/BombHeiRed.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/MtxUtil.hpp"

BombHeiRed::BombHeiRed(const char* pName) : NPCActor(pName), mJointController(NULL), mZenmaiAngle(0.0f) {}

NPCActor::~NPCActor() {}

void BombHeiRed::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("BombHeiRed");
    caps.setDefault();

    const char* turn = "Turn";
    const char* wait = "Wait";
    const char* talk = "Talk";
    mParam._14 = wait;
    mParam._18 = turn;
    mParam._1C = talk;
    mParam._20 = turn;
    mSpinAction = cNPCSpinDefault;
    mTrampledAction = cNPCTrampledDefault;
    mPointingAction = cNPCPointingDefault;
    mReactionAction = cNPCReactionDefault;
    mSpinAttackRange = 450.0f;

    initialize(rIter, caps, "BombHeiRed", nullptr, nullptr);

    const char* jointName = "Zenmai";
    bool (BombHeiRed::*pCalcFunc)(TPos3f*, const JointControllerInfo&) = &BombHeiRed::calcZenmaiRotation;
    JointControlDelegator< BombHeiRed >* pController = new JointControlDelegator< BombHeiRed >(this, pCalcFunc, nullptr);
    MR::setJointControllerParam(pController, this, jointName);
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

BombHeiRed::~BombHeiRed() {}

template class JointControlDelegator<BombHeiRed>;
