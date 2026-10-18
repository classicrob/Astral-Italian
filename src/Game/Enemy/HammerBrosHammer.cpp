#include "Game/Enemy/HammerBros.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern "C" void fn_8022BC70(LiveActor*);
extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_80034630(LiveActor*, const char*, HitSensor*, HitSensor*);
extern "C" void fn_80034910(LiveActor*);
extern "C" void fn_80012B90(LiveActor*, f32, f32, HitSensor*, HitSensor*, s32);
extern "C" void fn_800348C0(LiveActor*);
extern "C" void fn_8022BAE0(LiveActor*);
extern "C" s32 lbl_807E3028;

// Per-state actor states (defined in the second half of the original unit, not ported yet).
class HammerBrosStateJump : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateJump(HammerBros*);
    u8 _pad[88 - sizeof(ActorStateBase< HammerBros >)];
};

class HammerBrosStateAttack : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateAttack(HammerBros*);
    u8 _pad[20 - sizeof(ActorStateBase< HammerBros >)];
};

class HammerBrosStateSwoon : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateSwoon(HammerBros*);
    void fn_8018AFF0();
    bool fn_8018B000() const;
    bool fn_8018B080() const;
    bool fn_8018B0E0() const;
    u8 _pad[24 - sizeof(ActorStateBase< HammerBros >)];
};

static TVec3f sThrowVel(0.0f, 11.0f, 27.0f);
HammerBrosHammer::HammerBrosHammer(const char* pName) : BrosProjectileBase(pName), mBind(nullptr) {
}

namespace NrvHammerBrosHammer {
    NEW_NERVE(HammerBrosHammerNrvFly, HammerBrosHammer, Fly);
    NEW_NERVE(HammerBrosHammerNrvSpin, HammerBrosHammer, Spin);
    NEW_NERVE(HammerBrosHammerNrvTongue, HammerBrosHammer, Tongue);
    NEW_NERVE(HammerBrosHammerNrvEaten, HammerBrosHammer, Eaten);
    NEW_NERVE(HammerBrosHammerNrvThrown, HammerBrosHammer, Thrown);

    class HammerBrosHammerNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            HammerBrosHammer* pActor = reinterpret_cast< HammerBrosHammer* >(pSpine->mExecutor);
            pActor->_B4 = true;
            MR::updateActorStateAndNextNerve(pActor, &HammerBrosHammerNrvSpin::sInstance);
        }
        static HammerBrosHammerNrvFreeze sInstance;
    };
    HammerBrosHammerNrvFreeze HammerBrosHammerNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvHammerBrosHammer

#define HNRV(n) (&NrvHammerBrosHammer::HammerBrosHammerNrv##n::sInstance)

void HammerBrosHammer::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "HammerBrosHammer", false);
    BrosProjectileBase::init(rIter);
    initNerve(HNRV(Fly), 1);
    mBind = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBind, HNRV(Freeze), "SupportFreeze");
    fn_8022BC70(this);
    makeActorDead();
}

void HammerBrosHammer::appear() {
    BrosProjectileBase::appear();
    setNerve(HNRV(Fly));
    MR::startAction(this, "Normal");
    MR::showModel(this);
}

void HammerBrosHammer::kill() {
    LiveActor::kill();
    if (!isNerve(HNRV(Tongue))) {
        fn_800348C0(this);
    }
}

void HammerBrosHammer::control() {
    BrosProjectileBase::control();
    if (isNerve(HNRV(Spin)) && mBind->tryStartPointBind()) {
        setNerve(HNRV(Freeze));
    }
}

void HammerBrosHammer::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Normal");
        MR::offBind(this);
    }
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
}

void HammerBrosHammer::exeSpin() {
    if (MR::isFirstStep(this)) {
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            MR::setBckFrame(this, 1.0f);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y -= 0.15f;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState1();
    }
}

void HammerBrosHammer::exeTongue() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
        MR::startAction(this, "Normal");
    }
    exeEaten();
}

void HammerBrosHammer::exeThrown() {
    if (MR::isFirstStep(this)) {
        fn_800345E0(this, "\x94\xad\x8e\xcb");
        exeEaten();
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            MR::setBckFrame(this, 1.0f);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y -= 0.3f;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState4();
    }
}

bool HammerBrosHammer::v78() {
    return isNerve(HNRV(Fly));
}

bool HammerBrosHammer::v7C() {
    return isNerve(HNRV(Tongue));
}

bool HammerBrosHammer::v80() {
    return isNerve(HNRV(Eaten));
}

bool HammerBrosHammer::v84() {
    return isNerve(HNRV(Thrown));
}

bool HammerBrosHammer::v88() {
    if (!v78() && !v7C() && !v80() && !isNerve(HNRV(Fly))) {
        setNerve(HNRV(Tongue));
        mLocalVel = TVec3f(0.0f);
        MR::zeroVelocity(this);
        MR::invalidateHitSensors(this);
        return true;
    }
    return false;
}

bool HammerBrosHammer::v8C() {
    if (v7C()) {
        setNerve(HNRV(Eaten));
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        return true;
    }
    return false;
}

bool HammerBrosHammer::v90() {
    if (v80()) {
        setNerve(HNRV(Thrown));
        MR::getPlayerThrowVec(&mFront);
        mLocalVel = sThrowVel;
        MR::validateHitSensors(this);
        MR::showModel(this);
        return true;
    }
    return false;
}

HammerBrosHammer::~HammerBrosHammer() {
}
