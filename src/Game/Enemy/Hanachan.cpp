#include "Game/Enemy/Hanachan.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const f32 hFarDistToPursuePlayer = 2000.0f;
    // hPursuePlayerTurnLimitRadian
    // hSearchTurnLimitRadian
    const f32 hCommonGravity = 0.5f;
    const f32 hBlowGravity = 1.0f;
    const f32 hOverturnGravity = 2.0f;
    const f32 hOverturnBoundGravity = 1.0f;
    const f32 hRecoverGravity = 1.0f;
    const f32 hWallHitEndGravity = 2.0f;
    // hGroundDamp
    // hAirDampH
    // hAirDampV
    // hUpdateUpRate
    // hUpdateFrontRate
    // hInfectionDelay
    const f32 hTrampleJumpVelV = 5.0f;
    const f32 hBecomeAngryJumpVelV = 5.0f;
    const f32 hOverturnJumpVelV = 25.0f;
    const f32 hOverturnBoundJumpVelV = 15.0f;
    const f32 hRecoverJumpVelV = 17.0f;
    const f32 hWallHitEndJumpVelV = 20.0f;
    // hToRailDiv
    const s32 hAngryPursueTime = 300;
    // hAngryEndTime
    // hOverturnRate
    // hRecoverRate
    const s32 hOverturnTime = 180;
    const f32 hWalkSpeed = 4.0f;
    const f32 hAngrySpeed = 13.0f;
    // hPartsToRailMul
    const s32 hWalkStep = 8;
    const f32 hWalkAnimRate = 1.0f;
    const f32 hAngryAnimRate = 3.0f;
    const s32 hBecomeAngryInterval = 10;
    const s32 hPunchStopSceneTime = 4;
    const s32 hSpinInterval = 3;
    const s32 hOverturnCanTrampleDeathTime = 10;
    const s32 hRecoverCanTrampleDeathTime = 60;
    const s32 hRecoverInit = 67;
    const s32 hRecoverInterval = 3;
    const s32 hRecoverAnimBegin = 55;
    const s32 hHipDroppedTime = 30;
    const f32 hNearHipDropReactionDist = 1000.0f;
    const s32 hOverturnBoundInterval = 2;
    const s32 hWallHitInterval = 3;
    const f32 hHitWallCheck = -0.9f;
    const f32 hInitBlowRadian = 0.2f;
    const s32 hBlowTime = 30;
    const f32 hBlowVelH = 30.0f;
    const f32 hBlowVelV = 20.0f;
    const s32 hBindMinStep = 10;
    const f32 hPushRate = 0.25f;
    const f32 hCollideRange = 2.0f;
    const s32 hRecoverSteamSeTime = 60;
    const s32 hRecoverGetupSeTime = 63;
    const s32 hOverturnBoundSeTime = 29;
    const f32 hAngryEndJumpVelV = 0.0f;
    // hToPrevVelMul
    // hPushPartsVel
    const s32 hTrampleInit = 0;
    const s32 hTrampleInterval = 0;
    const s32 hTrampleAnimBegin = 0;
    const s32 hSpinInit = 0;
    const s32 hOverturnAnimBegin = 0;
    const s32 hHipDroppedInit = 0;
    const s32 hHipDroppedInterval = 0;
    const s32 hOverturnBoundInit = 0;
    const s32 hOverturnBoundAnimBegin = 0;
};  // namespace

namespace NrvHanachan {
    NEW_NERVE(HanachanPartsNrvWalk, HanachanParts, Walk);
    NEW_NERVE_ONEND(HanachanPartsNrvBecomeAngry, HanachanParts, BecomeAngry, BecomeAngry);
    NEW_NERVE_ONEND(HanachanPartsNrvAngryPursue, HanachanParts, AngryPursue, AngryPursue);
    NEW_NERVE(HanachanPartsNrvAngryEnd, HanachanParts, AngryEnd);
    NEW_NERVE(HanachanPartsNrvWallHitEnd, HanachanParts, WallHitEnd);
    NEW_NERVE(HanachanPartsNrvTrample, HanachanParts, Trample);
    NEW_NERVE(HanachanPartsNrvOverturn, HanachanParts, Overturn);
    NEW_NERVE(HanachanPartsNrvOverturnBound, HanachanParts, OverturnBound);
    NEW_NERVE(HanachanPartsNrvOverturnWait, HanachanParts, OverturnWait);
    NEW_NERVE_ONEND(HanachanPartsNrvRecover, HanachanParts, Recover, Recover);
    NEW_NERVE(HanachanPartsNrvHipDropped, HanachanParts, HipDropped);
    NEW_NERVE(HanachanPartsNrvOverturnHipDropped, HanachanParts, HipDropped);
    NEW_NERVE(HanachanPartsNrvBlow, HanachanParts, Blow);
    NEW_NERVE_ONEND(HanachanPartsNrvStarPointerBind, HanachanParts, StarPointerBind, StarPointerBind);

