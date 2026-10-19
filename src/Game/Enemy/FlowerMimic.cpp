#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Enemy/FlowerMimic.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"

namespace {
    // Nerve objects of FlowerMimic, declared in the sinit order of the target.
#define FLOWER_MIMIC_NERVE(name, fn)                                                                                                              \
    class name : public Nerve {                                                                                                                  \
    public:                                                                                                                                      \
        virtual void execute(Spine* pSpine) const {                                                                                              \
            FlowerMimic* actor = reinterpret_cast< FlowerMimic* >(pSpine->mExecutor);                                                            \
            actor->fn();                                                                                                                         \
        };                                                                                                                                       \
        static name sInstance;                                                                                                                   \
    };                                                                                                                                           \
    name name::sInstance ATTRIBUTE_WEAK;

    FLOWER_MIMIC_NERVE(FlowerMimicNrvWait, exeMimicWait);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttackSign, exeMimicAttackSign);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttack, exeMimicAttack);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttackEnd, exeMimicAttackEnd);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvEat, exeMimicEat);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvEatEnd, exeMimicEatEnd);

    class FlowerMimicNrvBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &FlowerMimicNrvWait::sInstance);
        };
        static FlowerMimicNrvBind sInstance;
    };
    FlowerMimicNrvBind FlowerMimicNrvBind::sInstance ATTRIBUTE_WEAK;
}  // namespace

FlowerMimic::FlowerMimic(const char* pName) : LiveActor(pName) {
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mOffset.set(0.0f, 0.0f, 0.0f);
    mPrevOffset.set(0.0f, 0.0f, 0.0f);
    mInitPos.set(0.0f, 0.0f, 0.0f);
    mScaleCtrl = nullptr;
    mSpring = nullptr;
    mBindStarPointer = nullptr;
    mJointCtrl = nullptr;
    mCollisionObj = nullptr;
    mTimer = 0;
    mAngle = 0.0f;
    mFloorTouchFlag = false;
}

FlowerMimic::~FlowerMimic() {}

void FlowerMimic::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "FlowerMimic", false);
    mInitPos.set(mPosition);

    mScaleCtrl = new AnimScaleController(nullptr);

    mJointCtrl = MR::createJointDelegatorWithNullChildFunc(this, &FlowerMimic::rotateJointByAngle, "LeafCenter");

    mSpring = new SpringValue();
    mSpring->setParam(0.0f, 0.0f, 0.05f, 0.88f, 0.0f);

    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);

    mCollisionObj = MR::createCollisionObj(this, "FlowerMimicUnder", getSensor("Collision"), nullptr);
    // CollisionObj is laid out as a LiveActor in the target (setScale is applied to it directly).
    MR::setScale(reinterpret_cast< LiveActor* >(mCollisionObj), mScale);

    initNerve(&FlowerMimicNrvWait::sInstance, 1);
    MR::initActorState(this, mBindStarPointer, &FlowerMimicNrvBind::sInstance, "SupportFreeze");

    if (MR::isInPlaneModeCube(mPosition)) {
        MR::invalidateShadow(this, nullptr);
    }
    makeActorAppeared();
}

// NONMATCHING: register allocation only (target keeps the inlined operator+ operand addresses in callee-saved registers)
// NONMATCHING: inlined TVec3 operator+ keeps &ret/&op in callee-saved regs (r31/r30) across the copy ctor in the target; ours rematerializes them in volatile regs (regalloc only).
void FlowerMimic::control() {
    mScaleCtrl->updateNerve();

    if (mFloorTouchFlag) {
        if (mTimer == 0) {
            mSpring->mVelocity += -8.0f;
        }
        mTimer = 15;
    } else {
        mTimer = mTimer - 1;
        if (mTimer < 0) {
            mTimer = 0;
        }
    }
    mFloorTouchFlag = false;

    if (isNotEatingNerve()) {
        if (mBindStarPointer->tryStartPointBind()) {
            MR::validateCollisionParts(this);
            setNerve(&FlowerMimicNrvBind::sInstance);
            return;
        }
    }

    mAngle = MR::repeatDegree(mAngle + 12.0f);

    mSpring->update();
    f32 offset = mSpring->mSpringValue;
    TVec3f up;
    MR::calcUpVec(&up, this);
    mPosition.set(mInitPos + up * offset);
}

