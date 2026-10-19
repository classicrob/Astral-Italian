#pragma auto_inline off
#include "Game/Enemy/Bussun.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

#define BATCH_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Bussun*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvBussun {
    BATCH_NERVE(BussunNrvWalk, exeBussunWalk);
    NEW_NERVE(BussunNrvWait, Bussun, Wait);
    NEW_NERVE(BussunNrvTurn, Bussun, Turn);
    BATCH_NERVE(BussunNrvTurnEnd, exeBussunTurnEnd);
    NEW_NERVE(BussunNrvAttack, Bussun, Attack);
    NEW_NERVE(BussunNrvHit, Bussun, Hit);
    NEW_NERVE(BussunNrvDown, Bussun, Down);
    NEW_NERVE(BussunNrvBlow, Bussun, Blow);
    BATCH_NERVE(BussunNrvDead, exeBussunDead);
    class BussunNrvFreeze : public Nerve { public: virtual void execute(Spine* spine) const { Bussun* actor = static_cast<Bussun*>(spine->mExecutor); MR::updateActorStateAndNextNerve(actor, actor->mNextNerve); } static BussunNrvFreeze sInstance; };
    BussunNrvFreeze BussunNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBussun

#define GET_BUSSUN_NERVE(name) (&NrvBussun::name::sInstance)

const Nerve* Bussun::getBussunContactReactionNerve() const {
    if (this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) || this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn))) {
        return GET_BUSSUN_NERVE(BussunNrvHit);
    }
    return GET_BUSSUN_NERVE(BussunNrvAttack);
}

const Nerve* Bussun::getBussunDamageReactionNerve() const {
    if (this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) || this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn))) {
        return GET_BUSSUN_NERVE(BussunNrvBlow);
    }
    return GET_BUSSUN_NERVE(BussunNrvDown);
}

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

void Bussun::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
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
    if (isBussunOnMetalGround()) MR::setSeVersion(this, 1);
    else MR::setSeVersion(this, 0);
    if (canBussunBindStarPointer() && mStarPointBind->tryStartPointBind()) {
        selectBussunPointBindReturnNerve();
        setNerve(GET_BUSSUN_NERVE(BussunNrvFreeze));
    }
}

void Bussun::kill() {
    MR::tryHitReactionDead(this);
    LiveActor::kill();
}

void Bussun::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::calcAndSetBaseMtxFromFrontAndUp(this, &mFront, &mUp);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void Bussun::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshi(pReceiver) && canBussunAttackPlayer()) {
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        TVec3f dir(0.0f);
        MR::calcSensorDirection(&dir, pSender, pReceiver);
        if (up.dot(dir) < 0.0f) return;
        if (up.dot(*MR::getPlayerGravity()) >= 0.0f) return;
        if (MR::isEqualSensor(pSender, this, "Attack")) {
            if (isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (MR::sendMsgEnemyAttackOrPush(pReceiver, pSender)) setNerve(getBussunContactReactionNerve());
        }
    }
}

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
    if (!MR::isEqualSensor(pReceiver, this, "Body") && !MR::isEqualSensor(pReceiver, this, "WeakBody")) {
        return false;
    }
    if (!canBussunReceiveAttack()) {
        return false;
    }
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    f32 dot = up.dot(*MR::getPlayerGravity());
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (dot >= 0.0f) return false;
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
                setNerve(getBussunContactReactionNerve());
            }
            return false;
        }
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            setNerve(getBussunDamageReactionNerve());
            return true;
        }
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, "WeakBody")) {
        if (dot < 0.0f) return false;
        TVec3f dir(0.0f);
        MR::calcSensorDirection(&dir, pSender, pReceiver);
        if (up.dot(dir) > 50.0f) return false;
        if (MR::isMsgPlayerHipDrop(msg)) {
            setNerve(GET_BUSSUN_NERVE(BussunNrvDead));
            return true;
        }
        return false;
    }
    return false;
}

void Bussun::exeBussunWalk() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
    }
    TVec3f dir;
    calcBussunRailDirection(&dir);
    MR::moveCoordAndFollowTrans(this, 4.5f);
    MR::turnDirection(this, &mFront, dir, MR::cosDegree(2.0f));
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
        MR::startAction(this, "TurnStart");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd));
    }
}

void Bussun::exeBussunTurnEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TurnEnd");
    }
    TVec3f dir;
    calcBussunRailDirection(&dir);
    MR::turnDirection(this, &mFront, dir, MR::cosDegree(2.0f));
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
        MR::startAction(this, "Guard");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvWalk));
    }
}

void Bussun::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd));
    }
}

void Bussun::exeBussunDead() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Die");
    }
    if (MR::isActionEnd(this)) {
        TVec3f pos(mPosition);
        MR::copyJointPos(this, "Center", &pos);
        pos += mUp * 150.0f;
        MR::appearCoinPop(this, pos, 1);
        MR::startActionSound(this, "Explosion", -1, -1, -1);
        kill();
    }
}

void Bussun::calcBussunRailDirection(TVec3f* out) {
    TVec3f dir;
    MR::calcRailDirection(&dir, this);
    if (!MR::isRailGoingToEnd(this)) dir = -dir;
    out->x = dir.x;
    out->y = dir.y;
    out->z = dir.z;
}

bool Bussun::canBussunReceiveAttack() const {
    return !this->isNerve(GET_BUSSUN_NERVE(BussunNrvDead));
}

bool Bussun::canBussunAttackPlayer() const {
    return !this->isNerve(GET_BUSSUN_NERVE(BussunNrvDead));
}

bool Bussun::canBussunBindStarPointer() const {
    if (isNerve(GET_BUSSUN_NERVE(BussunNrvDead)) || isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) return false;
    return true;
}

void Bussun::selectBussunPointBindReturnNerve() {
    if (this->isNerve(GET_BUSSUN_NERVE(BussunNrvFreeze))) {
        return;
    }
    if (this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurn)) || this->isNerve(GET_BUSSUN_NERVE(BussunNrvTurnEnd)) ||
    this->isNerve(GET_BUSSUN_NERVE(BussunNrvHit)) || this->isNerve(GET_BUSSUN_NERVE(BussunNrvBlow))) {
        this->mNextNerve = GET_BUSSUN_NERVE(BussunNrvTurnEnd);
        return;
    }
    this->mNextNerve = GET_BUSSUN_NERVE(BussunNrvWalk);
}

bool Bussun::isBussunOnMetalGround() const {
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f line = -up * 100.0f;
    TVec3f pos = mPosition + up * 5.0f;
    TVec3f hit(0.0f);
    Triangle triangle;
    if (MR::getFirstPolyOnLineToMapExceptActor(&hit, &triangle, pos, line, this) &&
    MR::isEqualString(MR::getSoundCodeString(&triangle), "Metal")) return true;
    return false;
}

Bussun::~Bussun() {
}

void Bussun::exeFreeze() {
    MR::updateActorStateAndNextNerve(this, mNextNerve);
}

