#include "Game/Screen/MissLayout.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvMissLayout {
    NEW_NERVE(MissLayoutNrvWait, MissLayout, Wait);
    NEW_NERVE(MissLayoutNrvActive, MissLayout, Active);
    NEW_NERVE(MissLayoutNrvEnd, MissLayout, End);
}

MissLayout::MissLayout() : LayoutActor("ミス!", true) {
}

void MissLayout::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayout(this);
    initLayoutManager("TooBad", 1);
    initEffectKeeper(0, nullptr, nullptr);
    initNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvWait));
    kill();
}

void MissLayout::kill() {
    LayoutActor::kill();
    setNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvWait));
}

bool MissLayout::isActive() const {
    return isNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvActive));
}

void MissLayout::start() {
    if (isNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvActive))) {
        return;
    }

    LayoutActor::appear();
    MR::hideLayout(this);
    setNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvActive));
}

void MissLayout::exeWait() {
}

void MissLayout::exeActive() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        MR::startAnim(this, "Appear", 0);
    }

    if (MR::isStep(this, 18)) {
        MR::emitEffect(this, "TooBad");
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvMissLayout, MissLayoutNrvEnd));
    }
}

void MissLayout::exeEnd() {
}
