#include "Game/Enemy/AttackRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"

#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

namespace NrvAttackRockTutorial {
    class AttackRockTutorialNrvDead : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvDead sInstance;
    };
    class AttackRockTutorialNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvWait sInstance;
    };
    class AttackRockTutorialNrvFloatUp : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvFloatUp sInstance;
    };
    class AttackRockTutorialNrvFloat : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvFloat sInstance;
    };
    class AttackRockTutorialNrvBound : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvBound sInstance;
    };
    class AttackRockTutorialNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockTutorialNrvBreak sInstance;
    };
    AttackRockTutorialNrvWait AttackRockTutorialNrvWait::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvFloatUp AttackRockTutorialNrvFloatUp::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvFloat AttackRockTutorialNrvFloat::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvBound AttackRockTutorialNrvBound::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvBreak AttackRockTutorialNrvBreak::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvDead AttackRockTutorialNrvDead::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRockTutorial


AttackRockTutorial::AttackRockTutorial(const char* pName)
    : LiveActor(pName), mProjmapEffectMtxSetter(nullptr), mBreakModel(nullptr), mCameraInfo(nullptr), mRide(nullptr), mStartPos(0.0f, 0.0f, 0.0f),
      mUpVec(0.0f, 0.0f, 0.0f), mRate(0.0f), mTouchCount(0), mTouched(false), mUseCamera(false) {
    mMtx90.identity();
    mQuatD4.set(0.0f, 0.0f, 0.0f, 1.0f);
}

#pragma auto_inline off
// NONMATCHING: Negation helper inlining differs.
void AttackRockTutorial::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatD4, this);
    mStartPos.set(mPosition);
    MR::calcUpVec(&mUpVec, this);
    TVec3f gravity;
    JMathInlineVEC::PSVECNegate(&mUpVec, &gravity);
    mGravity.set(gravity);
    mProjmapEffectMtxSetter = fn_80155660(this);
    MR::setEffectHostMtx(this, "Attack", mMtx90.toMtxPtr());
    MR::setSensorFollowPos(this, "Land", &mStartPos);
    fn_801552A0(this);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    MR::setShadowProjection(this, nullptr, mStartPos, mUpVec, true);
    mBreakModel = MR::createBreakModel(this, nullptr);
    MR::invalidateClipping(mBreakModel);
    mHillModel = MR::createPartsModelMapObj2(this, "\220\267\202\350\223y\203\202\203f\203\213", "AttackRockHill", nullptr);
    mHillModel->makeActorDead();
    mRide = new AttackRockRide(this);
    BaseMatrixFollowValidater* pValidater = MR::createBaseMatrixFollowValidateDelegator(this, &AttackRockTutorial::fn_80155090);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\215U\214\202\222\206");
    initNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance, 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}
#pragma auto_inline on

void AttackRockTutorial::makeActorAppeared() {
    if (!isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvDead::sInstance)) {
        LiveActor::makeActorAppeared();
    }
}

void AttackRockTutorial::makeActorDead() {
    LiveActor::makeActorDead();
    mRide->fn_80152F20();
}

void AttackRockTutorial::control() {
    mRide->fn_80152C00();
    if (!mTouched) {
        mTouchCount = 0;
    }
    mTouched = false;
}

void AttackRockTutorial::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    fn_80155540(mProjmapEffectMtxSetter, &mQuatD4, &mPosition);
}

void AttackRockTutorial::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
        isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBound::sInstance) && MR::sendMsgAttackRockReflect(pReceiver, pSender)) {
        setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
    }
}

bool AttackRockTutorial::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance) && MR::isGreaterStep(this, 60)) {
        fn_80155360(&mVelocity, &mQuatD4, pSender, pReceiver, 120.0f);
        mUseCamera = fn_801552E0(this, &mQuatD4) <= 70.0f;
        setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRockTutorial::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mRide->fn_80152E20(msg, pSender, pReceiver)) {
        return true;
    }
    if (MR::isMsgFloorTouch(msg)) {
        mTouchCount++;
        if (mTouchCount > 10) {
            mTouchCount = 10;
        }
        mTouched = true;
        return true;
    }
    return false;
}

void AttackRockTutorial::fn_80154B60() {
    this->makeActorDead();
    this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvDead::sInstance);
}

void AttackRockTutorial::fn_80154BB0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::offBind(this);
    }
    if (MR::isGreaterStep(this, 60)) {
        this->mRate = -400.0f;
        TVec3f pos = this->mStartPos + this->mUpVec * -400.0f;
        MR::setPosition(this, pos);
        MR::resetPosition(this);
        this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloatUp::sInstance);
    }
}

void AttackRockTutorial::fn_80154C60() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::validateCollisionParts(this);
        MR::validateHitSensors(this);
        MR::startAction(this, "Appear");
    }
    MR::startActionSound(this, "FloatUp", -1, -1, -1);
    this->mRate = MR::calcNerveEaseOutValue(this, 0, 60, -400.0f, 0.0f);
    TVec3f pos = this->mStartPos + this->mUpVec * this->mRate;
    MR::setPosition(this, pos);
    if (MR::isGreaterStep(this, 60)) {
        this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance);
    }
}

