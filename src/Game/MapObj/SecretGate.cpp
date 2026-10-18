#include "Game/MapObj/SecretGate.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CollectCounter.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"

extern "C" void fn_8000E850(TVec3f*, LiveActor*);

namespace NameObjFactory {
    LiveActor* initChildObj(const JMapInfoIter&, s32);
};  // namespace NameObjFactory

// group name / "SecretGate" / CollectCounter name / "SecretGateCounter" in one table, like the target .data block
static const char sSecretGateNames[] = "\x83\x56\x81\x5B\x83\x4E\x83\x8C\x83\x62\x83\x67\x83\x51\x81\x5B\x83\x67\x8A\xC4\x8E\x8B\x00\x00\x53\x65\x63\x72\x65\x74\x47\x61\x74\x65\x00\x00\x92\xCA\x89\xDF\x83\x51\x81\x5B\x83\x67\x83\x4A\x83\x45\x83\x93\x83\x67\x00\x00\x53\x65\x63\x72\x65\x74\x47\x61\x74\x65\x43\x6F\x75\x6E\x74\x65\x72\x00\x00\x00";

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

namespace NrvSecretGateCounter {
    NEW_NERVE(SecretGateCounterNrvCount, SecretGateCounter, Count);
    NEW_NERVE(SecretGateCounterNrvMovementOn, SecretGateCounter, MovementOn);
};  // namespace NrvSecretGateCounter

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
    fn_8000E850(&side, this);
    if (!(front.dot(side) < 0.001f) && !(front.dot(toPlayer) > 350.0f)) {
        setNerve(GET_NERVE(SecretGate, SecretGateNrvPass));
    }
}

SecretGate::~SecretGate() {
}

SecretGateGroup::SecretGateGroup(const char* pName, int num) : LiveActorGroup(pName, num) {
}

SecretGateGroup::~SecretGateGroup() {
}

SecretGateCounter::SecretGateCounter(const char* pName) : LiveActor(pName), mGroup(nullptr), mPassCount(0) {
}

void SecretGateCounter::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, rIter);
    s32 num = MR::getChildObjNum(rIter);
    mGroup = new SecretGateGroup(&sSecretGateNames[0], num);
    for (s32 i = 0; i < num; i++) {
        const char* pName;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, &sSecretGateNames[0x18])) {
            mGroup->registerActor(NameObjFactory::initChildObj(rIter, i));
        }
    }
    MR::invalidateClipping(this);
    mCounter = new CollectCounter(&sSecretGateNames[0x24]);
    mCounter->initWithoutIter();
    initSound(3, &sSecretGateNames[0x38], nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(SecretGateCounter, SecretGateCounterNrvCount), 0);
    makeActorAppeared();
}

void SecretGateCounter::exeCount() {
    s32 count = 0;
    s32 num = mGroup->getObjNum();
    for (s32 i = 0; i < num; i++) {
        if (static_cast< SecretGate* >(mGroup->getActor(i))->isNerveWaitAfterPass()) {
            count++;
        }
    }
    if (mPassCount < count) {
        mPassCount = count;
        mCounter->setCount(count);
        if (mPassCount < num) {
            MR::startActionSound(this, "Pass", mPassCount - 1, -1, -1);
        } else {
            MR::startActionSound(this, "PassFinal", mPassCount - 1, -1, -1);
        }
        setNerve(GET_NERVE(SecretGateCounter, SecretGateCounterNrvMovementOn));
    }
}

void SecretGateCounter::exeMovementOn() {
    if (MR::isFirstStep(this)) {
        MR::requestMovementOn(mCounter);
    }
    setNerve(GET_NERVE(SecretGateCounter, SecretGateCounterNrvCount));
}

SecretGateCounter::~SecretGateCounter() {
}
