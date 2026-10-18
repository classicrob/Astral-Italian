#include "Game/NPC/Rosetta.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TurnJointCtrl.hpp"
#include "Game/Util.hpp"

extern "C" const char lbl_806B7AEC[];
extern "C" const char lbl_806B7AE0[];
extern "C" const char lbl_806B7AD4[];
extern "C" const char lbl_806B7AC8[];

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

// NONMATCHING: string-pointer register allocation, store scheduling and stack frame size differ.
void Rosetta::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("Rosetta");
    caps.setDefault();
    caps.mSwitchAppear = false;
    caps.mMakeActor = false;
    const char* turnAction = "Turn";
    mParam._8 = 2.0f;
    mParam._14 = "WaitA";
    mParam._18 = turnAction;
    mParam._1C = mParam._20 = "TalkA";
    _130 = lbl_806B7AEC;
    _134 = lbl_806B7AE0;
    _138 = lbl_806B7AD4;
    _13C = lbl_806B7AC8;
    _12C = 1500.0f;
    NPCActor::initialize(rIter, caps, "Rosetta", nullptr, nullptr);

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

    if (_168) {
        mTakeOutStar = new TakeOutStar(this, "TakeOutStar", "TakeOutStar", &NrvRosetta::RosettaNrvWait::sInstance, nullptr);
        MR::declarePowerStar(this);
    }
    MR::tryRegisterDemoCast(this, "\203O\203\211\203\223\203h\203X\203^\201[\213A\212\322[\202Q\211\361\226\332\210\310\215~]", rIter);
    makeActorAppeared();
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