void AttackRockTutorial::fn_80154D40() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Float");
        MR::validateShadow(this, nullptr);
        this->mHillModel->makeActorDead();
        this->mRide->fn_80152F10();
    }
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(this)) {
        this->mRide->fn_80152F10();
    }
    this->mRate = MR::calcNerveEaseInOutValue(this, 0, 120, 0.0f, 800.0f);
    TVec3f front;
    MR::calcFrontVec(&front, this);
    this->mRide->fn_801529E0(&front);
    f32 max = MR::getBrkFrameMax(this);
    MR::setBrkFrameAndStop(this, this->mRate * max / 800.0f);
    TVec3f pos = this->mStartPos + this->mUpVec * this->mRate;
    MR::setPosition(this, pos);
    MR::startActionSound(this, "Power", -1, -1, -1);
    if (MR::isStep(this, 120)) {
        MR::onBind(this);
    }
}

void AttackRockTutorial::fn_80154E90() {
    if (MR::isFirstStep(this)) {
        if (this->mUseCamera && this->mCameraInfo != nullptr) {
            MR::startMultiActorCameraTargetSelf(this, this->mCameraInfo, "\215U\214\202\222\206", -1);
        }
        MR::startAction(this, "Attack");
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::invalidateCollisionParts(this);
        this->mRide->fn_80152F20();
    }
    fn_801554D0(&this->mMtx90, this);
    if (MR::isBinded(this)) {
        MR::sendMsgToBindedAttackRockReflect(this, getSensor("Body"));
        this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 65)) {
        this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
    }
}

void AttackRockTutorial::fn_80154F90() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        this->mBreakModel->appear();
        MR::startBrk(this->mBreakModel, "Inner");
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        MR::invalidateShadow(this, nullptr);
        MR::zeroVelocity(this);
        this->mRide->fn_80152F20();
    }
    if (MR::isGreaterStep(this, 10) && MR::isDead(this->mBreakModel)) {
        if (this->mUseCamera && this->mCameraInfo != nullptr) {
            MR::endMultiActorCamera(this, this->mCameraInfo, "\215U\214\202\222\206", false, -1);
        }
        this->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvWait::sInstance);
    }
}

bool AttackRockTutorial::fn_80155090(s32 arg) const {
    const AttackRockTutorial* pActor = this;
    if (pActor->isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance)) {
        if (arg == 1) {
            return MR::isGreaterStep(pActor, 60);
        }
        return true;
    }
    return false;
}

AttackRockTutorial::~AttackRockTutorial() {
}

namespace NrvAttackRockTutorial {
    inline void AttackRockTutorialNrvDead::execute(Spine* pSpine) const {
    }
    inline void AttackRockTutorialNrvWait::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor))->fn_80154BB0();
    }
    inline void AttackRockTutorialNrvFloatUp::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor))->fn_80154C60();
    }
    inline void AttackRockTutorialNrvFloat::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor))->fn_80154D40();
    }
    inline void AttackRockTutorialNrvBound::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor))->fn_80154E90();
    }
    inline void AttackRockTutorialNrvBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor))->fn_80154F90();
    }
};  // namespace NrvAttackRockTutorial

// ---- shared helpers ----

void fn_801552A0(LiveActor* pActor) {
    MR::setBinderIgnoreMovingCollision(pActor);
    MR::setBinderExceptSensorType(pActor, 60);
    MR::offBind(pActor);
}

#pragma auto_inline off
// NONMATCHING: Negation helper remains out of line instead of paired-single expansion.
f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat) {
    TVec3f dir;
    MR::normalizeOrZero(&dir, pActor->mVelocity);
    TVec3f zdir;
    pQuat->getZDir(zdir);
    TVec3f negZ;
    JMathInlineVEC::PSVECNegate(&zdir, &negZ);
    return 57.29578f * MR::diffAngleAbs(dir, negZ);
}
#pragma auto_inline on

void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed) {
    TVec3f zdir;
    pQuat->getZDir(zdir);
    fn_801553D0(pOut, &zdir, pSender, pReceiver, speed);
}

#pragma auto_inline off
// NONMATCHING: Vector temporary stack slots and paired-single/GPR allocation differ.
void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed) {
    TVec3f sensorDir;
    MR::calcSensorDirectionNormalize(&sensorDir, pSender, pReceiver);
    TVec3f jump = sensorDir * -10.0f;
    MR::jumpPlayer(jump);
    TVec3f dir = *pDir * 0.5f;
    TVec3f result;
    TVec3f difference(sensorDir);
    JMathInlineVEC::PSVECSubtract(&difference, &dir, &difference);
    result.set(difference);
    if (MR::normalizeOrZero(&result)) {
        result.set(sensorDir);
    }
    TVec3f vel = result * speed;
    pOut->set(vel);
}
#pragma auto_inline on

void fn_801554D0(TPos3f* pMtx, LiveActor* pActor) {
    TVec3f dir;
    if (!MR::normalizeOrZero(&dir, pActor->mVelocity)) {
        MR::turnMtxToYDirRate(pMtx, pActor->mVelocity, 1.0f);
    }
    pMtx->setTrans(pActor->mPosition);
}

// NONMATCHING: Quaternion-matrix scheduling and paired-single copy registers differ.
void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos) {
    TPos3f mtx;
    mtx.setQT(*pQuat, *pPos);
    mtx.invert(mtx);
    pSetter->mEffectMtx.set(mtx);
}

ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor) {
    ProjmapEffectMtxSetter* pSetter = MR::initDLMakerProjmapEffectMtxSetter(pActor);
    MR::newDifferedDLBuffer(pActor);
    pSetter->updateMtxUseBaseMtx();
    return pSetter;
}
