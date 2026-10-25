#include "Game/MapObj/MysteryMountainPlanet.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMysteryMountainPlanet {
class MysteryMountainPlanetNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const;
    static MysteryMountainPlanetNrvWait sInstance;
};
MysteryMountainPlanetNrvWait MysteryMountainPlanetNrvWait::sInstance;
NEW_NERVE(MysteryMountainPlanetNrvBoxVanish, MysteryMountainPlanet, BoxVanish);
NEW_NERVE(MysteryMountainPlanetNrvCakeVanish, MysteryMountainPlanet, CakeVanish);
class MysteryMountainPlanetNrvWaitVanished : public Nerve {
public:
    virtual void execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isFirstStep(actor)) MR::invalidateClipping(actor); }
    static MysteryMountainPlanetNrvWaitVanished sInstance;
};
MysteryMountainPlanetNrvWaitVanished MysteryMountainPlanetNrvWaitVanished::sInstance;
inline void MysteryMountainPlanetNrvWait::execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isValidSwitchA(actor) && MR::isOnSwitchA(actor)) actor->setNerve(&MysteryMountainPlanetNrvBoxVanish::sInstance); }
}
MysteryMountainPlanet::MysteryMountainPlanet(const char* name) : LiveActor(name) {}
void MysteryMountainPlanet::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    if (MR::calcDistanceToPlayer(this) < 3000.0f) {
        MR::startAction(this, "CakeVanish");
        MR::setAllAnimFrameAtEnd(this, "CakeVanish");
        initNerve(&NrvMysteryMountainPlanet::MysteryMountainPlanetNrvWaitVanished::sInstance);
    } else {
        MR::startAction(this, "BoxVanish");
        MR::setAllAnimFrameAndStop(this, "BoxVanish", 0.0f);
        initNerve(&NrvMysteryMountainPlanet::MysteryMountainPlanetNrvWait::sInstance);
    }
    MR::initUseStageSwitchReadA(this, iter);
    makeActorAppeared();
}
void MysteryMountainPlanet::exeBoxVanish() { if (MR::isFirstStep(this)) MR::startAction(this, "BoxVanish"); if (MR::isActionEnd(this)) setNerve(&NrvMysteryMountainPlanet::MysteryMountainPlanetNrvCakeVanish::sInstance); }
void MysteryMountainPlanet::exeCakeVanish() { if (MR::isFirstStep(this)) MR::startAction(this, "CakeVanish"); if (MR::isActionEnd(this)) setNerve(&NrvMysteryMountainPlanet::MysteryMountainPlanetNrvWaitVanished::sInstance); }
MysteryMountainPlanet::~MysteryMountainPlanet() {}
