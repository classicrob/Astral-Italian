#include "Game/MapObj/RailMoveNormalLift.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvRailMoveNormalLift { NEW_NERVE(RailMoveNormalLiftNrvMove,RailMoveNormalLift,Move); }
RailMoveNormalLift::RailMoveNormalLift(const char* name) : LiveActor(name),mRailMover(nullptr),mClippingCenter(0.0f,0.0f,0.0f) {}
RailMoveNormalLift::~RailMoveNormalLift() {}
void RailMoveNormalLift::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,false);
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToStartPos(this);
        mRailMover=new MapPartsRailMover(this);
        mRailMover->init(iter);
        mRailMover->start();
        f32 radius=0.0f;
        MR::calcModelBoundingRadius(&radius,this);
        MR::initAndSetRailClipping(&mClippingCenter,this,100.0f,radius);
    }
    initNerve(&NrvRailMoveNormalLift::RailMoveNormalLiftNrvMove::sInstance);
    makeActorAppeared();
}
void RailMoveNormalLift::exeMove() {
    if (mRailMover) {
        mRailMover->movement();
        if (mRailMover->isWorking()) {
            MR::setPosition(this,mRailMover->mPosition);
            mRailMover->tryResetPositionRepeat();
        }
    }
}