    class HanachanNrv_801922D0 : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static HanachanNrv_801922D0 sInstance;
    };
    HanachanNrv_801922D0 HanachanNrv_801922D0::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(HanachanNrvHanachanWalk, Hanachan, Walk);
    NEW_NERVE(HanachanNrvHanachanBecomeAngry, Hanachan, BecomeAngry);
    NEW_NERVE(HanachanNrvHanachanAngryPursue, Hanachan, AngryPursue);
    NEW_NERVE(HanachanNrvHanachanAngryEnd, Hanachan, AngryEnd);
    NEW_NERVE(HanachanNrvHanachanWallHitEnd, Hanachan, WallHitEnd);
    NEW_NERVE(HanachanNrvHanachanTrample, Hanachan, Trample);
    NEW_NERVE(HanachanNrvHanachanOverturn, Hanachan, Overturn);
    NEW_NERVE(HanachanNrvHanachanOverturnBound, Hanachan, OverturnBound);
    NEW_NERVE(HanachanNrvHanachanOverturnWait, Hanachan, OverturnWait);
    NEW_NERVE(HanachanNrvHanachanRecover, Hanachan, Recover);
    NEW_NERVE(HanachanNrvHanachanHipDropped, Hanachan, HipDropped);
    NEW_NERVE(HanachanNrvHanachanOverturnHipDropped, Hanachan, HipDropped);
    NEW_NERVE(HanachanNrvHanachanBlow, Hanachan, Blow);
    NEW_NERVE_ONEND(HanachanNrvHanachanStarPointerBind, Hanachan, StarPointerBind, StarPointerBind);
    NEW_NERVE(HanachanNrvHanachanStarPointerBindEnd, Hanachan, WallHitEnd);
    NEW_NERVE_ONEND(HanachanNrvHanachanStarPointerBindOverturn, Hanachan, StarPointerBind, StarPointerBind);
    NEW_NERVE(HanachanNrvHanachanStarPointerBindEndOverturn, Hanachan, WallHitEnd);
};  // namespace NrvHanachan

HanachanParts::HanachanParts(Hanachan* pHost, s32 partsIndex, const char* pName)
    : LiveActor(pName), mHost(pHost), mPushVelocity(0.0f, 0.0f, 0.0f), mFallVelocity(0.0f, 0.0f, 0.0f), mPartsType(PartsType_Body),
      mRotationQuat(0.0f, 0.0f, 0.0f, 1.0f), mPartsIndex(partsIndex), mActionStartStep(-1), mIsLanded() {
}

void HanachanParts::init(const JMapInfoIter& rIter) {
    const char* pName;
    if (mPartsIndex == 0) {
        mPartsType = PartsType_Head;
        pName = "HanachanHead";
    } else if (mPartsIndex % 2 == 1) {
        mPartsType = PartsType_BodyS;
        pName = "HanachanBodyS";
    } else {
        mPartsType = PartsType_Body;
        pName = "HanachanBody";
    }
    MR::initActor(this, pName, false);

    MR::addEffect(this, "PointerTouch");
    initNerve(GET_NERVE(Hanachan, HanachanPartsNrvWalk));
    MR::startBrk(this, "Normal");

    if (mPartsType == PartsType_Head) {
        MR::startBva(this, "normal");
    }
}

// NONMATCHING: host/iterator register allocation.
namespace {
    inline void killHanachanParts(HanachanParts** end, HanachanParts** first) {
        while (first < end) {
            (*first)->kill();
            ++first;
        }
    }
}

void Hanachan::killAllHanachanParts() {
    this->kill();

    killHanachanParts(&mBodyParts[5], &mBodyParts[0]);
}

void HanachanParts::control() {
    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;

    if (MR::isInDeath(this, zero) || MR::isBindedGroundDamageFire(this) || MR::isInWater(mPosition)) {
        mHost->killAllHanachanParts();
    }
}

bool HanachanParts::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInhaleBlackHole(msg)) {
        mHost->killAllHanachanParts();
        return true;
    }

    return false;
}

void HanachanParts::kill() {
    LiveActor::kill();

    MR::emitEffect(this, mPartsType == PartsType_Head ? "HeadDeath" : "BodyDeath");

    if (mPartsType == PartsType_Body || mPartsType == PartsType_Head) {
        MR::appearStarPiece(mHost, mPosition, 2, 10.0f, 40.0f, false);
    }
}

const TVec3f* HanachanParts::getCommonGravity() const {
    if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanTrample)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanBecomeAngry)) ||
        mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound)) ||
        mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanRecover))) {
        return &mHost->mBodyParts[2]->mGravity;
    }

    return &mGravity;
}

bool Hanachan::isOwnSensor(HitSensor* pSensor) {
    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        if ((*current)->getSensor("Body") == pSensor) {
            return true;
        }
    }

    return false;
}

