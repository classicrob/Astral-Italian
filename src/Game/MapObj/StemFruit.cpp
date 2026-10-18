#include "Game/MapObj/StemFruit.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// NONMATCHING-PARTIAL: init (0x8031BCD0), exeSpin (0x8031BF20), exePull (0x8031C0D0),
// updateChildState (0x8031C6C0), the child ctor and its methods, and the matrix helpers
// are not written yet. See the report.

namespace NrvStemFruit {
    NEW_NERVE(StemFruitNrvWait, StemFruit, Wait);
    NEW_NERVE(StemFruitNrvTrampled, StemFruit, Trampled);
    NEW_NERVE(StemFruitNrvSpin, StemFruit, Spin);
    NEW_NERVE(StemFruitNrvShake, StemFruit, Shake);
    NEW_NERVE(StemFruitNrvGet, StemFruit, Get);
    NEW_NERVE(StemFruitNrvComeOff, StemFruit, ComeOff);
    NEW_NERVE(StemFruitNrvPull, StemFruit, Pull);
    NEW_NERVE(StemFruitNrvPullEnd, StemFruit, PullEnd);
    NEW_NERVE(StemFruitNrvComeOffAlt, StemFruit, ComeOffAlt);
}

StemFruit::StemFruit(const char* pName) : LiveActor(pName) {
    mChild = nullptr;
    mTimer = 0;
}

void StemFruit::slot0CC() {
}

void StemFruit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mChild, "Wait");
        MR::validateClipping(this);
    }
}

void StemFruit::exeTrampled() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mChild, "Trampled");
    }
    if (MR::isActionEnd(mChild)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mChild, "Shake");
    }
    if (MR::isActionEnd(mChild)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeGet() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::startAction(mChild, "Get");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

void StemFruit::exeComeOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

void StemFruit::exePullEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PulledOut");
        MR::startAction(mChild, "PulledOut");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeComeOffAlt() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

bool StemFruit::tryStartTrampled() {
    if (isNerveActive()) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvTrampled));
        return true;
    }
    return false;
}

bool StemFruit::tryStartSpin() {
    if (isNerveActive() && !isNerve(GET_NERVE(StemFruit, StemFruitNrvSpin))) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvSpin));
        return true;
    }
    return false;
}

bool StemFruit::tryStartShake() {
    if (isNerve(GET_NERVE(StemFruit, StemFruitNrvWait)) && mTimer == 0) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvShake));
        return true;
    }
    mTimer = 30;
    return false;
}

void StemFruit::setNerveGet() {
    setNerve(GET_NERVE(StemFruit, StemFruitNrvGet));
}

void StemFruit::setNerveWait() {
    setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
}

void StemFruit::onEat() {
}

void StemFruit::control() {
    if (mTimer != 0) {
        mTimer--;
    }
}

bool StemFruit::isNerveActive() const {
    return isNerve(GET_NERVE(StemFruit, StemFruitNrvWait)) || isNerve(GET_NERVE(StemFruit, StemFruitNrvTrampled)) ||
           isNerve(GET_NERVE(StemFruit, StemFruitNrvSpin)) || isNerve(GET_NERVE(StemFruit, StemFruitNrvShake));
}
