#include "Game/MapObj/WorldWarpPoint.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
namespace {
    char sWarpPointModelName[] = "MiniWorldWarpPoint";
    char sWarpPointArchiveName[] = "MarioFaceShip";
    char sWarpPointEntranceName[] = "WorldWarpPointEntrance";
    char sWarpPointPreviousAction[] = "Previous";
    char sWarpPointExitName[] = "WorldWarpPointExit";
    char sWarpPointReturnDemo[] = "グランドスター帰還";
    char sWarpPointLaterReturnDemo[] = "グランドスター帰還[ワールド２以降]";
    char sWarpPointNextAction[] = "Next";
    char sWarpPointCloseAction[] = "Close";
}

namespace NrvWorldWarpPoint {
class WorldWarpPointNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static WorldWarpPointNrvWait sInstance;
};
WorldWarpPointNrvWait WorldWarpPointNrvWait::sInstance;
class WorldWarpPointNrvOpen : public Nerve {
public:
    virtual void execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isBckOneTimeAndStopped(actor)) { MR::startAction(actor, sWarpPointNextAction); actor->setNerve(&WorldWarpPointNrvWait::sInstance); } }
    static WorldWarpPointNrvOpen sInstance;
};
WorldWarpPointNrvOpen WorldWarpPointNrvOpen::sInstance;
}
WorldWarpPoint::WorldWarpPoint(const char* name) : LiveActor(name) {}
void WorldWarpPoint::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, sWarpPointModelName, sWarpPointArchiveName, false);
    initNerve(&NrvWorldWarpPoint::WorldWarpPointNrvWait::sInstance);
    const char* name = 0;
    MR::getObjectName(&name, iter);
    if (MR::isEqualString(name, sWarpPointEntranceName)) {
        if (GameSequenceFunction::isWorldMapFlag22D()) { MR::startAction(this, sWarpPointPreviousAction); makeActorAppeared(); return; }
        makeActorDead();
        return;
    }
    if (MR::isEqualString(name, sWarpPointExitName)) {
        MR::tryRegisterDemoCast(this, sWarpPointReturnDemo, iter);
        MR::tryRegisterDemoActionNerveDirect(this, &NrvWorldWarpPoint::WorldWarpPointNrvOpen::sInstance, sWarpPointReturnDemo, 0);
        MR::tryRegisterDemoCast(this, sWarpPointLaterReturnDemo, iter);
        MR::tryRegisterDemoActionNerveDirect(this, &NrvWorldWarpPoint::WorldWarpPointNrvOpen::sInstance, sWarpPointLaterReturnDemo, 0);
        if (GameSequenceFunction::isWorldMapFlag22C()) {
            if (GameSequenceFunction::hasGrandStarOfCurrentWorld()) MR::startAction(this, sWarpPointNextAction);
            else MR::startAction(this, sWarpPointCloseAction);
            makeActorAppeared();
            return;
        }
        makeActorDead();
    }
}
WorldWarpPoint::~WorldWarpPoint() {}
