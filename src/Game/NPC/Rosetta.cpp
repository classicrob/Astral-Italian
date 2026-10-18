#include "Game/NPC/Rosetta.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TurnJointCtrl.hpp"
#include "Game/Util.hpp"

namespace NrvRosetta {
    class RosettaNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static RosettaNrvWait sInstance;
    };
    RosettaNrvWait RosettaNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvRosetta

Rosetta::Rosetta(const char* pName) : NPCActor(pName) {
    _168 = false;
    mTakeOutStar = nullptr;
}

// NONMATCHING: caps fields, name strings and the TakeOutStar setup are not yet recovered from the SB4E01 asm.
void Rosetta::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("Rosetta");
    caps.setDefault();
    NPCActor::initialize(rIter, caps, nullptr, nullptr, nullptr);

    MR::startBrk(this, "Normal");

    mTurnJointCtrl = new TurnJointCtrl(this);
    mTurnJointCtrl->init(40.0f, 0.0f, 5.0f);
    mTurnJointCtrl->addFace("Head", 0.6f, TurnJointCtrl::Z, TurnJointCtrl::Y, TurnJointCtrl::X);
    mTurnJointCtrl->addWaist("Spine2", 0.4f, TurnJointCtrl::Z, TurnJointCtrl::X, TurnJointCtrl::Y);

    MR::startBckNoInterpole(this, "WaitA");
    MR::calcAnimDirect(this);

    if (mMsgCtrl != nullptr) {
        MR::setDistanceToTalk(mMsgCtrl, 200.0f);
    }
}

void Rosetta::calcAndSetBaseMtx() {
    mTurnJointCtrl->setCallBackFunction();
    NPCActor::calcAndSetBaseMtx();
}

void Rosetta::control() {
    if (canUpdateStarePos()) {
        TVec3f starePos;
        MR::calcPlayerFaceStarePos(&starePos, MR::getJointMtx(this, "Head"), getBaseMtx());
        mTurnJointCtrl->setStarePos(starePos);
    }

    mTurnJointCtrl->update();

    NPCActor::control();
}

bool Rosetta::canUpdateStarePos() const {
    TVec3f dir = *MR::getPlayerPos() - mPosition;
    TVec3f yDir;
    MR::extractMtxYDir(getBaseMtx(), &yDir);

    if (MR::normalizeOrZero(&dir)) {
        return false;
    }

    return !(MR::vecKillElement(dir, yDir, &dir) > 0.95f);
}
