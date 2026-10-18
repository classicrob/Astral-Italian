#pragma auto_inline off


#include "Game/NPC/SupportTico.hpp"
#include "Game/Boss/DinoPackun.hpp"
#include "Game/Enemy/EnemyUtilAll.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"





#define DECL_NRV(name)                                                                                                                              \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                                   \
        static name sInstance;                                                                                                                       \
    };                                                                                                                                               \
    name name::sInstance ATTRIBUTE_WEAK;

namespace NrvSupportTico {
    DECL_NRV(SupportTicoNrvBehavior);
    NEW_NERVE(SupportTicoNrvDisappear, SupportTico, Disappear);
};  // namespace NrvSupportTico

SupportTico::SupportTico(const char* pName) : LiveActor(pName) {
    mPose = new ActorPoseTQ();
    mAttachGroup = nullptr;
    mTarget = nullptr;
    mNearTarget = nullptr;
    mHoldSensor = nullptr;
    mReleasedSensor = nullptr;
    mExecutor = nullptr;
    mTargetTimer = 0;
    _B0 = 0;
    mAttachRange = 2500.0f;
    mControlOn = true;
    _B9 = false;
    mInForbidArea = false;
    mFollowPlayerYDir = false;
    mShow = true;
    MR::createPlayerFollowPass();
    mAttachGroup = new LiveActorGroup("\x83\x8a\x83\x4e\x83\x47\x83\x58\x83\x67\x83\x4f\x83\x8b\x81\x5b\x83\x76", 0x20);
}

NO_INLINE TVec3f* SupportTico::getFollowPos() const {
    return &mPose->mFront;
}

NO_INLINE void SupportTico::setAttachRange(f32 range) {
    mAttachRange = range;
}

void SupportTico::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mGravity.set(*MR::getPlayerGravity());
    setNerve(&NrvSupportTico::SupportTicoNrvBehavior::sInstance);
}

void SupportTico::makeActorDead() {
    LiveActor::makeActorDead();
    mAttachGroup->clear();
    mTargetTimer = 0;
    mTarget = nullptr;
    releaseHoldSensor();
}

void SupportTico::init(const JMapInfoIter& rIter) {
    MR::joinSupportPlayGroup(this);
    MR::initActor(this, rIter, "SupportTico", false);
    MR::setPosition(this, *MR::getPlayerPos());
    MR::initActorPoseTQ(mPose, this);
    initNerve(&NrvSupportTico::SupportTicoNrvBehavior::sInstance, 1);
    mExecutor = new SupportTicoExecutor(this);
    MR::initActorState(this, mExecutor, &NrvSupportTico::SupportTicoNrvBehavior::sInstance, "Active");
    MR::invalidateClipping(this);
    makeActorDead();
}

void SupportTico::control() {
    MR::calcGravityOrZero(this);
    TVec3f up(0.0f);
    if (mFollowPlayerYDir || MR::isCurrentRushSpinDriver()) {
        MR::extractMtxYDir(MR::getPlayerBaseMtx(), &up);
    } else {
        up.set(-mGravity);
    }
    TVec3f front(mPose->mFront);
    if (MR::isCurrentRushSpinDriver()) {
        MR::extractMtxZDir(MR::getPlayerBaseMtx(), &front);
    }
    MR::blendQuatUpFront(mPose, up, front, 0.2f, 0.2f);
    mAttachGroup->clear();
    mNearTarget = nullptr;
    if (mHoldSensor != nullptr) {
        if (MR::isDead(MR::getSensorHost(mHoldSensor))) {
            mHoldSensor = nullptr;
        } else {
            MR::calcPositionUpOffset(&MR::getSensorHost(mHoldSensor)->mPosition, this, 0.0f);
        }
    }
    if (mReleasedSensor != nullptr) {
        if (MR::isDead(MR::getSensorHost(mReleasedSensor))) {
            mReleasedSensor = nullptr;
        }
    }
    mInForbidArea = MR::isInAreaObj("ForbidSupportTicoArea", mPosition);
}

void SupportTico::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
}

void SupportTico::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!mInForbidArea && isNerve(&NrvSupportTico::SupportTicoNrvBehavior::sInstance)) {
        mExecutor->attackSensor(pSender, pReceiver);
    }
}

bool SupportTico::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSupportPlayGroupStart(msg)) {
        appear();
        return true;
    }
    if (MR::isMsgSupportPlayGroupStop(msg)) {
        setNerve(&NrvSupportTico::SupportTicoNrvDisappear::sInstance);
        return true;
    }
    if (isNerve(&NrvSupportTico::SupportTicoNrvBehavior::sInstance)) {
        return mExecutor->receiveMessage(msg, pSender, pReceiver);
    }
    return false;
}

void SupportTico::exeDisappear() {
    if (!mExecutor->isActive()) {
        kill();
        return;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
    }
    MR::calcVecToPlayerH(&mPose->mFront, this, nullptr);
    SupportTicoStateFunction::calcFollowOffset(this);
    if (MR::isActionEnd(this)) {
        kill();
    }
}

