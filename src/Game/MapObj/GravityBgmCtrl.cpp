#pragma auto_inline off
#include "Game/MapObj/GravityBgmCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace NrvGravityBgmCtrl {
    NEW_NERVE(GravityBgmCtrlNrvChangeBack, GravityBgmCtrl, ChangeBack);
    NEW_NERVE(GravityBgmCtrlNrvChange, GravityBgmCtrl, Change);
    class GravityBgmCtrlNrvWait : public Nerve {
    public:
        void execute(Spine* spine) const {
            GravityBgmCtrl* actor = static_cast<GravityBgmCtrl*>(spine->mExecutor);
            if (MR::isGravityBGMPlaying() && !actor->tryChange())
                actor->setNerve(&GravityBgmCtrlNrvChangeBack::sInstance);
        }
        static GravityBgmCtrlNrvWait sInstance;
    };
    GravityBgmCtrlNrvWait GravityBgmCtrlNrvWait::sInstance ATTRIBUTE_WEAK;
}
namespace {
    const TVec3f sNormalGravity(0.0f, -1.0f, 0.0f);
    const TVec3f sReverseGravity(0.0f, 1.0f, 0.0f);
}
GravityBgmCtrl::GravityBgmCtrl(const char* name) : LiveActor(name) {}
void GravityBgmCtrl::init(const JMapInfoIter&) {
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(&NrvGravityBgmCtrl::GravityBgmCtrlNrvChangeBack::sInstance);
    initSound(3, "GravityBgmCtrl", nullptr, TVec3f(0.0f));
    makeActorAppeared();
}
bool GravityBgmCtrl::checkGravityDir(const TVec3f& direction) {
    return MR::isSameDirection(*MR::getPlayerGravity(), direction, 0.01f)
        && MR::getPlayerGravity()->dot(direction) > 0.0f;
}
void GravityBgmCtrl::exeChangeBack() {
    if (MR::isFirstStep(this)) {
        if (MR::isGravityBGMPlaying()) MR::setStageBGMState(1, 15);
        if (!MR::isDemoActive() && !MR::isStageStateScenarioOpeningCamera() && !MR::isExecScenarioStarter())
            MR::startActionSound(this, "ChangeBack");
    }
    if (tryChange()) return;
    if (tryWait()) return;
}
void GravityBgmCtrl::exeChange() {
    if (MR::isFirstStep(this)) {
        MR::setStageBGMState(2, 15);
        if (!MR::isDemoActive() && !MR::isStageStateScenarioOpeningCamera() && !MR::isExecScenarioStarter())
            MR::startActionSound(this, "Change");
    }
    if (tryChangeBack()) return;
    if (tryWait()) return;
}
bool GravityBgmCtrl::tryChangeBack() {
    if (checkGravityDir(sNormalGravity)) {
        setNerve(&NrvGravityBgmCtrl::GravityBgmCtrlNrvChangeBack::sInstance);
        return true;
    }
    return false;
}
bool GravityBgmCtrl::tryChange() {
    if (checkGravityDir(sReverseGravity)) {
        setNerve(&NrvGravityBgmCtrl::GravityBgmCtrlNrvChange::sInstance);
        return true;
    }
    return false;
}
bool GravityBgmCtrl::tryWait() {
    if (!MR::isGravityBGMPlaying()) {
        setNerve(&NrvGravityBgmCtrl::GravityBgmCtrlNrvWait::sInstance);
        return true;
    }
    return false;
}
GravityBgmCtrl::~GravityBgmCtrl() {}
