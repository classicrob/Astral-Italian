#include "Game/MapObj/DrillHole.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

#ifndef GALAXY_NATIVE
namespace MR {
    void createSceneObj(s32);
};  // namespace MR
#endif


namespace { char sDrillHoleAppear[] = "Appear"; }

namespace NrvDrillHole {
    class DrillHoleNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static DrillHoleNrvAppear sInstance;
    };

    class DrillHoleNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {};

        static DrillHoleNrvWait sInstance;
    };

    inline void DrillHoleNrvAppear::execute(Spine* pSpine) const {
            DrillHole* actor = reinterpret_cast< DrillHole* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, sDrillHoleAppear);
            }
            MR::setNerveAtBckStopped(actor, &DrillHoleNrvWait::sInstance);
        }

    DrillHoleNrvAppear DrillHoleNrvAppear::sInstance ATTRIBUTE_WEAK;
    DrillHoleNrvWait DrillHoleNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvDrillHole

DrillHole::DrillHole(const char* pModelName, const char* pName) : LiveActor(pName) {
    mModelName = pModelName;
    mCounter = 0;
    mBaseMtx.identity();
}

DrillHole::~DrillHole() {}

void DrillHole::init(const JMapInfoIter&) {
    MR::initActor(this, mModelName, false);
    initNerve(&NrvDrillHole::DrillHoleNrvAppear::sInstance, 0);
    makeActorDead();
}

void DrillHole::appear() {
    LiveActor::appear();
    mCounter = 0;
    setNerve(&NrvDrillHole::DrillHoleNrvAppear::sInstance);
}

void DrillHole::control() {
    mCounter++;
}

void DrillHole::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
}
