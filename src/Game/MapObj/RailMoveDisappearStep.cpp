#pragma auto_inline off
#include "Game/MapObj/RailMoveDisappearStep.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvRailMoveDisappearStep {
class RailMoveDisappearStepNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const;
    static RailMoveDisappearStepNrvWait sInstance;
};
RailMoveDisappearStepNrvWait RailMoveDisappearStepNrvWait::sInstance;
NEW_NERVE(RailMoveDisappearStepNrvSign, RailMoveDisappearStep, Sign);
NEW_NERVE(RailMoveDisappearStepNrvMove, RailMoveDisappearStep, Move);
NEW_NERVE(RailMoveDisappearStepNrvBreak, RailMoveDisappearStep, Break);
class RailMoveDisappearStepNrvDisappear : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static RailMoveDisappearStepNrvDisappear sInstance;
};
RailMoveDisappearStepNrvDisappear RailMoveDisappearStepNrvDisappear::sInstance;
void RailMoveDisappearStepNrvWait::execute(Spine* spine) const {
    LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor);
    if (MR::isOnPlayer(actor)) { MR::invalidateClipping(actor); actor->setNerve(&RailMoveDisappearStepNrvSign::sInstance); }
}
}
#pragma auto_inline on
RailMoveDisappearStep::RailMoveDisappearStep(const char* name) : MapObjActor(name), mRailClipPosition(0.0f, 0.0f, 0.0f) {}
#pragma auto_inline off
void RailMoveDisappearStep::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(0, 0);
    info.setupSound(6, 0);
    info.setupGroupClipping(32);
    info.setupRailMover();
    info.setupShadow(0);
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvRailMoveDisappearStep::RailMoveDisappearStepNrvWait::sInstance);
    initialize(iter, info);
    if (MR::isConnectedWithRail(iter)) {
        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&mRailClipPosition, this, 100.0f, radius);
        mRailMover->resetToInitPos();
    }
}
void RailMoveDisappearStep::exeSign() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Sign");
    if (MR::isActionEnd(this)) setNerve(&NrvRailMoveDisappearStep::RailMoveDisappearStepNrvMove::sInstance);
}
void RailMoveDisappearStep::exeMove() {
    if (MR::isFirstStep(this)) startMapPartsFunctions();
    if (mRailMover->isReachedEnd()) setNerve(&NrvRailMoveDisappearStep::RailMoveDisappearStepNrvBreak::sInstance);
}
void RailMoveDisappearStep::exeBreak() {
    if (MR::isFirstStep(this)) {
        ModelObj* model = mModelObj;
        model->appear();
        MR::invalidateClipping(model);
        MR::startAction(model, "Break");
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
    }
    if (MR::isActionEnd(mModelObj)) {
        mModelObj->kill();
        MR::validateClipping(this);
        setNerve(&NrvRailMoveDisappearStep::RailMoveDisappearStepNrvDisappear::sInstance);
    }
}
RailMoveDisappearStep::~RailMoveDisappearStep() {}
