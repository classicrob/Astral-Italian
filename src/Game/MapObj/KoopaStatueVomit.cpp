#include "Game/MapObj/KoopaStatueVomit.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvKoopaStatueVomit {
NEW_NERVE(KoopaStatueVomitNrvWait, KoopaStatueVomit, Wait);
NEW_NERVE(KoopaStatueVomitNrvRotate, KoopaStatueVomit, Rotate);
class KoopaStatueVomitNrvEnd : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static KoopaStatueVomitNrvEnd sInstance;
};
KoopaStatueVomitNrvEnd KoopaStatueVomitNrvEnd::sInstance;
}
KoopaStatueVomit::KoopaStatueVomit(const char* name) : MapObjActor(name), mRotateRight(false) {}
void KoopaStatueVomit::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MR::getJMapInfoArg0NoInit(iter, &mRotateRight);
    MapObjActorInitInfo info;
    info.setupModelName("KoopaStatue");
    info.setupLightCtrl();
    info.setupMultiFur(2);
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    info.setupSound(4, "KoopaStatueVomit");
    info.setupNerve(&NrvKoopaStatueVomit::KoopaStatueVomitNrvWait::sInstance);
    initialize(iter, info);
}
void KoopaStatueVomit::exeWait() { if (MR::isFirstStep(this)) MR::startAction(this, "Wait"); if (MR::isOnSwitchB(this)) setNerve(&NrvKoopaStatueVomit::KoopaStatueVomitNrvRotate::sInstance); }
void KoopaStatueVomit::exeRotate() {
    if (MR::isFirstStep(this)) {
        if (mRotateRight) MR::startAction(this, "RotateRight");
        else MR::startAction(this, "RotateLeft");
    }
    if (MR::isActionEnd(this)) {
        if (MR::isValidSwitchA(this)) MR::onSwitchA(this);
        setNerve(&NrvKoopaStatueVomit::KoopaStatueVomitNrvEnd::sInstance);
    }
}
void KoopaStatueVomit::initCaseNoUseSwitchB(const MapObjActorInitInfo&) {}
void KoopaStatueVomit::initCaseNoUseSwitchA(const MapObjActorInitInfo&) {}
void KoopaStatueVomit::initCaseUseSwitchB(const MapObjActorInitInfo&) {}
void KoopaStatueVomit::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
KoopaStatueVomit::~KoopaStatueVomit() {}
