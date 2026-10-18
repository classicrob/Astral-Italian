#include "Game/Enemy/Bussun.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvBussun {
    NEW_NERVE(BussunNrvWalk, Bussun, Walk);
    NEW_NERVE(BussunNrvWait, Bussun, Wait);
    NEW_NERVE(BussunNrvTurn, Bussun, Turn);
    NEW_NERVE(BussunNrvTurnEnd, Bussun, TurnEnd);
    NEW_NERVE(BussunNrvAttack, Bussun, Attack);
    NEW_NERVE(BussunNrvHit, Bussun, Hit);
    NEW_NERVE(BussunNrvDown, Bussun, Down);
    NEW_NERVE(BussunNrvBlow, Bussun, Blow);
    NEW_NERVE(BussunNrvDead, Bussun, Dead);
    NEW_NERVE(BussunNrvFreeze, Bussun, Freeze);
};  // namespace NrvBussun

#define GET_BUSSUN_NERVE(name) (&NrvBussun::name::sInstance)

// Unnamed SMG2 helpers used by Bussun.
extern "C" void fn_8005B750(LiveActor*, s32);
extern "C" void fn_8000F910(LiveActor*, TVec3f*, TVec3f*);
extern "C" bool fn_80175A70(const LiveActor*);
extern "C" const Nerve* fn_80175850(const LiveActor*);
extern "C" const Nerve* fn_801758B0(const LiveActor*);
extern "C" bool fn_80175910(const LiveActor*);
extern "C" bool fn_80175940(const LiveActor*);
extern "C" bool fn_80175970(const LiveActor*);
extern "C" void fn_801759D0(LiveActor*);
extern "C" TVec3f* fn_801757C0(TVec3f*, TVec3f*, LiveActor*);

Bussun::Bussun(const char* pName) : LiveActor(pName) {
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 0.0f;
    mUp.x = 0.0f;
    mUp.y = 0.0f;
    mUp.z = 0.0f;
    mRailPointNo = 0;
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mNextNerve = nullptr;
}

Bussun::~Bussun() {
}

void Bussun::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, nullptr, false);
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::calcRailDirection(&mFront, this);
    mRailPointNo = MR::getCurrentRailPointNo(this);
    MR::calcUpVec(&mUp, this);
    initNerve(GET_BUSSUN_NERVE(BussunNrvWalk), 1);

    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_BUSSUN_NERVE(BussunNrvFreeze), "SupportFreeze");
    MR::declareCoin(this, 1);
    makeActorAppeared();
}

void Bussun::control() {
    mAnimScaleCtrl->updateNerve();
    fn_8005B750(this, fn_80175A70(this) ? 1 : 0);
    if (fn_80175970(this) && mStarPointBind->tryStartPointBind()) {
        fn_801759D0(this);
        setNerve(GET_BUSSUN_NERVE(BussunNrvFreeze));
    }
}

void Bussun::kill() {
    MR::tryHitReactionDead(this);
    LiveActor::kill();
}

void Bussun::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    fn_8000F910(this, &mFront, &mUp);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// NONMATCHING: the SMG2 ground/dot test is simplified to the sensor and nerve checks.
void Bussun::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshi(pSender) && fn_80175940(this) && MR::isEqualSensor(pReceiver, this, "Body")) {
        if (isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
            MR::sendMsgPush(pSender, pReceiver);
            return;
        }
        if (MR::sendMsgEnemyAttackOrPush(pSender, pReceiver)) {
            setNerve(fn_80175850(this));
        }
    }
}

// NONMATCHING: hit classification simplified; the nerve choices follow the draft.
bool Bussun::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        if (!isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
            mAnimScaleCtrl->startHitReaction();
        }
        return true;
    }
    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Attack") && !MR::isEqualSensor(pReceiver, this, "WeakBody")) {
        return false;
    }
    if (!fn_80175910(this)) {
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, "Attack")) {
        if (isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
            if (MR::isMsgPlayerTrample(msg)) {
                return true;
            }
            if (MR::isMsgPlayerHipDrop(msg)) {
                MR::sendMsgAwayJump(pSender, pReceiver);
                return true;
            }
            return false;
        }
        if (MR::isMsgPlayerTrample(msg)) {
            if (MR::sendMsgEnemyAttackOrPush(pSender, pReceiver)) {
                setNerve(fn_80175850(this));
            }
            return false;
        }
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            setNerve(fn_801758B0(this));
            return true;
        }
        return false;
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvDead));
        return true;
    }
    return false;
}

