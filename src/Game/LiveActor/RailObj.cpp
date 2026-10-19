#include "Game/LiveActor/RailObj.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

RailObj::RailObj(const char* pName) : LiveActor(pName) {
    mArg0 = -1;
    mArg1 = -1;
    mArg2 = -1;
    mArg3 = -1;
    mArg4 = -1;
    mArg5 = -1;
    mArg6 = -1;
    mArg7 = -1;
}

void RailObj::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0WithInit(rIter, &mArg0);
    MR::getJMapInfoArg1WithInit(rIter, &mArg1);
    MR::getJMapInfoArg2WithInit(rIter, &mArg2);
    MR::getJMapInfoArg3WithInit(rIter, &mArg3);
    MR::getJMapInfoArg4WithInit(rIter, &mArg4);
    MR::getJMapInfoArg5WithInit(rIter, &mArg5);
    MR::getJMapInfoArg6WithInit(rIter, &mArg6);
    MR::getJMapInfoArg7WithInit(rIter, &mArg7);
    initRailRider(rIter);
    MR::invalidateClipping(this);
    makeActorDead();
}

RailObj::~RailObj() {
}
