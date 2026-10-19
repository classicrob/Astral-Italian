#include "Game/Enemy/NokonokoLandBig.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"

// NEW_NERVE declarations, in SMG2 sbss order (lbl_807D2CE8 .. lbl_807D2D08). Execute thunks name the exe functions.
namespace NrvNokonokoLandBig {
    NEW_NERVE(NokonokoLandBigNrvWalk, NokonokoLandBig, Walk);
    class NokonokoLandBigNrvHitRecovery : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isGreaterEqualStep(actor, 5)) {
                actor->setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
            }
        };
        static NokonokoLandBigNrvHitRecovery sInstance;
    };
    NokonokoLandBigNrvHitRecovery NokonokoLandBigNrvHitRecovery::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(NokonokoLandBigNrvTurnStart, NokonokoLandBig, TurnStart);
    NEW_NERVE(NokonokoLandBigNrvTurnLoop, NokonokoLandBig, TurnLoop);
    NEW_NERVE(NokonokoLandBigNrvTurnEnd, NokonokoLandBig, TurnEnd);
    NEW_NERVE(NokonokoLandBigNrvAttack, NokonokoLandBig, Attack);
    NEW_NERVE(NokonokoLandBigNrvAttackTurn, NokonokoLandBig, Attack);
    NEW_NERVE(NokonokoLandBigNrvFall, NokonokoLandBig, Fall);
    NEW_NERVE(NokonokoLandBigNrvDead, NokonokoLandBig, Dead);

    class NokonokoLandBigNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
        };
        static NokonokoLandBigNrvBindStarPointer sInstance;
    };
    NokonokoLandBigNrvBindStarPointer NokonokoLandBigNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvNokonokoLandBig

namespace {
    void startBckBtp(LiveActor* pActor, const char* pBck, const char* pBtp = nullptr) {
        if (pBtp == nullptr) {
            pBtp = pBck;
        }

        MR::startAction(pActor, pBck);
        if (MR::isExistBtp(pActor, pBtp)) {
            MR::startBtp(pActor, pBtp);
        }
    }
}  // namespace

NokonokoLandBig::NokonokoLandBig(const char* pName) : LiveActor(pName), mArg0(0), mAnimScaleController(nullptr), mUpVec(TVec3f(0.0f, 1.0f, 0.0f)) {
    mRailCoord = 0.0f;
    mBindStarPointerState = nullptr;
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mBaseQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void NokonokoLandBig::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk), 1);
    MR::getJMapInfoArg0NoInit(rIter, &mArg0);
    MR::startBrk(this, "NokonokoLandBig");
    MR::setBrkFrameAndStop(this, (f32)mArg0);
    mAnimScaleController = new AnimScaleController(nullptr);
    mAnimScaleController->setParamTight();
    mBindStarPointerState = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mBindStarPointerState, GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer), "SupportFreeze");
    MR::moveCoordAndTransToNearestRailPos(this);
    TVec3f up = -mGravity;
    MR::makeQuatFromVec(&mQuat, MR::getRailDirection(this), up);
    mBaseQuat = mQuat;
    mRailCoord = MR::getRailCoord(this);
    MR::declareCoin(this, 1);
    MR::initUseStageSwitchReadA(this, rIter);
    if (MR::isValidSwitchA(this)) {
        MR::FunctorV0M<NokonokoLandBig*, void (NokonokoLandBig::*)()> callback(this, &NokonokoLandBig::updateRailPoseOnSwitch);
        MR::listenStageSwitchOnA(this, callback);
    }
    MR::useStageSwitchSyncAppear(this, rIter);
}

void NokonokoLandBig::appear() {
    LiveActor::appear();
}

void NokonokoLandBig::kill() {
    MR::tryHitReactionDead(this);
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    LiveActor::kill();
}

void NokonokoLandBig::control() {
    mAnimScaleController->updateNerve();
    if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk)) && mBindStarPointerState->tryStartPointBind()) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer));
    }
}