void HanachanParts::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (!MR::isPlayerDamaging()) {
            if (isNerve(GET_NERVE(Hanachan, HanachanPartsNrvAngryPursue)) && MR::isOnGroundPlayer()) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                    mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd));
                    return;
                }

                MR::sendMsgPush(pReceiver, pSender);
            } else if (isNerve(GET_NERVE(Hanachan, HanachanPartsNrvWalk)) && MR::isOnGroundPlayer()) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    return;
                }

                MR::sendMsgPush(pReceiver, pSender);
            } else if (!mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanHipDropped)) &&
                       !mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnHipDropped))) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        if (!mHost->isOwnSensor(pReceiver) &&
            (isNerve(GET_NERVE(Hanachan, HanachanPartsNrvBecomeAngry)) || isNerve(GET_NERVE(Hanachan, HanachanPartsNrvAngryPursue))) &&
            MR::sendMsgToEnemyAttackBlow(pReceiver, pSender) && mPartsType == PartsType_Head) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd));
            return;
        }

        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool HanachanParts::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInvincibleAttack(msg)) {
        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanBlow)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanHipDropped))) {
            return false;
        }

        mHost->setNerveBlow(*MR::getSensorPos(pSender));
        return true;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk))) {
            return false;
        }

        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait))) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound));
            mHost->mAttackPos = *MR::getSensorPos(pSender);
            return true;
        }

        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk))) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEnd));
            mHost->mAttackPos = *MR::getSensorPos(pSender);
            return true;
        }

        return false;
    }

    if (isNerve(GET_NERVE(Hanachan, HanachanPartsNrvHipDropped)) || isNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnHipDropped))) {
        return false;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        if (!mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanHipDropped)) &&
            !mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnHipDropped))) {
            if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn)) ||
                mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound)) ||
                mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait)) ||
                mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanRecover))) {
                mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnHipDropped));
            } else {
                mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanHipDropped));
            }

            mHost->mAttackPos = *MR::getSensorPos(pSender);
            return true;
        }

        return false;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        MR::startActionSound(mHost, "EmHanachanTrample");

        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryPursue)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryEnd)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd)) || mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanTrample)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanBecomeAngry))) {
            return true;
        }

        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait)) ||
            (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn)) && MR::isGreaterStep(mHost, ::hOverturnCanTrampleDeathTime)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound)) ||
            (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanRecover)) && MR::isLessStep(mHost, ::hRecoverCanTrampleDeathTime)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindOverturn)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEndOverturn))) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnHipDropped));
            mHost->mAttackPos = *MR::getSensorPos(pSender);
            return true;
        }

        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk))) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanTrample));
            mHost->mAttackPos = *MR::getSensorPos(pSender);
            return true;
        }

        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn)) ||
            mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound))) {
            return false;
        }

        MR::stopScene(::hPunchStopSceneTime);

        if (mHost->isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait))) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound));
        } else {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn));
        }

        mHost->mAttackPos = *MR::getSensorPos(pSender);
        return true;
    }

    return false;
}

bool HanachanParts::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return true;
}

bool HanachanParts::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pSender)) {
        return false;
    }

    TVec3f dir(*MR::getSensorPos(pReceiver));
    dir.sub(*MR::getSensorPos(pSender));

    f32 depth;

    if (!mHost->isOwnSensor(pSender)) {
        depth = (MR::getSensorRadius(pReceiver) + MR::getSensorRadius(pSender)) - dir.length();
    } else {
        depth = (MR::getSensorRadius(pReceiver) / 2.0f + MR::getSensorRadius(pSender) / 2.0f) - dir.length();

        if (depth < 0.0f) {
            return false;
        }
    }

    dir.setLength(::hPushRate * depth);
    mPushVelocity.add(dir);
    return true;
}

void HanachanParts::calcAndSetBaseMtx() {
    TQuat4f rot;
    rot = mRotationQuat;

    TPos3f mtx;
    mtx.makeQuat(rot);
    mtx.setTrans(mPosition);

    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mHost->mScaleController);
}

