#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/RepeatBlockNrv.hpp"

namespace {
    const char* sAppearActions[] = {"XPlus", "XMinus", "YPlus", "YMinus", "ZPlus", "ZMinus"};
    const char* sDisappearActions[] = {"XMinus", "XPlus", "YMinus", "YPlus", "ZMinus", "ZPlus"};
}

RepeatBlockAtom::RepeatBlockAtom(RepeatBlock* host) : LiveActor("リピートブロックアトム") {
    MR::setRotation(this,host->mRotation);
    MR::initActor(this,"RepeatBlock",false);
    MR::initCollisionParts(this,"RepeatBlock",host->getSensor("Body"),nullptr);
    initNerve(&NrvRepeatBlock::RepeatBlockNrvWait::sInstance);
}

void RepeatBlockAtom::appear() {
    LiveActor::appear();
    setNerve(&NrvRepeatBlock::RepeatBlockNrvAtomWait::sInstance);
}

void RepeatBlockAtom::startAppear(RepeatBlock* host, s32 direction) {
    MR::startAction(this, sAppearActions[direction]);
    f32 rate = host->getCellSize(direction) / 200.0f;
    MR::setBtkRate(this, rate * MR::getBtkFrameMax(this));
    setNerve(&NrvRepeatBlock::RepeatBlockNrvAtomA::sInstance);
}

void RepeatBlockAtom::startDisappear(RepeatBlock* host, s32 direction) {
    MR::startAction(this, sDisappearActions[direction]);
    f32 rate = host->getCellSize(direction) / 200.0f;
    MR::setBtkFrame(this, MR::getBtkFrameMax(this));
    MR::setBtkRate(this, rate * -MR::getBtkFrameMax(this));
    setNerve(&NrvRepeatBlock::RepeatBlockNrvAtomDisappear::sInstance);
}

// NONMATCHING: commuted operands in the next-frame floating addition.
void RepeatBlockAtom::exeAppear() {
    f32 frameMax = MR::getBtkFrameMax(this);
    f32 rate = MR::getBtkRate(this);
    if (rate + MR::getBtkFrame(this) >= frameMax) {
        setNerve(&NrvRepeatBlock::RepeatBlockNrvAtomWait::sInstance);
    }
}
