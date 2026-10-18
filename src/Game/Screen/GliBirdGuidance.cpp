#include "Game/Screen/GliBirdGuidance.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sGuidanceAnimations[][3] = {
        {"Slope", "SlopeHold", "SlopeOK"},
        {"TurnLeft", "TurnLeftHold", "TurnLeftOK"},
        {"TurnDown", "TurnDownHold", "TurnDownOK"}
    };
}

namespace NrvGliBirdGuidance {
    NEW_NERVE(GliBirdGuidanceNrvAppear, GliBirdGuidance, Appear);
    NEW_NERVE(GliBirdGuidanceNrvStart, GliBirdGuidance, Start);
    NEW_NERVE(GliBirdGuidanceNrvHold, GliBirdGuidance, Hold);
    NEW_NERVE(GliBirdGuidanceNrvSuccess, GliBirdGuidance, Success);
}
using namespace NrvGliBirdGuidance;

GliBirdGuidance::GliBirdGuidance(const char* name) : LayoutActor(name, true), mMode(0) {}

void GliBirdGuidance::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initLayoutManager("TiltGuidance", 2);
    initNerve(&GliBirdGuidanceNrvAppear::sInstance);
    kill();
}

void GliBirdGuidance::start(s32 mode) {
    mMode = mode;
    setNerve(&GliBirdGuidanceNrvStart::sInstance);
}

void GliBirdGuidance::hold(s32 mode) {
    mMode = mode;
    setNerve(&GliBirdGuidanceNrvHold::sInstance);
}

void GliBirdGuidance::success(s32 mode) {
    mMode = mode;
    setNerve(&GliBirdGuidanceNrvSuccess::sInstance);
}

void GliBirdGuidance::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
        MR::startAnimAndSetFrameAndStop(this, sGuidanceAnimations[mMode][0], 0.0f, 1);
    }
}

void GliBirdGuidance::exeStart() {
    if (MR::isFirstStep(this)) MR::startAnim(this, sGuidanceAnimations[mMode][0], 1);
}

void GliBirdGuidance::exeHold() {
    if (MR::isFirstStep(this)) MR::startAnim(this, sGuidanceAnimations[mMode][1], 1);
}

void GliBirdGuidance::exeSuccess() {
    if (MR::isFirstStep(this)) MR::startAnim(this, sGuidanceAnimations[mMode][2], 1);
}

GliBirdGuidance::~GliBirdGuidance() {}