void NokonokoLandBig::calcAndSetBaseMtx() {
    TPos3f baseMtx;
    baseMtx.set(getBaseMtx());
    TQuat4f quat;
    baseMtx.getQuat(quat);
    quat.slerp(mQuat, 0.3f);
    MR::setBaseTRMtx(this, quat);
    MR::updateBaseScale(this, mAnimScaleController);
}

// NONMATCHING: MWCC merges an extra return branch after sendMsgPush.
void NokonokoLandBig::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Push")) {
        if (MR::isSensorPlayer(pReceiver)) {
            return;
        }
        if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvHitRecovery)) ||
            isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver) && !tryPushEnemyAttack(pSender, pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
        return;
    }
    if (pSender == getSensor("Neck")) {
        MR::sendMsgPush(pReceiver, pSender);
    }
    if (pSender == getSensor("Body")) {
        bool attack = false;
        if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttack)) ||
            isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttackTurn))) {
            attack = true;
        }
        if (attack) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool NokonokoLandBig::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isWalkNerve()) {
        return false;
    }
    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender) || MR::isSensorRide(pSender)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvHitRecovery));
        return true;
    }
    return false;
}

bool NokonokoLandBig::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Push")) {
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, "DrillDamage")) {
        if (!MR::isMsgDrillAttack(msg)) {
            return false;
        }
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        TVec3f dir(0.0f);
        MR::fn_8000E650(&dir, this, MR::getSensorPos(pSender));
        f32 d = up.dot(dir);
        f32 range = 20.0f;
        if (d < -range || d > 0.0f) {
            return false;
        }
        TVec3f playerUp(0.0f);
        MR::getPlayerUpVec(&playerUp);
        if (up.dot(playerUp) > 0.0f) {
            return false;
        }
        return tryDrillFall(pSender, pReceiver);
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return tryBodyTouch(pSender, pReceiver);
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }
    return MR::isMsgStarPieceReflect(msg);
}

bool NokonokoLandBig::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgRushEnd(msg)) {
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, "attack")) {
        MR::tryHitReaction(this, "ドリルキャンセル[顔]");
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "BodyAttack")) {
        MR::tryHitReaction(this, "ドリルキャンセル[甲羅]");
        return true;
    }
    return false;
}

// NONMATCHING: Remaining GPR assignment and vector temporary scheduling differ.
bool NokonokoLandBig::tryDrillFall(HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver == getSensor("DrillDamage") && isMovingOrBindNerve()) {
        MR::tryHitReaction(this, "ドリルダメージ");
        const TVec3f* hostVelocity = &MR::getSensorHost(pSender)->mVelocity;
        TVec3f velocity(*hostVelocity);
        if (!MR::isNearZero(velocity, 0.001f)) {
            mVelocity.set(velocity);
        } else {
            const TVec3f* senderPos = MR::getSensorPos(pSender);
            TVec3f delta(*MR::getSensorPos(pReceiver));
            delta.sub(*senderPos);
            mVelocity.set(delta);
        }
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvFall));
        return true;
    }
    return false;
}

bool NokonokoLandBig::tryBodyTouch(HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver == getSensor("Body") && this->isMovingOrBindNerve()) {
        return true;
    }
    return false;
}

bool NokonokoLandBig::isWalkOrBindNerve() const {
    return isWalkNerve() || isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer));
}

bool NokonokoLandBig::isWalkNerve() const {
    return isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk)) ||
           isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvHitRecovery)) ||
           isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnStart)) ||
           isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop)) ||
           isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnEnd));
}

