#include "Game/Util.hpp"
#include "Game/Enemy/OtaRockChief.hpp"
#include "Game/Enemy/OtaRockTank.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace {
    const char* sJointName = "OtaRockPos";
    const char* sModelName = "OtaRockChief";
}

namespace NrvOtaRockChief {
    // Wait nerve: its execute is an empty function in the target (bare blr).
    class OtaRockChiefNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static OtaRockChiefNrvWait sInstance;
    };
    OtaRockChiefNrvWait OtaRockChiefNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvOtaRockChief

OtaRockChief::OtaRockChief(OtaRockTank* pHost, const char* pName) : PartsModel(pHost, pName, sModelName, nullptr, -1, false) {
    mAnimScaleCtrl = nullptr;
}

OtaRockChief::~OtaRockChief() {}

void OtaRockChief::init(const JMapInfoIter& rIter) {
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    PartsModel::init(rIter);
    initFixedPosition(sJointName);
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