void Bussun::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
    }
    TVec3f dir;
    MR::turnDirection(this, &mFront, dir, 0.0f);
    s32 railNo = MR::getCurrentRailPointNo(this);
    if (!MR::isLoopRail(this)) {
        if (MR::isRailReachedGoal(this)) {
            mRailPointNo = railNo;
            MR::reverseRailDirection(this);
            setNerve(GET_BUSSUN_NERVE(BussunNrvTurn));
            return;
        }
    }
    if (railNo != mRailPointNo) {
        mRailPointNo = railNo;
        setNerve(GET_BUSSUN_NERVE(BussunNrvWait));
    }
}

void Bussun::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    if (MR::isGreaterEqualStep(this, 0x78)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvWalk));
    }
}

void Bussun::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd));
    }
}

void Bussun::exeTurnEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TurnEnd");
    }
    TVec3f dir;
    MR::turnDirection(this, &mFront, dir, 0.0f);
    if (mFront.dot(dir) > 0.9999f) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvWalk));
    }
}

void Bussun::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvWalk));
    }
}

void Bussun::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd));
    }
}

void Bussun::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvWalk));
    }
}

void Bussun::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd));
    }
}

// Dead: copies the joint position, drops a coin and kills (fn_801756F0). NONMATCHING: coin pop arguments approximated.
void Bussun::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Dead");
    }
    if (MR::isActionEnd(this)) {
        TVec3f pos(mPosition);
        MR::appearCoinPop(this, pos, 1);
        MR::startActionSound(this, "BussunDead", -1, -1, -1);
        kill();
    }
}

void Bussun::exeFreeze() {
    MR::updateActorStateAndNextNerve(this, GET_BUSSUN_NERVE(BussunNrvWalk));
}

// Picks the hit-reaction nerve from the current state (fn_80175850).
extern "C" const Nerve* fn_80175850(const LiveActor* pActor) {
    if (pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) || pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn))) {
        return GET_BUSSUN_NERVE(BussunNrvHit);
    }
    return GET_BUSSUN_NERVE(BussunNrvAttack);
}

// Picks the blow-away nerve from the current state (fn_801758B0).
extern "C" const Nerve* fn_801758B0(const LiveActor* pActor) {
    if (pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) || pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn))) {
        return GET_BUSSUN_NERVE(BussunNrvBlow);
    }
    return GET_BUSSUN_NERVE(BussunNrvDown);
}

// True unless dead (fn_80175910 / fn_80175940).
extern "C" bool fn_80175910(const LiveActor* pActor) {
    return !pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvDead));
}

extern "C" bool fn_80175940(const LiveActor* pActor) {
    return !pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvDead));
}

// True unless dead or frozen (fn_80175970).
extern "C" bool fn_80175970(const LiveActor* pActor) {
    return !(pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvDead)) || pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze)));
}

// Stores the nerve to resume after a freeze (fn_801759D0). NONMATCHING: stores into the Bussun's next-nerve slot.
extern "C" void fn_801759D0(LiveActor* pActor) {
    Bussun* pBussun = static_cast<Bussun*>(pActor);
    if (pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
        return;
    }
    if (pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn)) || pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) ||
        pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvHit)) || pActor->isNerve(GET_BUSSUN_NERVE(BussunNrvBlow))) {
        pBussun->mNextNerve = GET_BUSSUN_NERVE(BussunNrvTurnEnd);
        return;
    }
    pBussun->mNextNerve = GET_BUSSUN_NERVE(BussunNrvWalk);
}

// Ground/rail check used by control (fn_80175A70). NONMATCHING: the SMG2 triangle probe is not reproduced.
extern "C" bool fn_80175A70(const LiveActor* pActor) {
    (void)pActor;
    return false;
}

// Rail direction toward the goal, written to pOut (fn_801757C0). NONMATCHING: arguments approximated.
extern "C" TVec3f* fn_801757C0(TVec3f* pDir, TVec3f* pOut, LiveActor* pActor) {
    (void)pDir;
    MR::calcRailDirection(pOut, pActor);
    return pOut;
}
