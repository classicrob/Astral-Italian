#define JGEOMETRY_VEC3_MI_NOINLINE
#include "Game/Enemy/HammerHeadPackun.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointRumbler.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>
#include <revolution/mtx.h>
#include <revolution/types.h>

HammerHeadPackun::HammerHeadPackun(const char* pName) : LiveActor(pName) {
    mLeafModel = nullptr;
    mImmunityTime = 0;
    mUp.x = 0.0f;
    mUp.y = 1.0f;
    mUp.z = 0.0f;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mTargetFront.x = 0.0f;
    mTargetFront.y = 0.0f;
    mTargetFront.z = 1.0f;
    mJointRumbler = nullptr;
    mNumbTime = 0;
    mIsSpike = false;
    mDropsStarPiece = true;
}

namespace NrvHammerHeadPackun {
    NEW_NERVE(HammerHeadPackunNrvWait, HammerHeadPackun, Wait);
    NEW_NERVE(HammerHeadPackunNrvSearch, HammerHeadPackun, Search);
    NEW_NERVE(HammerHeadPackunNrvAttack, HammerHeadPackun, Attack);
    NEW_NERVE(HammerHeadPackunNrvRest, HammerHeadPackun, Rest);
    NEW_NERVE(HammerHeadPackunNrvRecover, HammerHeadPackun, Recover);
    NEW_NERVE(HammerHeadPackunNrvDamage, HammerHeadPackun, Damage);
    NEW_NERVE(HammerHeadPackunNrvRise, HammerHeadPackun, Rise);
    NEW_NERVE(HammerHeadPackunNrvHit, HammerHeadPackun, Hit);
    NEW_NERVE(HammerHeadPackunNrvCrow, HammerHeadPackun, Crow);
    NEW_NERVE(HammerHeadPackunNrvSmashKill, HammerHeadPackun, SmashKill);
    NEW_NERVE(HammerHeadPackunNrvBlowKill, HammerHeadPackun, BlowKill);
    NEW_NERVE(HammerHeadPackunNrvBlowKillRock, HammerHeadPackun, BlowKillRock);
    NEW_NERVE_ONEND(HammerHeadPackunNrvNumb, HammerHeadPackun, Numb, Numb);
};  // namespace NrvHammerHeadPackun

void HammerHeadPackun::init(const JMapInfoIter& rIter) {
    mIsSpike = MR::isEqualObjectName(rIter, "HammerHeadPackunSpike");
    MR::initDefaultPos(this, rIter);
    initPosture();
    MR::initActor(this, rIter, mIsSpike ? "PackunFlowerSpike" : "PackunFlower", false);
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);
    initSensor();
    initEffectKeeper(1, mIsSpike ? "PackunFlowerSpike" : "PackunFlower", false);
    MR::addEffect(this, "PointerTouchManual");
    MR::setEffectHostMtx(this, "PointerTouchManual", MR::getJointMtx(this, "MouthUp"));
    initSound(4, "HammerHeadPackun", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait), 0);
    MR::initUseStageSwitchWriteDead(this, rIter);
    initShadow();
    initLeaf(rIter);
    MR::initStarPointerTargetAtJoint(this, "Head", 169.0f, TVec3f(120.0f, 0.0f, 0.0f));
    MR::declareStarPiece(this, 9);
    mJointRumbler = new JointRumbler(this, "AllRoot", 8.0f, 0.06f, 60, true, 0);
    MR::useStageSwitchAwake(this, rIter);

    s32 arg0;
    MR::getJMapInfoArg0WithInit(rIter, &arg0);
    if (arg0 == -1) {
        mDropsStarPiece = true;
    } else {
        mDropsStarPiece = false;
    }

    makeActorAppeared();
}

void HammerHeadPackun::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait));
}

void HammerHeadPackun::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    MR::startActionSound(this, "EmHhpackunDead");
    appearStarPiece();
    LiveActor::kill();
}

void HammerHeadPackun::appearStarPiece() {
    if (mDropsStarPiece) {
        TVec3f pos;
        MR::copyJointPos(this, "Head", &pos);
        MR::appearStarPiece(this, pos, 9, 10.0f, 40.0f, false);
    }
}

void HammerHeadPackun::control() {
    mJointRumbler->update();
}

