#include "Game/MapObj/MarioFacePlanet.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EventUtil.hpp"
namespace NrvMarioFacePlanet {
class MarioFacePlanetNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static MarioFacePlanetNrvWait sInstance;
};
MarioFacePlanetNrvWait MarioFacePlanetNrvWait::sInstance;
}
MarioFacePlanet::MarioFacePlanet(const char* name) : LiveActor(name) {}
void MarioFacePlanet::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    MR::initFurPlanet(this);
    initNerve(&NrvMarioFacePlanet::MarioFacePlanetNrvWait::sInstance);
    if (MR::tryRegisterDemoCast(this, iter)) MR::tryRegisterDemoActionFunctor(this, MR::FunctorV0M<MarioFacePlanet*, void (MarioFacePlanet::*)()>(this, &MarioFacePlanet::listenDemoAction), 0);
    MR::registerDemoSimpleCastAll(this);
    MR::useStageSwitchSyncAppear(this, iter);
}
void MarioFacePlanet::listenDemoAction() { MR::explainMarioFacePlanetAtFirst(this); }
MarioFacePlanet::~MarioFacePlanet() {}
