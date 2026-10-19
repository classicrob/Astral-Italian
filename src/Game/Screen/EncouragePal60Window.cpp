#include "Game/Screen/EncouragePal60Window.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Util/GamePadUtil.hpp"

#ifdef GALAXY_NATIVE
// Progress telemetry and the automated smoke run (native/app); observation only.
#include <petari/milestone.hpp>
#define NATIVE_MILESTONE(name) petari_milestone(name)
#else
#define NATIVE_MILESTONE(name)
#endif
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 DISPLAY_MIN_SECOND = 2;
};  // namespace

namespace NrvEncouragePal60Window {
    NEW_NERVE(EncouragePal60WindowNrvAppear, EncouragePal60Window, Appear);
    NEW_NERVE(EncouragePal60WindowNrvDisplay, EncouragePal60Window, Display);
    NEW_NERVE(EncouragePal60WindowNrvDisappear, EncouragePal60Window, Disappear);
}

EncouragePal60Window::EncouragePal60Window() : LayoutActor("PAL60推奨画面", true) {
}

void EncouragePal60Window::init(const JMapInfoIter& rIter) {
    initLayoutManager("SysPALInfo", 1);
    MR::connectToSceneLayout(this);

    mAButtonIcon = MR::createAndSetupIconAButton(this, true, false);

    initNerve(GET_NERVE_DIRECT(NrvEncouragePal60Window, EncouragePal60WindowNrvAppear));
    kill();
}

void EncouragePal60Window::appear() {
    LayoutActor::appear();
    setNerve(GET_NERVE_DIRECT(NrvEncouragePal60Window, EncouragePal60WindowNrvAppear));
}

void EncouragePal60Window::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::openWipeFade(-1);
        MR::startAnim(this, "Appear", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvEncouragePal60Window, EncouragePal60WindowNrvDisplay));
    }
}

void EncouragePal60Window::exeDisplay() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Wait", 0);
    }

    if (MR::isStep(this, ::DISPLAY_MIN_SECOND * 60)) {
        mAButtonIcon->openWithoutMessage();
    }

    if (MR::isGreaterStep(this, ::DISPLAY_MIN_SECOND * 60) && MR::testSystemPadTriggerDecide()) {
        MR::startSystemSE("SE_SY_TALK_FOCUS_ITEM");
        setNerve(GET_NERVE_DIRECT(NrvEncouragePal60Window, EncouragePal60WindowNrvDisappear));
    }
}

void EncouragePal60Window::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
        MR::startAnim(this, "End", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}