void HammerHeadPackun::exeWait() {
    if (MR::isFirstStep(this)) {
        const char* wait = "Wait";
        MR::startAction(this, wait);
        MR::startAction(mLeafModel, wait);
    }

    if (!tryShiftNumb() && isTargetInRange()) {
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSearch));
    }
}

void HammerHeadPackun::exeSearch() {
    TVec3f dir;
    if (calcPlayerDir(&dir) && !tryShiftNumb()) {
        turnTo(dir, 1.0f);
        if (isInFieldOfView(dir)) {
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvAttack));
        }
    }
}

void HammerHeadPackun::exeAttack() {
    if (MR::isFirstStep(this)) {
        const char* attack = "Attack";
        MR::startAction(this, attack);
        MR::startAction(mLeafModel, attack);
    }

    TVec3f dir;
    if (!tryShiftNumb()) {
        if (MR::isStep(this, 156)) {
            MR::shakeCameraNormalStrong();
            MR::tryRumblePadStrong(this, WPAD_CHAN0);
        }

        if (MR::isLessEqualStep(this, 60)) {
            if (calcPlayerDir(&dir)) {
                turnTo(dir, 1.0f);
            }
        } else {
            turnToAttackVec(60);
        }
        MR::setNerveAtBckStopped(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRest));
    }
}

void HammerHeadPackun::exeRest() {
    if (MR::isFirstStep(this)) {
        const char* chance = "Chance";
        MR::startAction(this, chance);
        MR::startAction(mLeafModel, chance);
        mImmunityTime = 0;
    }

    if (!tryShiftNumb()) {
        MR::setNerveAtStep(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRecover), 90);
    }
}

void HammerHeadPackun::exeRecover() {
    if (MR::isFirstStep(this)) {
        const char* recovery = "Recovery";
        MR::startAction(this, recovery);
        MR::startAction(mLeafModel, recovery);
    }

    if (!tryShiftNumb()) {
        MR::startActionSound(this, "EmLvHhpackunRecovery");
        MR::setNerveAtBckStopped(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait));
    }
}

void HammerHeadPackun::exeDamage() {
    if (MR::isFirstStep(this)) {
        const char* swoonStart = "SwoonStart";
        MR::startAction(this, swoonStart);
        MR::startAction(mLeafModel, swoonStart);
        MR::startBlowHitSound(this);
        mImmunityTime++;
    }

    if (MR::isStep(this, 76)) {
        MR::shakeCameraNormal();
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        const char* swoon = "Swoon";
        MR::startAction(this, swoon);
        MR::startAction(mLeafModel, swoon);
    }

    if (MR::isGreaterEqualStep(this, 60)) {
        MR::startActionSound(this, "EmLvSwoonS");
    }
    MR::setNerveAtStep(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRise), 180);
}

void HammerHeadPackun::exeRise() {
    if (MR::isFirstStep(this)) {
        const char* swoonEnd = "SwoonEnd";
        MR::startAction(this, swoonEnd);
        MR::startAction(mLeafModel, swoonEnd);
    }

    if (!tryShiftNumb()) {
        if (MR::isStep(this, 56)) {
            MR::shakeCameraNormalWeak();
            MR::tryRumblePadWeak(this, WPAD_CHAN0);
        }

        if (MR::isActionEnd(this)) {
            if (isTargetInRange()) {
                const char* wait = "Wait";
                MR::startAction(this, wait);
                MR::startAction(mLeafModel, wait);
                setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSearch));
            } else {
                setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait));
            }
        }
    }
}

inline void HammerHeadPackun::exeHit() {
    if (!tryShiftNumb()) {
        MR::setNerveAtBckStopped(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvCrow));
    }
}

void HammerHeadPackun::exeCrow() {
    if (MR::isFirstStep(this)) {
        const char* crow = "Crow";
        MR::startAction(this, crow);
        MR::startAction(mLeafModel, crow);
    }

    if (!tryShiftNumb()) {
        if (MR::isStep(this, 45)) {
            MR::shakeCameraNormalWeak();
            MR::tryRumblePadWeak(this, WPAD_CHAN0);
        }
        MR::setNerveAtBckStopped(this, GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait));
    }
}

