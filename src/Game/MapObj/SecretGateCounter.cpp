#include "Game/MapObj/SecretGate.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CollectCounter.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"



namespace NrvSecretGateCounter {
    NEW_NERVE(SecretGateCounterNrvCount, SecretGateCounter, Count);
    NEW_NERVE(SecretGateCounterNrvMovementOn, SecretGateCounter, MovementOn);
};  // namespace NrvSecretGateCounter

inline SecretGateGroup::SecretGateGroup(const char* pName, int num) : LiveActorGroup(pName, num) {
}

SecretGateGroup::~SecretGateGroup() {
}

SecretGateCounter::SecretGateCounter(const char* pName) : LiveActor(pName), mGroup(nullptr), mPassCount(0) {
}

void SecretGateCounter::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, rIter);
    s32 num = MR::getChildObjNum(rIter);
    mGroup = new SecretGateGroup("シークレットゲート監視", num);
    for (s32 i = 0; i < num; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, "SecretGate")) {
            mGroup->registerActor(static_cast<LiveActor*>(NameObjFactory::initChildObj(rIter, i)));
        }
    }
    MR::invalidateClipping(this);
    mCounter = new CollectCounter("通過ゲートカウント", 110.0f);
    mCounter->initWithoutIter();
    initSound(3, "SecretGateCounter", nullptr, TVec3f(0.0f));
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
