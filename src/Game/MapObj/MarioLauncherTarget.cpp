#include "Game/MapObj/MarioLauncherTarget.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMarioLauncherTarget {
    class MarioLauncherTargetNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static MarioLauncherTargetNrvWait sInstance;
    };
    MarioLauncherTargetNrvWait MarioLauncherTargetNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(MarioLauncherTargetNrvHit, MarioLauncherTarget, Hit);
    class MarioLauncherTargetNrvNoHit : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            MarioLauncherTarget* actor = static_cast<MarioLauncherTarget*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) MR::startAction(actor, "NoHit");
        }
        static MarioLauncherTargetNrvNoHit sInstance;
    };
    MarioLauncherTargetNrvNoHit MarioLauncherTargetNrvNoHit::sInstance ATTRIBUTE_WEAK;
}
MarioLauncherTarget::MarioLauncherTarget(const char* name) : LiveActor(name), mRailMover(nullptr), mClippingCenter(0.0f, 0.0f, 0.0f) {}
MarioLauncherTarget::~MarioLauncherTarget() {}
void MarioLauncherTarget::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    if (MR::isExistRail(this)) {
        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(iter);
        mRailMover->start();
        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&mClippingCenter, this, 100.0f, radius);
    }
    initNerve(&NrvMarioLauncherTarget::MarioLauncherTargetNrvWait::sInstance);
    makeActorAppeared();
    if (MR::startBrkIfExist(this, "Hit")) MR::setAllAnimFrameAndStop(this, "Hit", 0.0f);
}
void MarioLauncherTarget::control() {
    if (mRailMover) {
        mRailMover->movement();
        if (mRailMover->isWorking()) {
            MR::setPosition(this, mRailMover->_28);
            mRailMover->tryResetPositionRepeat();
        }
    }
}
void MarioLauncherTarget::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        MR::startCSSound("CS_PERC_AUD_GOOD", nullptr, -1, 15, 1.0f);
        MR::addPlayerLeft(1);
    }
}
bool MarioLauncherTarget::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (msg != 0xF4) return false;
    if (MR::isEqualSensor(receiver, this, "Body")) {
        if (!isNerve(&NrvMarioLauncherTarget::MarioLauncherTargetNrvWait::sInstance)) return false;
        TVec3f pos(*MR::getSensorPos(this, "Hit"));
        f32 distance = PSVECDistance(&pos, MR::getSensorPos(sender));
        f32 radius = MR::getSensorRadius(sender);
        f32 targetRadius = MR::getSensorRadius(this, "Hit");
        if (targetRadius + radius > distance) {
            MR::tryHitReaction(this, "1up当たり");
            setNerve(&NrvMarioLauncherTarget::MarioLauncherTargetNrvHit::sInstance);
        } else {
            MR::tryHitReaction(this, "はずれヒット");
            setNerve(&NrvMarioLauncherTarget::MarioLauncherTargetNrvNoHit::sInstance);
        }
        return false;
    }
    return false;
}