void HammerHeadPackun::exeSmashKill() {
    if (MR::isFirstStep(this)) {
        const char* down = "Down";
        MR::startAction(this, down);
        MR::startAction(mLeafModel, down);
        MR::startActionSound(this, "EmHhpackunStomped");
        MR::startActionSound(this, "EmStompedS");
        MR::invalidateClipping(this);
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void HammerHeadPackun::exeBlowKill() {
    if (MR::isFirstStep(this)) {
        const char* blowDown = "BlowDown";
        MR::startAction(this, blowDown);
        MR::startAction(mLeafModel, blowDown);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::startActionSound(this, "EmHhpackunHitPunch");
        MR::startActionSound(this, "EvHhpackunDead");
        MR::invalidateClipping(this);
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        kill();
    }
}

void HammerHeadPackun::exeBlowKillRock() {
    if (MR::isFirstStep(this)) {
        const char* blowDown = "BlowDownRock";
        MR::startAction(this, blowDown);
        MR::startAction(mLeafModel, blowDown);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::startActionSound(this, "EmHhpackunHitPunch");
        MR::startActionSound(this, "EvHhpackunDead");
        MR::invalidateClipping(this);
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        kill();
    }
}

void HammerHeadPackun::exeNumb() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        mEffectKeeper->forceDeleteEmitterAll();
        MR::stopBck(this);
        MR::stopBck(mLeafModel);
        MR::emitEffectWithScale(this, "PointerTouchManual", 1.5f, -1);
        mNumbTime = 0;
    }

    MR::startDPDFreezeLevelSound(this);
    if (!(getNerveStep() % 30)) {
        mJointRumbler->start();
    }

    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        mNumbTime = 0;
    }

    mNumbTime++;
    if (mNumbTime >= 20) {
        MR::deleteEffect(this, "PointerTouchManual");
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait));
    }
}

void HammerHeadPackun::endNumb() {
    MR::deleteEffect(this, "PointerTouchManual");
}

void HammerHeadPackun::calcAndSetBaseMtx() {
    TVec3f vec;
    TPos3f mtx;
    vec.cross(mUp, mFront);
    mtx[0][0] = vec.x;
    mtx[1][0] = vec.y;
    mtx[2][0] = vec.z;
    mtx[0][1] = mUp.x;
    mtx[1][1] = mUp.y;
    mtx[2][1] = mUp.z;
    mtx[0][2] = mFront.x;
    mtx[1][2] = mFront.y;
    mtx[2][2] = mFront.z;
    mtx[0][3] = mPosition.x;
    mtx[1][3] = mPosition.y;
    mtx[2][3] = mPosition.z;
    MR::setBaseTRMtx(this, mtx);
}

inline bool HammerHeadPackun::checkBlowAttack() const {
    return isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSmashKill)) || isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKill)) ||
           isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKillRock));
}

