#pragma auto_inline off
#include "Game/Ride/RollingRock.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraHolder.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include <JSystem/JMath.hpp>
#include <cmath>

extern "C" {
void fn_80010D10(LiveActor*, const TVec3f&, f32);
bool fn_80012BA0(LiveActor*, HitSensor*, HitSensor*, f32);
void fn_8004FFA0();
void fn_8004FFB0();
bool fn_80050350();
bool fn_80051080();
void fn_800513C0(const TQuat4f*, const TVec3f*);
void fn_800514B0(const TQuat4f*, TVec3f*, LiveActor*);
void* fn_80114940(void*);
void fn_803F5E20(f32*);
}

#define sNameBroken "\x89\xf3\x82\xea\x83\x82\x83""f\x83\x8b"
#define sNameEffect "\x83G\x83t\x83""F\x83N\x83g\x83\x82\x83""f\x83\x8b"
#define sNameHammer "\x83p\x83""b\x83N\x83\x93\x83t\x83\x89\x83\x8f\x81[(\x83g\x83Q\x83n\x83\x93\x83}\x81[)"
#define sNameRock "\x8a\xe2\x83}\x83\x8a\x83I"
#define sNameParentState "\x8a\xe2\x83}\x83\x8a\x83I\x95\xcf\x90g\x8f\xf3\x91\xd4"
#define sNameRollState "\x8a\xe2\x83}\x83\x8a\x83I\x83S\x83\x8d\x83S\x83\x8d\x8f\xf3\x91\xd4"
#define sNameCameraTarget "\x83J\x83\x81\x83\x89\x83^\x81[\x83Q\x83""b\x83g(Rock)"
#define sRumblePushReflect "\x89\x9f\x82\xb3\x82\xea\x94\xbd\x8e\xcb"
#define sNameCage1 "\x82\xb2\x82\xeb\x8a\xe2\x94j\x89\xf3\x83P\x81[\x83W"
#define sNameCage2 "\x83\x8d\x83""b\x83N\x83P\x81[\x83W"
#define sRumbleBoardReflect "\x94\xbd\x8e\xcb\x94\xc2\x94\xbd\x8e\xcb"
#define sRumbleLand "\x92\x85\x92n"
#define sRumbleRollEnd "\x83S\x83\x8d\x83S\x83\x8d\x8fI\x97\xb9"
#define sRumbleRollCancel "\x83S\x83\x8d\x83S\x83\x8d\x83L\x83\x83\x83\x93\x83Z\x83\x8b"
#define sBckRollJump "\x83S\x83\x8d\x8a\xe2\x89\xf0\x8f\x9c\x83W\x83\x83\x83\x93\x83v"
#define sBckRollHitJump "\x83S\x83\x8d\x8a\xe2\x8f\xd5\x93\xcb\x89\xf0\x8f\x9c\x83W\x83\x83\x83\x93\x83v"
#define sRumbleRollStart "\x83S\x83\x8d\x83S\x83\x8d\x8aJ\x8en"
#define sRumbleAccel "\x89\xc1\x91\xac"
#define sRumbleForceDash "\x8b\xad\x90\xa7\x83_\x83""b\x83V\x83\x85"
#define sRumbleJump "\x83W\x83\x83\x83\x93\x83v"
namespace {
    static const f32 sStickScale = 1.2f;
    static const f32 sStickMin = 0.9f;
    static const f32 sBlendRate = 1.0f;
    static const f32 sHeightOffset = 200.0f;
    static const f32 sBinderRadiusMin = 40.0f;
    static const f32 sBinderRadiusMax = 106.0f;
    static const s32 sVolumeScale = 450;
    static const s32 sPitchScale = 29;
    static const s32 sPitchBase = -200;
};  // namespace

inline void addVec(TVec3f& rDst, const TVec3f& rSrc) {
    JMathInlineVEC::PSVECAdd(&rDst, &rSrc, &rDst);
}

namespace NrvRollingRock {
    NEW_NERVE(RollingRockNrvWait, RollingRock, Wait);
};  // namespace NrvRollingRock

RollingRock::RollingRock(const char* pName)
    : LiveActor(pName), mBlurModel(nullptr), mBreakModel(nullptr), mState(nullptr), mArg7(0), mEffectPos(TVec3f(0.0f)), mFront(0.0f, 0.0f, 1.0f) {
    mQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    mMtx.identity();
    mEffectMtx.identity();
}

void RollingRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, MR::isPlayerLuigi() ? "\x52\x6F\x63\x6B\x4C\x75\x69\x67\x69\x52\x6F\x6C\x6C\x69\x6E\x67\x52\x6F\x63\x6B" : "RockMarioRollingRock", false);
    MR::getJMapInfoArg7NoInit(rIter, &mArg7);
    MR::makeQuatFromRotate(&mQuat, this);
    mMtx.setInline(getBaseMtx());
    MR::setSensorFollowPos(this, "Bind", &mEffectPos);
    MR::setSensorFollowPos(this, "Body", &mEffectPos);
    initNerve(&NrvRollingRock::RollingRockNrvWait::sInstance, 1);
    mState = new RollingRockStateParent(this);
    MR::initActorState(this, mState, &NrvRollingRock::RollingRockNrvWait::sInstance, nullptr);
    MR::setAllEffectHostMtx(this, mEffectMtx);
    mBlurModel = createBlurModel(mEffectMtx);
    mBreakModel = createBreakModel(mEffectMtx);
    MR::onCalcGravity(this);
    MR::hideModel(this);
    makeActorDead();
}

