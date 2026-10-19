#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#pragma auto_inline off
#include "Game/Ride/RollingRock.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraParamChunk.hpp"
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

void clampSliderStick(f32*);

#define sNameBroken "\x89\xf3\x82\xea\x83\x82\x83""f\x83\x8b"
#define sNameEffect "\x83G\x83t\x83""F\x83N\x83g\x83\x82\x83""f\x83\x8b"
#define sNameHammer "\x83p\x83""b\x83N\x83\x93\x83t\x83\x89\x83\x8f\x81[(\x83g\x83Q\x83n\x83\x93\x83}\x81[)"
#define sNameRock "\x8a\xe2\x83}\x83\x8a\x83I"
#define sNameParentState "\x8a\xe2\x83}\x83\x8a\x83I\x95\xcf\x90g\x8f\xf3\x91\xd4"
namespace {
    static const f32 sStickScale = 1.2f;
    static const f32 sStickMin = 0.9f;
    static const f32 sBlendRate = 1.0f;
    static const f32 sHeightOffset = 200.0f;
};  // namespace

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
    clampSliderStick(&xy[0]);
    clampSliderStick(&xy[1]);
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
    CameraParamChunk* pCam = MR::getCameraDirector()->getCurrentGameCameraChunk();
    if (pCam == nullptr) {
        isCamera = false;
    } else {
        const char* names[5] = {"CAM_TYPE_WONDER_PLANET", "CAM_TYPE_TRIPOD_PLANET", "CAM_TYPE_CUBE_PLANET", "CAM_TYPE_MEDIAN_PLANET",
                                "CAM_TYPE_FOO_FIGHTER_PLANET"};
        isCamera = false;
        for (u32 i = 0; i < 5; i++) {
            CameraDirector* pDir = MR::getCameraDirector();
            u8 cur = pCam->mCameraTypeIndex;
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
            if (!MR::isPlayerDamaging() && !MR::isPlayerFireDamageActive()) {
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

// NONMATCHING: the empty intermediate base destructor retains one redundant null check.
RollingRockStateParent::~RollingRockStateParent() {
}

void RollingRockStateParent::appear() {
    mIsDead = false;
    MR::onCalcGravity(mHost);
    MR::onCalcShadow(mHost, nullptr);
    MR::tryRumblePadMiddle(mHost, 0);
    if (MR::isPlayerDamaging() || MR::isPlayerFireDamageActive()) {
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
    MR::getPlayerQuatAndPos(&mHost->mQuat, &pos);
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

#pragma dont_inline on
bool RollingRockStateParent::isRockModeEnd() const {
    return !MR::isPlayerElementModeRock();
}
#pragma dont_inline reset

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
        MR::setPlayerQuatAndPos(&quat, &pos);
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
