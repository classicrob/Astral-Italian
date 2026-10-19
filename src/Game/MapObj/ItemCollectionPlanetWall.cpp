#pragma auto_inline off
#include "Game/MapObj/ItemCollectionPlanetWall.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include <cmath>

namespace { const char* sDemoName = "パワースター帰還[アイテム惑星]"; }
namespace NrvItemCollectionPlanetWall {
    class ItemCollectionPlanetWallNrvWait : public Nerve {
    public:
        void execute(Spine*) const {}
        static ItemCollectionPlanetWallNrvWait sInstance;
    };
    ItemCollectionPlanetWallNrvWait ItemCollectionPlanetWallNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(ItemCollectionPlanetWallNrvFadeIn, ItemCollectionPlanetWall, FadeIn);
}
ItemCollectionPlanetWall::ItemCollectionPlanetWall(const char* name) : LiveActor(name) {}
void ItemCollectionPlanetWall::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvWait::sInstance);
    MR::startAction(this, "FadeIn");
    MR::setAllAnimFrameAtEnd(this, "FadeIn");
    if (MR::tryRegisterDemoCast(this, sDemoName, iter)) {
        MR::registerDemoActionNerveDirect(this, &NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvFadeIn::sInstance, sDemoName, nullptr);
        MR::registerDemoActionFunctorDirect(this, MR::FunctorV0M<ItemCollectionPlanetWall*, void (ItemCollectionPlanetWall::*)()>(this, &ItemCollectionPlanetWall::listenDemoAction), sDemoName, nullptr);
    }
    makeActorAppeared();
}
void ItemCollectionPlanetWall::listenDemoAction() { MR::startAction(this, "FadeIn"); }
void ItemCollectionPlanetWall::exeFadeIn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FadeIn");
        MR::setAllAnimFrameAndStop(this, "FadeIn", 0.0f);
    }
    if (MR::isDemoLastStep()) setNerve(&NrvItemCollectionPlanetWall::ItemCollectionPlanetWallNrvWait::sInstance);
}
ItemCollectionPlanetWall::~ItemCollectionPlanetWall() {}

