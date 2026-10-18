#include "Game/Enemy/Chorobon.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvChorobon {
    NEW_NERVE(ChorobonNrvWait, Chorobon, Wait);
    NEW_NERVE(ChorobonNrvMove, Chorobon, Move);
    NEW_NERVE(ChorobonNrvAttack, Chorobon, Attack);
    NEW_NERVE(ChorobonNrvReaction, Chorobon, Reaction);

    // Freeze has a custom execute: it hands control to the star-pointer state, then goes to Move.
    class ChorobonNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ChorobonNrvFreeze sInstance;
    };
    ChorobonNrvFreeze ChorobonNrvFreeze::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(ChorobonNrvKill, Chorobon, Kill);
};  // namespace NrvChorobon

#define GET_CHOROBON_NERVE(name) (&NrvChorobon::name::sInstance)

extern "C" void fn_8022BA10(LiveActor*);

// Helpers of this unit (unnamed in SMG2 symbols).
extern "C" bool fn_80176D90(Chorobon*);
extern "C" bool fn_80176E00(Chorobon*);
extern "C" bool fn_80176E30(Chorobon*);
extern "C" bool fn_80176E60(Chorobon*);

Chorobon::Chorobon(const char* pName) : LiveActor(pName) {
    mRailClip.x = 0.0f;
    mRailClip.y = 0.0f;
    mRailClip.z = 0.0f;
    mMoveSpeed = 3.0f;
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
}

void Chorobon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, nullptr, false);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        MR::initAndSetRailClipping(&mRailClip, this, 100.0f, 500.0f);
    }

    MR::getJMapInfoArg0NoInit(rIter, &mMoveSpeed);
    initNerve(GET_CHOROBON_NERVE(ChorobonNrvWait), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_CHOROBON_NERVE(ChorobonNrvFreeze), "SupportFreeze");
    makeActorAppeared();
}

void Chorobon::control() {
    mAnimScaleCtrl->updateNerve();
    if (fn_80176E60(this) && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvFreeze));
    }
}

void Chorobon::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// NONMATCHING: the SMG2 body inlines the fn_80176D90/E00 nerve checks and branches differently (~51%).
void Chorobon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && fn_80176D90(this) && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvAttack));
        return;
    }

    if (fn_80176E00(this)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Chorobon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg) && fn_80176E30(this)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvReaction));
        return true;
    }

    if (MR::isMsgPlayerHitProjectiles(msg) && fn_80176E30(this)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvKill));
        return true;
    }

    return false;
}

extern "C" bool fn_80176D90(Chorobon* pActor) {
    if (pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvWait)) || pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvMove)) ||
        pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvFreeze))) {
        return true;
    }
    return false;
}

extern "C" bool fn_80176E00(Chorobon* pActor) {
    return !pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvKill));
}

extern "C" bool fn_80176E30(Chorobon* pActor) {
    return !pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvKill));
}

extern "C" bool fn_80176E60(Chorobon* pActor) {
    if (pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvWait)) || pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvMove)) ||
        pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvAttack))) {
        return true;
    }

    if (pActor->isNerve(GET_CHOROBON_NERVE(ChorobonNrvReaction))) {
        return MR::isGreaterStep(pActor, 20);
    }

    return false;
}

void Chorobon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::isGreaterStep(this, 40)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvMove));
    }
}

void Chorobon::exeMove() {
    if (MR::isFirstStep(this)) {
        if (MR::getRailDirection(this).dot(mGravity) < 0.0f) {
            MR::startAction(this, "MoveUp");
        } else {
            MR::startAction(this, "MoveDown");
        }
    }

    if (MR::isExistRail(this)) {
        MR::moveCoordAndFollowTrans(this, mMoveSpeed);
        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
            setNerve(GET_CHOROBON_NERVE(ChorobonNrvWait));
        }
    }
}

void Chorobon::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvMove));
    }
}

void Chorobon::exeReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceHit");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_CHOROBON_NERVE(ChorobonNrvMove));
    }
}

void Chorobon::exeKill() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }

    if (MR::isActionEnd(this)) {
        fn_8022BA10(this);
        kill();
    }
}

Chorobon::~Chorobon() {
}

void NrvChorobon::ChorobonNrvFreeze::execute(Spine* pSpine) const {
    MR::updateActorStateAndNextNerve(reinterpret_cast<LiveActor*>(pSpine->mExecutor), GET_CHOROBON_NERVE(ChorobonNrvMove));
}
