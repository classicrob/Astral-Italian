#include "Game/MapObj/Togepin.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"

extern "C" void fn_803245E0(LiveActor*, Togepin*);
extern "C" void fn_800347C0(Togepin*, TVec3f*);
extern "C" void fn_80034910(Togepin*);
extern "C" void fn_8022BA10(Togepin*);

namespace NrvTogepin {
    NEW_NERVE(TogepinNrvAppear, Togepin, Appear);
    NEW_NERVE(TogepinNrvPanic, Togepin, Panic);
    NEW_NERVE(TogepinNrvWait, Togepin, Wait);
    NEW_NERVE(TogepinNrvReaction, Togepin, Reaction);
    NEW_NERVE(TogepinNrvHide, Togepin, Hide);
    class TogepinNrvHidden : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TogepinNrvHidden sInstance;
    };
    TogepinNrvHidden TogepinNrvHidden::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(TogepinNrvBlow, Togepin, Blow);
};  // namespace NrvTogepin


TogepinHole::TogepinHole(const char* pName, LiveActor* pConnector) : LiveActor(pName) {
    mConnector = (MapObjConnector*)pConnector;
}

void TogepinHole::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "TogepinHole", false);
    makeActorAppeared();
}

void TogepinHole::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    if (mConnector != nullptr) {
        mConnector->connect(this);
    }
}

Togepin::Togepin(const char* pName, LiveActor* pParent) : LiveActor(pName) {
    mParent = pParent;
    mConnector = nullptr;
    mHole = nullptr;
    mInitPos.x = 0.0f;
    mInitPos.y = 0.0f;
    mInitPos.z = 0.0f;
    mIsRushRockMario = false;
}

void Togepin::init(const JMapInfoIter& rIter) {
    mConnector = new MapObjConnector(this);
    MR::initActor(this, rIter, "Togepin", false);
    initNerve(GET_NERVE(Togepin, TogepinNrvAppear), 0);
    MR::offBind(this);

    mHole = new TogepinHole(
        "\x8C\x8A\x83\x82\x83\x66\x83\x8B", (LiveActor*)mConnector);
    MR::copyTransRotateScale(this, mHole);
    mHole->initWithoutIter();
    mInitPos.set(mPosition);
    MR::initUseStageSwitchReadA(this, rIter);
    makeActorAppeared();
}

void Togepin::initAfterPlacement() {
    mConnector->attachToUnder();
}

void Togepin::control() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this) && !mIsRushRockMario) {
            tryHide();
            return;
        }
        mIsRushRockMario = MR::isCurrentRushRockMario();
        if (isNerve(GET_NERVE(Togepin, TogepinNrvHidden))) {
            setNerve(GET_NERVE(Togepin, TogepinNrvAppear));
            MR::offBind(this);
        }
    }
}

void Togepin::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    if (isNerveTypeBlow()) {
        mConnector->connect();
    }
}

void Togepin::attackSensor(HitSensor* pSelf, HitSensor* pOther) {
    if (isNerveTypeHittable() && MR::isSensorPlayer(pOther)) {
        MR::sendMsgPush(pOther, pSelf);
    }
}

bool Togepin::receiveMsgPlayerAttack(u32 msg, HitSensor* pSelf, HitSensor* pOther) {
    if (MR::isMsgRockAttack(msg)) {
        return tryBlow(pSelf, pOther);
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        if (isNerve(GET_NERVE(Togepin, TogepinNrvWait)) || isNerve(GET_NERVE(Togepin, TogepinNrvReaction))) {
            setNerve(GET_NERVE(Togepin, TogepinNrvReaction));
            return true;
        }
    }
    if ((MR::isMsgPlayerHitAll(msg) || MR::isMsgPlayerTrample(msg)) && isNerve(GET_NERVE(Togepin, TogepinNrvWait))) {
        setNerve(GET_NERVE(Togepin, TogepinNrvReaction));
        return true;
    }
    return false;
}

bool Togepin::tryBlow(HitSensor* pSelf, HitSensor* pOther) {
    if (!isNerveTypeHittable()) {
        return false;
    }
    MR::setVelocitySeparateHV(this, pSelf, pOther, 40.0f, 50.0f);
    setNerve(GET_NERVE(Togepin, TogepinNrvBlow));
    return true;
}

void Togepin::tryHide() {
    if (isNerve(GET_NERVE(Togepin, TogepinNrvWait)) || isNerve(GET_NERVE(Togepin, TogepinNrvPanic)) || isNerve(GET_NERVE(Togepin, TogepinNrvReaction))) {
        setNerve(GET_NERVE(Togepin, TogepinNrvHide));
    }
}

void Togepin::tryAppear() {
    if (isNerve(GET_NERVE(Togepin, TogepinNrvHidden))) {
        setNerve(GET_NERVE(Togepin, TogepinNrvAppear));
        MR::offBind(this);
    }
}

void Togepin::reappear() {
    MR::resetPosition(this, mInitPos);
    MR::zeroVelocity(this);
    setNerve(GET_NERVE(Togepin, TogepinNrvAppear));
    MR::offBind(this);
    appear();
}

void Togepin::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::startAction(mHole, "Appear");
        MR::validateShadow(this, nullptr);
        MR::offBind(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Togepin, TogepinNrvWait));
    }
}

void Togepin::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mHole, "Wait");
    }
    if (MR::isCurrentRushRockMario()) {
        setNerve(GET_NERVE(Togepin, TogepinNrvPanic));
    }
}

void Togepin::exeReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
        MR::tryStartActionNotPlaying(mHole, "Wait");
    }
    if (MR::isCurrentRushRockMario()) {
        setNerve(GET_NERVE(Togepin, TogepinNrvPanic));
        return;
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Togepin, TogepinNrvWait));
    }
}

void Togepin::exePanic() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Panic");
    }
    if (!MR::isCurrentRushRockMario()) {
        setNerve(GET_NERVE(Togepin, TogepinNrvWait));
    }
}

void Togepin::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::startAction(mHole, "Hide");
        MR::invalidateShadow(this, nullptr);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Togepin, TogepinNrvHidden));
    }
}

void Togepin::exeBlow() {
    if (MR::isFirstStep(this)) {
        if (mParent != nullptr) {
            fn_803245E0(mParent, this);
        }
        fn_800347C0(this, &mPosition);
        MR::startAction(this, "Blow");
        MR::invalidateClipping(this);
        MR::onBind(this);
    }
    MR::addVelocityToGravity(this, 2.0f);
    MR::attenuateVelocity(this, 0.985f);
    if (MR::isGreaterStep(this, 50) || (MR::isGreaterStep(this, 5) && MR::isBinded(this))) {
        MR::zeroVelocity(this);
        fn_80034910(this);
        MR::offBind(this);
        kill();
        if (mParent == nullptr) {
            fn_8022BA10(this);
        }
    }
}

bool Togepin::isNerveTypeHittable() const {
    if (isNerve(GET_NERVE(Togepin, TogepinNrvWait)) || isNerve(GET_NERVE(Togepin, TogepinNrvPanic))) {
        return true;
    }
    return false;
}

bool Togepin::isNerveTypeBlow() const {
    return isNerve(GET_NERVE(Togepin, TogepinNrvBlow)) == false;
}

Togepin::~Togepin() {
}

TogepinHole::~TogepinHole() {
}
