#include "Game/MapObj/NeedlePlant.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Player/YoshiFunction.hpp"


namespace {
    // Shared name pool: object name, fly object name, demo object name (Shift-JIS, +0x1C), model name and joint name.
    char sNamePool[] = "NeedlePlant\0"
                       "NeedlePlantFly\0\0"
                       "\x83\x67\x83\x51\x90\x41\x95\xA8\x8E\x63\x8A\x5B"
                       "\0\0\0\0\0\0\0\0"
                       "NeedlePlantHill\0"
                       "Needle1\0";
}  // namespace

namespace NrvNeedlePlant {
    NEW_NERVE(NeedlePlantNrvWait, NeedlePlant, Wait);
    NEW_NERVE(NeedlePlantNrvShake, NeedlePlant, Shake);
};  // namespace NrvNeedlePlant

namespace {
    NEW_NERVE(NeedlePlantNrvSupportFreeze, NeedlePlant, SupportFreeze);
    NEW_NERVE(NeedlePlantNrvEatStart, NeedlePlant, EatStart);
    NEW_NERVE(NeedlePlantNrvEatPull, NeedlePlant, EatPull);
    NEW_NERVE(NeedlePlantNrvEatPullEnd, NeedlePlant, EatPullEnd);

    // Eaten nerve: empty execute, no exe function of its own
    class NeedlePlantNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static NeedlePlantNrvEaten sInstance;
    };
#ifdef GALAXY_NATIVE
    NeedlePlantNrvEaten NeedlePlantNrvEaten::sInstance;
#else
    NeedlePlantNrvEaten NeedlePlantNrvEaten::sInstance ATTRIBUTE_WEAK;
#endif

    NEW_NERVE(NeedlePlantNrvBroken, NeedlePlant, Broken);
};  // namespace

NeedlePlant::NeedlePlant(const char* pName)
    : LiveActor(pName), mYoshiLockOn(nullptr), mDemoObj(nullptr), mTargetPos(0.0f, 0.0f, 0.0f), mObjArg0(-1), mUnkA8(0.0f), mIsFly(false) {}

void NeedlePlant::init(const JMapInfoIter& rIter) {
    char* pNames = sNamePool;
    MR::initActor(this, rIter, false);

    bool isShadow = false;
    if (MR::isObjectName(rIter, pNames)) {
        isShadow = true;
    } else if (MR::isObjectName(rIter, pNames + 12)) {
        MR::getJMapInfoArg1NoInit(rIter, &isShadow);
        mIsFly = true;
    }

    if (!isShadow) {
        MR::invalidateShadow(this, nullptr);
    }

    if (MR::isObjectName(rIter, pNames)) {
        mDemoObj = reinterpret_cast<LiveActor*>(MR::createDemoObj(pNames + 28, pNames + 48, nullptr));
        MR::copyTransRotate(this, mDemoObj);
        mDemoObj->makeActorDead();
    }

    MR::addEffectHitNormal(this, nullptr);
    MR::getJMapInfoArg0NoInit(rIter, &mObjArg0);

    if (mObjArg0 == -1) {
        MR::declareCoin(this, 1);
    }

    TVec3f offset(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, MR::getJointMtx(this, pNames + 64), offset);
    initNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait), 0);
    makeActorAppeared();
}

void NeedlePlant::initAfterPlacement() {
    NameObj::initAfterPlacement();
    MR::calcGravity(this);
}

void NeedlePlant::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }

    updateYoshiLockOn();

    if (MR::attachSupportTicoToTarget(this)) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE_ANON(NeedlePlantNrvSupportFreeze));
    }
}

void NeedlePlant::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shake");
        MR::startActionSound(this, "OjLeavesSwing");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait));
        return;
    }

    updateYoshiLockOn();

    if (MR::isGreaterStep(this, 30)) {
        if (MR::attachSupportTicoToTarget(this)) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE_ANON(NeedlePlantNrvSupportFreeze));
        }
    }
}

void NeedlePlant::exeSupportFreeze() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SupportFreeze");
    }

    updateYoshiLockOn();

    if (!MR::attachSupportTicoToTarget(this)) {
        setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait));
    }
}

void NeedlePlant::exeEatStart() {
    if (MR::isOnGroundPlayer() || MR::isPlayerSwimming()) {
        TVec3f* pPos = MR::getPlayerPos();
        mTargetPos.x = pPos->x;
        mTargetPos.y = pPos->y;
        mTargetPos.z = pPos->z;
        MR::invalidateClipping(this);
        setNerve(GET_NERVE_ANON(NeedlePlantNrvEatPull));
    }
}

