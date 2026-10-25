#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotBreakableStep {
    class KoopaJrRobotBreakableStepNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotBreakableStepNrvWait sInstance;
    };
    KoopaJrRobotBreakableStepNrvWait KoopaJrRobotBreakableStepNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KoopaJrRobotBreakableStepNrvBreak, KoopaJrRobotBreakableStep, Break);
    NEW_NERVE(KoopaJrRobotBreakableStepNrvBlow, KoopaJrRobotBreakableStep, Blow);
    NEW_NERVE(KoopaJrRobotBreakableStepNrvFall, KoopaJrRobotBreakableStep, Fall);
};  // namespace NrvKoopaJrRobotBreakableStep

KoopaJrRobotBreakableStep::KoopaJrRobotBreakableStep(const char* pName) : LiveActor(pName) {}

void KoopaJrRobotBreakableStep::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(&NrvKoopaJrRobotBreakableStep::KoopaJrRobotBreakableStepNrvWait::sInstance, 0);
    MR::startAction(this, "Normal");
    makeActorAppeared();
}

void KoopaJrRobotBreakableStep::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
    }
    if (MR::isStep(this, 12)) {
        setNerve(&NrvKoopaJrRobotBreakableStep::KoopaJrRobotBreakableStepNrvBlow::sInstance);
    }
}

// NONMATCHING: regalloc of the TVec3 temp (f2/f0 swap)
inline TVec3f calcRobotStepBlow(const TVec3f& gravity, f32 speed) {
    TVec3f velocity(gravity);
    velocity.scale(speed);
    return velocity;
}

inline void addRobotStepVelocity(TVec3f& velocity, const TVec3f& delta) {
    velocity += delta;
}

void KoopaJrRobotBreakableStep::exeBlow() {
    if (MR::isFirstStep(this)) {
        addRobotStepVelocity(mVelocity, calcRobotStepBlow(mGravity, 15.0f));
    }
    if (0.5f < PSVECMag(&mVelocity)) {
        mVelocity.scale(0.92f);
    }
    if (MR::isGreaterStep(this, 150)) {
        setNerve(&NrvKoopaJrRobotBreakableStep::KoopaJrRobotBreakableStepNrvFall::sInstance);
    }
}

void KoopaJrRobotBreakableStep::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::onCalcGravity(this);
        MR::invalidateClipping(this);
    }
    MR::addVelocityToGravity(this, 0.1f);
    if (MR::isGreaterStep(this, 180)) {
        MR::validateClipping(this);
        kill();
    }
}

bool KoopaJrRobotBreakableStep::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool hit = false;
    if (MR::isSensorType(pSender, 0x33) || MR::isSensorType(pSender, 0x2E) || MR::isSensorType(pSender, 0x71)) {
        hit = true;
    }
    if (!hit) {
        return false;
    }
    if (!isNerve(&NrvKoopaJrRobotBreakableStep::KoopaJrRobotBreakableStepNrvWait::sInstance)) {
        return false;
    }
    setNerve(&NrvKoopaJrRobotBreakableStep::KoopaJrRobotBreakableStepNrvBreak::sInstance);
    return true;
}

KoopaJrRobotBreakableStep::~KoopaJrRobotBreakableStep() {}