// NONMATCHING: register allocation and floating-point load order.
bool SupportTico::tryAttachToTarget(const LiveActor* pActor) {
    if (mTarget != nullptr && mTarget == pActor) {
        _B9 = true;
    }
    if (!isNerve(&NrvSupportTico::SupportTicoNrvBehavior::sInstance)) {
        return false;
    }
    if (!mControlOn) {
        return false;
    }
    if (mHoldSensor != nullptr) {
        return false;
    }
    StarPointerTarget* pTarget = pActor->mPointerTarget;
    if (pTarget == nullptr) {
        return false;
    }
    if (MR::isStarPointerPointing2POnPressButton(pActor, "\x8e\xe3", true, false)) {
        TVec3f pos(0.0f);
        pTarget->calcPosition(&pos);
        if (MR::isInAreaObj("ForbidSupportTicoArea", pos)) {
            return false;
        }
        if (mAttachGroup->isFull()) {
            return false;
        }
        bool found = false;
        s32 num = mAttachGroup->mObjectCount;
        for (s32 i = 0; i < num; i++) {
            if (mAttachGroup->getActor(i) == pActor) {
                found = true;
                break;
            }
        }
        if (!found) {
            mAttachGroup->registerActor(const_cast< LiveActor* >(pActor));
        }
    }
    if (mTarget != pActor) {
        return false;
    }
    TVec3f pos2;
    pTarget->calcPosition(&pos2);
    bool near = MR::isNear(this, pos2, 50.0f + pTarget->mRadius3d);
    if (near) {
        mNearTarget = pActor;
    }
    return near;
}

bool SupportTico::calcTargetPos(TVec3f* pOut) {
    if (mTarget == nullptr) {
        pOut->set(mPosition);
        return false;
    }
    StarPointerTarget* pTarget = mTarget->mPointerTarget;
    TVec3f pos;
    pTarget->calcPosition(&pos);
    TVec3f dir(*MR::getPlayerCenterPos());
    dir.sub(pos);
    MR::normalizeOrZero(&dir);
    f32 radius = pTarget->mRadius3d;
    TVec3f off(dir);
    off.x *= radius;
    off.y *= radius;
    off.z *= radius;
    JMathInlineVEC::PSVECAdd(&pos, &off, &pos);
    pOut->set(pos);
    return true;
}

LiveActor* SupportTico::findNearestTarget() {
    s32 num = mAttachGroup->mObjectCount;
    if (num == 0) {
        return nullptr;
    }
    f32 minDist = mAttachRange;
    LiveActor* pNearest = nullptr;
    for (s32 i = 0; i < num; i++) {
        LiveActor* pActor = mAttachGroup->getActor(i);
        if (!MR::isDead(pActor)) {
            f32 dist = MR::calcDistanceToPlayer(pActor);
            if (dist < minDist) {
                pNearest = pActor;
                minDist = dist;
            }
        }
    }
    return pNearest;
}

void SupportTico::clearTarget() {
    mAttachGroup->clear();
    mNearTarget = nullptr;
}

// NONMATCHING: equivalent boolean comparison lowering.
void SupportTico::updateTarget() {
    LiveActor* pNearest = findNearestTarget();
    if (pNearest != nullptr && mTarget == nullptr) {
        mTarget = pNearest;
        mTargetTimer = 40;
        _B9 = true;
        return;
    }
    const LiveActor* pTarget = mTarget;
    mTargetTimer--;
    if (pTarget != nullptr) {
        if (!_B9 || MR::isDead(pTarget) || MR::isStarPointerTargetClipped(mTarget) ||
            MR::isNearPlayerAnyTime(mTarget, 1.2f * mAttachRange) == false) {
            mAttachGroup->clear();
            mNearTarget = nullptr;
            mTarget = nullptr;
            mTargetTimer = 0;
        } else if (SupportTicoStateFunction::isReleaseA() && mTargetTimer <= 0) {
            mTarget = nullptr;
            mTargetTimer = 0;
        }
    }
    _B9 = false;
}

bool SupportTico::isTargeting() const {
    if (!mControlOn) {
        return false;
    }
    if (mHoldSensor != nullptr) {
        return false;
    }
    return mTarget != nullptr;
}

void SupportTico::offControl() {
    if (mControlOn) {
        mAttachGroup->clear();
        mNearTarget = nullptr;
        mControlOn = false;
    }
}

void SupportTico::onControl() {
    mControlOn = true;
}

void SupportTico::setFollowPlayerYDir() {
    mFollowPlayerYDir = true;
}

void SupportTico::resetFollowPlayerYDir() {
    mFollowPlayerYDir = false;
}

void SupportTico::requestHide() {
    mShow = false;
}

void SupportTico::requestShow() {
    mShow = true;
}

void SupportTico::releaseHoldSensor() {
    if (mHoldSensor != nullptr) {
        LiveActor* pHost = MR::getSensorHost(mHoldSensor);
        MR::calcGravity(pHost);
        TVec3f vec;
        MR::calcVecToPlayerH(&vec, this, nullptr);
        MR::setVelocitySeparateHV(MR::getSensorHost(mHoldSensor), vec, 10.0f, 15.0f);
        MR::sendArbitraryMsg(0x8E, mHoldSensor, getSensor("Body"));
        mReleasedSensor = mHoldSensor;
        mHoldSensor = nullptr;
    }
}

bool SupportTico::tryHold(HitSensor* pSender, HitSensor* pReceiver) {
    if (mHoldSensor == nullptr && mReleasedSensor == nullptr && mTarget != nullptr && mTarget == MR::getSensorHost(pReceiver) &&
        MR::sendArbitraryMsg(0x8B, pReceiver, pSender)) {
        mHoldSensor = pReceiver;
        return true;
    }
    return false;
}

void SupportTico::resetTarget() {
    mTarget = nullptr;
    mTargetTimer = 0;
}

SupportTico::~SupportTico() {
}


namespace NrvSupportTico {
    void SupportTicoNrvBehavior::execute(Spine* pSpine) const {
        MR::updateActorState(reinterpret_cast< SupportTico* >(pSpine->mExecutor));
    }
};  // namespace NrvSupportTico

