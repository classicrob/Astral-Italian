#include "Game/NPC/PenguinMaster.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

namespace NrvPenguinMaster {
    NEW_NERVE(PenguinMasterNrvReaction, PenguinMaster, Reaction);
};  // namespace NrvPenguinMaster

PenguinMaster::PenguinMaster(const char* pName) : NPCActor(pName) {
}

void PenguinMaster::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("PenguinMaster");
    caps.setDefault();
    caps.mReactionNerve = GET_NERVE(PenguinMaster, PenguinMasterNrvReaction);
    caps.mMessageOffset = TVec3f(0.0f, 400.0f, 0.0f);
    initialize(rIter, caps, "PenguinMaster", nullptr, nullptr);

    const char* talkAction = "Talk";
    mParam._14 = "Wait";
    mParam._1C = talkAction;
    mParam._18 = nullptr;
    mParam._20 = nullptr;
    mParam._0 = false;
    mParam._1 = false;
    mTrampledAction = "Trampled";
    mReactionAction = "Reaction";
    mSpinAction = "Reaction";
    mPointingAction = "Pointing";

    if (mMsgCtrl != nullptr) {
        MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    }
}

bool PenguinMaster::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !MR::isMsgSupportTicoSpin(msg)) {
        mSpinAttacked = true;

        return true;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

// NONMATCHING: saved-register and link-register restoration order differs.
void PenguinMaster::exeReaction() {
    if (mTrampledStart) {
        MR::startActionSound(this, "SvPenguinOldTrampled");
        MR::startActionSound(this, "SmPenguinTrampled");
    }

    if (isPointingSe()) {
        MR::startActionSound(this, "SvPenguinOldPointed2p");
        MR::startDPDHitSound();
    }

    if (mSpinAttackedStart) {
        MR::startActionSound(this, "SvPenguinOldSpinHit");
    }

    if (mReactionStart) {
        MR::startActionSound(this, "SvPenguinOldStarPiece");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}
