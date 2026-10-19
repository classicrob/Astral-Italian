#include "Game/Enemy/JellyfishElectric.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"


namespace NrvJellyfishElectric {
    NEW_NERVE(JellyfishElectricNrvWait, JellyfishElectric, Wait);
    NEW_NERVE(JellyfishElectricNrvWaitWithRightTurn, JellyfishElectric, WaitWithRightTurn);
    NEW_NERVE(JellyfishElectricNrvWaitWithLeftTurn, JellyfishElectric, WaitWithLeftTurn);
    NEW_NERVE(JellyfishElectricNrvDamage, JellyfishElectric, Damage);
    NEW_NERVE(JellyfishElectricNrvDeath, JellyfishElectric, Death);
    NEW_NERVE(JellyfishElectricNrvAttack, JellyfishElectric, Attack);
    NEW_NERVE(JellyfishElectricNrvRailGoal, JellyfishElectric, RailGoal);
    NEW_NERVE(JellyfishElectricNrvDPDSwoon, JellyfishElectric, DPDSwoon);

};  // namespace NrvJellyfishElectric

namespace {
    static Color8 sPointLightColor(0x9C, 0xFF, 0xFF, 0xFF);
};  // namespace

JellyfishElectric::JellyfishElectric(const char* pName) : LiveActor(pName) {
    mController = nullptr;
    mBindStarPtr = nullptr;
    mWaveStep = 0;
    mFront.set(0.0f, 0.0f, 1.0f);
    mIsDanger = false;
    mIsConnectedRail = 0;
    mRailSpeed = 5.0f;
    mGoalWaitStep = 60;
    mIsLoopRail = 0;
    mRailClipPos.set(0.0f, 0.0f, 0.0f);
}

void JellyfishElectric::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "JellyfishElectric", false);
    MR::connectToSceneEnemy(this);
    MR::addToAttributeGroupSearchTurtle(this);
    initHitSensor(1);
    TVec3f sensorOffs;
    sensorOffs.set(0.0f, -60.0f, 0.0f);
    MR::addHitSensorEnemy(this, "body", 8, 350.0f, sensorOffs);
    initBinder(400.0f, 0.0f, 0);
    MR::offBind(this);
    mController = new AnimScaleController(nullptr);
    mController->setParamTight();
    initEffectKeeper(2, nullptr, false);
    MR::addEffectHitNormal(this, nullptr);
    initSound(4, "JellyfishElectric", nullptr, TVec3f(0.0f));
    MR::initStarPointerTarget(this, 400.0f, TVec3f(0.0f, 0.0f, 0.0f));
    mController = new AnimScaleController(nullptr);
    mController->setParamTight();
    mBindStarPtr = new WalkerStateBindStarPointer(this, mController);
    MR::initShadowVolumeSphere(this, 230.0f);
    MR::declareStarPiece(this, 10);

    if (MR::isConnectedWithRail(rIter)) {
        mIsConnectedRail = true;
        initRailRider(rIter);
        MR::initAndSetRailClipping(&mRailClipPos, this, 100.0f, 500.0f);
        MR::moveCoordToNearestPos(this, mPosition);
        MR::getJMapInfoArg0NoInit(rIter, &mRailSpeed);
        MR::getJMapInfoArg1NoInit(rIter, &mGoalWaitStep);
        s32 arg2 = 0;
        MR::getJMapInfoArg2NoInit(rIter, &arg2);
        mIsLoopRail = arg2 == 1;
        MR::onCalcGravity(this);
    } else {
        MR::setClippingTypeSphere(this, 900.0f);
        MR::calcGravity(this);
    }

    MR::calcFrontVec(&mFront, this);
    MR::startBrk(this, "Wait");
    initNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait), 1);
    MR::initActorState(this, mBindStarPtr, GET_NERVE(JellyfishElectric, JellyfishElectricNrvDPDSwoon), "SupportFreeze");
    makeActorAppeared();
}

void JellyfishElectric::kill() {
    MR::appearStarPiece(this, mPosition, 10, 20.0f, 40.0f, false);
    LiveActor::kill();
}

#pragma push
#pragma opt_propagation off
void JellyfishElectric::control() {
    MR::requestPointLight(this, TVec3f(mPosition), ::sPointLightColor, 0.9999f, -1);
    MR::changeShowModelFlagSyncNearClipping(this, 400.0f);
    mController->updateNerve();

    if (!tryDPDSwoon() && !isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDeath)) && !isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDPDSwoon))) {
        if (mIsConnectedRail) {
            MR::moveCoordAndFollowTrans(this, mRailSpeed);
            if (MR::isRailReachedGoal(this)) {
                if (!isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvRailGoal))) {
                    setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvRailGoal));
                    return;
                }
            }
        }

        f32 wave = MR::sinDegree(mWaveStep + 45);
        const TVec3f& rGravity = mGravity;
        mVelocity.scale(wave, rGravity);
        mWaveStep++;
    }
}
#pragma pop