void HanachanParts::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WalkNormal");
        s32 frameMax = MR::getBckFrameMax(this, "Walk") - 1;
        f32 unused = (frameMax + ::hWalkStep * mPartsIndex) % frameMax;
        MR::setBckFrameAndStop(this, ::hWalkStep * mPartsIndex);
        MR::setBckRate(this, ::hWalkAnimRate);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hCommonGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeTrample() {
    if (MR::isFirstStep(this)) {
        mIsLanded = false;
    }

    if (MR::isStep(this, mActionStartStep + ::hTrampleAnimBegin)) {
        MR::startAction(this, "AngryStart");
    }

    if (MR::isStep(this, mActionStartStep)) {
        mFallVelocity.set(*getCommonGravity() * -::hTrampleJumpVelV);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        if (!mIsLanded && MR::isGreaterStep(this, mActionStartStep)) {
            mIsLanded = true;
            changeHeadAngry();
            MR::startAction(this, "AngryDash");

            s32 frameMax = MR::getBckFrameMax(this, "Walk") - 1;
            f32 unused = (frameMax + mPartsIndex * ::hWalkStep) % frameMax;

            MR::setBckFrameAndStop(this, mPartsIndex * ::hWalkStep);
            MR::setBckRate(this, ::hAngryAnimRate);
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hCommonGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeBecomeAngry() {
    if (MR::isFirstStep(this)) {
        mFallVelocity.set(*getCommonGravity() * -::hBecomeAngryJumpVelV);
        mActionStartStep = 0;
        mIsLanded = false;
        MR::startBrk(this, "Anger");
        MR::invalidateClipping(mHost);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        if (!mIsLanded && MR::isGreaterStep(this, mActionStartStep)) {
            mIsLanded = true;
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hCommonGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::endBecomeAngry() {
    changeHeadCalmDown();
}

void HanachanParts::exeAngryPursue() {
    if (MR::isFirstStep(this)) {
        changeHeadAngry();
    }

    bool isWallHit = isHeadHitWall() && MR::isBindedWallOfMap(this);

    if (isWallHit) {
        mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd));
    } else {
        bool isMoveLimitHit = isHeadHitWall() && MR::isBindedWallOfMoveLimit(this);

        if (isMoveLimitHit) {
            mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryEnd));
        } else {
            mVelocity.set(mFallVelocity);
            mVelocity.add(mPushVelocity);

            if (MR::isOnGround(this)) {
                mFallVelocity.zero();
            } else {
                mFallVelocity.add(mGravity * ::hCommonGravity);
                mFallVelocity.mult(0.98f);
            }
        }
    }
}

void HanachanParts::endAngryPursue() {
    changeHeadCalmDown();
}

void HanachanParts::exeAngryEnd() {
    if (MR::isFirstStep(this)) {
        mIsLanded = false;
    }

    if (MR::isStep(this, mActionStartStep)) {
        MR::startAction(this, "AngryEnd");
        MR::startBrk(this, "Normal");
        MR::validateClipping(mHost);
    }

    if (MR::isStep(this, mActionStartStep)) {
        mFallVelocity.set(*getCommonGravity() * -::hAngryEndJumpVelV);
    }

    bool isWallHit = isHeadHitWall() && MR::isBindedWallOfMap(this);

    if (isWallHit) {
        mHost->setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd));
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        if (!mIsLanded && MR::isGreaterStep(this, mActionStartStep)) {
            mIsLanded = true;
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hCommonGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeWallHitEnd() {
    if (MR::isFirstStep(this)) {
        mIsLanded = false;
    }

    if (MR::isStep(this, mActionStartStep)) {
        MR::startBrk(this, "Normal");
        MR::validateClipping(mHost);
        mFallVelocity.set(*getCommonGravity() * -::hWallHitEndJumpVelV);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        if (!mIsLanded && MR::isGreaterStep(this, mActionStartStep)) {
            mIsLanded = true;
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hWallHitEndGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeOverturn() {
    if (MR::isStep(this, mActionStartStep + ::hOverturnAnimBegin)) {
        MR::startAction(this, "Overturn");
        MR::startBrk(this, "Normal");
        MR::validateClipping(mHost);
        changeHeadCalmDown();
    }

    if (MR::isStep(this, mActionStartStep)) {
        mFallVelocity.set(*getCommonGravity() * -::hOverturnJumpVelV);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        mFallVelocity.zero();

        if (MR::isGreaterStep(this, mActionStartStep)) {
            setNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait));
        }
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hOverturnGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeOverturnBound() {
    if (MR::isStep(this, mActionStartStep + ::hOverturnBoundAnimBegin)) {
        MR::startAction(this, "OverturnBound");
    }

    if (MR::isStep(this, mActionStartStep)) {
        mFallVelocity.set(*getCommonGravity() * -::hOverturnBoundJumpVelV);
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        mFallVelocity.zero();

        if (MR::isGreaterStep(this, mActionStartStep)) {
            setNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait));
        }
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hOverturnBoundGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeOverturnWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "OverturnLand");
        mIsLanded = false;
    }

    if (!mIsLanded && MR::isActionEnd(this)) {
        MR::startAction(this, "OverturnWait");
        s16 frameMax = MR::getBckFrameMax(this, "OverturnWait");
        s32 frame = mPartsIndex * ::hWalkStep;
        f32 unused = ((frameMax - 1) + frame) % (frameMax - 1);
        MR::setBckFrameAndStop(this, frame);
        MR::setBckRate(this, ::hWalkAnimRate);
        mIsLanded = true;
    }

    mVelocity.zero();
    mVelocity.add(mPushVelocity);
}

void HanachanParts::exeRecover() {
    if (MR::isFirstStep(this)) {
        mIsLanded = false;
    }

    if (MR::isStep(this, mActionStartStep - ::hRecoverAnimBegin)) {
        MR::startBrk(this, "Anger");
        MR::invalidateClipping(mHost);
        MR::startAction(this, "Recover");
    }

    if (MR::isStep(this, mActionStartStep)) {
        mFallVelocity.set(*getCommonGravity() * -::hRecoverJumpVelV);
    }

    if (MR::isGreaterStep(this, mActionStartStep - 50) && mPartsType == PartsType_Head) {
        MR::startBva(this, "Anger");
    }

    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isOnGround(this)) {
        if (mActionStartStep == 0) {
            if (MR::isBckOneTimeAndStopped(this)) {
                mIsLanded = true;
                MR::startAction(this, "AngryDash");
                MR::setBckRate(this, ::hAngryAnimRate);
            }
        } else if (MR::isGreaterStep(this, mActionStartStep)) {
            MR::startAction(this, "RecoverLand");
            mActionStartStep = 0;
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * ::hRecoverGravity);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::endRecover() {
    changeHeadCalmDown();
}

void HanachanParts::exeHipDropped() {
    mVelocity.set(mFallVelocity);
    mVelocity.add(mPushVelocity);

    if (MR::isFirstStep(this)) {
        MR::forceBindOnGround(this, 0.0f, 1.0f);
        mIsLanded = false;
    }

    if (MR::isStep(this, mActionStartStep)) {
        mHost->calcNearestInfectionId();
        bool isOverturn = isNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnHipDropped));
        MR::startAction(this, isOverturn ? "OverturnPress" : "Press");
    }

    if (MR::isOnGround(this)) {
        if (MR::isGreaterStep(this, mActionStartStep)) {
            if (mPartsIndex == mHost->calcNearestInfectionId()) {
                MR::isActionEnd(this);
                mIsLanded = true;
            } else {
                mIsLanded = true;
            }
        }

        mFallVelocity.zero();
    } else {
        mFallVelocity.add(*getCommonGravity() * 2.0f);
        mFallVelocity.mult(0.98f);
    }
}

void HanachanParts::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rotate");
        mVelocity.set(mPushVelocity);

        if (mPartsType == PartsType_Head) {
            MR::invalidateExCollisionParts(this);
        }
    }

    mVelocity.add(mGravity * ::hBlowGravity);

    if ((MR::isGreaterStep(this, 15) && MR::isBinded(this)) || MR::isGreaterStep(this, ::hBlowTime)) {
        kill();
    }
}

