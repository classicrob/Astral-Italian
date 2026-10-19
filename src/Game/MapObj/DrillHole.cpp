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


namespace NrvDrillHole {
    class DrillHoleNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {};

        static DrillHoleNrvWait sInstance;
    };

    class DrillHoleNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            DrillHole* actor = reinterpret_cast< DrillHole* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Appear");
            }
            MR::setNerveAtBckStopped(actor, &DrillHoleNrvWait::sInstance);
        };

        static DrillHoleNrvAppear sInstance;
    };

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

DrillHoleHolder::DrillHoleHolder(const char* pName) : NameObj(pName) {
    mHoles1 = nullptr;
    mHoles2 = nullptr;
}

void DrillHoleHolder::init(const JMapInfoIter&) {
    mHoles1 = new DrillHole*[8];
    mHoles2 = new DrillHole*[8];
    for (s32 i = 0; i < 8; i++) {
        mHoles1[i] = new DrillHole("DrillHoleIn", "\203h\203\212\203\213\202\314\214\212");
        mHoles1[i]->initWithoutIter();
        mHoles2[i] = new DrillHole("DrillHoleOut", "\203h\203\212\203\213\202\314\214\212");
        mHoles2[i]->initWithoutIter();
    }
}

DrillHole* getFreeDrillHole(DrillHoleHolder*, DrillHole** pHoles) {
    DrillHole* pBest = nullptr;
    s32 bestCounter = -1;
    for (s32 i = 0; i < 8; i++) {
        DrillHole* pHole = pHoles[i];
        if (MR::isDead(pHole)) {
            return pHole;
        }
        if (bestCounter < pHole->mCounter) {
            bestCounter = pHole->mCounter;
            pBest = pHole;
        }
    }
    return pBest;
}

void appearDrillHole(DrillHoleHolder* pHolder, DrillHole** pHoles, const TPos3f* pMtx) {
    DrillHole* pHole = getFreeDrillHole(pHolder, pHoles);
    if (!MR::isDead(pHole)) {
        pHole->kill();
    }
    pHole->mBaseMtx.set(*pMtx);
    pHole->mBaseMtx.getTrans(pHole->mPosition);
    pHole->appear();
}

void createDrillHoleHolderSceneObj() {
    MR::createSceneObj(0x86);
}

void appearDrillHoleFromArea1(const TPos3f* pMtx) {
    TVec3f trans;
    pMtx->getTrans(trans);
    if (!MR::isInAreaObj("DrillHoleHiddenArea", trans)) {
        DrillHoleHolder* pHolder = static_cast< DrillHoleHolder* >(MR::getSceneObjHolder()->getObj(0x86));
        appearDrillHole(pHolder, pHolder->mHoles1, pMtx);
    }
}

void appearDrillHoleFromArea2(const TPos3f* pMtx) {
    TVec3f trans;
    pMtx->getTrans(trans);
    if (!MR::isInAreaObj("DrillHoleHiddenArea", trans)) {
        DrillHoleHolder* pHolder = static_cast< DrillHoleHolder* >(MR::getSceneObjHolder()->getObj(0x86));
        appearDrillHole(pHolder, pHolder->mHoles2, pMtx);
    }
}

DrillHoleHolder::~DrillHoleHolder() {}
