#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Enemy/BakuBaku.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorParam.hpp"

// Unnamed SMG2 helpers used by this unit (declared here, defined in their own units).

namespace NrvBakuBaku {
    NEW_NERVE(BakuBakuNrvMove, BakuBaku, Move);
    NEW_NERVE(BakuBakuNrvTurn, BakuBaku, Turn);

    // Nerve Freeze: hands control to the star-pointer state, then goes to Move.
    class BakuBakuNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static BakuBakuNrvFreeze sInstance;
    };
    BakuBakuNrvFreeze BakuBakuNrvFreeze::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BakuBakuNrvAttackSuccess, BakuBaku, Success);
};  // namespace NrvBakuBaku

#define GET_BAKUBAKU_NERVE(name) (&NrvBakuBaku::name::sInstance)


// Static helpers of this unit (SMG2 fn_ names).
static void fn_80155D10(BakuBaku* pActor);
static void fn_80155DC0(BakuBaku* pActor);
static void fn_80155E60(BakuBaku* pActor);

BakuBaku::BakuBaku(const char* pName) : LiveActor(pName) {
    mPose = nullptr;
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    _9C.x = 0.0f;
    _9C.y = 0.0f;
    _9C.z = 0.0f;
    mMoveLimit = 5.0f;
    mPose = new ActorPoseTQ();
}

void BakuBaku::init(const JMapInfoIter& rIter) {
    MR::initActorPoseTQ(mPose, this);
    initNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    if (MR::isExistRail(this)) {
        MR::offBind(this);
        MR::moveCoordAndTransToNearestRailPos(this);
    }
    s32 arg = 0;
    if (MR::getRailArg0NoInit(this, &arg)) {
        mMoveLimit = (f32)arg;
    }
    MR::initActorState(this, mStarPointBind, GET_BAKUBAKU_NERVE(BakuBakuNrvFreeze), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

void BakuBaku::initAfterPlacement() {
}

void BakuBaku::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorRide(pSender)) {
        if (MR::isSensorPlayerOrYoshi(pReceiver)) {
            setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvAttackSuccess));
            return;
        }
    }
}

bool BakuBaku::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg) || MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgPlayerTrample(msg)) {
        return isAttackable();
    }

    if (MR::isMsgPlayerHipDrop(msg) && isAttackable()) {
        return true;
    }

    return false;
}

bool BakuBaku::isAttackable() const {
    if (isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove))) {
        MR::zeroVelocity(const_cast<BakuBaku*>(this));
        const_cast<BakuBaku*>(this)->setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn));
        return true;
    }
    return false;
}

void BakuBaku::control() {
    mAnimScaleCtrl->updateNerve();
    MR::fn_80010600(mPose, this, 0.05f, 140.0f);
    if (this->fn_80155F60() && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvFreeze));
    }
}

void BakuBaku::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void BakuBaku::exeTurn() {
    if (MR::isFirstStep(this)) {
        bool exist = MR::isExistRail(this);
        MR::startAction(this, "Turn");
        if (exist) {
            MR::reverseRailDirection(this);
        }
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
    }
}

void BakuBaku::exeMove() {
    if (MR::isExistRail(this)) {
        fn_80155DC0(this);
    } else {
        fn_80155E60(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn));
    }
}

void BakuBaku::exeSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
    }
}

BakuBaku::~BakuBaku() {
}

// NONMATCHING: sound-check helper approximated from the draft.
static void fn_80155D10(BakuBaku* pActor) {
    if (MR::checkPassBckFrame(pActor, 14.0f) || MR::checkPassBckFrame(pActor, 27.0f)) {
        MR::startActionSound(pActor, "Bite", -1, -1, -1);
    }
}

// NONMATCHING: rail-move helper approximated from the draft.
static void fn_80155DC0(BakuBaku* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::fn_80008890(pActor, "Move");
    }
    fn_80155D10(pActor);
    MR::moveCoordAndFollowTrans(pActor, pActor->mMoveLimit);
    if (MR::isRailReachedGoal(pActor)) {
        MR::reverseRailDirection(pActor);
    }
}

// NONMATCHING: non-rail move helper approximated from the draft.
static void fn_80155E60(BakuBaku* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::fn_80008890(pActor, "Move");
    }
    fn_80155D10(pActor);
    {
        TVec3f dir(pActor->_9C);
        MR::turnDirectionToPlayerDegree(pActor, &dir, 1.0f);
    }
    if (!MR::isBindedGround(pActor)) {
        MR::addVelocityToGravity(pActor, 1.0f);
    }
}

// NONMATCHING: Move/Turn check approximated from the draft.
bool BakuBaku::fn_80155F00() {
    return this->isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove)) || this->isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn));
}

bool BakuBaku::fn_80155F60() {
    return this->fn_80155F00() || this->isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvAttackSuccess));
}

void NrvBakuBaku::BakuBakuNrvFreeze::execute(Spine* pSpine) const {
    MR::updateActorStateAndNextNerve(reinterpret_cast<LiveActor*>(pSpine->mExecutor), GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
}
