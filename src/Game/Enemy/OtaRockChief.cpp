#include "Game/Util.hpp"
#include "Game/Enemy/OtaRockChief.hpp"
#include "Game/Enemy/OtaRockTank.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvOtaRockChief {
    // Wait nerve: its execute is an empty function in the target (bare blr).
    class OtaRockChiefNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static OtaRockChiefNrvWait sInstance;
    };
    OtaRockChiefNrvWait OtaRockChiefNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvOtaRockChief

// NONMATCHING: target loads the model-name pointer from a global (lwz) instead of a literal (lis/addi)
OtaRockChief::OtaRockChief(OtaRockTank* pHost, const char* pName) : PartsModel(pHost, pName, "OtaRockChief", nullptr, -1, false) {
    mAnimScaleCtrl = nullptr;
}

OtaRockChief::~OtaRockChief() {}

// NONMATCHING: target loads the joint-name pointer from a global (lwz) instead of a literal
void OtaRockChief::init(const JMapInfoIter& rIter) {
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    PartsModel::init(rIter);
    initFixedPosition("OtaRockPos");
    initNerve(&NrvOtaRockChief::OtaRockChiefNrvWait::sInstance, 0);
}

void OtaRockChief::control() {
    mAnimScaleCtrl->updateNerve();
}

void OtaRockChief::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void OtaRockChief::startHitReaction() {
    mAnimScaleCtrl->startHitReaction();
}
