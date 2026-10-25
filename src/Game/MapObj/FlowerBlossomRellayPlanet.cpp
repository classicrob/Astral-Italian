#include "Game/MapObj/FlowerBlossomRellayPlanet.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace { char sFlowerBloomReaction[] = "咲いた"; }
namespace NrvFlowerBlossomRelayPlanet {
    class FlowerBlossomRelayPlanetNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static FlowerBlossomRelayPlanetNrvWait sInstance;
    };
    FlowerBlossomRelayPlanetNrvWait FlowerBlossomRelayPlanetNrvWait::sInstance ATTRIBUTE_WEAK;
    class FlowerBlossomRelayPlanetNrvBloomStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static FlowerBlossomRelayPlanetNrvBloomStart sInstance;
    };
    FlowerBlossomRelayPlanetNrvBloomStart FlowerBlossomRelayPlanetNrvBloomStart::sInstance ATTRIBUTE_WEAK;
    class FlowerBlossomRelayPlanetNrvBloomEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static FlowerBlossomRelayPlanetNrvBloomEnd sInstance;
    };
    FlowerBlossomRelayPlanetNrvBloomEnd FlowerBlossomRelayPlanetNrvBloomEnd::sInstance ATTRIBUTE_WEAK;
    inline void FlowerBlossomRelayPlanetNrvBloomEnd::execute(Spine* spine) const {
        LiveActor* actor=static_cast<LiveActor*>(spine->mExecutor);
        if (MR::isDemoActive() && MR::isDemoPartLastStep(nullptr)) MR::onSwitchA(actor);
    }
    inline void FlowerBlossomRelayPlanetNrvBloomStart::execute(Spine* spine) const {
        LiveActor* actor=static_cast<LiveActor*>(spine->mExecutor);
        if (MR::tryStartDemoRegistered(actor,nullptr)) {
            MR::tryHitReaction(actor,sFlowerBloomReaction);
            actor->setNerve(&FlowerBlossomRelayPlanetNrvBloomEnd::sInstance);
        }
    }
    inline void FlowerBlossomRelayPlanetNrvWait::execute(Spine* spine) const {
        LiveActor* actor=static_cast<LiveActor*>(spine->mExecutor);
        if (MR::isValidSwitchB(actor) && MR::isOnSwitchB(actor)) actor->setNerve(&FlowerBlossomRelayPlanetNrvBloomStart::sInstance);
    }
}
FlowerBlossomRellayPlanet::FlowerBlossomRellayPlanet(const char* name) : LiveActor(name) {}
FlowerBlossomRellayPlanet::~FlowerBlossomRellayPlanet() {}
void FlowerBlossomRellayPlanet::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,false);
    initNerve(&NrvFlowerBlossomRelayPlanet::FlowerBlossomRelayPlanetNrvWait::sInstance);
    MR::tryRegisterDemoCast(this,iter);
    makeActorAppeared();
}