void RollingRock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::offBind(this);
    setNerve(&NrvRollingRock::RollingRockNrvWait::sInstance);
}

void RollingRock::kill() {
    if (!mState->mIsDead) {
        MR::curePlayerElementMode();
        mState->kill();
    }
    LiveActor::kill();
}

void RollingRock::calcAnim() {
    LiveActor::calcAnim();
    TVec3f off(0.0f);
    TVec3f pos(mPosition);
    if (MR::isPlayerInRush() && fn_80051080() && mState && !mState->mIsDead) {
        pos.set(*MR::getPlayerPos());
    }
    TVec3f sum(pos);
    sum.add(off);
    mEffectPos.set(sum);
}

bool RollingRock::isRollingReady() const {
    bool ret = false;
    if (!mState->mIsDead && mState->isRollingReady()) {
        ret = true;
    }
    return ret;
}

void RollingRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void RollingRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        mState->attackSensor(pSender, pReceiver);
        return;
    }
    if (!MR::isSensorPlayer(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool RollingRock::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool RollingRock::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveMsgPush(pSender, pReceiver);
    }
    return false;
}

bool RollingRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    MR::isMsgRushCancel(msg);
    MR::isMsgRushForceCancel(msg);
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

void RollingRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    if (MR::updateActorState(this)) {
        MR::zeroVelocity(this);
        mMtx.getTrans(mPosition);
        MR::offBind(this);
    }
}

ModelObj* RollingRock::createBreakModel(MtxPtr pMtx) {
    ModelObj* pModel = MR::createModelObjMapObjStrongLight(sNameBroken, "RockMarioRollingRockBreak", pMtx);
    MR::invalidateClipping(pModel);
    pModel->makeActorDead();
    return pModel;
}

void RollingRock::appearBreakStart() {
    mBreakModel->appear();
    MR::startAction(mBreakModel, "RollStart");
}

void RollingRock::appearBreakEnd() {
    mBreakModel->appear();
    MR::startAction(mBreakModel, "RollEnd");
}

void RollingRock::showRock() {
    MR::showModel(this);
}

void RollingRock::hideRock() {
    MR::hideModel(this);
}

ModelObj* RollingRock::createBlurModel(MtxPtr pMtx) {
    ModelObj* pModel = MR::createModelObjMapObjStrongLight(sNameEffect, "RockMarioRollingRockBlur", pMtx);
    MR::invalidateClipping(pModel);
    pModel->makeActorDead();
    return pModel;
}

void RollingRock::appearBlur() {
    mBlurModel->appear();
    MR::startAction(mBlurModel, "RockMarioRollingRockBlur");
}

void RollingRock::killBlur() {
    mBlurModel->kill();
}

bool RollingRock::onHitEnemy(HitSensor* pSender, HitSensor* pReceiver) {
    bool isBig = false;
    if (MR::isSensorType(pSender, 0x2F) || MR::isSensorType(pSender, 0x30)) {
        isBig = true;
    }
    if (isBig) {
        MR::tryRumblePadVeryStrong(this, 0);
        MR::shakeCameraVeryStrong();
    }
    MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
    return true;
}

bool RollingRock::isNotBreakableSensor(const HitSensor* pSensor) {
    return !MR::isSensorType(pSensor, 0x2F);
}

bool RollingRock::isBreakableSensor(const HitSensor* pSensor) {
    bool isBig = false;
    if (MR::isSensorType(pSensor, 0x2F) || MR::isSensorType(pSensor, 0x30)) {
        isBig = true;
    }
    if (isBig) {
        return false;
    }
    return !MR::isEqualString(MR::getSensorHost(pSensor)->mName, sNameHammer);
}

bool RollingRock::isOnForceDashGround() const {
    if (MR::isBindedGround(this)) {
        return MR::isGroundCodeForceDash(&mBinder->mGroundInfo.mParentTriangle);
    }
    return false;
}

bool RollingRock::calcForceDashVelocity(TVec3f* pDst, const LiveActor* pActor, f32 speed) {
    AreaObj* pArea = MR::getAreaObj("ForceDashCube", pActor->mPosition);
    if (pArea == nullptr) {
        return false;
    }
    s32 arg = MR::getAreaObjArg(pArea, 0);
    if (arg != -1) {
        speed = arg;
    }
    MR::calcCubeAxisZ(pArea, pDst);
    f32 sq = pDst->squared();
    if (sq <= JGeometry::TUtil< f32 >::epsilon()) {
    } else {
        f32 k = JGeometry::TUtil< f32 >::inv_sqrt(sq) * speed;
        pDst->x *= k;
        pDst->y *= k;
        pDst->z *= k;
    }
    return true;
}

bool RollingRock::getForceDashStep(s32* pDst, const LiveActor* pActor, s32 defaultStep) {
    AreaObj* pArea = MR::getAreaObj("ForceDashCube", pActor->mPosition);
    if (pArea == nullptr) {
        return false;
    }
    s32 arg = MR::getAreaObjArg(pArea, 1);
    if (arg != -1) {
        defaultStep = arg;
    }
    *pDst = defaultStep;
    return true;
}

