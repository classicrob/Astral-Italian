#include "Game/MapObj/GliderTree.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvGliderTree {
class GliderTreeNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static GliderTreeNrvWait sInstance;
};
GliderTreeNrvWait GliderTreeNrvWait::sInstance;
NEW_NERVE(GliderTreeNrvSwing, GliderTree, Swing);
}
GliderTree::GliderTree(const char* name) : LiveActor(name) {}
void GliderTree::init(const JMapInfoIter& iter) {
    const char* name;
    MR::getObjectName(&name, iter);
    MR::initActor(this, iter, name, false);
    initNerve(&NrvGliderTree::GliderTreeNrvWait::sInstance);
    makeActorAppeared();
}
bool GliderTree::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor*) {
    if (!isNerve(&NrvGliderTree::GliderTreeNrvWait::sInstance)) return false;
    if (!MR::isSensorPlayerOrRide(sender)) return false;
    if (msg == 0x55) { setNerve(&NrvGliderTree::GliderTreeNrvSwing::sInstance); return true; }
    return false;
}
void GliderTree::exeSwing() { if (MR::isFirstStep(this)) MR::startAction(this, "Swing"); if (MR::isActionEnd(this)) setNerve(&NrvGliderTree::GliderTreeNrvWait::sInstance); }
GliderTree::~GliderTree() {}