void JellyfishElectric::calcAndSetBaseMtx() {
    TPos3f pos;
    MR::makeMtxFrontUpPos(&pos, mFront, -mGravity, mPosition);
    MR::setBaseTRMtx(this, pos);
    MR::updateBaseScale(this, mController);
}

void JellyfishElectric::exeWait() {
    if (MR::isFirstStep(this)) {
        if (!mIsDanger && !MR::isBckPlaying(this, "Wait")) {
            MR::startAction(this, "Wait");
        } else {
            if (mIsDanger && !MR::isBckPlaying(this, "DangerWait")) {
                MR::startAction(this, "DangerWait");
                MR::startBasWithUpdateFoot(this, "DangerBrk", false, true, 0.0f, 0.0f);
            }
        }
    }

    selectNerveAfterWait();
}

void JellyfishElectric::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "EmJellyelecDamage");
        MR::startAction(this, "Damage");
    }

    if (MR::isActionEnd(this)) {
        MR::startAction(this, "DangerWait");
        MR::startBasWithUpdateFoot(this, "DangerBrk", false, true, 0.0f, 0.0f);
        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait));
    }
}

void JellyfishElectric::exeDeath() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::invalidateHitSensors(this);
        MR::stopScene(5);
        MR::startAction(this, "Death");
    }

    MR::startActionSound(this, "EmLvJellyelecPreDead");
    if (MR::isStep(this, 30) || MR::isBinded(this)) {
        MR::startActionSound(this, "EmJellyelecDead");
        kill();
    }
}

void JellyfishElectric::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    MR::startActionSound(this, "EmLvJellyelecAttack");

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait));
    }
}

void JellyfishElectric::exeRailGoal() {
    if (MR::isStep(this, mGoalWaitStep)) {
        if (mIsLoopRail) {
            MR::moveCoordAndTransToRailPoint(this, 0);
            const TVec3f& dir = MR::getRailDirection(this);
            mFront.x = dir.x;
            mFront.y = dir.y;
            mFront.z = dir.z;
            mIsDanger = 0;
        } else {
            MR::reverseRailDirection(this);
        }

        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait));
    }
}

void JellyfishElectric::exeDPDSwoon() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
    }

    mVelocity.zero();
    MR::updateActorStateAndNextNerve(this, GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait));
}

void JellyfishElectric::exeWaitWithLeftTurn() {
    waitTurn();
    selectNerveAfterWait();
}

void JellyfishElectric::exeWaitWithRightTurn() {
    waitTurn();
    selectNerveAfterWait();
}

void JellyfishElectric::endDPDSwoon() {
    mBindStarPtr->kill();
}

void JellyfishElectric::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender) && MR::isSensorPlayer(pReceiver)) {
        tryToAttackElectric(pReceiver, pSender);
    } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
        if (MR::isSensorEnemy(pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool JellyfishElectric::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        mController->startHitReaction();
        return true;
    } else if (MR::isMsgJetTurtleAttack(msg)) {
        knockOut();
        return true;
    } else if (MR::isMsgInvincibleAttack(msg)) {
        mIsDanger = 1;
        knockOut();
        return true;
    } else {
        if (MR::isMsgPlayerSpinAttack(msg)) {
            return tryToAttackElectric(pSender, pReceiver);
        }
    }

    return false;
}

bool JellyfishElectric::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && !isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDeath))) {
        knockOut();
        return true;
    }

    return false;
}

void JellyfishElectric::waitTurn() {
    f32 turnDirection;
    f32 turnDecay = (1.0f - (getNerveStep() / 280.0f));

    if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWaitWithLeftTurn))) {
        turnDirection = 1.0f;
    } else {
        turnDirection = -1.0f;
    }

    MR::rotateVecDegree(&mFront, -mGravity, (turnDirection * (0.2f * turnDecay)));
}

void JellyfishElectric::knockOut() {
    if (!mIsDanger) {
        mIsDanger = true;
        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDamage));
    } else {
        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDeath));
    }
}

bool JellyfishElectric::tryToAttackElectric(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::sendMsgEnemyAttackElectric(pSender, pReceiver)) {
        MR::emitEffectHitBetweenSensors(this, pReceiver, pSender, 0.0f, nullptr);
        setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvAttack));
        return true;
    }

    MR::sendMsgPush(pSender, pReceiver);
    return false;
}

bool JellyfishElectric::tryDPDSwoon() {
    if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDamage))) {
        return false;
    }

    if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDeath))) {
        return false;
    }

    if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvRailGoal))) {
        return false;
    }

    if (!mBindStarPtr->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvDPDSwoon));
    return true;
}

bool JellyfishElectric::selectNerveAfterWait() {
    if (MR::isStep(this, 280)) {
        s32 rand;

        if (isNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait))) {
            rand = MR::getRandom(0l, 3l);
        } else {
            rand = 0;
        }

        if (rand == 0) {
            setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWait));
        } else if (rand == 1) {
            setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWaitWithRightTurn));
        } else {
            setNerve(GET_NERVE(JellyfishElectric, JellyfishElectricNrvWaitWithLeftTurn));
        }

        return true;
    }

    return false;
}

JellyfishElectric::~JellyfishElectric() {
}
