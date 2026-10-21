#ifdef GALAXY_NATIVE
#include <petari/wm_route_diag.hpp>
#endif
#include "Game/System/GrandGalaxyMapStates.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateWait {
    NEW_NERVE(GrandGalaxyMapStateWaitWait, GrandGalaxyMapStateWait, Wait);
};  // namespace NrvGrandGalaxyMapStateWait

using namespace NrvGrandGalaxyMapStateWait;

GrandGalaxyMapStateWait::GrandGalaxyMapStateWait(GrandGalaxyMap* pHost)
    : ActorStateBase< GrandGalaxyMap >("State[ウェイト]", pHost), mWorldNo(-1), mPointingPoint(-1), mPointingTimer(0) {}

void GrandGalaxyMapStateWait::init() {
    initNerve(&GrandGalaxyMapStateWaitWait::sInstance, 0);
    mIsDead = true;
}

void GrandGalaxyMapStateWait::appear() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("GrandGalaxyMapStateWait::appear", 0);
#endif
    mIsDead = false;
    mHost->startAreaCamera(-1, -1);
    setNerve(&GrandGalaxyMapStateWaitWait::sInstance);
    mWorldNo = mHost->getCurrentWarpTarget();
    mHost->setupWorldName(mWorldNo);
}

void GrandGalaxyMapStateWait::kill() {
    mIsDead = true;
}

void GrandGalaxyMapStateWait::exeWait() {
    if (mWorldNo > 0) {
        mHost->requestAppearWorldName();
    }

    const char* pGalaxyName = mHost->getSelectedIconState();
    if (pGalaxyName != nullptr) {
        mHost->showGalaxyInfo(pGalaxyName);
    }

    mPointingTimer = MR::max(mPointingTimer - 1, 0);
}

void GrandGalaxyMapStateWait::startPointing(s32 point) {
    if (mPointingTimer == 0) {
        mHost->getWorld()->startRoutePointOpenSound(point);
    }

    mPointingPoint = point;
    mPointingTimer = 2;
}

void GrandGalaxyMapStateWait::endPointing() {
    if (mPointingTimer != 0) {
        mHost->getWorld()->startRoutePointCloseSound(mPointingPoint);
    }

    mPointingPoint = -1;
}

GrandGalaxyMapStateWait::~GrandGalaxyMapStateWait() {}