// NONMATCHING: Vector expression temporary stack slots differ.
void HammerHeadPackun::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!checkBlowAttack() && MR::isEqualSensor(pSender, this, "blow") && MR::getSensorHost(pReceiver) &&
        !strcmp(MR::getSensorHost(pReceiver)->mName, "パンチングキノコ")) {
        f32 temp = MR::getSensorRadius(pReceiver);
        temp = MR::getSensorRadius(pSender) + temp;
        temp = MR::getSensorRadius(pReceiver) / temp;
        TVec3f receiverWeighted = *MR::getSensorPos(pReceiver) * (1.0f - temp);
        TVec3f senderWeighted = *MR::getSensorPos(pSender) * temp;
        TVec3f hitMtx = senderWeighted + receiverWeighted;
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::emitEffectHit(this, hitMtx, nullptr);
        MR::stopScene(8);
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKill));
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
        bool isSensorBody = getSensor("Body") != pSender;
        if (isSensorBody) {
            if (checkBlowAttack() || !MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (isAttackSensorValid() && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            MR::shakeCameraNormal();
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
            if (isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvAttack)) && !MR::isNewNerve(this)) {
                setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvHit));
            } else {
                mJointRumbler->start();
            }
        } else {
            if (!checkBlowAttack()) {
                MR::sendMsgPush(pReceiver, pSender);
            }
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

// NONMATCHING: Vector expression temporary stack slots differ.
bool HammerHeadPackun::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    MR::isEqualSensor(pReceiver, this, "Body");
    MR::isEqualSensor(pReceiver, this, "blow");
    MR::isEqualSensor(pReceiver, this, "push1");
    MR::isEqualSensor(pReceiver, this, "push2");
    MR::isEqualSensor(pReceiver, this, "push3");
    MR::isEqualSensor(pReceiver, this, "push4");

    if (MR::isMsgSupportTicoSpin(msg)) {
        return false;
    }
    if (!checkBlowAttack() && MR::isMsgPlayerHitProjectiles(msg)) {
        f32 temp = MR::getSensorRadius(pReceiver);
        temp = MR::getSensorRadius(pSender) + temp;
        temp = MR::getSensorRadius(pSender) / temp;
        if (MR::isMsgRockAttack(msg)) {
            MR::invalidateHitSensors(this);
            MR::sendMsgIsRushEnable(pSender, pReceiver);
            TVec3f senderWeighted = *MR::getSensorPos(pSender) * (1.0f - temp);
            TVec3f receiverWeighted = *MR::getSensorPos(pReceiver) * temp;
            TVec3f hitMtx = receiverWeighted + senderWeighted;
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
            MR::emitEffectHit(this, hitMtx, nullptr);
            MR::stopScene(8);
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKillRock));
        } else {
            TVec3f senderWeighted = *MR::getSensorPos(pSender) * (1.0f - temp);
            TVec3f receiverWeighted = *MR::getSensorPos(pReceiver) * temp;
            TVec3f hitMtx = receiverWeighted + senderWeighted;
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
            MR::emitEffectHit(this, hitMtx, nullptr);
            MR::stopScene(8);
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKill));
        }
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        return true;
    }
    if (isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvWait)) ||
        isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSearch))) {
        if (MR::isMsgStarPieceAttack(msg)) {
            mJointRumbler->start();
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvAttack));
            return true;
        }
    } else if (MR::isMsgStarPieceReflect(msg)) {
        mJointRumbler->start();
        MR::startActionSound(this, "EmStarpieceRefrectPliable");
        return true;
    }

    if (pReceiver != getSensor("Body")) {
        return false;
    }

    if (isChance()) {
        return receiveMsgPlayerAttackChance(msg, pSender, pReceiver);
    }

    if (checkBlowAttack()) {
        return MR::isMsgPlayerTrample(msg);
    } else {
        return false;
    }
}

// NONMATCHING: Vector temporary stack slots and commutative add operand order differ.
bool HammerHeadPackun::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackBlow(msg) && !checkBlowAttack()) {
        f32 sendRadius = MR::getSensorRadius(pReceiver);
        sendRadius = MR::getSensorRadius(pSender) + sendRadius;
        f32 temp = MR::getSensorRadius(pSender) / sendRadius;
        TVec3f senderWeighted = *MR::getSensorPos(pSender) * (1.0f - temp);
        TVec3f receiverWeighted = *MR::getSensorPos(pReceiver) * temp;
        TVec3f hitMtx = receiverWeighted + senderWeighted;
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::emitEffectHit(this, hitMtx, nullptr);
        MR::stopScene(8);
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKill));
        return true;
    }

    return false;
}

void HammerHeadPackun::initPosture() {
    TPos3f mtx;
    MR::makeMtxTR(mtx, this);
    mtx.getYDir(mUp);
    mtx.getZDir(mFront);
}

