#include "Game/MapObj/WhiteSnowTree.hpp"
#include "Game/Util.hpp"
WhiteSnowTree::WhiteSnowTree(const char* name) : LiveActor(name) {}
WhiteSnowTree::~WhiteSnowTree() {}
void WhiteSnowTree::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "WhiteSnowTree", false);
    makeActorAppeared();
}
