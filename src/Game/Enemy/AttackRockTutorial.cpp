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

class BaseMatrixFollowValidater;
class KoopaBattleAccessor {
public:
    void addAttackRockFinal(AttackRockFinal*);
};

namespace MR {
    bool isMsgKoopaCastAttackRockStart(u32);
    bool isMsgKoopaCastAttackRockEnd(u32);
    bool isMsgKoopaPunchAttack(u32);
    bool isMsgKoopaReleaseAttackRock(u32);
    bool isMsgKoopaFinalBattleStart(u32);
    bool isMsgAutoRushBegin(u32);
    bool isMsgUpdateBaseMtx(u32);
    bool isMsgRushCancel(u32);
    bool isMsgFloorTouch(u32);
    f32 calcNerveEaseOutValue(const NerveExecutor*, s32, s32, f32, f32);
    void createKoopaBattleAccessor();
    KoopaBattleAccessor* getKoopaBattleAccessor();
};  // namespace MR

// Unnamed SMG2 helpers outside this unit (Koopa battle functions).
extern "C" void fn_800CB710(HitSensor*);
extern "C" void fn_800CB090(TVec3f*);
extern "C" void fn_800CB6D0(TVec3f*);
extern "C" f32 fn_800CB410(const TVec3f*);
extern "C" void fn_800CAF80(TVec3f*, const TVec3f*);
extern "C" void fn_800CAFE0(TVec3f*, const TVec3f*);
extern "C" void fn_800CB460(TVec3f*);
extern "C" void fn_800CC210(HitSensor*, HitSensor*);
extern "C" bool fn_800CC1F0(HitSensor*, HitSensor*);
extern "C" void fn_800CC220(LiveActor*, HitSensor*);
extern "C" bool fn_800CC230(LiveActor*, HitSensor*);
extern "C" bool fn_800CC270(u32);
extern "C" bool fn_800CC280(u32);
extern "C" bool fn_800CC290(u32);
extern "C" bool fn_800CC2B0(u32);
extern "C" void fn_800E4420(void*);
extern "C" void fn_800E4700(const TVec3f*);
extern "C" void fn_80010020(LiveActor*, const TVec3f*, LiveActor*);
extern "C" void fn_800513C0(TQuat4f*, TVec3f*);
extern "C" TVec3f* fn_800514B0(TQuat4f*, TVec3f*);
extern "C" void fn_8000E850(TVec3f*, const LiveActor*);

// Unit-local helpers (unnamed in the target).
extern "C" {
f32 fn_801510C0() NO_INLINE;
bool fn_801510D0(LiveActor*) NO_INLINE;
void fn_80151160(AttackRock*) NO_INLINE;
bool fn_801517C0(LiveActor*, s32) NO_INLINE;
void fn_80151850(void*) NO_INLINE;
void fn_801552A0(LiveActor*) NO_INLINE;
void fn_80155360(TVec3f*, TQuat4f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801553D0(TVec3f*, TVec3f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801554D0(TPos3f*, LiveActor*) NO_INLINE;
void fn_80155540(ProjmapEffectMtxSetter*, TQuat4f*, TVec3f*) NO_INLINE;
ProjmapEffectMtxSetter* fn_80155660(LiveActor*) NO_INLINE;
void fn_80152F10(AttackRockRide*) NO_INLINE;
void fn_80152F20(AttackRockRide*) NO_INLINE;
bool fn_80152E20(AttackRockRide*, u32, HitSensor*, HitSensor*) NO_INLINE;
void fn_80152A00(AttackRockRide*) NO_INLINE;
void fn_80152C00(AttackRockRide*) NO_INLINE;
void fn_801529E0(AttackRockRide*, const TVec3f*) NO_INLINE;
AttackRockRide* fn_801528E0(AttackRockRide*, LiveActor*) NO_INLINE;
bool fn_80153400(AttackRockStateHold*) NO_INLINE;
bool fn_80153460(AttackRockStateHold*) NO_INLINE;
f32 fn_801552E0(LiveActor*, TQuat4f*) NO_INLINE;
}

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
    AttackRockTutorialNrvDead AttackRockTutorialNrvDead::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvWait AttackRockTutorialNrvWait::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvFloatUp AttackRockTutorialNrvFloatUp::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvFloat AttackRockTutorialNrvFloat::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvBound AttackRockTutorialNrvBound::sInstance ATTRIBUTE_WEAK;
    AttackRockTutorialNrvBreak AttackRockTutorialNrvBreak::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRockTutorial

extern "C" {
void fn_80154B60(AttackRockTutorial*) NO_INLINE;
void fn_80154BB0(AttackRockTutorial*) NO_INLINE;
void fn_80154C60(AttackRockTutorial*) NO_INLINE;
void fn_80154D40(AttackRockTutorial*) NO_INLINE;
void fn_80154E90(AttackRockTutorial*) NO_INLINE;
void fn_80154F90(AttackRockTutorial*) NO_INLINE;
}

AttackRockTutorial::AttackRockTutorial(const char* pName)
    : LiveActor(pName), mProjmapEffectMtxSetter(nullptr), mBreakModel(nullptr), mCameraInfo(nullptr), mRide(nullptr), mStartPos(0.0f, 0.0f, 0.0f),
      mUpVec(0.0f, 0.0f, 0.0f), mRate(0.0f), mTouchCount(0), mTouched(false), mUseCamera(false) {
    mMtx90.identity();
    mQuatD4.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void AttackRockTutorial::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AttackRock", true);
    MR::makeQuatFromRotate(&mQuatD4, this);
    mStartPos.set(mPosition);
    MR::calcUpVec(&mUpVec, this);
    TVec3f gravity = -mUpVec;
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
    mRide = new AttackRockRide;
    fn_801528E0(mRide, this);
    BaseMatrixFollowValidater* pValidater = nullptr;
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\215U\214\202\222\206");
    initNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloatUp::sInstance, 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void AttackRockTutorial::makeActorAppeared() {
    if (!isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvDead::sInstance)) {
        LiveActor::makeActorAppeared();
    }
}

void AttackRockTutorial::makeActorDead() {
    LiveActor::makeActorDead();
    fn_80152F20(mRide);
}

void AttackRockTutorial::control() {
    fn_80152C00(mRide);
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
        isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBound::sInstance) && fn_800CC1F0(pReceiver, pSender)) {
        setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
    }
}

