#include "Game/Enemy/Kanina.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Util.hpp"
namespace KaninaYoshiStateFunction {
    LiveActor* createClaw(Kanina* host) NO_INLINE {
        LiveActor* claw = MR::createDemoObj("カニーナのツメ", "KaninaClaw", host->getBaseMtx());
        claw->initWithoutIter();
        claw->makeActorDead();
        return claw;
    }
}
