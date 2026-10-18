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
    caps.mMessageOffset = TVec3f(0.0f, 400.0f, 0.0f);
    caps.mReactionNerve = GET_NERVE(PenguinMaster, PenguinMasterNrvReaction);
    initialize(rIter, caps, "PenguinMaster", nullptr, nullptr);

    mParam._14 = "Wait";
    mParam._1C = "Talk";
    mParam._18 = nullptr;
    mParam._20 = nullptr;
    mParam._0 = false;
    mParam._1 = false;
    _134 = "Trampled";
    _13C = "Reaction";
    _130 = "Reaction";
    _138 = "Pointing";

    if (mMsgCtrl != nullptr) {
        MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    }
}

bool PenguinMaster::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !MR::isMsgSupportTicoSpin(msg)) {
        _E3 = true;

        return true;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

void PenguinMaster::exeReaction() {
    if (_D8) {
        MR::startActionSound(this, "SvPenguinOldTrampled");
        MR::startActionSound(this, "SmPenguinTrampled");
    }

    if (isPointingSe()) {
        MR::startActionSound(this, "SvPenguinOldPointed2p");
        MR::startDPDHitSound();
    }

    if (_D9) {
        MR::startActionSound(this, "SvPenguinOldSpinHit");
    }

    if (_DB) {
        MR::startActionSound(this, "SvPenguinOldStarPiece");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}
