#include "Game/MapObj/StarPieceCluster.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace NrvStarPieceCluster {
class StarPieceClusterNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static StarPieceClusterNrvWait sInstance;
};
StarPieceClusterNrvWait StarPieceClusterNrvWait::sInstance;
NEW_NERVE(StarPieceClusterNrvBreak, StarPieceCluster, Break);
}
StarPieceCluster::StarPieceCluster(const char* name) : LiveActor(name) {}
void StarPieceCluster::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    MR::getChildObjNum(iter);
    mChild = static_cast<LiveActor*>(NameObjFactory::initChildObj(iter, 0));
    MR::setPosition(mChild, mPosition);
    initNerve(&NrvStarPieceCluster::StarPieceClusterNrvWait::sInstance);
    makeActorAppeared();
}
void StarPieceCluster::exeBreak() {
    if (MR::isFirstStep(this)) { MR::shakeCameraStrong(); MR::tryRumblePadVeryStrong(this, 0); MR::emitEffect(this, "Break"); MR::startActionSound(this, "Break"); MR::onSwitchA(this); }
    kill();
}
void StarPieceCluster::attackSensor(HitSensor*, HitSensor* receiver) { if (MR::isSensorPlayerOrRide(receiver)) setNerve(&NrvStarPieceCluster::StarPieceClusterNrvBreak::sInstance); }
StarPieceCluster::~StarPieceCluster() {}