void HanachanParts::exeStarPointerBind() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        MR::stopBck(this);
        mVelocity.zero();
        MR::emitEffect(this, "PointerTouch");
    }

    MR::startDPDFreezeLevelSound(this);
}

void HanachanParts::endStarPointerBind() {
    MR::deleteEffect(this, "PointerTouch");
    MR::setBckRate(this, 1.0f);
}

bool HanachanParts::isHeadHitWall() {
    if (mPartsType == PartsType_Head && MR::isBindedWall(this)) {
        TVec3f wallNormal = MR::getWallNormal(this);

        if (wallNormal.dot(mHost->mFrontDir) <= ::hHitWallCheck) {
            return true;
        }
    }

    return false;
}

bool HanachanParts::isLandedInNerve(const Nerve* pNerve) {
    return isNerve(pNerve) && mIsLanded;
}

bool HanachanParts::isHipDroppedLanded() {
    return (isNerve(GET_NERVE(Hanachan, HanachanPartsNrvHipDropped)) || isNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnHipDropped))) &&
           mIsLanded;
}

void HanachanParts::changeHeadAngry() {
    if (mPartsType == PartsType_Head) {
        MR::startBva(this, "Anger");
        MR::emitEffect(this, "AngrySmoke");
    }
}

void HanachanParts::changeHeadCalmDown() {
    if (mPartsType == PartsType_Head) {
        MR::startBva(this, "Normal");
        MR::deleteEffect(this, "AngrySmoke");
    }
}

Hanachan::Hanachan(const char* pName) : LiveActor(pName), mAttackPos(0, 0, 0), mFrontDir(0, 0, 1), mIsChasePlayer(true), mScaleController() {
}

void Hanachan::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::isConnectedWithRail(rIter);
    }

    MR::initDefaultPos(this, rIter);
    initRailRider(rIter);

    s32 arg0 = 0;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);

    if (arg0 == 1) {
        mIsChasePlayer = false;
    } else {
        mIsChasePlayer = true;
    }

    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::connectToSceneEnemyMovement(this);
    initNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk));
    initSound(6, "Hanachan", nullptr, TVec3f(0.0f));
    MR::validateClipping(this);
    mScaleController = new AnimScaleController(nullptr);
    MR::declareStarPiece(this, 6);
    MR::setGroupClipping(this, rIter, 16);

    mBodyParts[0] = new HanachanParts(this, 0, "頭パーツ");
    mBodyParts[1] = new HanachanParts(this, 1, "体パーツ");
    mBodyParts[2] = new HanachanParts(this, 2, "体パーツ");
    mBodyParts[3] = new HanachanParts(this, 3, "体パーツ");
    mBodyParts[4] = new HanachanParts(this, 4, "体パーツ");

    mBodyParts[0]->initWithoutIter();
    mBodyParts[1]->initWithoutIter();
    mBodyParts[2]->initWithoutIter();
    mBodyParts[3]->initWithoutIter();
    mBodyParts[4]->initWithoutIter();

    MR::moveCoordToNearestPos(this, mPosition);
    MR::moveTransToOtherActorRailPos(mBodyParts[0], this);
    MR::reverseRailDirection(this);

    MR::moveCoord(this, MR::getSensorRadius(mBodyParts[0], "Body") / 2.0f + MR::getSensorRadius(mBodyParts[1], "Body") / 2.0f);
    MR::moveTransToOtherActorRailPos(mBodyParts[1], this);
    MR::moveCoord(this, MR::getSensorRadius(mBodyParts[1], "Body") / 2.0f + MR::getSensorRadius(mBodyParts[2], "Body") / 2.0f);
    MR::moveTransToOtherActorRailPos(mBodyParts[2], this);
    MR::moveCoord(this, MR::getSensorRadius(mBodyParts[2], "Body") / 2.0f + MR::getSensorRadius(mBodyParts[3], "Body") / 2.0f);
    MR::moveTransToOtherActorRailPos(mBodyParts[3], this);
    MR::moveCoord(this, MR::getSensorRadius(mBodyParts[3], "Body") / 2.0f + MR::getSensorRadius(mBodyParts[4], "Body") / 2.0f);
    MR::moveTransToOtherActorRailPos(mBodyParts[4], this);
    MR::reverseRailDirection(this);

    mBodyParts[0]->appear();
    mBodyParts[1]->appear();
    mBodyParts[2]->appear();
    mBodyParts[3]->appear();
    mBodyParts[4]->appear();

    MR::useStageSwitchAwake(this, rIter);
    appear();
}

