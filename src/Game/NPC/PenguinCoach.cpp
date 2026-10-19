#include "Game/NPC/PenguinCoach.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TurnJointCtrl.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

namespace NrvPenguinCoach {
    NEW_NERVE(PenguinCoachNrvReaction, PenguinCoach, Reaction);
}

PenguinCoach::PenguinCoach(const char* pName) : NPCActor(pName), mTurnJointCtrl(nullptr) {}

void PenguinCoach::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("PenguinCoach");
    caps.setDefault();
    caps.mReactionNerve = &NrvPenguinCoach::PenguinCoachNrvReaction::sInstance;
    caps.mMessageOffset = TVec3f(0.0f, 400.0f, 0.0f);
    initialize(rIter, caps, "PenguinCoach", nullptr, nullptr);
    mTurnJointCtrl = new TurnJointCtrl(this);
    mTurnJointCtrl->init(90.0f, 10.0f, 40.0f);
    mTurnJointCtrl->addFace("Head", 1.0f, TurnJointCtrl::X, TurnJointCtrl::Y, TurnJointCtrl::Z);
    mTurnJointCtrl->mStareBlendRate = 0.1f;
    mTurnJointCtrl->mStareDistanceLimit = 800.0f;
    mParam.setMoveAction("Wait", "Turn");
    mParam.setTalkAction("Talk", "TalkTurn");
    mParam._8 = 1.0f;
    mTrampledAction = "Trampled";
    mReactionAction = "Reaction";
    mSpinAction = "Reaction";
    mPointingAction = "Pointing";
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionFunctor(this, MR::FunctorV0M<PenguinCoach*, void (PenguinCoach::*)()>(this, &PenguinCoach::resetPose), nullptr);
    }
    if (mMsgCtrl != nullptr) {
        MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    }
}

void PenguinCoach::calcAndSetBaseMtx() {
    if (mTurnJointCtrl != nullptr) {
        mTurnJointCtrl->setCallBackFunction();
    }
    NPCActor::calcAndSetBaseMtx();
}

bool PenguinCoach::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !MR::isMsgSupportTicoSpin(msg)) {
        mSpinAttacked = true;
        return true;
    }
    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

void PenguinCoach::resetPose() {
    MR::setDefaultPose(this);
}

void PenguinCoach::control() {
    TVec3f starePos;
    MR::calcPlayerFaceStarePos(&starePos, MR::getJointMtx(this, "Head"), getBaseMtx());
    mTurnJointCtrl->setStarePos(starePos);
    mTurnJointCtrl->update();
    NPCActor::control();
}

void PenguinCoach::exeReaction() {
    if (MR::isFirstStep(this)) {
        mTurnJointCtrl->endCtrl(4);
    }

    if (mTrampledStart) {
        MR::startActionSound(this, "SmPenguinTrampled");
        MR::startActionSound(this, "SvPenguinLTrampled");
    }

    if (isPointingSe()) {
        MR::startActionSound(this, "SvPenguinLPointed2p");
        MR::startDPDHitSound();
    }

    if (mSpinAttackedStart) {
        MR::startActionSound(this, "SvPenguinLSpinHit");
        MR::startActionSound(this, "SmPenguinLSpinHit");
    }

    if (mReactionStart) {
        MR::startActionSound(this, "SvPenguinLStarPiece");
        MR::startActionSound(this, "SmPenguinLSpinHit");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        mTurnJointCtrl->startCtrl(60);
    }
}