void HammerHeadPackun::initSensor() {
    initHitSensor(6);
    MR::addHitSensorAtJointMapObj(this, "Body", "Head", 32, 130.0f, TVec3f(120.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObj(this, "blow", "Spin1", 32, 120.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObj(this, "push1", "Spin2", 8, 45.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObj(this, "push2", "Spin2", 8, 45.0f, TVec3f(50.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObj(this, "push3", "Spin3", 8, 45.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObj(this, "push4", "Spin3", 8, 60.0f, TVec3f(80.0f, 0.0f, 0.0f));
}

void HammerHeadPackun::initShadow() {
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    mGravity.set(-mUp);
}

inline s32 JMapInfo_getEntries(const JMapInfo* jmap) {
    return jmap->mData ? jmap->mData->mNumEntries : nullptr;
}

inline bool JMapInfo_inRange(const JMapInfoIter& rIter) {
    return rIter.mInfo && rIter.mIndex >= 0 && rIter.mIndex < JMapInfo_getEntries(rIter.mInfo);
}

void HammerHeadPackun::initLeaf(const JMapInfoIter& rIter) {
    const char* model = mIsSpike ? "PackunLeafSpike" : "PackunLeaf";
    mLeafModel = new PartsModel(this, "パックンの葉", model, nullptr, 19, false);
    MR::initLightCtrl(mLeafModel);
    s32 arg = -1;
    if (JMapInfo_inRange(rIter)) {
        MR::getJMapInfoArg0NoInit(rIter, &arg);
    }

    if (arg > 0) {
        mLeafModel->initFixedPosition(TVec3f(100.0f, 0.0f, 0.0f), TVec3f(0.0f, -90.0f, -90.0f), "Spin1");
    } else {
        mLeafModel->initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, -90.0f, -90.0f), "Spin1");
    }

    mLeafModel->initWithoutIter();
}

bool HammerHeadPackun::isTargetInRange() const {
    TVec3f vec;
    MR::getPlayerUpVec(&vec);
    MtxPtr baseMtx1 = getBaseMtx();
    MtxPtr baseMtx2 = getBaseMtx();
    MtxPtr baseMtx3 = getBaseMtx();
    TVec3f a2(baseMtx3[0][1], baseMtx2[1][1], baseMtx1[2][1]);

    if (vec.dot(a2) <= 0.0f) {
        return false;
    } else if (MR::calcDistanceToPlayer(mPosition) < 800.0f) {
        return true;
    }

    return false;
}

bool HammerHeadPackun::isInFieldOfView(const TVec3f& rPos) const {
    f32 deg = 30.0f;
    if (30.0f < 0.0f) {
        deg = -30.0f;
    }

    return rPos.dot(mFront) >= MR::cosDegree(deg);
}

bool HammerHeadPackun::isAttackSensorValid() const {
    if (checkBlowAttack()) {
        return false;
    }
    bool valid;
    if (mIsSpike) {
        valid = true;
    } else {
        valid = false;
        if (!(isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvAttack)) && MR::isNewNerve(this)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRest)) &&
            !(isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRecover)) && MR::isLessEqualStep(this, 40)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvDamage)) &&
            !(isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRise)) && MR::isLessStep(this, 60)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvNumb)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSmashKill)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKill)) &&
            !isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvBlowKillRock))) {
            valid = true;
        }
    }
    return valid;
}

// NONMATCHING: Early-return register and epilogue load scheduling differ.
bool HammerHeadPackun::isChance() const {
    if (mIsSpike) {
        return false;
    }

    bool isChance = false;
    if (isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRest)) && !MR::isNewNerve(this) || isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvDamage)) ||
        isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRecover)) && MR::isLessEqualStep(this, 50) ||
        isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRise)) && MR::isLessEqualStep(this, 30) || isNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvNumb))) {
        isChance = true;
    }
    return isChance;
}

// NONMATCHING: Vector subtraction is expanded inline instead of an operator call.
bool HammerHeadPackun::calcPlayerDir(TVec3f* pPos) const {
    pPos->set(*MR::getPlayerPos() - mPosition);
    pPos->orthogonalize2(mUp);
    if (MR::isNearZero(*pPos)) {
        return false;
    }

    MR::normalize(pPos);
    return true;
}

void HammerHeadPackun::turnTo(const TVec3f& rFrom, f32 rate) {
    f32 absAngle = mFront.angle(rFrom);
    f32 ratio = 1.0f;
    f32 maxAngle = rate * MR::pi() / 180.0f;
    if (absAngle > maxAngle) {
        ratio = maxAngle / absAngle;
    }
    TQuat4f quat;
    quat.setRotate(mFront, rFrom, ratio);
    quat.transform(mFront);
    verticalizeFrontVec();
}

void HammerHeadPackun::turnToAttackVec(s32 rate) {
    TQuat4f mtx;
    if (getNerveStep() <= rate) {
        f32 step = (f32)getNerveStep() / rate;
        if (step > 1.0f) {
            step = 1.0f;
        }

        mtx.setRotate(mFront, mTargetFront, (step * step));
        mtx.transform(mFront);
        verticalizeFrontVec();
    }
}

void HammerHeadPackun::verticalizeFrontVec() {
    mFront.orthogonalize2(mUp);
    MR::normalize(&mFront);
}

bool HammerHeadPackun::tryShiftNumb() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvNumb));
        return true;
    }
    return false;
}

bool HammerHeadPackun::receiveMsgPlayerAttackChance(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSmashKill));
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    } else if (MR::isMsgPlayerTrample(msg)) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvSmashKill));
        return true;
    } else if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (mImmunityTime >= 3u) {
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvRise));
        } else {
            setNerve(GET_NERVE(HammerHeadPackun, HammerHeadPackunNrvDamage));
        }
        return true;
    }
    return false;
}