bool RollingRock::calcStickDir(TVec3f* pDst) {
    f32 stickY = MR::getSubPadStickY(0);
    f32 stickX = MR::getSubPadStickX(0);
    f32 xy[2];
    xy[0] = stickX * sStickScale;
    xy[1] = stickY * sStickScale;
    fn_803F5E20(&xy[0]);
    fn_803F5E20(&xy[1]);
    f32 len = JGeometry::TUtil< f32 >::sqrt(xy[0] * xy[0] + xy[1] * xy[1]);
    if (len < sStickMin) {
        return false;
    }
    MR::calcPlayerWorldPadDir(pDst, xy[0], xy[1]);
    return true;
}

void RollingRock::endBindWait() {
    MR::endBindAndPlayerWait(this);
}

void RollingRock::endBindDamage(u32 msg, const TVec3f& rDir) {
    if (MR::isPlayerInRush()) {
        MR::endBindAndPlayerDamageMsg(this, msg, rDir);
    }
}

void RollingRock::endBindFlip(u32 msg, const TVec3f& rDir) {
    if (MR::isPlayerInRush()) {
        MR::endBindAndPlayerFlip(this, rDir);
    }
}

bool RollingRock::isPlayerRolling() {
    LiveActor* pRush = MR::getCurrentRushActor();
    if (pRush == nullptr) {
        return false;
    }
    if (!MR::isEqualString(pRush->mName, sNameRock)) {
        return false;
    }
    if (pRush == nullptr) {
        return false;
    }
    bool isCamera;
    u8* pCam = (u8*)fn_80114940(MR::getCameraDirector());
    if (pCam == nullptr) {
        isCamera = false;
    } else {
        const char* names[5] = {"CAM_TYPE_WONDER_PLANET", "CAM_TYPE_TRIPOD_PLANET", "CAM_TYPE_CUBE_PLANET", "CAM_TYPE_MEDIAN_PLANET",
                                "CAM_TYPE_FOO_FIGHTER_PLANET"};
        isCamera = false;
        for (u32 i = 0; i < 5; i++) {
            CameraDirector* pDir = MR::getCameraDirector();
            u8 cur = pCam[8];
            if (cur == pDir->mHolder->getIndexOf(names[i])) {
                isCamera = true;
                break;
            }
        }
    }
    if (isCamera) {
        return false;
    }
    return static_cast< RollingRock* >(pRush)->isRollingReady();
}

namespace NrvRollingRockStateParent {
    NEW_NERVE(RollingRockStateParentNrvWait, RollingRockStateParent, Wait);
    NEW_NERVE_ONEND(RollingRockStateParentNrvRoll, RollingRockStateParent, Roll, Roll);
    NEW_NERVE(RollingRockStateParentNrvEnd, RollingRockStateParent, End);
    class RollingRockStateParentNrvDamage : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            RollingRockStateParent* pState = reinterpret_cast< RollingRockStateParent* >(pSpine->mExecutor);
            if (!MR::isPlayerDamaging() && !fn_80050350()) {
                pState->setNerve(&RollingRockStateParentNrvWait::sInstance);
            }
        }
        static RollingRockStateParentNrvDamage sInstance;
    };
    RollingRockStateParentNrvDamage RollingRockStateParentNrvDamage::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvRollingRockStateParent

RollingRockStateParent::RollingRockStateParent(RollingRock* pHost)
    : RollingRockStateBase(sNameParentState, pHost), mRollState(nullptr), mItemSensor(nullptr), mIsCancel(false) {
    initNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvWait::sInstance, 2);
    mRollState = new RollingRockStateRoll(mHost);
    MR::initActorState(this, mRollState, &NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance, nullptr);
}

RollingRockStateParent::~RollingRockStateParent() {
}

void RollingRockStateParent::appear() {
    mIsDead = false;
    MR::onCalcGravity(mHost);
    MR::onCalcShadow(mHost, nullptr);
    MR::tryRumblePadMiddle(mHost, 0);
    if (MR::isPlayerDamaging() || fn_80050350()) {
        setNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvDamage::sInstance);
    } else {
        setNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvWait::sInstance);
    }
}

void RollingRockStateParent::kill() {
    if (!mRollState->mIsDead) {
        mRollState->kill();
    }
    MR::offCalcGravity(mHost);
    MR::offCalcShadow(mHost, nullptr);
    mHost->hideRock();
    MR::setGameCameraTargetToPlayer();
    if (mItemSensor != nullptr) {
        MR::sendArbitraryMsg(0x8F, mItemSensor, mHost->getSensor("Body"));
        mItemSensor = nullptr;
    }
    mIsDead = true;
}

void RollingRockStateParent::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance)) {
        if ((MR::isEqualSensor(pSender, mHost, "Body") || MR::isEqualSensor(pSender, mHost, "Attack")) && MR::tryGetItem(pSender, pReceiver)) {
            if (mItemSensor == nullptr) {
                mItemSensor = pReceiver;
                return;
            }
            MR::sendArbitraryMsg(0x8F, mItemSensor, pSender);
            mItemSensor = pReceiver;
            return;
        }
        mRollState->attackSensor(pSender, pReceiver);
    }
}

bool RollingRockStateParent::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance)) {
        if (mRollState->receiveMsgEnemyAttack(msg, pSender, pReceiver)) {
            kill();
            return true;
        }
    }
    return false;
}

