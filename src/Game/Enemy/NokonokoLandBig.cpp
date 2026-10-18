#include "Game/Enemy/NokonokoLandBig.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"

// String blob of the unit: action, model and sensor names are read at fixed offsets from its base.
// PLACEHOLDER(enemies): unnamed data at this address (owner unit not named); resolve when the owner unit names it.
extern char lbl_8068DE30[];

// Unnamed SMG2 helpers that are not part of this unit.

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
    TQuat4f blended;
    blended.slerp(quat, mQuat, 0.3f);
    MR::setBaseTRMtx(this, blended);
    MR::updateBaseScale(this, mAnimScaleController);
}

void NokonokoLandBig::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    bool doRest = true;
    if (MR::isEqualSensor(pSender, this, lbl_8068DE30 + 0x30)) {
        doRest = false;
        if (!MR::isSensorPlayer(pReceiver)) {
            if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvHitRecovery)) ||
                isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvBindStarPointer))) {
                MR::sendMsgPush(pReceiver, pSender);
            }
            doRest = true;
        }
    }

    if (doRest) {
        if (MR::isSensorEnemyAttack(pSender)) {
            if (MR::isSensorPlayerOrRide(pReceiver) && !this->fn_801D73E0(pSender, pReceiver)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else {
            if (pSender == getSensor(lbl_8068DE30 + 0x38)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
            if (pSender == getSensor(lbl_8068DE30 + 0x40)) {
                if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttack)) ||
                    isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttackTurn))) {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            }
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
    if (MR::isEqualSensor(pReceiver, this, lbl_8068DE30 + 0x30)) {
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, lbl_8068DE30 + 0x48)) {
        if (!MR::isMsgDrillAttack(msg)) {
            return false;
        }
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        TVec3f dir(0.0f);
        MR::fn_8000E650(&dir, this, MR::getSensorPos(pSender));
        f32 d = up.dot(dir);
        if (d < -20.0f || d > 0.0f) {
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
    if (MR::isEqualSensor(pReceiver, this, lbl_8068DE30 + 0x54)) {
        MR::tryHitReaction(this, lbl_8068DE30 + 0x5C);
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, lbl_8068DE30 + 0x74)) {
        MR::tryHitReaction(this, lbl_8068DE30 + 0x80);
        return true;
    }
    return false;
}

bool NokonokoLandBig::tryDrillFall(HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver == getSensor(lbl_8068DE30 + 0x48) && this->fn_801D7290()) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvFall));
        return true;
    }
    return false;
}

bool NokonokoLandBig::tryBodyTouch(HitSensor* pSender, HitSensor* pReceiver) {
    if (pSender == getSensor(lbl_8068DE30 + 0x40) && this->fn_801D7290()) {
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
        if (!MR::isAnyAnimPlaying(this, lbl_8068DE30 + 0xA8)) {
            startBckBtp(this, lbl_8068DE30 + 0xA8, lbl_8068DE30 + 0xB8);
        }
        MR::setRailCoordSpeed(this, 6.0f);
    }
    MR::moveCoordAndFollowTrans(this);
    TVec3f neg = -mUpVec;
    MR::makeQuatFromVec(&mQuat, MR::getRailDirection(this), neg);
    if (MR::isRailReachedGoal(this)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnStart));
    }
}

void NokonokoLandBig::exeTurnStart() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, lbl_8068DE30 + 0xC4, lbl_8068DE30 + 0xB8);
    }
    if (MR::isActionEnd(this)) {
        MR::reverseRailDirection(this);
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop));
    }
}

void NokonokoLandBig::exeTurnLoop() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, lbl_8068DE30 + 0xD0, lbl_8068DE30 + 0xB8);
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        startBckBtp(this, lbl_8068DE30 + 0xE0, lbl_8068DE30 + 0xB8);
    }
    if (isAlignedToRail()) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnEnd));
    }
}

void NokonokoLandBig::exeTurnEnd() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, lbl_8068DE30 + 0xF0);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
    }
}

void NokonokoLandBig::exeAttack() {
    if (MR::isFirstStep(this)) {
        startBckBtp(this, lbl_8068DE30 + 0x100);
    }
    if (MR::isActionEnd(this)) {
        if (isNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvAttackTurn))) {
            setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvTurnLoop));
            return;
        }
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvWalk));
    }
}

void NokonokoLandBig::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_8068DE30 + 0xF8);
    }
    if (MR::isStep(this, 45)) {
        setNerve(GET_NERVE_DIRECT(NrvNokonokoLandBig, NokonokoLandBigNrvDead));
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
    f32 x = mQuat.x;
    f32 y = mQuat.y;
    f32 z = mQuat.z;
    f32 w = mQuat.w;
    TVec3f front(2.0f * (x * z + w * y), 2.0f * (y * z - w * x), 1.0f - 2.0f * (x * x) - 2.0f * (y * y));
    TVec3f railDir = MR::getRailDirection(this);
    if (MR::isSameDirection(front, railDir, 0.01f)) {
        return front.dot(railDir) >= 0.0f;
    }
    return false;
}

bool NokonokoLandBig::fn_801D7290() {
    return this->isWalkOrBindNerve();
}

// Sensor-driven helper: a hit from an enemy-attack sensor while walking is answered with an attack push.
bool NokonokoLandBig::fn_801D73E0(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemyAttack(pSender)) {
        return false;
    }
    if (!this->isWalkNerve()) {
        return false;
    }
    return MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
}

