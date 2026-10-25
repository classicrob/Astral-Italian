#define ACTOR_STATE_BASE_CTOR_NOINLINE
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

namespace NrvSandyStatePhase2 {
    class SandyStatePhase2NrvClose;
    class SandyStatePhase2NrvThrow;
    class SandyStatePhase2NrvOpen;
    class SandyStatePhase2NrvOpenWait;
    class SandyStatePhase2NrvDamageClose;

    class SandyStatePhase2NrvStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            SandyStatePhase2* actor = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
            actor->exeStart();
        }
        static SandyStatePhase2NrvStart sInstance;
    };
    class SandyStatePhase2NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvClose sInstance;
    };
    class SandyStatePhase2NrvThrow : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvThrow sInstance;
    };
    class SandyStatePhase2NrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvOpen sInstance;
    };
    class SandyStatePhase2NrvOpenWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvOpenWait sInstance;
    };
    class SandyStatePhase2NrvDamage : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            SandyStatePhase2* actor = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
            actor->exeDamage();
        }
        static SandyStatePhase2NrvDamage sInstance;
    };
    class SandyStatePhase2NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvDamageClose sInstance;
    };

    SandyStatePhase2NrvStart SandyStatePhase2NrvStart::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvClose SandyStatePhase2NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvThrow SandyStatePhase2NrvThrow::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvOpen SandyStatePhase2NrvOpen::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvOpenWait SandyStatePhase2NrvOpenWait::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvDamage SandyStatePhase2NrvDamage::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvDamageClose SandyStatePhase2NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase2NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase2NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase2NrvThrow::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateThrow(pState, 4, 90, true)) {
            pState->setNerve(&SandyStatePhase2NrvOpen::sInstance);
        }
    }
    inline void SandyStatePhase2NrvOpen::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateOpenStart(pState, 60)) {
            pState->setNerve(&SandyStatePhase2NrvOpenWait::sInstance);
        }
    }
    inline void SandyStatePhase2NrvOpenWait::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateOpenWait(pState, 120)) {
            pState->setNerve(&SandyStatePhase2NrvClose::sInstance);
        }
    }
    inline void SandyStatePhase2NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase2NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase2

// 80101D50
SandyStatePhase2::SandyStatePhase2(Sandy* pSandy) : SandyStateBase("\203t\203F\201[\203Y\202Q", pSandy), mDamage(nullptr) {
    initNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 120, 120, -1);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance, nullptr);
}

// 80101DF0
void SandyStatePhase2::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvStart::sInstance);
}

// 80101E00
void SandyStatePhase2::control() {
    if (!isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance)) {
        mHost->updateTarget();
    }
}

// 80101E40
bool SandyStatePhase2::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance);
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

// 80101F70
void SandyStatePhase2::exeStart() {
    if (MR::isFirstStep(this)) {
        mHost->setHandsStep(90);
        mHost->setHandsBallType0();
    }
    setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvThrow::sInstance);
}

// 80101FD0
void SandyStatePhase2::exeDamage() {
    if (MR::updateActorState(this)) {
        mHost->invalidateHands();
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamageClose::sInstance);
        }
    }
}

// 80102040
SandyStatePhase2::~SandyStatePhase2() {
}
