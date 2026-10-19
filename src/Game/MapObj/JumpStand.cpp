#include "Game/MapObj/JumpStand.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include "revolution/wpad.h"

namespace NrvJumpStand {
    NEW_NERVE(JumpStandNrvWait, JumpStand, Wait);
    NEW_NERVE(JumpStandNrvTrampleBound, JumpStand, TrampleBound);
    NEW_NERVE(JumpStandNrvHipDropBound, JumpStand, HipDropBound);
    NEW_NERVE(JumpStandNrvStarPieceBound, JumpStand, StarPieceBound);
};  // namespace NrvJumpStand

JumpStand::JumpStand(const char* pName) : LiveActor(pName), mBindedActor(), mIsMarioJumpingHigh() {
    mCollisionMtx.identity();
}

// NONMATCHING: TVec3f(0.0f) sound-offset temp is out of line (__ct__TVec3<f>Ff) in target, inlined in ours.
void JumpStand::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "JumpStand", false);
    MR::connectToSceneMapObjStrongLight(this);
    MR::initLightCtrl(this);
    initHitSensor(2);
    MR::addBodyMessageSensorMapObj(this);
    TVec3f binderOffs;
    binderOffs.set(0.0f, 100.0f, 0.0f);
    MR::addHitSensorPriorBinder(this, "binder", 4, 200.0f, binderOffs);
    MR::invalidateHitSensor(this, "binder");
    mCollisionMtx.set(MR::getJointMtx(this, "JumpStandJoint03"));
    MR::initCollisionParts(this, "JumpStand", getSensor("Body"), mCollisionMtx);
    initEffectKeeper(0, nullptr, false);
    initSound(4, "JumpStand", nullptr, TVec3f(0.0f));
    MR::setGroupClipping(this, rIter, 16);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::onCalcGravity(this);
    initNerve(GET_NERVE(JumpStand, JumpStandNrvWait), 0);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
    MR::startAction(this, "Wait");
}

void JumpStand::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
    }
}

void JumpStand::exeTrampleBound() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Bound");
        MR::startActionSound(this, "OjJumpStandLandS", -1, -1, -1);
    }

    if (MR::isLessEqualStep(this, 5) && MR::testCorePadTriggerA(0)) {
        mIsMarioJumpingHigh = true;
        MR::startAction(this, "BoundJump");
        MR::setBckFrame(this, getNerveStep());
    }

    if (MR::isStep(this, 5)) {
        if (mIsMarioJumpingHigh) {
            MR::startActionSound(this, "SE_OJ_JUMP_STAND_BOUND_M", -1, -1, -1);
            MR::startLevelSoundPlayer("SE_PV_JUMP_L", -1, -1);
        } else {
            MR::startActionSound(this, "SE_OJ_JUMP_STAND_BOUND_S", -1, -1, -1);
            MR::startLevelSoundPlayer("SE_PV_JUMP_M", -1, -1);
        }

        if (mIsMarioJumpingHigh) {
            endBindAndShootUp(32.0f, Jump_Middle);
        } else {
            endBindAndShootUp(26.0f, Jump_Low);
        }

        MR::validateCollisionParts(this);
    }

    if (MR::isStep(this, 10)) {
        MR::validateCollisionParts(this);
    }

    if (MR::isActionEnd(this)) {
        mIsMarioJumpingHigh = false;
        setNerve(GET_NERVE(JumpStand, JumpStandNrvWait));
    }
}

void JumpStand::exeHipDropBound() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BoundHipDrop");
        MR::stopSound(mBindedActor, "SE_PM_HIPDROP", 0);
        MR::startActionSound(this, "SE_OJ_JUMP_STAND_LAND_L", -1, -1, -1);
    }

    if (MR::isLessStep(this, 15) && mBindedActor != nullptr) {
        f32 pushBack = (6.0f - MR::calcNerveEaseOutRate(this, 15));
        TVec3f upVec;
        MR::calcUpVec(&upVec, this);
        mBindedActor->mPosition.scaleAdd(-pushBack, upVec, mBindedActor->mPosition);
    }

    if (MR::isStep(this, 15)) {
        MR::startActionSound(this, "SE_OJ_JUMP_STAND_BOUND_L", -1, -1, -1);
        MR::startLevelSoundPlayer("SE_PV_JUMP_JOY", -1, -1);
        endBindAndShootUp(43.0f, Jump_High);
    }

    if (MR::isStep(this, 20)) {
        MR::validateCollisionParts(this);
    }

    if (MR::isActionEnd(this)) {
        MR::validateCollisionParts(this);
        setNerve(GET_NERVE(JumpStand, JumpStandNrvWait));
    }
}