bool RollingRockStateParent::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance)) {
        return mRollState->receiveMsgPush(pSender, pReceiver);
    }
    return false;
}

bool RollingRockStateParent::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgRushBegin(msg)) {
        return tryStartRoll();
    }
    if (MR::isMsgRushCancel(msg) || MR::isMsgRushForceCancel(msg)) {
        mIsCancel = true;
        if (MR::isSensorType(pSender, 0x4A) && tryEndRush()) {
            kill();
            return true;
        }
        if (isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance)) {
            mRollState->kill();
            if (tryEndRush()) {
                setNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvWait::sInstance);
                return true;
            }
        }
        tryEndRush();
        kill();
        return true;
    }
    if (isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance)) {
        return mRollState->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

bool RollingRockStateParent::isRollingReady() const {
    bool ret = false;
    if (!mIsDead && isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance) && getNerveStep() == 2) {
        ret = true;
    }
    return ret;
}

bool RollingRockStateParent::tryStartRoll() {
    if (!isNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvWait::sInstance)) {
        return false;
    }
    if (!MR::isPlayerElementModeRock()) {
        return false;
    }
    mRollState->initFront();
    setNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvRoll::sInstance);
    return true;
}

void RollingRockStateParent::followPlayer() {
    TVec3f pos;
    fn_800514B0(&mHost->mQuat, &pos, mHost);
    TVec3f up;
    up.negate(*MR::getPlayerGravity());
    TVec3f off(up);
    off.x *= sHeightOffset;
    off.y *= sHeightOffset;
    off.z *= sHeightOffset;
    TVec3f sum(pos);
    sum.add(off);
    mHost->mPosition = sum;
}

bool RollingRockStateParent::tryEndRush() {
    if (!MR::isPlayerInRush()) {
        if (!MR::isPlayerDamaging()) {
            MR::setGameCameraTargetToPlayer();
            return true;
        }
        return false;
    }
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(mHost, mHost->mVelocity);
    MR::setGameCameraTargetToPlayer();
    return true;
}

bool RollingRockStateParent::isRockModeEnd() const {
    return !MR::isPlayerElementModeRock();
}

void RollingRockStateParent::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(mHost);
    }
    if (isRockModeEnd()) {
        kill();
        return;
    }
    followPlayer();
}

void RollingRockStateParent::exeRoll() {
    if (MR::isFirstStep(this)) {
        mIsCancel = false;
    }
    MR::updateActorStateAndNextNerve(this, &NrvRollingRockStateParent::RollingRockStateParentNrvEnd::sInstance);
}

void RollingRockStateParent::endRoll() {
    mHost->hideRock();
    if (!mIsCancel) {
        TVec3f pos(mHost->mPosition);
        TQuat4f quat(mHost->mQuat);
        MR::blendQuatFromGroundAndFront(&quat, mHost, mHost->mFront, sBlendRate, sBlendRate);
        fn_800513C0(&quat, &pos);
    }
}

void RollingRockStateParent::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::shakeCameraNormalWeak();
        return;
    }
    if (tryEndRush()) {
        setNerve(&NrvRollingRockStateParent::RollingRockStateParentNrvWait::sInstance);
        return;
    }
    kill();
}

namespace NrvRollingRockStateRoll {
    class RollingRockStateRollNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static RollingRockStateRollNrvWait sInstance;
    };
    RollingRockStateRollNrvWait RollingRockStateRollNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(RollingRockStateRollNrvStart, RollingRockStateRoll, Start);
    NEW_NERVE(RollingRockStateRollNrvAppear, RollingRockStateRoll, Appear);
    NEW_NERVE(RollingRockStateRollNrvMove, RollingRockStateRoll, Move);
    NEW_NERVE(RollingRockStateRollNrvAccel, RollingRockStateRoll, Accel);
    NEW_NERVE(RollingRockStateRollNrvForceDash, RollingRockStateRoll, ForceDash);
    NEW_NERVE(RollingRockStateRollNrvJump, RollingRockStateRoll, Jump);
};  // namespace NrvRollingRockStateRoll

RollingRockStateRoll::RollingRockStateRoll(RollingRock* pHost)
    : RollingRockStateBase(sNameRollState, pHost), mCameraTarget(nullptr), mDir(0.0f, 0.0f, 1.0f) {
    mCameraTarget = new CameraTargetMtx(sNameCameraTarget);
    mDashStepMax = 180;
    mDashStep = 0;
    mAirStep = 0;
    mTurnRate = 3.0f;
    mStep = 0;
    _34 = 0.0f;
    mSpeed = 0.0f;
    mJumpBuffer = 0;
    mStickRate = 0.0f;
    mIsFirstRoll = false;
    initNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance, 0);
}

RollingRockStateRoll::~RollingRockStateRoll() {
}

void RollingRockStateRoll::appear() {
    mIsDead = false;
    mStep = 0;
    mDashStep = 0;
    mDashStepMax = 180;
    mSpeed = 0.0f;
    mHost->appearBlur();
    mHost->appearBreakStart();
    fn_8004FFA0();
    if (MR::isValidSwitchA(mHost)) {
        MR::onSwitchA(mHost);
    }
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance);
}

