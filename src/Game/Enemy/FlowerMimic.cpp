#include "Game/Enemy/FlowerMimic.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include <cmath>

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

    FLOWER_MIMIC_NERVE(FlowerMimicNrvWait, fn_801810D0);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttackSign, fn_80181150);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttack, fn_801811C0);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvAttackEnd, fn_80181250);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvEat, fn_801812C0);
    FLOWER_MIMIC_NERVE(FlowerMimicNrvEatEnd, fn_80181320);

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

// NONMATCHING: 72.4% (ctor store order / register allocation)
FlowerMimic::FlowerMimic(const char* pName) : LiveActor(pName) {
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
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
    mFloorTouchFlag = 0;
}

FlowerMimic::~FlowerMimic() {}

// NONMATCHING: 78.7% (register allocation / scheduling; see sg.py diff)
void FlowerMimic::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "FlowerMimic", false);
    mInitPos = mPosition;

    mScaleCtrl = new AnimScaleController(nullptr);

    mJointCtrl = new JointControlDelegator< FlowerMimic >(this, &FlowerMimic::fn_80180DE0, nullptr);
    MR::setJointControllerParam(mJointCtrl, this, "LeafCenter");

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

// NONMATCHING: 57.5% (register allocation / scheduling; see sg.py diff)
void FlowerMimic::control() {
    mScaleCtrl->updateNerve();

    if (mFloorTouchFlag != 0) {
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
    mFloorTouchFlag = 0;

    if (fn_80181060()) {
        if (mBindStarPointer->tryStartPointBind()) {
            MR::validateCollisionParts(this);
            setNerve(&FlowerMimicNrvBind::sInstance);
        }
    }

    f32 angle = mAngle + 12.0f;
    mAngle = fmod(angle, 360.0f);

    mSpring->update();
    mPosition = mInitPos;
}

void FlowerMimic::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleCtrl);
    if (mJointCtrl != nullptr) {
        mJointCtrl->registerCallBack();
    }
}

bool FlowerMimic::fn_80180DE0(TPos3f* pMtx, const JointControllerInfo&) {
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
        mFloorTouchFlag = 1;
    }
    if (MR::isMsgAutoRushBegin(msg)) {
        if (isNerve(&FlowerMimicNrvAttack::sInstance)) {
            MR::fn_800514B0(&mQuat, &mOffset);
            mPrevOffset = mOffset;
            MR::calcPositionUpOffset(&mOffset, this, -75.0f);
            setNerve(&FlowerMimicNrvEat::sInstance);
            return true;
        }
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::fn_800513C0(&mQuat, &mOffset);
        return true;
    }
    return false;
}

bool FlowerMimic::fn_80181060() const {
    if (isNerve(&FlowerMimicNrvEat::sInstance) || isNerve(&FlowerMimicNrvEatEnd::sInstance) || isNerve(&FlowerMimicNrvBind::sInstance)) {
        return false;
    }
    return true;
}

void FlowerMimic::fn_801810D0() {
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

void FlowerMimic::fn_80181150() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(&FlowerMimicNrvAttack::sInstance);
    }
}

void FlowerMimic::fn_801811C0() {
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

void FlowerMimic::fn_80181250() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
        MR::validateCollisionParts(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvWait::sInstance);
    }
}

void FlowerMimic::fn_801812C0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Eat");
        MR::hidePlayer();
    }
    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvEatEnd::sInstance);
    }
}

void FlowerMimic::fn_80181320() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "EatEnd");
        mPrevOffset = mOffset;
        MR::showPlayer();
    }

    TVec3f offset;
    MR::calcPositionUpOffset(&offset, this, 200.0f);
    mOffset = offset;

    if (MR::isStep(this, 1)) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f sp2C(2.0f * (mQuat.x * mQuat.z) + 2.0f * (mQuat.w * mQuat.y),
                    2.0f * (mQuat.y * mQuat.z) - 2.0f * (mQuat.w * mQuat.x),
                    (1.0f - 2.0f * (mQuat.x * mQuat.x)) - 2.0f * (mQuat.y * mQuat.y));
        TVec3f sp20 = sp2C * 80.0f;
        MR::fn_800145D0(&sp20, mPosition);
        MR::endBindAndPlayerDamage(this, sp20);
        MR::validateCollisionParts(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(&FlowerMimicNrvWait::sInstance);
    }
}