void NokonokoLandBig::exeWalk() {
    if (MR::isFirstStep(this)) {
        if (!MR::isAnyAnimPlaying(this, "WalkFastWait")) {
            startBckBtp(this, "WalkFastWait", "WalkWait");
        }
        MR::setRailCoordSpeed(this, 6.0f);
    }
    MR::moveCoordAndFollowTrans(this);
    TVec3f neg = -mGravity;
    MR::makeQuatFromVec(&mQuat, MR::getRailDirection(this), neg);
    if (MR::isRailReachedGoal(this)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnStart));
    }
}

void NokonokoLandBig::exeTurnStart() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, "TurnStart", "WalkWait");
    }
    if (MR::isActionEnd(this)) {
        MR::reverseRailDirection(this);
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop));
    }
}

// NONMATCHING: The three matrix-column loads are scheduled in a different order.
void NokonokoLandBig::exeTurnLoop() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, "TurnLoopStart", "WalkWait");
        TPos3f mtx;
        mtx.set(getBaseMtx());
        mUpVec.set<f32>(mtx.mMtx[0][1], mtx.mMtx[1][1], mtx.mMtx[2][1]);
        MR::normalize(&mUpVec);
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        startBckBtp(this, "TurnLoop", "WalkWait");
    }
    s32 turnSteps = 139;
    TQuat4f rotation;
    rotation.setRotate(mUpVec, -(3.1415927f / turnSteps));
    PSQUATMultiply(&rotation, &mQuat, &mQuat);
    if (isAlignedToRail()) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnEnd));
    }
}

void NokonokoLandBig::exeTurnEnd() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, "TurnEnd", "WalkWait");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
    }
}

// NONMATCHING: Remaining vector temporary stack slots and load scheduling differ.
void NokonokoLandBig::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rotate");
        TPos3f mtx;
        mtx.set(getBaseMtx());
        TVec3f front;
        front.set<f32>(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
        front = -front;
        MR::normalize(&front);
        TVec3f up = -mGravity;
        TVec3f lift(up);
        lift.scale(45.0f);
        TVec3f forward(front);
        forward.scale(5.0f);
        TVec3f velocity(forward);
        velocity.add(lift);
        mVelocity.set(velocity);
        MR::makeQuatFromVec(&mQuat, -front, -mGravity);
    }
    TVec3f acceleration(mGravity);
    acceleration.scale(1.2f);
    mVelocity.add(acceleration);
    mVelocity.scale(0.99f);
    if (MR::isStep(this, 45)) {
        mVelocity.z = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.x = 0.0f;
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvDead));
    }
}

void NokonokoLandBig::exeAttack() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, "Attack");
    }
    if (MR::isActionEnd(this)) {
        if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttackTurn))) {
            setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop));
            return;
        }
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
    }
}

void NokonokoLandBig::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        if (mArg0 != 1) {
            MR::appearCoinPop(this, mPosition, 1);
        }
        kill();
    }
}

bool NokonokoLandBig::isAlignedToRail() const {
    TVec3f front;
    mQuat.getZDir(front);
    TVec3f railDir = MR::getRailDirection(this);
    if (MR::isSameDirection(front, railDir, 0.01f)) {
        return 0.0f <= front.dot(railDir);
    }
    return false;
}

bool NokonokoLandBig::isMovingOrBindNerve() {
    bool result = false;
    if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk)) ||
        isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvHitRecovery)) ||
        isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnStart)) ||
        isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop)) ||
        isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnEnd)) ||
        isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer))) {
        result = true;
    }
    return result;
}

// Sensor-driven helper: a hit from an enemy-attack sensor while walking is answered with an attack push.
#pragma push
#pragma global_optimizer off
bool NokonokoLandBig::tryPushEnemyAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemyAttack(pSender)) {
        return false;
    }
    if (!isWalkNerve()) {
        return false;
    }
    return MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
}
#pragma pop

void NokonokoLandBig::updateRailPoseOnSwitch() {
    MR::setBaseTRMtx(this, mBaseQuat);
    MR::setRailCoord(this, mRailCoord);
}

NokonokoLandBig::~NokonokoLandBig() {
}