void RollingRockStateRoll::kill() {
    if (!mIsDead) {
        mHost->killBlur();
        MR::deleteEffect(mHost, "Move");
        MR::forceDeleteEffect(mHost, "MoveBlur");
        mHost->appearBreakEnd();
        fn_8004FFB0();
        MR::offBind(mHost);
        MR::invalidateHitSensors(mHost);
        if (MR::isValidSwitchA(mHost)) {
            MR::offSwitchA(mHost);
        }
        mIsDead = true;
    }
}

void RollingRockStateRoll::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, mHost, "Attack") && isAttackEnable() && MR::sendMsgRockAttack(pReceiver, pSender)) {
        mHost->onHitEnemy(pSender, pReceiver);
    }
}

bool RollingRockStateRoll::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        if (MR::isSensorNpc(pSender)) {
            MR::addVelocityFromPush(mHost, 0.1f, pSender, pReceiver);
            return true;
        }
    }
    if (!isAttackEnable()) {
        return false;
    }
    if (!RollingRock::isBreakableSensor(pSender)) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Body")) {
        return false;
    }
    if (fn_80012BA0(mHost, pReceiver, pSender, 0.7f)) {
        if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
            MR::tryHitReaction(mHost, sRumblePushReflect);
        }
        return true;
    }
    return false;
}

bool RollingRockStateRoll::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x62 && RollingRock::isNotBreakableSensor(pSender) && MR::isEqualSensor(pReceiver, mHost, "Body")) {
        if (!mIsDead) {
            if (MR::isMsgToEnemyAttackBlow(msg) && MR::isSensorEnemy(pSender)) {
                mHost->endBindWait();
                changeState(3);
                return false;
            }
            TVec3f neg;
            neg.negate(mHost->mGravity);
            TVec3f dir(neg);
            dir.scale(10.0f);
            if (MR::reflectVelocityH(mHost, pReceiver, pSender, 1.0f)) {
                dir.set(mHost->mVelocity);
                TVec3f scaled(dir);
                scaled.scale(1.0f);
                dir = scaled;
            }
            mHost->endBindDamage(msg, dir);
            changeState(3);
            return true;
        }
        return false;
    }
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance)) {
        if (MR::isEqualSensor(pReceiver, mHost, "Body")) {
            if (MR::isMsgToEnemyAttackBlow(msg) && MR::isSensorEnemy(pSender)) {
                TVec3f neg;
                neg.negate(mHost->mGravity);
                TVec3f dir(neg);
                dir.scale(10.0f);
                if (MR::reflectVelocityH(mHost, pReceiver, pSender, 1.0f)) {
                    dir.set(mHost->mVelocity);
                    TVec3f scaled(dir);
                    scaled.scale(1.0f);
                    dir = scaled;
                }
                mHost->endBindFlip(msg, dir);
                changeState(3);
                return false;
            }
            TVec3f zero;
            zero.setAll< f32 >(0.0f);
            mHost->endBindDamage(msg, zero);
            changeState(2);
            return true;
        }
    }
    return false;
}

bool RollingRockStateRoll::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgUpdateBaseMtx(msg)) {
        setPlayerPose();
        return true;
    }
    if (MR::isMsgIsRushEnable(msg)) {
        changeState(5);
        return true;
    }
    return false;
}

void RollingRockStateRoll::initFront() {
    MR::getPlayerFrontVec(&mDir);
    MR::vecKillElement(mDir, mHost->mGravity, &mDir);
    MR::normalize(&mDir);
}

void RollingRockStateRoll::control() {
    updateStep();
    updateVelocity();
    updateMoment();
    updatePose();
    updateEffect();
    updatePoseNoMoment();
    if (mJumpBuffer > 0) {
        mJumpBuffer--;
    }
    tryDeath();
    if (!MR::isNearZero(mHost->mVelocity, 0.001f)) {
        mHost->mFront.set(mHost->mVelocity);
        MR::normalize(&mHost->mFront);
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
        if (MR::getBrkRate(mHost) <= 0.0f && MR::isBindedGroundSnow(mHost)) {
            MR::setBrkRate(mHost, 1.0f);
        }
    }
}

bool RollingRockStateRoll::tryEnd() {
    if (mHost->mArg7 == 1) {
        return false;
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
        return false;
    }
    if (!MR::testSubPadTriggerZ(0)) {
        return false;
    }
    changeState(4);
    return true;
}

bool RollingRockStateRoll::tryJump() {
    bool trig = MR::testCorePadTriggerA(0);
    bool air = mAirStep < 5;
    if (mJumpBuffer > 0 || (air && trig)) {
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance);
        return true;
    }
    return false;
}

bool RollingRockStateRoll::tryDeath() {
    TVec3f zero(0.0f, 0.0f, 0.0f);
    if (MR::isInDeath(mHost, zero) || mAirStep > 600) {
        MR::forceKillPlayerByAbyss();
        return true;
    }
    return false;
}

static bool isBreakCageSensor(const HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return false;
    }
    bool ret = true;
    if (!MR::isEqualString(MR::getSensorHost(pSensor)->mName, sNameCage1) && !MR::isEqualString(MR::getSensorHost(pSensor)->mName, sNameCage2)) {
        ret = false;
    }
    return ret;
}

