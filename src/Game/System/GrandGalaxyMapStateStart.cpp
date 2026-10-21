#ifdef GALAXY_NATIVE
#include <petari/wm_route_diag.hpp>
#endif
#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/System/GrandGalaxyMapStates.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateStart {
    NEW_NERVE(GrandGalaxyMapStateStartStart, GrandGalaxyMapStateStart, Start);
};  // namespace NrvGrandGalaxyMapStateStart

using namespace NrvGrandGalaxyMapStateStart;

GrandGalaxyMapStateStart::GrandGalaxyMapStateStart(GrandGalaxyMap* pHost) : ActorStateBase< GrandGalaxyMap >("State[開始状態]", pHost) {}

void GrandGalaxyMapStateStart::init() {
    initNerve(&GrandGalaxyMapStateStartStart::sInstance, 0);
    mIsDead = true;
}

void GrandGalaxyMapStateStart::appear() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("GrandGalaxyMapStateStart::appear", 0);
#endif
    mIsDead = false;
    mHost->changeRocketActionBck("Wait");
    mHost->startAreaCamera(-1, 0);
    MR::openSystemWipeWhiteFade(-1);
}

void GrandGalaxyMapStateStart::kill() {
    mIsDead = true;
}

void GrandGalaxyMapStateStart::exeStart() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("GrandGalaxyMapStateStart::exeStart", getNerveStep());
#endif
    if (MR::isFirstStep(this)) {
        getHost()->faceRocketInstant(TVec3f(0.0f, 0.0f, -1.0f));
    } else {
        if (MR::isLessStep(this, 30)) {
            getHost()->faceRocketInstant(TVec3f(0.0f, 0.0f, -1.0f));
        } else {
            mHost->setRocketSpeed(0.08f);
        }

        if (MR::isStep(this, 60)) {
            kill();
        }
    }
}

GrandGalaxyMapStateStart::~GrandGalaxyMapStateStart() {}