void Hanachan::initAfterPlacement() {
    MR::trySetMoveLimitCollision(mBodyParts[0]);
}


void Hanachan::makeActorDead() {
    LiveActor::makeActorDead();

    for (u32 i = 0; i < 5; i++) {
        mBodyParts[i]->makeActorDead();
    }
}

void Hanachan::makeActorAppeared() {
    if (!isNerve(&NrvHanachan::HanachanNrv_801922D0::sInstance)) {
        LiveActor::makeActorAppeared();

        for (u32 i = 0; i < 5; i++) {
            mBodyParts[i]->makeActorAppeared();
        }
    }
}

void Hanachan::kill() {
    MR::startActionSound(this, "EmExplodeS");
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    setNerve(&NrvHanachan::HanachanNrv_801922D0::sInstance);
}

void Hanachan::startClipped() {
    LiveActor::startClipped();

    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        (*current)->startClipped();
    }
}

void Hanachan::endClipped() {
    LiveActor::endClipped();

    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        (*current)->endClipped();
    }
}

void Hanachan::control() {
    mScaleController->updateNerve();

    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        if (!MR::isDead(*current)) {
            (*current)->movement();
            (*current)->mPushVelocity.zero();
        }
    }

    if ((isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk)) || isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryPursue)) ||
         isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait))) &&
        isStarPointerPointing()) {
        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait))) {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindOverturn));
        } else {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBind));
        }
    }

    MR::setPosition(this, mBodyParts[2]->mPosition);

    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk))) {
            MR::validateClipping(this);
        } else {
            MR::invalidateClipping(this);
        }
    }
}

void Hanachan::exeWalk() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvWalk));
    }

    moveHeadAlongRail(::hWalkSpeed);
    moveBodyAlongHead();
    applyPlayerHipDropReaction();
}

void Hanachan::exeTrample() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvTrample));
        setDelayAllPartsAtId(calcNearestInfectionId(), ::hTrampleInit, ::hTrampleInterval);
        MR::startActionSound(this, "EvHanachanTrample");
    }

    moveBodyAlongHead();

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvTrample)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvTrample))) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanBecomeAngry));
    }
}

void Hanachan::exeBecomeAngry() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvBecomeAngry));
        setDelayAllPartsAtId(0, 0, ::hBecomeAngryInterval);
    }

    MR::startActionSound(this, "EmLvHanachanSteam");
    MR::startActionSound(this, "EmLvHanachanPreAngry");
    moveBodyAlongHead();

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvBecomeAngry)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvBecomeAngry))) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryPursue));
    }
}

void Hanachan::exeAngryPursue() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvAngryPursue));
    }

    MR::startActionSound(this, "EmLvHanachanWhistle");
    MR::startActionSound(this, "EmLvHanachanMoveAngry");

    if (mIsChasePlayer) {
        moveHeadToPlayer(::hAngrySpeed, 0.08f);
    } else {
        moveHeadAlongRail(::hAngrySpeed);
    }

    moveBodyAlongHead();
    applyPlayerHipDropReaction();

    if (MR::isGreaterStep(this, ::hAngryPursueTime)) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryEnd));
    }
}

void Hanachan::exeAngryEnd() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvAngryEnd));
        setDelayAllPartsAtId(4, 0, 0);
        MR::startActionSound(this, "EvHanachanCooldown");
    }

    MR::startActionSound(this, "EmLvHanachanBrake");

    f32 speed = 1.0f - getNerveStep() / 60.0f;
    speed *= ::hAngrySpeed;
    mBodyParts[0]->mRotationQuat.getZDir(mFrontDir);
    mBodyParts[0]->mPushVelocity.add(mFrontDir * speed);

    HanachanParts* pHead = mBodyParts[0];
    MR::blendQuatUpFront(&pHead->mRotationQuat, -pHead->mGravity, mFrontDir, 0.5f, 0.5f);

    moveBodyAlongHead();
    applyPlayerHipDropReaction();

    if (MR::isGreaterStep(this, 60)) {
        if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvAngryEnd)) &&
            mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvAngryEnd))) {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk));
        }
    }
}

void Hanachan::exeWallHitEnd() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "EvHanachanCooldown");
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvWallHitEnd));

        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEnd)) ||
            isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEndOverturn))) {
            setDelayAllPartsAtId(calcNearestInfectionId(), 0, ::hWallHitInterval);
        } else if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWallHitEnd))) {
            setDelayAllPartsAtId(0, 0, ::hWallHitInterval);
            MR::emitEffect(mBodyParts[0], "WallHit");
        }
    }

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvWallHitEnd)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvWallHitEnd))) {
        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEndOverturn))) {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait));
        } else {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanWalk));
        }
    }
}

void Hanachan::exeOverturn() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvOverturn));
        setDelayAllPartsAtId(calcNearestInfectionId(), ::hSpinInit, ::hSpinInterval);
        MR::startBlowHitSound(this);
        MR::startActionSound(this, "EvHanachanOverturn");
    }

    if (MR::isStep(this, ::hOverturnBoundSeTime)) {
        MR::startActionSound(this, "EmHanachanBound");
    }

    moveBodyAlongHead();

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait))) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait));
    }
}