bool RollingRockStateRoll::tryReflect() {
    if (MR::isBindedWall(mHost)) {
        LiveActor* pHost = mHost;
        HitSensor* pAttack = pHost->getSensor("Attack");
        if (MR::sendMsgReflect(MR::getWallSensor(pHost), pAttack)) {
            MR::tryHitReaction(mHost, sRumbleBoardReflect);
            mDashStep = 0;
            f32 speed;
            if (MR::isNearZero(mSpeed, 0.001f)) {
                speed = 42.0f;
            } else {
                speed = mSpeed;
            }
            MR::restrictVelocity(mHost, speed);
            return true;
        }
    }
    LiveActor* pHost = mHost;
    bool hit;
    if (!MR::isBinded(pHost)) {
        hit = false;
    } else if (MR::isBindedWall(pHost) && isBreakCageSensor(MR::getWallSensor(pHost))) {
        hit = true;
    } else if (MR::isBindedGround(pHost) && isBreakCageSensor(MR::getGroundSensor(pHost))) {
        hit = true;
    } else if (MR::isBindedRoof(pHost) && isBreakCageSensor(MR::getRoofSensor(pHost))) {
        hit = true;
    } else {
        hit = false;
    }
    if (hit) {
        if (!(MR::isBindedWall(mHost) && MR::sendMsgToWallSensor(0x11, mHost->getSensor("Attack"))) &&
            !(MR::isBindedGround(mHost) && MR::sendMsgToGroundSensor(0x11, mHost->getSensor("Attack"))) && MR::isBindedRoof(mHost)) {
            MR::sendMsgToRoofSensor(0x11, mHost->getSensor("Attack"));
        }
        changeState(1);
        return true;
    }
    if (!MR::isBindedWall(mHost)) {
        return false;
    }
    MR::sendMsgToWallSensor(0x11, mHost->getSensor("Attack"));
    changeState(1);
    return true;
}

bool RollingRockStateRoll::tryLand() {
    if (MR::isLessStep(this, 15)) {
        return false;
    }
    if (!MR::isBindedGround(mHost)) {
        return false;
    }
    MR::tryHitReaction(mHost, sRumbleLand);
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
    return true;
}

void RollingRockStateRoll::updateVelocity() {
    f32 gravityAccel = 1.8f;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) && !MR::isBindedGround(mHost)) {
        MR::attenuateVelocityExceptDirection(mHost, mHost->mGravity, 1.0f);
        gravityAccel = 2.0f;
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        if (MR::isBindedGround(mHost)) {
            if (mAirStep > 5) {
                TVec3f proj;
                proj.setAll< f32 >(0.0f);
                TVec3f dir(mHost->mGravity);
                MR::normalize(&dir);
                proj.scale(dir.dot(mHost->mVelocity), dir);
                MR::startActionSound(mHost, "Land", (s32)(1000.0f * MR::fabs(PSVECMag(proj))), -1, -1);
            }
            mAirStep = 0;
        } else {
            MR::addVelocityToGravity(mHost, gravityAccel);
            mAirStep++;
        }
    }
    f32 speed = PSVECMag(mHost->mVelocity);
    TVec3f before(mHost->mVelocity);
    MR::reboundVelocityFromCollision(mHost, 0.0f, 0.0f, 1.0f);
    TVec3f after(mHost->mVelocity);
    TVec3f diff = after - before;
    mHost->mVelocity.setLength(speed);
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance)) {
        MR::restrictVelocity(mHost, 65.0f);
    }
}

void RollingRockStateRoll::updatePoseNoMoment() {
    if (MR::isSameDirection(mHost->mGravity, mDir, 0.01f)) {
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpNoSupportPos(&mCameraTarget->mMatrix, up, mHost->mPosition);
    } else {
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpFrontPos(&mCameraTarget->mMatrix, up, mDir, mHost->mPosition);
    }
}

void RollingRockStateRoll::setPlayerPose() {
    fn_800513C0(&mHost->mQuat, &mHost->mPosition);
}

void RollingRockStateRoll::updateStep() {
    mStep++;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance) ||
        (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance) && mDashStepMax != 180)) {
        mDashStep++;
    }
}

void RollingRockStateRoll::updateMoment() {
    TVec3f moment;
    moment.setAll< f32 >(0.0f);
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        TVec3f vel(mDir);
        vel.scale(48.0f);
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::calcMomentRollBall(&moment, vel, up, MR::getSensorRadius(mHost, "Body"));
    } else if (&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) {
        TVec3f up;
        up.negate(mHost->mGravity);
        if (MR::isBindedGround(mHost)) {
            up.set(MR::getGroundNormal(mHost));
        }
        TVec3f vel(mHost->mVelocity);
        vel.scale(1.2f);
        MR::calcMomentRollBall(&moment, vel, up, MR::getSensorRadius(mHost, "Body"));
        MR::rotateQuatMoment(&mHost->mQuat, moment);
    }
}