bool AttackRockTutorial::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance) && MR::isGreaterStep(this, 60)) {
        fn_80155360(&mVelocity, &mQuatD4, pSender, pReceiver, 120.0f);
        mUseCamera = fn_801552E0(this, &mQuatD4) < 70.0f;
        setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBound::sInstance);
        return true;
    }
    return false;
}

bool AttackRockTutorial::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_80152E20(mRide, msg, pSender, pReceiver)) {
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

extern "C" void fn_80154B60(AttackRockTutorial* pActor) {
    pActor->kill();
    pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvDead::sInstance);
}

extern "C" void fn_80154BB0(AttackRockTutorial* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Wait");
        MR::offBind(pActor);
    }
    if (MR::isGreaterStep(pActor, 60)) {
        pActor->mRate = -400.0f;
        TVec3f pos = pActor->mUpVec * -400.0f + pActor->mStartPos;
        MR::setPosition(pActor, pos);
        MR::resetPosition(pActor);
        pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloatUp::sInstance);
    }
}

extern "C" void fn_80154C60(AttackRockTutorial* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::showModel(pActor);
        MR::validateCollisionParts(pActor);
        MR::validateHitSensors(pActor);
        MR::startAction(pActor, "Appear");
    }
    MR::startActionSound(pActor, "FloatUp", -1, -1, -1);
    pActor->mRate = MR::calcNerveEaseOutValue(pActor, 0, 60, -400.0f, 0.0f);
    TVec3f pos = pActor->mUpVec * pActor->mRate + pActor->mStartPos;
    MR::setPosition(pActor, pos);
    if (MR::isGreaterStep(pActor, 60)) {
        pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvFloat::sInstance);
    }
}

extern "C" void fn_80154D40(AttackRockTutorial* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Float");
        MR::validateShadow(pActor, nullptr);
        pActor->mHillModel->kill();
        fn_80152F10(pActor->mRide);
    }
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(pActor)) {
        fn_80152F10(pActor->mRide);
    }
    pActor->mRate = MR::calcNerveEaseInOutValue(pActor, 0, 120, 0.0f, 800.0f);
    TVec3f front;
    MR::calcFrontVec(&front, pActor);
    fn_801529E0(pActor->mRide, &front);
    f32 max = MR::getBrkFrameMax(pActor);
    MR::setBrkFrameAndStop(pActor, pActor->mRate * max / 800.0f);
    TVec3f pos = pActor->mUpVec * pActor->mRate + pActor->mStartPos;
    MR::setPosition(pActor, pos);
    MR::startActionSound(pActor, "Power", -1, -1, -1);
    if (MR::isStep(pActor, 120)) {
        MR::onBind(pActor);
    }
}