void Hanachan::exeOverturnWait() {
    if (MR::isGreaterStep(this, ::hOverturnTime)) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanRecover));
    } else {
        MR::startActionSound(this, "EmLvHanachanSwoon");
        applyPlayerHipDropReaction();
        moveBodyAlongHead();
    }
}

void Hanachan::exeOverturnBound() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvOverturnBound));
        setDelayAllPartsAtId(calcNearestInfectionId(), ::hOverturnBoundInit, ::hOverturnBoundInterval);
        MR::startBlowHitSound(this);
    }

    if (MR::isStep(this, ::hOverturnBoundSeTime)) {
        MR::startActionSound(this, "EmHanachanBound");
    }

    moveBodyAlongHead();

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvOverturnWait))) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait));
    }
}

void Hanachan::exeRecover() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvRecover));
        setDelayAllPartsAtId(0, ::hRecoverInit, ::hRecoverInterval);
    }

    MR::startActionSound(this, "EmLvHanachanPreAngry");

    if (MR::isGreaterEqualStep(this, ::hRecoverSteamSeTime)) {
        MR::startActionSound(this, "EmLvHanachanSteam");
    }

    if (MR::isStep(this, ::hRecoverGetupSeTime)) {
        MR::startActionSound(this, "EmHanachanGetup");
        MR::startActionSound(this, "EvHanachanAngry");
    }

    moveBodyAlongHead();

    if (mBodyParts[0]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvRecover)) &&
        mBodyParts[4]->isLandedInNerve(GET_NERVE(Hanachan, HanachanPartsNrvRecover))) {
        setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanAngryPursue));
    }
}

void Hanachan::exeHipDropped() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnHipDropped))) {
            setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvOverturnHipDropped));
        } else {
            setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvHipDropped));
        }

        setDelayAllPartsAtId(calcNearestInfectionId(), ::hHipDroppedInit, ::hHipDroppedInterval);
        MR::startActionSound(this, "EmStompedS");
        MR::startActionSound(this, "EvHanachanDead");
        mScaleController->startHitReaction();
    }

    if (MR::isGreaterStep(this, ::hHipDroppedTime)) {
        if (mBodyParts[0]->isHipDroppedLanded() && mBodyParts[4]->isHipDroppedLanded()) {
            for (s32 i = 0; i < mBodyParts.size(); i++) {
                mBodyParts[i]->kill();
            }

            kill();
        }
    }
}

void Hanachan::exeBlow() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvBlow));
        MR::startBlowHitSound(this);
    }

    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        if (!MR::isDead(*current)) {
            return;
        }
    }

    kill();
}

void Hanachan::exeStarPointerBind() {
    if (MR::isFirstStep(this)) {
        setNerveAllParts(GET_NERVE(Hanachan, HanachanPartsNrvStarPointerBind));
        mScaleController->startDpdHitVibration();
    }

    if (!isStarPointerPointing() && MR::isGreaterStep(this, ::hBindMinStep)) {
        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindOverturn))) {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEndOverturn));
        } else {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanStarPointerBindEnd));
        }

        mAttackPos = *MR::getStarPointerWorldPosUsingDepth(0);
    }
}

void Hanachan::endStarPointerBind() {
    mScaleController->startAnim();
}

s32 Hanachan::calcNearestInfectionId() {
    f32 nearestDist = 9999999.0f;
    s32 nearestId = -1;

    for (s32 i = 0; i < mBodyParts.size(); i++) {
        f32 dist = MR::getSensorPos(mBodyParts[i], "Body")->squared(mAttackPos);

        if (dist < nearestDist) {
            nearestDist = dist;
            nearestId = i;
        }
    }

    if (nearestId == -1) {
        return 0;
    }

    return nearestId;
}



#pragma push
#pragma opt_propagation off
void Hanachan::setNerveBlow(const TVec3f& rPos) {
    TVec3f dir = *getPosition() - rPos;
    MR::vecKillElement(dir, mGravity, &dir);
    MR::normalizeOrZero(&dir);

    const TVec3f& rGravity = mGravity;
    TVec3f side = dir.cross(rGravity);

    f32 angle = ::hInitBlowRadian;
    f32 numSegments = mBodyParts.size() - 1;
    f32 angleStep = (MR::pi() - ::hInitBlowRadian * 2.0f) / numSegments;
    TVec3f blow;
    TVec3f up;

    TVec3f headDir = *mBodyParts[0]->getPosition() - rPos;

    if (headDir.dot(side) < 0.0f) {
        angleStep = -angleStep;
        angle = MR::pi() - angle;
    }

    for (HanachanParts** current = mBodyParts.begin(); current < mBodyParts.end(); current++) {
        blow = side * MR::cos(angle);
        blow.add(dir * MR::sin(angle));
        blow.setLength(::hBlowVelH);

        up.set((*current)->mGravity * -1.0f);
        up.scale(::hBlowVelV);
        (*current)->mPushVelocity.set(blow + up);

        angle += angleStep;
    }

    setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanBlow));
}
#pragma pop