void RollingRockStateRoll::updatePose() {
    TVec3f front;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        front.set(mDir);
        MR::vecKillElement(front, mHost->mGravity, &front);
    } else {
        if (!MR::isNearZero(mHost->mVelocity, 0.001f)) {
            front.set(mHost->mVelocity);
        } else {
            front.set< f32 >(mHost->mEffectMtx.mMtx[0][2], mHost->mEffectMtx.mMtx[1][2], mHost->mEffectMtx.mMtx[2][2]);
        }
        MR::vecKillElement(front, mHost->mGravity, &front);
    }
    if (MR::normalizeOrZero(&front)) {
        f32 x = mHost->mQuat.x;
        f32 y = mHost->mQuat.y;
        f32 z = mHost->mQuat.z;
        f32 w = mHost->mQuat.w;
        front.set< f32 >(2.0f * (x * z) + 2.0f * (w * y), 2.0f * (y * z) - 2.0f * (w * x), (1.0f - 2.0f * (x * x)) - 2.0f * (y * y));
    }
    if (MR::isSameDirection(front, mHost->mGravity, 0.01f)) {
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpNoSupportPos(&mHost->mEffectMtx, up, mHost->mPosition);
    } else {
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpFrontPos(&mHost->mEffectMtx, up, front, mHost->mPosition);
    }
}

void RollingRockStateRoll::updateEffect() {
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance)) {
        if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
            return;
        }
        if (mAirStep > 5) {
            if (MR::isEffectValid(mHost, "Move")) {
                MR::deleteEffect(mHost, "Move");
            }
        } else if (!MR::isEffectValid(mHost, "Move")) {
            MR::emitEffect(mHost, "Move");
        }
    }
}

bool RollingRockStateRoll::isAttackEnable() const {
    if (mIsDead) {
        return false;
    }
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance)) {
        return false;
    }
    return !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance);
}

void RollingRockStateRoll::startMove() {
    TVec3f vel(mDir);
    vel.scale(36.0f);
    MR::setVelocity(mHost, vel);
    MR::emitEffect(mHost, "Move");
    MR::emitEffect(mHost, "MoveBlur");
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
}

void RollingRockStateRoll::changeState(s32 type) {
    LiveActor* pHost = mHost;
    TVec3f* pVel = &pHost->mVelocity;
    switch (type) {
    case 0: {
        TVec3f front(mHost->mFront);
        TVec3f pos(mHost->mPosition);
        TVec3f a(front);
        a.scale(1.0f - 0.72f);
        TVec3f b(pHost->mGravity);
        b.scale(0.72f);
        TVec3f c = a - b;
        pVel->set(c);
        pVel->setLength(15.0f);
        break;
    }
    case 1: {
        TVec3f neg;
        neg.negate(pHost->mGravity);
        TVec3f v(neg);
        v.scale(18.0f);
        pVel->set(v);
        break;
    }
    case 5:
    case 3:
        if (MR::isNearZero(*pVel, 0.001f)) {
            TVec3f neg;
            neg.negate(pHost->mGravity);
            TVec3f v(neg);
            v.scale(18.0f);
            pVel->set(v);
        } else {
            TVec3f up;
            up.negate(pHost->mGravity);
            TVec3f side;
            MR::vecKillElement(*pVel, up, &side);
            MR::normalizeOrZero(&side);
            side.negate();
            TVec3f a(up);
            a.scale(1.0f - 0.3f);
            TVec3f b(side);
            b.scale(0.3f);
            TVec3f c(b + a);
            TVec3f d(c);
            d.scale(18.0f);
            pVel->set(d);
        }
        break;
    case 4:
        pVel->setAll< f32 >(0.0f);
        break;
    }
    switch (type) {
    case 2:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        mHost->hideRock();
        break;
    case 3:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        break;
    case 0:
    case 4:
        MR::tryHitReaction(mHost, sRumbleRollCancel);
        MR::startBckPlayerJ(sBckRollJump);
        break;
    case 1:
    case 5:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        MR::shakeCameraStrong();
        MR::tryRumblePadMiddle(mHost, 0);
        MR::startBckPlayerJ(sBckRollHitJump);
        break;
    default:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        break;
    }
    kill();
}

void RollingRockStateRoll::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        MR::onCalcGravity(mHost);
        MR::onBind(mHost);
        MR::addVelocityJump(mHost, 20.0f);
        MR::startBckPlayer("RollingRockStart", nullptr);
        MR::startLevelSoundPlayer("RockRollStart", -1, -1);
        MR::tryHitReaction(mHost, sRumbleRollStart);
        CameraTargetArg arg(mCameraTarget);
        MR::setGameCameraTarget(arg);
        mIsFirstRoll = true;
        MR::setBinderRadius(mHost, sBinderRadiusMin);
        return;
    }
    LiveActor* pHost = mHost;
    MR::setBinderRadius(pHost, MR::calcNerveValue(this, 20, sBinderRadiusMin, sBinderRadiusMax));
    turnByStickStart();
    if (MR::isBindedGround(mHost) && MR::isGreaterStep(this, 5)) {
        if (MR::isLessEqualStep(this, (s32)sBinderRadiusMin)) {
            MR::setBinderRadius(mHost, sBinderRadiusMax);
        }
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance);
    }
}

void RollingRockStateRoll::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        MR::tryHitReaction(mHost, sRumbleLand);
        mHost->showRock();
        MR::startBrk(mHost, "Snow");
        MR::setBrkRate(mHost, 0.0f);
    }
    turnByStickAppear();
    if (MR::isGreaterStep(this, 30)) {
        startMove();
    }
}

