#include "Game/Enemy/HammerBros.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"


extern const f32 cHammerSpinGravity;
extern const f32 cHammerThrownGravity;
extern const f32 cHammerLimitAngle;
extern const f32 cHammerOne;
extern const f32 cHammerScaleStep;
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
    MR::disableAllItemGenerator(this);
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
        MR::tryHitReactionBreak(this);
    }
}

void HammerBrosHammer::control() {
    BrosProjectileBase::control();
    if (isNerve(HNRV(Spin)) && mBind->tryStartPointBind()) {
        setNerve(HNRV(Freeze));
    }
}

// NONMATCHING: vector stack slots, saved GPRs, and floating-point/condition register allocation differ
void HammerBrosHammer::v70(const TVec3f& pos, const TVec3f& target) {
    mOwner = nullptr;
    MR::setPosition(this, pos);
    if (MR::isDead(this)) {
        appear();
    }
    setNerve(HNRV(Spin));
    MR::offBind(this);
    TVec3f gravity(mGravity);
    TVec3f playerGravity(*MR::getPlayerGravity());
    TVec3f horizontal(0.0f);
    if (MR::isSameDirection(playerGravity, gravity, 0.01f)) {
        TVec3f delta = pos - target;
        gravity.dot(delta);
        TVec3f player = *MR::getPlayerPos() - (playerGravity * 100.0f);
        TVec3f direction(0.0f);
        TVec3f difference = player - pos;
        MR::normalizeOrZero(&direction, difference);
        f32 cosine = MR::cosDegree(cHammerLimitAngle);
        if (direction.dot(gravity) < cosine) {
            f32 dot = direction.dot(gravity);
            TVec3f down(gravity);
            down *= dot;
            direction = direction - down;
            MR::normalizeOrZero(&direction);
            direction = (direction * (cHammerOne + cosine)) + (gravity * cosine);
        }
        TVec3f velocity(direction);
        velocity *= 18.0f;
        mVelocity.x = velocity.x;
        mVelocity.y = velocity.y;
        mVelocity.z = velocity.z;
        MR::normalize(&mFront, velocity);
        TVec3f front(0.0f);
        MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
        MR::calcFrontVec(&front, this);
        MR::vecKillElement(difference, gravity, &horizontal);
        f32 distance = MR::clamp(PSVECMag(&horizontal), 200.0f, 2500.0f);
        f32 time = MR::clamp(distance / 18.0f, 0.0f, 100.0f);
        TVec3f up = -gravity;
        TVec3f a(up);
        a *= 0.5f;
        TVec3f b(a);
        b *= time;
        TVec3f c(b);
        c *= cHammerSpinGravity;
        TVec3f d(c);
        d *= cHammerOne;
        velocity = velocity + d;
        TPos3f inverse;
        PSMTXInverse(getBaseMtx(), inverse.toMtxPtr());
        PSMTXMultVecSR(inverse.toMtxPtr(), &velocity, &mLocalVel);
        return;
    }
    TVec3f player = *MR::getPlayerPos() - (playerGravity * 100.0f);
    TVec3f ground(0.0f);
    MR::getPlayerGroundPos(&ground);
    TVec3f delta = pos - target;
    f32 height = -gravity.dot(delta);
    TVec3f playerDelta = player - ground;
    f32 playerHeight = -playerGravity.dot(playerDelta);
    TVec3f direction(0.0f);
    TVec3f difference = player - pos;
    MR::normalizeOrZero(&direction, difference);
    if (MR::isNearZero(direction, 0.001f)) {
        MR::calcFrontVec(&direction, this);
    }
    MR::normalize(&mFront, direction);
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    MR::calcFrontVec(&mFront, this);
    TVec3f side(1.0f, 0.0f, 0.0f);
    MR::calcSideVec(&side, this);
    f32 angle = MR::diffAngleSigned(gravity, playerGravity, side);
    f32 distance = PSVECMag(&difference);
    f32 heightDifference = playerHeight - height;
    distance = MR::sqrt<f32>(distance * distance - heightDifference * heightDifference);
    distance = MR::clamp(distance, 200.0f, 2500.0f);
    register f32 halfAngle = 0.5f * angle;
    register f32 zero = 0.0f;
    asm {
        fcmpo cr0, halfAngle, zero
    }
    f32 speed = MR::sqrt<f32>(0.5f * (distance * distance));
    speed = 2.0f * speed;
    f32 time = MR::clamp(speed / 18.0f, 0.0f, 100.0f);
    TVec3f velocity;
    velocity.x = 0.0f;
    velocity.y = heightDifference;
    velocity.z = speed;
    MR::normalize(&velocity);
    velocity *= 18.0f;
    velocity.y += cHammerOne * (0.5f * time * cHammerSpinGravity);
    mLocalVel = velocity;
}

void HammerBrosHammer::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Normal");
        MR::offBind(this);
    }
    mScale.x = MR::clamp(cHammerScaleStep + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
}

void HammerBrosHammer::exeSpin() {
    if (MR::isFirstStep(this)) {
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            s32 frame = 1;
            MR::setBckFrame(this, frame);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y += -cHammerSpinGravity;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(cHammerScaleStep + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState1();
        kill();
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
        MR::tryHitReaction(this, "\x94\xad\x8e\xcb");
        exeEaten();
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            s32 frame = 1;
            MR::setBckFrame(this, frame);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y += -cHammerThrownGravity;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(cHammerScaleStep + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState4();
        kill();
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
    if (!v7C() && !v80() && !v84() && !isNerve(HNRV(Fly))) {
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

const f32 cHammerSpinGravity = 0.15f;
const f32 cHammerThrownGravity = 0.3f;
const f32 cHammerLimitAngle = 105.0f;
const f32 cHammerOne = 1.0f;

const f32 cHammerScaleStep = 1.0f;
