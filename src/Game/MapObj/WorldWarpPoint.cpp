#include "Game/MapObj/WorldWarpPoint.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
namespace NrvWorldWarpPoint {
class WorldWarpPointNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static WorldWarpPointNrvWait sInstance;
};
WorldWarpPointNrvWait WorldWarpPointNrvWait::sInstance;
class WorldWarpPointNrvOpen : public Nerve {
public:
    virtual void execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isBckOneTimeAndStopped(actor)) { MR::startAction(actor, "Next"); actor->setNerve(&WorldWarpPointNrvWait::sInstance); } }
    static WorldWarpPointNrvOpen sInstance;
};
WorldWarpPointNrvOpen WorldWarpPointNrvOpen::sInstance;
}
WorldWarpPoint::WorldWarpPoint(const char* name) : LiveActor(name) {}
void WorldWarpPoint::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "MiniWorldWarpPoint", "MarioFaceShip", false);
    initNerve(&NrvWorldWarpPoint::WorldWarpPointNrvWait::sInstance);
    const char* name = 0;
    MR::getObjectName(&name, iter);
    if (MR::isEqualString(name, "WorldWarpPointEntrance")) {
        if (GameSequenceFunction::isWorldMapFlag22D()) { MR::startAction(this, "Previous"); makeActorAppeared(); return; }
        makeActorDead();
        return;
    }
    if (MR::isEqualString(name, "WorldWarpPointExit")) {
        MR::tryRegisterDemoCast(this, "グランドスター帰還", iter);
        MR::tryRegisterDemoActionNerveDirect(this, &NrvWorldWarpPoint::WorldWarpPointNrvOpen::sInstance, "グランドスター帰還", 0);
        MR::tryRegisterDemoCast(this, "グランドスター帰還[ワールド２以降]", iter);
        MR::tryRegisterDemoActionNerveDirect(this, &NrvWorldWarpPoint::WorldWarpPointNrvOpen::sInstance, "グランドスター帰還[ワールド２以降]", 0);
        if (GameSequenceFunction::isWorldMapFlag22C()) {
            if (GameSequenceFunction::hasGrandStarOfCurrentWorld()) MR::startAction(this, "Next");
            else MR::startAction(this, "Close");
            makeActorAppeared();
            return;
        }
        makeActorDead();
    }
}
WorldWarpPoint::~WorldWarpPoint() {}