void FlowerMimic::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleCtrl);
    if (mJointCtrl != nullptr) {
        mJointCtrl->registerCallBack();
    }
}

bool FlowerMimic::rotateJointByAngle(TPos3f* pMtx, const JointControllerInfo&) {
    MR::rotateMtxLocalYDegree(pMtx->mMtx, mAngle);
    return true;
}

// NONMATCHING: 99.6% (register allocation / scheduling; see sg.py diff)
void FlowerMimic::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body")) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

// NONMATCHING: 99.2% (register allocation / scheduling; see sg.py diff)
bool FlowerMimic::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerTrample(msg)) {
            mScaleCtrl->startScaleVelocityY(-0.2f);
            MR::tryHitReaction(this, "\x93\xa5\x82\xdd");
            return true;
        }
        if (MR::isMsgPlayerHipDrop(msg)) {
            mScaleCtrl->startScaleVelocityY(-0.2f);
            MR::sendMsgAwayJump(pSender, pReceiver);
            MR::tryHitReaction(this, "\x93\xa5\x82\xdd");
            return true;
        }
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        mScaleCtrl->startHitReaction();
        return true;
    }
    return false;
}

// NONMATCHING: 98.3% (register allocation / scheduling; see sg.py diff)
bool FlowerMimic::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgFloorTouch(msg)) {
        mFloorTouchFlag = true;
    }
    if (MR::isMsgAutoRushBegin(msg)) {
        if (isNerve(&FlowerMimicNrvAttack::sInstance)) {
            MR::getPlayerQuatAndPos(&mQuat, &mOffset);
            mPrevOffset.set(mOffset);
            MR::calcPositionUpOffset(&mOffset, this, -75.0f);
            setNerve(&FlowerMimicNrvEat::sInstance);
            return true;
        }
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerQuatAndPos(&mQuat, &mOffset);
        return true;
    }
    return false;
}

bool FlowerMimic::isNotEatingNerve() const {
    if (isNerve(&FlowerMimicNrvEat::sInstance) || isNerve(&FlowerMimicNrvEatEnd::sInstance) || isNerve(&FlowerMimicNrvBind::sInstance)) {
        return false;
    }
    return true;
}

void FlowerMimic::exeMimicWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
        MR::validateCollisionParts(this);
    }
    if (mTimer > 0) {
        MR::invalidateClipping(this);
        setNerve(&FlowerMimicNrvAttackSign::sInstance);
    }
}

void FlowerMimic::exeMimicAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(&FlowerMimicNrvAttack::sInstance);
    }
}

void FlowerMimic::exeMimicAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        mTimer = 0;
    }
    if (MR::isStep(this, 5)) {
        MR::invalidateCollisionParts(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvAttackEnd::sInstance);
    }
}

void FlowerMimic::exeMimicAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
        MR::validateCollisionParts(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvWait::sInstance);
    }
}

void FlowerMimic::exeMimicEat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Eat");
        MR::hidePlayer();
    }
    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvEatEnd::sInstance);
    }
}

// NONMATCHING: register allocation only (target keeps the inlined operator- operand addresses in callee-saved registers)
// NONMATCHING: same inlined TVec3 operator-/operator+ pointer-register residue as control().
void FlowerMimic::exeMimicEatEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "EatEnd");
        mPrevOffset.set(mOffset);
        MR::showPlayer();
    }

    TVec3f offset;
    MR::calcPositionUpOffset(&offset, this, 200.0f);
    mOffset.set(offset);

    if (MR::isStep(this, 1)) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f sp2C;
        sp2C.set< f32 >(2.0f * (mQuat.x * mQuat.z) + 2.0f * (mQuat.w * mQuat.y),
                       2.0f * (mQuat.y * mQuat.z) - 2.0f * (mQuat.w * mQuat.x),
                       (1.0f - 2.0f * (mQuat.x * mQuat.x)) - 2.0f * (mQuat.y * mQuat.y));
        TVec3f sp20 = (up - sp2C * 0.1f) * 80.0f;
        MR::restrictVelocityToPlaneModeArea(&sp20, mPosition);
        MR::endBindAndPlayerDamage(this, sp20);
        MR::validateCollisionParts(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvWait::sInstance);
    }
}