extern "C" void fn_80154E90(AttackRockTutorial* pActor) {
    if (MR::isFirstStep(pActor)) {
        if (pActor->mUseCamera && pActor->mCameraInfo != nullptr) {
            MR::startMultiActorCameraTargetSelf(pActor, pActor->mCameraInfo, "\215U\214\202\222\206", -1);
        }
        MR::startAction(pActor, "Attack");
        MR::onBind(pActor);
        MR::onCalcShadow(pActor, nullptr);
        MR::invalidateCollisionParts(pActor);
        fn_80152F20(pActor->mRide);
    }
    fn_801554D0(&pActor->mMtx90, pActor);
    if (MR::isBinded(pActor)) {
        fn_800CC220(pActor, pActor->getSensor("Body"));
        pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
        return;
    }
    if (MR::isGreaterStep(pActor, 65)) {
        pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvBreak::sInstance);
    }
}

extern "C" void fn_80154F90(AttackRockTutorial* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Break");
        pActor->mBreakModel->appear();
        MR::startBrk(pActor->mBreakModel, "Inner");
        MR::hideModelAndOnCalcAnim(pActor);
        MR::invalidateCollisionParts(pActor);
        MR::invalidateHitSensors(pActor);
        MR::invalidateShadow(pActor, nullptr);
        MR::zeroVelocity(pActor);
        fn_80152F20(pActor->mRide);
    }
    if (MR::isGreaterStep(pActor, 10) && MR::isDead(pActor->mBreakModel)) {
        if (pActor->mUseCamera && pActor->mCameraInfo != nullptr) {
            MR::endMultiActorCamera(pActor, pActor->mCameraInfo, "\215U\214\202\222\206", false, -1);
        }
        pActor->setNerve(&NrvAttackRockTutorial::AttackRockTutorialNrvDead::sInstance);
    }
}

extern "C" bool fn_80155090(LiveActor* pActor, s32 arg) {
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
        fn_80154BB0(reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor));
    }
    inline void AttackRockTutorialNrvFloatUp::execute(Spine* pSpine) const {
        fn_80154C60(reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor));
    }
    inline void AttackRockTutorialNrvFloat::execute(Spine* pSpine) const {
        fn_80154D40(reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor));
    }
    inline void AttackRockTutorialNrvBound::execute(Spine* pSpine) const {
        fn_80154E90(reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor));
    }
    inline void AttackRockTutorialNrvBreak::execute(Spine* pSpine) const {
        fn_80154F90(reinterpret_cast< AttackRockTutorial* >(pSpine->mExecutor));
    }
};  // namespace NrvAttackRockTutorial

// ---- shared helpers ----

extern "C" void fn_801552A0(LiveActor* pActor) {
    MR::setBinderIgnoreMovingCollision(pActor);
    MR::setBinderExceptSensorType(pActor, 60);
    MR::offBind(pActor);
}

extern "C" f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat) {
    TVec3f dir;
    MR::normalizeOrZero(&dir, pActor->mVelocity);
    TVec3f zdir;
    pQuat->getZDir(zdir);
    TVec3f negZ = -zdir;
    return 57.29578f * MR::diffAngleAbs(dir, negZ);
}

extern "C" void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed) {
    TVec3f zdir;
    pQuat->getZDir(zdir);
    fn_801553D0(pOut, &zdir, pSender, pReceiver, speed);
}

// NONMATCHING: vector math from m2c, unverified.
extern "C" void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed) {
    TVec3f sensorDir;
    MR::calcSensorDirectionNormalize(&sensorDir, pSender, pReceiver);
    TVec3f jump = sensorDir * -10.0f;
    MR::jumpPlayer(jump);
    TVec3f dir(*pDir);
    TVec3f result;
    result.set(dir);
    if (MR::normalizeOrZero(&result)) {
        result.set(sensorDir);
    }
    TVec3f vel = result * speed;
    pOut->set(vel);
}

extern "C" void fn_801554D0(TPos3f* pMtx, LiveActor* pActor) {
    TVec3f dir;
    if (!MR::normalizeOrZero(&dir, pActor->mVelocity)) {
        MR::turnMtxToYDirRate(pMtx, dir, 1.0f);
    }
    pMtx->setTrans(pActor->mPosition);
}

// NONMATCHING: quaternion->matrix expansion order and TPos3f invert/psq copy differ.
extern "C" void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos) {
    TPos3f mtx;
    mtx.setQT(*pQuat, *pPos);
    mtx.invert(mtx);
}

extern "C" ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor) {
    ProjmapEffectMtxSetter* pSetter = MR::initDLMakerProjmapEffectMtxSetter(pActor);
    MR::newDifferedDLBuffer(pActor);
    pSetter->updateMtxUseBaseMtx();
    return pSetter;
}