void NeedlePlant::exeEatPull() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pull");
    }

    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, nullptr);

    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, dir);

    mtx.getEuler(mRotation);

    mRotation.x *= 57.2957763671875f;
    mRotation.y *= 57.2957763671875f;
    mRotation.z *= 57.2957763671875f;

    f32 distance = PSVECDistance(&mTargetPos, &mPosition);
    mUnkA8 = distance;
    bool far = !MR::isNearPlayer(this, 75.0f + distance);
    if (far) {
        tryAppearDemoObj();
        MR::tryHitReaction(this, "ヨッシー舌引き抜き");
        MR::fn_80422310();
        setNerve(GET_NERVE_ANON(NeedlePlantNrvEaten));
    }
}

void NeedlePlant::exeEatPullEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PullEnd");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait));
    } else {
        updateYoshiLockOn();
    }
}

void NeedlePlant::exeBroken() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        tryAppearDemoObj();
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
    }
}

void NeedlePlant::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isAttackable()) {
        return;
    }

    f32 distance = MR::calcDistance(pSender, pReceiver, nullptr);
    f32 radius = MR::getSensorRadius(pReceiver);
    if (distance > 70.0f * mScale.x + radius) {
        return;
    }

    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
            setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvShake));
            return;
        }
    }

    if (MR::isSensorPlayerOrRide(pReceiver) || MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        f32 radius = MR::getSensorRadius(getSensor(nullptr));
        MR::setSensorRadius(getSensor(nullptr), 70.0f * mScale.x);
        MR::sendMsgPush(pReceiver, pSender);
        MR::setSensorRadius(getSensor(nullptr), radius);
    }
}

bool NeedlePlant::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvShake))) {
        return false;
    }

    if (!MR::isSensorEnemy(pSender)) {
        return false;
    }

    if (!isNotEating()) {
        return false;
    }

    setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvShake));
    return true;
}

bool NeedlePlant::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNotEating()) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        if (!isNerve(GET_NERVE_ANON(NeedlePlantNrvSupportFreeze))) {
            setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvShake));
        }

        return true;
    }

    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgFireBallAttack(msg) || MR::isMsgRockAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        breakNeedle();
        return true;
    }

    return false;
}

bool NeedlePlant::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNotEating()) {
        return false;
    }

    breakNeedle();
    return true;
}

bool NeedlePlant::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isNotEating()) {
            if (mIsFly) {
                setNerve(GET_NERVE_ANON(NeedlePlantNrvEaten));
                return true;
            }

            MR::tryHitReaction(this, "ヨッシー舌アクセス");
            MR::invalidateClipping(this);
            setNerve(GET_NERVE_ANON(NeedlePlantNrvEatStart));
            return true;
        }
    }

    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }

    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        if (isNerve(GET_NERVE_ANON(NeedlePlantNrvEatPull))) {
            MR::fn_804222D0();
            setNerve(GET_NERVE_ANON(NeedlePlantNrvEatPullEnd));
            return true;
        }

        if (isNerve(GET_NERVE_ANON(NeedlePlantNrvEaten))) {
            MR::tryHitReaction(this, "ヨッシー捕食");
            breakNeedle();
            return true;
        }

        MR::fn_804222D0();
        setNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait));
        return true;
    }

    return false;
}

bool NeedlePlant::tryAppearDemoObj() {
    if (!MR::isDead(mDemoObj)) {
        return false;
    }

    mDemoObj->appear();
    MR::startAction(mDemoObj, "Appear");
    return true;
}

bool NeedlePlant::isNotEating() {
    if (isNerve(GET_NERVE_ANON(NeedlePlantNrvEaten)) || isNerve(GET_NERVE_ANON(NeedlePlantNrvEatPull)) ||
        isNerve(GET_NERVE_ANON(NeedlePlantNrvEatStart))) {
        return false;
    }

    return true;
}

bool NeedlePlant::isAttackable() {
    if (isNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvWait)) || isNerve(GET_NERVE(NeedlePlant, NeedlePlantNrvShake)) ||
        isNerve(GET_NERVE_ANON(NeedlePlantNrvEatStart)) || isNerve(GET_NERVE_ANON(NeedlePlantNrvEatPullEnd))) {
        return true;
    }

    return false;
}

void NeedlePlant::updateYoshiLockOn() {
    if (mIsFly) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    } else {
        YoshiFunction::tryYoshiPull(mYoshiLockOn);
    }
}

void NeedlePlant::breakNeedle() {
    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "OjNeedlePlantBreak");

    if (!isNerve(GET_NERVE_ANON(NeedlePlantNrvEaten))) {
        if (mObjArg0 == 0) {
            MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
        } else if (mObjArg0 == -1) {
            MR::appearCoinPop(this, mPosition, 1);
        }
    }

    if (mDemoObj != nullptr) {
        setNerve(GET_NERVE_ANON(NeedlePlantNrvBroken));
        return;
    }

    kill();
}

NeedlePlant::~NeedlePlant() {}