void JumpStand::exeStarPieceBound() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Bound");
        MR::startActionSound(this, "OjJumpStandLandS", -1, -1, -1);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpStand, JumpStandNrvWait));
    }
}

void JumpStand::control() {
    TVec3f pos;
    MR::copyJointPos(this, "JumpStandJoint03", &pos);
    mCollisionMtx.setTrans(pos);
}

bool JumpStand::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(JumpStand, JumpStandNrvTrampleBound))) {
        return false;
    }

    if (isNerve(GET_NERVE(JumpStand, JumpStandNrvHipDropBound))) {
        return false;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(JumpStand, JumpStandNrvStarPieceBound));
        return true;
    }

    return false;
}

bool JumpStand::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgUpdateBaseMtx(msg) && mBindedActor != nullptr) {
        updateBindActorMtx();
        return true;
    }

    if (MR::isMsgFloorTouch(msg)) {
        MR::validateHitSensor(this, "binder");
        return true;
    }

    if (!MR::isMsgAutoRushBegin(msg)) {
        return false;
    }

    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }

    if (!MR::isOnPlayer(getSensor("Body"))) {
        return false;
    } else {
        if (!MR::isPlayerHipDropLand()) {
            if (!tryToStartBind(pSender)) {
                return false;
            } else {
                setNerve(GET_NERVE(JumpStand, JumpStandNrvTrampleBound));
                return true;
            }

        } else if (MR::isPlayerHipDropLand()) {
            if (!tryToStartBind(pSender)) {
                return false;
            } else {
                setNerve(GET_NERVE(JumpStand, JumpStandNrvHipDropBound));
                return true;
            }
        }
    }

    return false;
}

bool JumpStand::tryToStartBind(HitSensor* pSensor) {
    if (mBindedActor != nullptr) {
        return false;
    }

    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    MR::invalidateCollisionParts(this);
    MR::invalidateClipping(this);
    mBindedActor = pSensor->mHost;
    MR::offBind(mBindedActor);
    return true;
}

void JumpStand::updateBindActorMtx() {
    TPos3f bindMtx;
    bindMtx.identity();
    TVec3f padDir;
    MR::calcPlayerWorldPadDir(&padDir, MR::getSubPadStickX(0), MR::getSubPadStickY(0));
    MR::calcMtxFromGravityAndZAxis(&bindMtx, mBindedActor, mGravity, padDir);
    MR::blendMtxRotate(mBindedActor->getBaseMtx(), bindMtx, 0.40f, bindMtx);
    bindMtx.setTrans(mBindedActor->mPosition);
    MR::setBaseTRMtx(mBindedActor, bindMtx);
}

void JumpStand::endBindAndShootUp(f32 shootSpeed, JumpType type) {
    TVec3f shootVel;

    if (MR::isPlayerInRush()) {
        if (type == Jump_High) {
            MR::calcUpVec(&shootVel, this);
        } else {
            shootVel.negate(mGravity);
        }

        shootVel *= shootSpeed;
        f32 stickLen = MR::sqrt(MR::getSubPadStickX(WPAD_CHAN0) * MR::getSubPadStickX(WPAD_CHAN0) +
                                MR::getSubPadStickY(WPAD_CHAN0) * MR::getSubPadStickY(WPAD_CHAN0));
        TVec3f stickDir;
        if (stickLen > 0.1f) {
            MR::normalize(shootVel, &stickDir);
            TVec3f stickWorldDir;
            MR::calcPlayerWorldPadDir(&stickWorldDir, MR::getSubPadStickX(WPAD_CHAN0), MR::getSubPadStickY(WPAD_CHAN0));
            TVec3f lateralVel;
            lateralVel.scale(3.0f * stickLen, stickWorldDir);
            lateralVel.orthogonalize(stickDir);
            shootVel.add(lateralVel);
        }

        if (type == Jump_Low) {
            MR::startBckPlayer("TrampolineJumpLow", nullptr);
        } else if (type == Jump_Middle) {
            MR::startBckPlayer("TrampolineJumpMiddle", nullptr);
        } else if (type == Jump_High) {
            MR::startBckPlayer("TrampolineJumpHigh", nullptr);
        }

        MR::endBindAndPlayerJump(this, shootVel, 0);

        if (type != Jump_High) {
            MR::becomePlayerNormalJumpStatus();
        }

        MR::invalidateHitSensor(this, "binder");
    }

    mBindedActor = nullptr;
}
