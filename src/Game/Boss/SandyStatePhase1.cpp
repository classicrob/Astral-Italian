#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

namespace NrvSandyStatePhase1 {
    class SandyStatePhase1NrvStart;
    class SandyStatePhase1NrvClose;
    class SandyStatePhase1NrvThrow;
    class SandyStatePhase1NrvOpen;
    class SandyStatePhase1NrvOpenWait;
    class SandyStatePhase1NrvDamage;
    class SandyStatePhase1NrvDamageClose;

    NEW_NERVE(SandyStatePhase1NrvStart, SandyStatePhase1, Start);
    class SandyStatePhase1NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvClose sInstance;
    };
    class SandyStatePhase1NrvThrow : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvThrow sInstance;
    };
    class SandyStatePhase1NrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvOpen sInstance;
    };
    class SandyStatePhase1NrvOpenWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvOpenWait sInstance;
    };
    NEW_NERVE(SandyStatePhase1NrvDamage, SandyStatePhase1, Damage);
    class SandyStatePhase1NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvDamageClose sInstance;
    };
    SandyStatePhase1NrvClose SandyStatePhase1NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvThrow SandyStatePhase1NrvThrow::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvOpen SandyStatePhase1NrvOpen::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvOpenWait SandyStatePhase1NrvOpenWait::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvDamageClose SandyStatePhase1NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase1NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase1NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase1NrvThrow::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateThrow(pState, 3, 20, false)) {
            pState->setNerve(&SandyStatePhase1NrvOpen::sInstance);
        }
    }
    inline void SandyStatePhase1NrvOpen::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateOpenStart(pState, 60)) {
            pState->setNerve(&SandyStatePhase1NrvOpenWait::sInstance);
        }
    }
    inline void SandyStatePhase1NrvOpenWait::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateOpenWait(pState, 240)) {
            pState->setNerve(&SandyStatePhase1NrvClose::sInstance);
        }
    }
    inline void SandyStatePhase1NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase1NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase1

// 80101750
SandyStatePhase1::SandyStatePhase1(Sandy* pSandy) : ActorStateBase< Sandy >("\203t\203F\201[\203Y\202P", pSandy), mDamage(nullptr) {
    initNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 240, 240, -1);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance, nullptr);
}

// 801017F0
void SandyStatePhase1::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvStart::sInstance);
}

// 80101800
void SandyStatePhase1::control() {
    if (!isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance)) {
        mHost->updateTarget();
    }
}

// 80101840
bool SandyStatePhase1::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance);
            } else {
                mHost->startScaleHitReaction();
            }
            return true;
        }
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
            mHost->startScaleHitReaction();
            return true;
        }
        return false;
    }
    return false;
}

// 80101970
void SandyStatePhase1::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->forceWaitHands();
    }
    if (MR::isGreaterEqualStep(this, 180) && MR::isBckLooped(mHost)) {
        mHost->setWaitHands();
        setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvThrow::sInstance);
    }
}

// 80101A00
void SandyStatePhase1::exeDamage() {
    if (MR::updateActorState(this)) {
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamageClose::sInstance);
        }
    }
}

// 80101A70
SandyStatePhase1::~SandyStatePhase1() {
}