void RollingRockStateRoll::exeMove() {
    TVec3f prevVel(mHost->mVelocity);
    TVec3f newVel;
    newVel.x = newVel.y = newVel.z = 0.0f;
    if (turnByStick()) {
        newVel = mHost->mVelocity;
        MR::diffAngleAbs(newVel, prevVel);
    }
    if (!tryJump() && !tryReflect() && !tryForceDash() && !tryTimeOver()) {
        s32 base = (s32)MR::clamp(sVolumeScale * (mStickRate - 0.1f), 0.0f, 1000.0f);
        s32 pitch = (s32)MR::clamp(sPitchBase + sPitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f);
        s32 volume = ((s32)MR::clamp(sPitchBase + sPitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f) + base) - 200;
        if (mAirStep > 5) {
            volume += 100;
            pitch = (s32)(pitch * 0.9f);
        }
        if (MR::isLessStep(this, 120)) {
            MR::startActionSound(mHost, "Move", pitch, volume, -1);
        } else {
            MR::startActionSound(mHost, "MoveEnding", pitch, volume, -1);
        }
        if (MR::isFirstStep(this)) {
            MR::startActionSound(mHost, "MoveStart", -1, -1, -1);
            if (mIsFirstRoll) {
                MR::startLevelSoundPlayer("RockDashStart", -1, -1);
                mIsFirstRoll = false;
            }
        }
    }
}

void RollingRockStateRoll::exeAccel() {
    if (MR::isFirstStep(this)) {
        MR::startLevelSoundPlayer("RockAccel", -1, -1);
        MR::tryHitReaction(mHost, sRumbleAccel);
        TVec3f vel(mHost->mVelocity);
        fn_80010D10(mHost, vel, mTurnRate);
        mHost->appearBlur();
        mDashStep -= 100;
        if (mDashStep <= 0) {
            mDashStep = 0;
        }
    }
    turnByStick();
    if (!tryJump() && !tryReflect() && !tryTimeOver()) {
        if (MR::isGreaterStep(this, 45)) {
            setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
        }
        s32 pitch = (s32)MR::clamp(sPitchBase + sPitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f);
        MR::startActionSound(mHost, "Move", pitch, (s32)MR::clamp(sPitchBase + sPitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f), -1);
    }
}

void RollingRockStateRoll::exeForceDash() {
    if (MR::isFirstStep(this)) {
        mHost->appearBlur();
        MR::startLevelSoundPlayer("RockAccel", -1, -1);
        MR::tryHitReaction(mHost, sRumbleForceDash);
        mDashStep = 0;
    }
    if (MR::testCorePadTriggerA(0)) {
        mJumpBuffer = 5;
    }
    if (!tryReflect() && !tryTimeOver() && MR::isGreaterStep(this, 45)) {
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
    }
}

bool RollingRockStateRoll::turnByStick() {
    mStickRate = 0.0f;
    if (mIsDead) {
        return false;
    }
    if (mAirStep > 5) {
        return false;
    }
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (MR::isNearZero(mHost->mVelocity, 0.001f)) {
        return false;
    }
    TVec3f stick(mHost->mVelocity);
    RollingRock::calcStickDir(&stick);
    TVec3f dir(mHost->mVelocity);
    MR::normalize(&dir);
    mStickRate = MR::fabs(dir.dot(stick));
    MR::turnDirectionDegree(mHost, &dir, stick, 1.2f);
    f32 speed = MR::calcVelocityLength(mHost);
    TVec3f vel(dir);
    vel.scale(speed);
    MR::setVelocity(mHost, vel);
    return true;
}

bool RollingRockStateRoll::turnByStickAppear() {
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (mStep < 30) {
        return false;
    }
    TVec3f stick;
    RollingRock::calcStickDir(&stick);
    MR::turnDirectionDegree(mHost, &mDir, stick, 1.75f);
    return true;
}

bool RollingRockStateRoll::turnByStickStart() {
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (mStep < 30) {
        return false;
    }
    TVec3f stick;
    RollingRock::calcStickDir(&stick);
    MR::turnDirectionDegree(mHost, &mDir, stick, 1.75f);
    return true;
}

bool RollingRockStateRoll::tryForceDash() {
    if (mAirStep > 5) {
        return false;
    }
    if (!mHost->isOnForceDashGround()) {
        return false;
    }
    TVec3f vel;
    if (!RollingRock::calcForceDashVelocity(&vel, mHost, 50.0f)) {
        return false;
    }
    mTurnRate = 20.0f;
    MR::setVelocity(mHost, vel);
    mSpeed = MR::calcVelocityLength(mHost);
    RollingRock::getForceDashStep(&mDashStepMax, mHost, 180);
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance);
    return true;
}

bool RollingRockStateRoll::tryPadSwing() {
    if (!MR::isPadSwing(0)) {
        return false;
    }
    if (mAirStep <= 5) {
        return false;
    }
    mTurnRate = 3.0f;
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance);
    return true;
}

bool RollingRockStateRoll::tryTimeOver() {
    if (mHost->mArg7 == 1) {
        return false;
    }
    if (mDashStep < mDashStepMax) {
        return false;
    }
    changeState(0);
    return true;
}

void RollingRockStateRoll::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReaction(mHost, sRumbleJump);
        MR::addVelocityJump(mHost, 33.0f);
        mJumpBuffer = 0;
    }
    if (MR::testCorePadTriggerA(0)) {
        mJumpBuffer = 5;
    }
    if (!tryEnd() && !tryPadSwing() && !tryReflect()) {
        tryLand();
    }
}
