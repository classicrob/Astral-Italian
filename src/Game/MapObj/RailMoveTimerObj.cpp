#include "Game/MapObj/RailMoveTimerObj.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
// PLACEHOLDER(m1): original read-only zero delay pool; resolve its semantic name and owning split.
extern const s32 lbl_807E30C0;
namespace NrvRailMoveTimerObj {
    NEW_NERVE(RailMoveTimerObjNrvAppear, RailMoveTimerObj, Appear);
    class RailMoveTimerObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static RailMoveTimerObjNrvWait sInstance;
    };
    RailMoveTimerObjNrvWait RailMoveTimerObjNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(RailMoveTimerObjNrvMove, RailMoveTimerObj, Move);
    NEW_NERVE(RailMoveTimerObjNrvDisappear, RailMoveTimerObj, Disappear);
    void RailMoveTimerObjNrvWait::execute(Spine* spine) const {
        RailMoveTimerObj* actor = static_cast<RailMoveTimerObj*>(spine->mExecutor);
        if (MR::isOnPlayer(actor)) actor->setNerve(&RailMoveTimerObjNrvMove::sInstance);
    }
}
RailMoveTimerObj::RailMoveTimerObj(const char* name) : MapObjActor(name) {}
RailMoveTimerObj::~RailMoveTimerObj() {}
void RailMoveTimerObj::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(nullptr,0);
    info.setupSound(6,nullptr);
    info.setupGroupClipping(32);
    info.setupRailMover();
    info.setupShadow(nullptr);
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvRailMoveTimerObj::RailMoveTimerObjNrvWait::sInstance);
    initialize(iter,info);
    if (MR::isConnectedWithRail(iter)) {
        f32 radius=0.0f;
        MR::calcModelBoundingRadius(&radius,this);
        MR::initAndSetRailClipping(&mClippingCenter,this,100.0f,radius);
        mRailMover->resetToInitPos();
    }
}
void RailMoveTimerObj::exeAppear() {
    if (MR::isFirstStep(this)) MR::tryEmitEffect(this,"Appear");
    if (MR::isGreaterEqualStep(this,lbl_807E30C0)) {
        MR::showModel(this);
        MR::validateCollisionParts(this);
        setNerve(&NrvRailMoveTimerObj::RailMoveTimerObjNrvWait::sInstance);
    }
}
void RailMoveTimerObj::exeMove() {
    if (MR::isFirstStep(this)) startMapPartsFunctions();
    if (mRailMover->isReachedEnd()) setNerve(&NrvRailMoveTimerObj::RailMoveTimerObjNrvDisappear::sInstance);
}
void RailMoveTimerObj::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::tryEmitEffect(this,"Disappear");
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
    }
    if (MR::isGreaterEqualStep(this,60)) {
        mRailMover->resetToInitPos();
        setNerve(&NrvRailMoveTimerObj::RailMoveTimerObjNrvAppear::sInstance);
    }
}
