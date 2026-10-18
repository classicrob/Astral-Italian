#include "Game/MapObj/OldFortressCover.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvOldFortressCover {
class OldFortressCoverNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const;
    static OldFortressCoverNrvWait sInstance;
};
OldFortressCoverNrvWait OldFortressCoverNrvWait::sInstance;
NEW_NERVE(OldFortressCoverNrvOpen, OldFortressCover, Open);
class OldFortressCoverNrvEnd : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static OldFortressCoverNrvEnd sInstance;
};
OldFortressCoverNrvEnd OldFortressCoverNrvEnd::sInstance;
void OldFortressCoverNrvWait::execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isValidSwitchA(actor) && MR::isOnSwitchA(actor)) actor->setNerve(&OldFortressCoverNrvOpen::sInstance); }
}
OldFortressCover::OldFortressCover(const char* name) : LiveActor(name) {}
void OldFortressCover::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    bool isOpen = false;
    MR::getJMapInfoArg0NoInit(iter, &isOpen);
    MR::initUseStageSwitchReadA(this, iter);
    if (isOpen) initNerve(&NrvOldFortressCover::OldFortressCoverNrvEnd::sInstance);
    else initNerve(&NrvOldFortressCover::OldFortressCoverNrvWait::sInstance);
    MR::startAction(this, "Move");
    MR::setAllAnimFrameAndStop(this, "Move", 0.0f);
    MR::tryRegisterDemoCast(this, iter);
    makeActorAppeared();
}
void OldFortressCover::exeOpen() { if (MR::isFirstStep(this)) MR::startAction(this, "Move"); if (MR::isActionEnd(this)) setNerve(&NrvOldFortressCover::OldFortressCoverNrvEnd::sInstance); }
OldFortressCover::~OldFortressCover() {}
