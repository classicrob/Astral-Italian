#include "Game/MapObj/SecretGate.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CollectCounter.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"


namespace NrvSecretGate {
    class SecretGateNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SecretGateNrvWait sInstance;
    };
    SecretGateNrvWait SecretGateNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(SecretGateNrvPass, SecretGate, Pass);

    class SecretGateNrvWaitAfterPass : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SecretGateNrvWaitAfterPass sInstance;
    };
    SecretGateNrvWaitAfterPass SecretGateNrvWaitAfterPass::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSecretGate

SecretGate::SecretGate(const char* pName) : LiveActor(pName) {
    mScaleA = 500.0f;
    mScaleB = 600.0f;
    mIsOffsetUp = false;
    mUnk9C.x = 0.0f;
    mUnk9C.y = 0.0f;
    mUnk9C.z = 0.0f;
    mEffectTrans.x = 0.0f;
    mEffectTrans.y = 0.0f;
    mEffectTrans.z = 0.0f;
}

#pragma dont_inline on
void SecretGate::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, false);
    MR::calcGravity(this);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mScaleA);
    MR::getJMapInfoArg1NoInit(rIter, &mScaleB);
    MR::getJMapInfoArg2NoInit(rIter, &mIsOffsetUp);
    f32 dropLength = 1000.0f;
    MR::getJMapInfoArg3NoInit(rIter, &dropLength);
    MR::setShadowDropLength(this, nullptr, dropLength);

    mEffectTrans = mPosition;
    if (!mIsOffsetUp) {
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        TVec3f offset = up * mScaleB;
        mEffectTrans += offset;
    }
    MR::setEffectHostSRT(this, "Pass", &mEffectTrans, nullptr, nullptr);
    initNerve(GET_NERVE(SecretGate, SecretGateNrvWait), 0);
    makeActorAppeared();
}
#pragma dont_inline reset

void SecretGate::exePass() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadStrong(this, 0);
        MR::invalidateHitSensor(this, "Body");
        MR::emitEffect(this, "Pass");
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        setNerve(GET_NERVE(SecretGate, SecretGateNrvWaitAfterPass));
    }
}

bool SecretGate::isNerveWaitAfterPass() const {
    return isNerve(GET_NERVE(SecretGate, SecretGateNrvWaitAfterPass));
}

void SecretGate::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(SecretGate, SecretGateNrvWait))) {
        return;
    }
    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return;
    }
    LiveActor* pRush = MR::getCurrentRushActor();
    if (pRush == nullptr) {
        return;
    }
    if (!MR::isEqualSensor(pReceiver, pRush, "Body")) {
        return;
    }
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f toPlayer(0.0f);
    TVec3f side(0.0f);
    MR::calcVecToPlayer(&toPlayer, this);
    MR::calcDirToPlayer(&side, this);
    if (!(front.dot(side) < 0.001f) && !(front.dot(toPlayer) > 350.0f)) {
        setNerve(GET_NERVE(SecretGate, SecretGateNrvPass));
    }
}

SecretGate::~SecretGate() {
}
