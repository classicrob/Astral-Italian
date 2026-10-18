// NONMATCHING: skeleton. The MoveBlockFunction helper (fn_802D4A30..fn_802D4B00), createPartsModelMapObjStrongLight,
// initUseStageSwitchReadA and listenStageSwitchOnOffA are not declared/ported yet, so init/listen/exeMove are approximations.
#include "Game/MapObj/SwitchingMoveBlock.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvSwitchingMoveBlock {
    class SwitchingMoveBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SwitchingMoveBlockNrvWait sInstance;
    };
    SwitchingMoveBlockNrvWait SwitchingMoveBlockNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(SwitchingMoveBlockNrvMove, SwitchingMoveBlock, Move);
};  // namespace NrvSwitchingMoveBlock

SwitchingMoveBlock::SwitchingMoveBlock(const char* pName) : LiveActor(pName) {
    mMoveFunction = nullptr;
}

void SwitchingMoveBlock::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, nullptr, false);
    MR::connectToSceneMapObj(this);
    MR::initMapPartsClipping(this, rIter, nullptr, false);
    initNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvWait::sInstance, 0);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void SwitchingMoveBlock::control() {
}

void SwitchingMoveBlock::listenSwitchOnA() {
    if (MR::isDead(this)) {
        return;
    }
    MR::invalidateClipping(this);
    setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvMove::sInstance);
    MR::startActionSound(this, "MoveStart", -1, -1, -1);
}

void SwitchingMoveBlock::listenSwitchOffA() {
    if (MR::isDead(this)) {
        return;
    }
    MR::invalidateClipping(this);
    setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvMove::sInstance);
    MR::startActionSound(this, "MoveStartReverse", -1, -1, -1);
}

void SwitchingMoveBlock::exeMove() {
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvWait::sInstance);
        MR::startActionSound(this, "MoveEnd", -1, -1, -1);
    } else {
        MR::startActionSound(this, "MoveLv", -1, -1, -1);
    }
}

SwitchingMoveBlock::~SwitchingMoveBlock() {
}
