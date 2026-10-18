#pragma auto_inline off
#include "Game/MapObj/ExterminationCheckerWoodBox.hpp"
#include "Game/MapObj/WoodBox.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvExterminationCheckerWoodBox {
NEW_NERVE(ExterminationCheckerWoodBoxNrvMove, ExterminationCheckerWoodBox, Move);
}
ExterminationCheckerWoodBox::ExterminationCheckerWoodBox(const char* name) : LiveActor(name), mBoxes(0), mBoxCount(0) {}
void ExterminationCheckerWoodBox::init(const JMapInfoIter& iter) {
    MR::connectToSceneMapObjMovement(this);
    mBoxCount = MR::getChildObjNum(iter);
    mBoxes = new WoodBox*[mBoxCount];
    for (s32 i = 0; i < mBoxCount; ++i) {
        mBoxes[i] = new WoodBox("タイムアタック用木箱");
        MR::initChildObj(mBoxes[i], iter, i);
        mBoxes[i]->registerBreakModelDemoCast();
        MR::registerDemoSimpleCastAll(mBoxes[i]);
    }
    MR::invalidateClipping(this);
    MR::needStageSwitchWriteA(this, iter);
    MR::needStageSwitchWriteB(this, iter);
    MR::registerDemoSimpleCastAll(this);
    initNerve(&NrvExterminationCheckerWoodBox::ExterminationCheckerWoodBoxNrvMove::sInstance);
    makeActorAppeared();
}
void ExterminationCheckerWoodBox::exeMove() {
    if (MR::isOnSwitchB(this)) {
        for (s32 i = 0; i < mBoxCount; ++i) mBoxes[i]->forceReappear();
        MR::offSwitchA(this);
        MR::offSwitchB(this);
        return;
    }
    if (!MR::isOnSwitchA(this) && checkAllDestroyed()) MR::onSwitchA(this);
}
bool ExterminationCheckerWoodBox::checkAllDestroyed() {
    for (s32 i = 0; i < mBoxCount; ++i) if (mBoxes[i]->mHitPoint) return false;
    return true;
}
ExterminationCheckerWoodBox::~ExterminationCheckerWoodBox() {}