void Hanachan::applyPlayerHipDropReaction() {
    if (MR::isPlayerHipDropLand()) {
        mAttackPos.set(*MR::getPlayerCenterPos());

        f32 nearestDist = 999999.0f;

        for (s32 i = 0; i < mBodyParts.size(); i++) {
            f32 dist = MR::calcDistanceToPlayer(mBodyParts[i]);

            if (dist < nearestDist) {
                nearestDist = dist;
            }
        }

        if (::hNearHipDropReactionDist < nearestDist) {
            return;
        }

        if (isNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnWait))) {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturnBound));
        } else {
            setNerve(GET_NERVE(Hanachan, HanachanNrvHanachanOverturn));
        }
    }
}

void Hanachan::setNerveAllParts(const Nerve* pNerve) {
    for (s32 i = 0; i < mBodyParts.size(); i++) {
        mBodyParts[i]->setNerve(pNerve);
        mBodyParts[i]->mIsLanded = false;
    }
}

void Hanachan::setDelayAllPartsAtId(s32 id, s32 delay, s32 step) {
    s32 i;
    s32 currentDelay = delay;

    for (i = id; i >= 0; i--) {
        mBodyParts[i]->mActionStartStep = currentDelay;
        currentDelay += step;
    }

    s32 nextDelay = delay + step;

    for (i = id + 1; i < mBodyParts.size(); i++) {
        mBodyParts[i]->mActionStartStep = nextDelay;
        nextDelay += step;
    }
}

bool Hanachan::isStarPointerPointing() {
    for (s32 i = 0; i < mBodyParts.size(); i++) {
        if (MR::isStarPointerPointing2PAttachTarget(mBodyParts[i])) {
            return true;
        }
    }

    return false;
}

void Hanachan::moveHeadAlongRail(f32 speed) {
    mBodyParts[0]->mRotationQuat.getZDir(mFrontDir);

    TVec3f railDir;
    f32 dist = MR::calcMovingDirectionAlongRail(this, &railDir, mBodyParts[0]->mPosition, 800.0f, false, nullptr);
    if (::hFarDistToPursuePlayer < dist) {
        moveHeadToPlayer(speed, 0.04f);
    } else {
        MR::vecKillElement(railDir, mBodyParts[0]->mGravity, &railDir);
        MR::normalizeOrZero(&railDir);

        if (!MR::isNearZero(railDir)) {
            MR::turnVecToVecRadian(&mFrontDir, mFrontDir, railDir, 0.08f, mBodyParts[0]->mGravity);
        }

        mBodyParts[0]->mPushVelocity.add(mFrontDir * speed);

        HanachanParts* pHead = mBodyParts[0];
        MR::blendQuatUpFront(&pHead->mRotationQuat, -pHead->mGravity, mFrontDir, 0.5f, 0.5f);
    }
}

void Hanachan::moveHeadToPlayer(f32 speed, f32 turnSpeed) {
    mBodyParts[0]->mRotationQuat.getZDir(mFrontDir);

    TVec3f playerDir;
    MR::calcVecToPlayerH(&playerDir, mBodyParts[0], nullptr);
    MR::turnVecToVecRadian(&mFrontDir, mFrontDir, playerDir, turnSpeed, mBodyParts[0]->mGravity);

    mBodyParts[0]->mPushVelocity.add(mFrontDir * speed);

    HanachanParts* pHead = mBodyParts[0];
    MR::blendQuatUpFront(&pHead->mRotationQuat, -pHead->mGravity, mFrontDir, 0.5f, 0.5f);
}

TVec3f Hanachan::calcSensorDirection(const HitSensor* pSensor, const HitSensor* pTarget) const {
    TVec3f dir = pTarget->mPosition - pSensor->mPosition;
    MR::normalizeOrZero(&dir);
    return dir;
}

// NONMATCHING: vector expression scheduling, stack slots and saved-register allocation.
namespace {
    inline void turnBodyPartToPrevious(TQuat4f* rotation, const TVec3f& position, const TVec3f& target, const TVec3f& gravity) {
        TVec3f direction = target - position;
        MR::normalizeOrZero(&direction);
        MR::blendQuatUpFront(rotation, -gravity, direction, 0.3f, 0.5f);
    }
}

void Hanachan::moveBodyAlongHead() {
    for (s32 i = 1; i < mBodyParts.size(); i++) {
        HanachanParts* const& body = mBodyParts[i];
        TVec3f toPrev = *MR::getSensorPos(mBodyParts[i - 1], "Body");
        toPrev.sub(*MR::getSensorPos(body, "Body"));

        TVec3f push = -toPrev;
        push.setLength(::hCollideRange + MR::getSensorRadius(mBodyParts[i - 1], "Body") / 2.0f +
                       MR::getSensorRadius(body, "Body") / 2.0f);

        body->mPushVelocity.add(toPrev + push);

        if (MR::isBindedWall(body)) {
            HanachanParts* parts = body;
            parts->mFallVelocity.set(parts->getCommonGravity()->scaleInline(-5.0f));
        }

        turnBodyPartToPrevious(&body->mRotationQuat, *MR::getSensorPos(body, "Body"),
                               *MR::getSensorPos(mBodyParts[i - 1], "Body"), body->mGravity);
    }
}
