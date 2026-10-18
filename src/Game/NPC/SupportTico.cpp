#pragma auto_inline off


#include "Game/NPC/SupportTico.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MapUtil.hpp"

// SMG2 helpers that have no header declaration yet (symbols exist in symbols.txt).
namespace MR {
    void joinSupportPlayGroup(LiveActor*);
    bool isMsgSupportPlayGroupStart(u32);
    bool isMsgSupportPlayGroupStop(u32);
    bool isMsgTouchPlantItem(u32);
    bool sendMsgSupportTicoSpin(HitSensor*, HitSensor*);
    bool isInAreaObj(const char*, const TVec3f&);
    bool isCurrentRushSpinDriver();
    MtxPtr getPlayerBaseMtx();
    void extractMtxYDir(MtxPtr, TVec3f*);
    void extractMtxZDir(MtxPtr, TVec3f*);
    void calcGravityOrZero(LiveActor*);
    void calcPositionUpOffset(TVec3f*, const LiveActor*, f32);
    void calcVecToPlayerH(TVec3f*, const LiveActor*, const TVec3f*);
    void calcVecToTargetPosH(TVec3f*, const LiveActor*, const TVec3f&, const TVec3f*);
    void blendQuatUpFront(TQuat4f*, const TVec3f&, const TVec3f&, f32, f32);
    void vecBlend(TVec3f*, const TVec3f&, const TVec3f&, f32);
    void setVelocitySeparateHV(LiveActor*, const TVec3f&, f32, f32);
    void forceDeleteEffectAll(LiveActor*);
    void zeroVelocity(LiveActor*);
    void resetPosition(LiveActor*);
    bool isDemoActive();
    bool isDemoActive(const char*);
    bool isPowerStarGetDemoActive();
    bool isExecScenarioStarter();
    bool isStageStateScenarioOpeningCamera();
    bool isStageFileSelect();
    bool isSupportTicoShow();
    bool isNear(const LiveActor*, const TVec3f&, f32);
    bool isNearPlayerAnyTime(const LiveActor*, f32);
    bool isActionEnd(const LiveActor*);
    f32 calcNerveRate(const NerveExecutor*, s32, s32);
    f32 calcNerveEaseInValue(const NerveExecutor*, s32, f32, f32);
    bool updateActorState(NerveExecutor*);
    bool updateActorStateAndNextNerve(NerveExecutor*, const Nerve*);
    void startDPDHitSound();
    void startCSSound2P(const char*, const char*, s32, f32);
    void stopAndStartCSSound1P(const char*, s32, const char*, f32);
    void stopAndStartCSSound2P(const char*, s32, const char*, f32);
    bool testCorePadButtonA(s32);
    bool testCorePadTriggerA(s32);
    bool testCorePadTrigger1(s32);
    bool testCorePadTrigger2(s32);
    bool testCorePadTriggerMinus(s32);
    bool testCorePadTriggerPlus(s32);
    bool testCorePadTriggerUp(s32);
    bool testCorePadTriggerDown(s32);
    bool testCorePadTriggerLeft(s32);
    bool testCorePadTriggerRight(s32);
    bool isPadSwing(s32);
    bool tryRumblePadMiddle(const void*, s32);
};  // namespace MR

extern "C" {
void fn_800102E0(void* pPose, LiveActor* pHost);
void* fn_8022CAE0(void* pThis);
void fn_80034690(LiveActor*, const char*, const TVec3f*);
f32 fn_8003B960(f32, f32, f32, f32, f32);
f32 fn_800483E0(const NerveExecutor*, s32, s32, s32);
bool fn_8005C2E0(const LiveActor*);
void fn_802DEB90(TVec3f*, f32);
f32 fn_8030B2A0();
}

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

namespace NrvSupportTicoExecutor {
    DECL_NRV(SupportTicoExecutorNrvAppear);
    DECL_NRV(SupportTicoExecutorNrvDemoHide);
    DECL_NRV(SupportTicoExecutorNrvFollowPlayer);
    DECL_NRV(SupportTicoExecutorNrvChase);
    DECL_NRV(SupportTicoExecutorNrvBack);
};  // namespace NrvSupportTicoExecutor

namespace NrvSupportTicoStateBack {
    DECL_NRV(SupportTicoStateBackNrvBack);
    DECL_NRV(SupportTicoStateBackNrvSpin);
};  // namespace NrvSupportTicoStateBack

namespace NrvSupportTicoStateChase {
    DECL_NRV(SupportTicoStateChaseNrvChase);
    DECL_NRV(SupportTicoStateChaseNrvSpin);
    DECL_NRV(SupportTicoStateChaseNrvItemGet);
};  // namespace NrvSupportTicoStateChase

namespace NrvSupportTicoStateDemoHide {
    NEW_NERVE(SupportTicoStateDemoHideNrvHide, SupportTicoStateDemoHide, Hide);
    NEW_NERVE(SupportTicoStateDemoHideNrvStop, SupportTicoStateDemoHide, Stop);
};  // namespace NrvSupportTicoStateDemoHide

namespace NrvSupportTicoStateFollowPlayer {
    DECL_NRV(SupportTicoStateFollowPlayerNrvBackEnd);
    DECL_NRV(SupportTicoStateFollowPlayerNrvWait);
    DECL_NRV(SupportTicoStateFollowPlayerNrvJump);
    DECL_NRV(SupportTicoStateFollowPlayerNrvSpin);
    DECL_NRV(SupportTicoStateFollowPlayerNrvDummy);
    DECL_NRV(SupportTicoStateFollowPlayerNrvItemDrop);
};  // namespace NrvSupportTicoStateFollowPlayer

namespace NrvSupportTicoStateJump {
    NEW_NERVE(SupportTicoStateJumpNrvJump, SupportTicoStateJump, Jump);
    NEW_NERVE(SupportTicoStateJumpNrvReturn, SupportTicoStateJump, Return);
};  // namespace NrvSupportTicoStateJump

namespace NrvSupportTicoStateSpin {
    NEW_NERVE(SupportTicoStateSpinNrvSpin, SupportTicoStateSpin, Spin);
    NEW_NERVE(SupportTicoStateSpinNrvStop, SupportTicoStateSpin, Stop);
};  // namespace NrvSupportTicoStateSpin

SupportTico::SupportTico(const char* pName) : LiveActor(pName) {
    void* pPose = ::operator new(0x1C);
    if (pPose != nullptr) {
        pPose = fn_8022CAE0(pPose);
    }
    mPose = pPose;
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
    return reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mPose) + 0x10);
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
    fn_800102E0(mPose, this);
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
    TVec3f front(*reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mPose) + 0x10));
    if (MR::isCurrentRushSpinDriver()) {
        MR::extractMtxZDir(MR::getPlayerBaseMtx(), &front);
    }
    MR::blendQuatUpFront(static_cast< TQuat4f* >(mPose), up, front, 0.2f, 0.2f);
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
    MR::setBaseTRMtx(this, static_cast< ActorPoseTQ* >(mPose));
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
    MR::calcVecToPlayerH(reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mPose) + 0x10), this, nullptr);
    SupportTicoStateFunction::calcFollowOffset(this);
    if (MR::isActionEnd(this)) {
        kill();
    }
}

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
    pos.add(off);
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
        if (!_B9 || MR::isDead(pTarget) || fn_8005C2E0(mTarget) || !MR::isNearPlayerAnyTime(mTarget, 1.2f * mAttachRange)) {
            mAttachGroup->clear();
            mNearTarget = nullptr;
            mTarget = nullptr;
            mTargetTimer = 0;
        } else if (SupportTicoStateFunction::isPressA() && mTargetTimer <= 0) {
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

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoExecutor

SupportTicoExecutor::SupportTicoExecutor(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203A\203N\203e\203B\203u\217\363\221\324", pHost, SupportTicoStateTag< 1 >()) {
    mStateDemoHide = nullptr;
    mStateFollowPlayer = nullptr;
    mStateChase = nullptr;
    mStateBack = nullptr;
    mIsFirstAppear = true;
    initNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance, 4);
    mStateDemoHide = new SupportTicoStateDemoHide(pHost);
    MR::initActorState(this, mStateDemoHide, &NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance, "DemoHide");
    mStateFollowPlayer = new SupportTicoStateFollowPlayer(pHost);
    MR::initActorState(this, mStateFollowPlayer, &NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance, "FollowPlayer");
    mStateChase = new SupportTicoStateChase(pHost);
    MR::initActorState(this, mStateChase, &NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance, "Chase");
    mStateBack = new SupportTicoStateBack(pHost);
    MR::initActorState(this, mStateBack, &NrvSupportTicoExecutor::SupportTicoExecutorNrvBack::sInstance, "Back");
}

void SupportTicoExecutor::appear() {
    mIsDead = false;
    mIsFirstAppear = true;
    if (SupportTicoStateFunction::isHideDemo()) {
        mStateDemoHide->setSilentHide(true);
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance);
        return;
    }
    setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvAppear::sInstance);
}

void SupportTicoExecutor::kill() {
    mIsDead = true;
}

void SupportTicoExecutor::control() {
    if (!isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance) && SupportTicoStateFunction::isHideDemo()) {
        mStateDemoHide->setSilentHide(false);
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance);
    }
}

void SupportTicoExecutor::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance)) {
        mStateFollowPlayer->attackSensor(pSender, pReceiver);
    } else if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvBack::sInstance)) {
        mStateBack->attackSensor(pSender, pReceiver);
    } else if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance)) {
        mStateChase->attackSensor(pSender, pReceiver);
    }
}

bool SupportTicoExecutor::receiveMessage(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgTouchPlantItem(msg)) {
        return isActive();
    }
    return false;
}

void SupportTicoExecutor::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(mHost);
        MR::startAction(mHost, mIsFirstAppear ? "AppearFirst" : "Appear");
        MR::startActionSound(mHost, mIsFirstAppear ? "AppearFirst" : "AppearAgain", -1, -1, -1);
        mIsFirstAppear = false;
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    if (MR::isActionEnd(mHost)) {
        mHost->resetTarget();
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance);
    }
}

void SupportTicoExecutor::exeBack() {
    if (MR::updateActorState(this)) {
        if (mHost->isTargeting()) {
            setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance);
            return;
        }
        mStateFollowPlayer->requestJump();
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance);
    }
}

bool SupportTicoExecutor::isActive() const {
    if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvAppear::sInstance) ||
        isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance)) {
        return false;
    }
    return true;
}

namespace NrvSupportTicoExecutor {
    void SupportTicoExecutorNrvBack::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor)->exeBack();
    }

    void SupportTicoExecutorNrvChase::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvBack::sInstance);
    }

    void SupportTicoExecutorNrvFollowPlayer::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvChase::sInstance);
    }

    void SupportTicoExecutorNrvDemoHide::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvAppear::sInstance);
    }

    void SupportTicoExecutorNrvAppear::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor)->exeAppear();
    }
};  // namespace NrvSupportTicoExecutor

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateBack

SupportTicoStateBack::SupportTicoStateBack(SupportTico* pHost)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\203v\203\214\203C\203\204\201[\202\326\226\337\202\351", pHost, SupportTicoStateTag< 2 >()), mStartPos(0.0f, 0.0f, 0.0f) {
    mTimer = 0.0f;
    initNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance, 1);
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance, "Spin");
}

void SupportTicoStateBack::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance);
    MR::zeroVelocity(mHost);
    mTimer = 0.0f;
}

void SupportTicoStateBack::control() {
    TVec3f vec;
    SupportTicoStateFunction::calcFollowOffset(&vec);
    MR::vecBlend(&mHost->mPosition, mStartPos, vec, fn_8003B960(mTimer / 30.0f, 0.0f, 1.0f, 0.0f, 1.0f));
    mTimer += 1.0f;
}

void SupportTicoStateBack::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

void SupportTicoStateBack::exeBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Back");
        mStartPos.set(mHost->mPosition);
    }
    if (!SupportTicoStateFunction::isHolding(mHost)) {
        mHost->updateTarget();
        if (mHost->isTargeting() && MR::isGreaterStep(this, 20)) {
            kill();
        }
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    if (mTimer >= 30.0f) {
        if (SupportTicoStateFunction::isHolding(mHost)) {
            mHost->resetTarget();
        }
        kill();
        return;
    }
    if (mTimer >= 15.0f && SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance);
    }
}

void SupportTicoStateBack::exeSpin() {
    if (MR::updateActorState(this)) {
        if (mTimer >= 30.0f) {
            kill();
            return;
        }
        setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance);
    }
}

namespace NrvSupportTicoStateBack {
    void SupportTicoStateBackNrvSpin::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateBack* >(pSpine->mExecutor)->exeSpin();
    }

    void SupportTicoStateBackNrvBack::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateBack* >(pSpine->mExecutor)->exeBack();
    }
};  // namespace NrvSupportTicoStateBack

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateChase

SupportTicoStateChase::SupportTicoStateChase(SupportTico* pHost)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\222\307\202\242\202\251\202\257\201I", pHost, SupportTicoStateTag< 3 >()), mStartPos(0.0f, 0.0f, 0.0f) {
    mStateSpin = nullptr;
    initNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance, 1);
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance, "Spin");
}

void SupportTicoStateChase::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance);
}

void SupportTicoStateChase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance)) {
        if (mHost->tryHold(pSender, pReceiver)) {
            MR::startActionSound(mHost, "ItemGet", -1, -1, -1);
            setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvItemGet::sInstance);
        }
    } else if (isNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

void SupportTicoStateChase::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Chase");
        mStartPos.set(mHost->mPosition);
        MR::startDPDHitSound();
    }
    if (MR::isStep(this, 3)) {
        MR::startActionSound(mHost, "ChaseStart", -1, -1, -1);
    }
    mHost->updateTarget();
    if (!mHost->isTargeting()) {
        kill();
        return;
    }
    TVec3f targetPos;
    mHost->calcTargetPos(&targetPos);
    MR::calcVecToTargetPosH(mHost->getFollowPos(), mHost, targetPos, nullptr);
    MR::vecBlend(&mHost->mPosition, mStartPos, targetPos, MR::calcNerveEaseInRate(this, 12));
    if (MR::isStep(this, 12)) {
        if (MR::hasME()) {
            MR::startSystemME("ME_DPD_HIT");
        } else {
            MR::startSystemSE("SE_SY_DPD_HIT", -1, -1);
        }
        MR::startCSSound2P("CS_DPD_HIT", nullptr, 0, 1.0f);
    }
    if (MR::isGreaterEqualStep(this, 12) && SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance);
    }
}

namespace NrvSupportTicoStateChase {
    void SupportTicoStateChaseNrvChase::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor)->exeChase();
    }

    void SupportTicoStateChaseNrvSpin::execute(Spine* pSpine) const {
        SupportTicoStateChase* pState = reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor);
        if (MR::updateActorState(pState)) {
            pState->kill();
        }
    }

    void SupportTicoStateChaseNrvItemGet::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor)->kill();
    }
};  // namespace NrvSupportTicoStateChase

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateDemoHide

SupportTicoStateDemoHide::SupportTicoStateDemoHide(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203f\203\202\224\361\225\134\216\246\217\363\221\324", pHost, SupportTicoStateTag< 4 >()) {
    mCounter = 0;
    mSilentHide = false;
    initNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvHide::sInstance, 0);
}

void SupportTicoStateDemoHide::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvHide::sInstance);
    mCounter = 0;
    MR::startAction(mHost, "DemoHide");
    mHost->clearTarget();
    SupportTicoStateFunction::hideHoldingObj(mHost);
    MR::hideModel(mHost);
    setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
    MR::forceDeleteEffectAll(mHost);
}

void SupportTicoStateDemoHide::kill() {
    mIsDead = true;
    SupportTicoStateFunction::showHoldingObj(mHost);
}

void SupportTicoStateDemoHide::setSilentHide(bool silent) {
    mSilentHide = silent;
}

void SupportTicoStateDemoHide::exeHide() {
    if (MR::isFirstStep(this)) {
        mHost->clearTarget();
        SupportTicoStateFunction::hideHoldingObj(mHost);
        if (mSilentHide) {
            MR::hideModel(mHost);
            setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
            return;
        }
        MR::startAction(mHost, "DemoHide");
        MR::startActionSound(mHost, "DemoHide", -1, -1, -1);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
    }
}

void SupportTicoStateDemoHide::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(mHost);
    }
    if (!SupportTicoStateFunction::isHideDemo()) {
        mCounter++;
    } else {
        mCounter = 0;
    }
    if (mCounter > 5) {
        MR::showModel(mHost);
        SupportTicoStateFunction::calcFollowOffset(mHost);
        MR::resetPosition(mHost);
        kill();
    }
}

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateFollowPlayer

SupportTicoStateFollowPlayer::SupportTicoStateFollowPlayer(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203}\203\212\203I\222\307\220\217\217\363\221\324", pHost, SupportTicoStateTag< 5 >()) {
    mStateJump = nullptr;
    mStateSpin = nullptr;
    mFollowDir.set(0.0f, 0.0f, 0.0f);
    mJumpOffset.set(0.0f, 0.0f, 0.0f);
    mRequestJump = false;
    initNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance, 2);
    mStateJump = new SupportTicoStateJump(pHost, &mJumpOffset);
    MR::initActorState(this, mStateJump, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvJump::sInstance, "Jump");
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance, "Spin");
}

void SupportTicoStateFollowPlayer::requestJump() {
    mRequestJump = true;
}

void SupportTicoStateFollowPlayer::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    mRequestJump = false;
}

void SupportTicoStateFollowPlayer::control() {
    mHost->mPosition = mHost->mPosition + mJumpOffset;
    SupportTicoStateFunction::startSupportTicoCSSound();
}

void SupportTicoStateFollowPlayer::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

void SupportTicoStateFollowPlayer::updateFollow() {
    if (MR::isFirstStep(this)) {
        mFollowDir.set(mJumpOffset);
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    if (mHost->isTargeting()) {
        kill();
    }
    if (!tryItemDrop() && !trySpin()) {
        if (tryJumpStart()) {
            return;
        }
        if (MR::isLessEqualStep(this, 30)) {
            f32 scale;
            if (MR::isLessStep(this, 20)) {
                scale = MR::calcNerveEaseInValue(this, 20, 1.0f, 0.0f);
            } else {
                f32 t = MR::calcNerveRate(this, 20, 30);
                scale = (10.0f * (2.0f * -(t * (1.0f + (t * (t - 2.0f)))))) / 20.0f;
            }
            TVec3f vec(mFollowDir);
            vec.x *= scale;
            vec.y *= scale;
            vec.z *= scale;
            mJumpOffset.set(vec);
        }
    }
}

void SupportTicoStateFollowPlayer::exeBackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "BackEnd");
    }
    updateFollow();
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    }
}

void SupportTicoStateFollowPlayer::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Wait");
    }
    updateFollow();
}

void SupportTicoStateFollowPlayer::exeJump() {
    MR::updateActorStateAndNextNerve(this, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    if (mHost->isTargeting()) {
        mHost->mPosition = mHost->mPosition + mJumpOffset;
        kill();
        return;
    }
    trySpin();
}

void SupportTicoStateFollowPlayer::exeSpin() {
    MR::updateActorStateAndNextNerve(this, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    tryJumpStart();
}

void SupportTicoStateFollowPlayer::exeItemDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ItemDrop");
        mHost->releaseHoldSensor();
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    if (MR::isGreaterStep(this, 60)) {
        mHost->clearTarget();
        mHost->resetTarget();
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    }
}

bool SupportTicoStateFollowPlayer::tryItemDrop() {
    if (SupportTicoStateFunction::isHolding(mHost) && SupportTicoStateFunction::isReleaseA()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvItemDrop::sInstance);
        return true;
    }
    return false;
}

bool SupportTicoStateFollowPlayer::tryJumpStart() {
    if (PSVECMag(mJumpOffset) < 50.0f && SupportTicoStateFunction::isTriggerA()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvJump::sInstance);
        return true;
    }
    return false;
}

bool SupportTicoStateFollowPlayer::trySpin() {
    if (SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance);
        return true;
    }
    return false;
}

namespace NrvSupportTicoStateFollowPlayer {
    void SupportTicoStateFollowPlayerNrvBackEnd::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeBackEnd();
    }
    void SupportTicoStateFollowPlayerNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeWait();
    }
    void SupportTicoStateFollowPlayerNrvJump::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeJump();
    }
    void SupportTicoStateFollowPlayerNrvSpin::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeSpin();
    }
    void SupportTicoStateFollowPlayerNrvDummy::execute(Spine* pSpine) const {
    }
    void SupportTicoStateFollowPlayerNrvItemDrop::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeItemDrop();
    }
};  // namespace NrvSupportTicoStateFollowPlayer

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateJump

SupportTicoStateJump::SupportTicoStateJump(SupportTico* pHost, TVec3f* pOffset)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\203W\203\203\203\223\203v\201I", pHost, SupportTicoStateTag< 6 >()) {
    mOffset = pOffset;
    initNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvJump::sInstance, 0);
}

void SupportTicoStateJump::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvJump::sInstance);
}

void SupportTicoStateJump::exeJump() {
    if (MR::isFirstStep(this)) {
        mOffset->z = 0.0f;
        mOffset->y = 0.0f;
        mOffset->x = 0.0f;
        MR::startAction(mHost, "Jump");
    }
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    f32 height = 150.0f * fn_800483E0(this, 20, 0, 20);
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f vec = up * height;
    mOffset->set(vec);
    if (MR::isGreaterEqualStep(this, 39)) {
        setNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvReturn::sInstance);
    }
}

void SupportTicoStateJump::exeReturn() {
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    f32 t = MR::calcNerveRate(this, 20) / 0.5f;
    f32 height = 0.5f * (150.0f * (2.0f * -(t * (1.0f + (t * (t - 2.0f))))));
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f vec = up * height;
    mOffset->set(vec);
    if (MR::isGreaterEqualStep(this, 10)) {
        kill();
    }
}

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateSpin

SupportTicoStateSpin::SupportTicoStateSpin(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\201A\203X\203s\203\223", pHost, SupportTicoStateTag< 7 >()) {
    initNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance, 0);
}

void SupportTicoStateSpin::appear() {
    mIsDead = false;
    MR::zeroVelocity(mHost);
    setNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance);
}

void SupportTicoStateSpin::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::startCSSound2P("CS_SPIN_HIT", nullptr, 0, 1.0f);
        MR::startAction(mHost, "Spin");
        SupportTicoStateFunction::trySpinAttack(mHost);
    }
    if (MR::isGreaterStep(this, 10)) {
        setNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvStop::sInstance);
    }
}

void SupportTicoStateSpin::exeStop() {
    if (MR::isGreaterStep(this, 40)) {
        mHost->resetTarget();
        kill();
    }
}

void SupportTicoStateSpin::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance) && MR::sendMsgSupportTicoSpin(pReceiver, pSender)) {
        fn_80034690(mHost, "\203X\203s\203\223\203q\203b\203g", &mHost->mPosition);
        MR::tryRumblePadMiddle(mHost, 1);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// SupportTicoStateFunction

namespace SupportTicoStateFunction {
    bool isEnable() {
        if (!MR::isExistSceneObj(0x62)) {
            return false;
        }
        return *(reinterpret_cast< u8* >(MR::getSceneObjHolder()->getObj(0x62)) + 0xB8);
    }

    f32 getFollowScale() {
        f32 a = 200.0f;
        f32 b = 100.0f + fn_8030B2A0();
        if (a < b) {
            a = b;
        }
        return a;
    }

    bool isSwing() {
        if (isEnable()) {
            return MR::isPadSwing(1);
        }
        return false;
    }

    bool isReleaseA() {
        if (isEnable()) {
            return !MR::testCorePadButtonA(1);
        }
        return false;
    }

    bool isPressA() {
        if (isEnable()) {
            return MR::testCorePadButtonA(1);
        }
        return false;
    }

    bool isTriggerA() {
        if (isEnable()) {
            return MR::testCorePadTriggerA(1);
        }
        return false;
    }

    void startSupportTicoCSSound() {
        if (!isEnable()) {
            return;
        }
        if (MR::testCorePadTrigger1(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound2P("CS_PERC_TICO2", 0x32, nullptr, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound2P("CS_PERC_TICO1", 0x32, nullptr, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTrigger2(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound2P("CS_PERC_COIN", 5, nullptr, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound2P("CS_PERC_1UP", 0x1F4, nullptr, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerMinus(1)) {
            MR::stopAndStartCSSound2P("CS_PERC_AUD_BAD", 0x96, nullptr, 1.0f);
        }
        if (MR::testCorePadTriggerPlus(1)) {
            MR::stopAndStartCSSound2P("CS_PERC_AUD_GOOD", 0x96, nullptr, 1.0f);
        }
        if (MR::testCorePadTriggerUp(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound1P("CS_PERC_INST2", 0xC8, nullptr, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound1P("CS_PERC_INST1", 0xC8, nullptr, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerDown(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound1P("CS_PERC_COIN", 5, nullptr, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound1P("CS_PERC_1UP", 0x1F4, nullptr, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerLeft(1)) {
            MR::stopAndStartCSSound1P("CS_PERC_AUD_BAD", 0x96, nullptr, 1.0f);
        }
        if (MR::testCorePadTriggerRight(1)) {
            MR::stopAndStartCSSound1P("CS_PERC_AUD_GOOD", 0x96, nullptr, 1.0f);
        }
    }

    bool isHideDemo() {
        if (!MR::isSupportTicoShow()) {
            return true;
        }
        if (MR::isDemoActive()) {
            if (MR::isDemoActive("\203}\203\212\203I\225\317\220g") || MR::isDemoActive("\212G\226{\217u\212\324\210\332\223\256\203f\203\202") ||
                MR::isDemoActive("\212G\226{\203y\201[\203W\202\337\202\255\202\350")) {
                return false;
            }
            return true;
        }
        if (MR::isPowerStarGetDemoActive() || MR::isExecScenarioStarter() || MR::isStageStateScenarioOpeningCamera() || MR::isStageFileSelect()) {
            return true;
        }
        return false;
    }

    bool isHolding(const SupportTico* pHost) {
        return pHost->mHoldSensor != nullptr;
    }

    void hideHoldingObj(SupportTico* pHost) {
        if (pHost->mHoldSensor != nullptr) {
            MR::hideModel(MR::getSensorHost(pHost->mHoldSensor));
            MR::sendSimpleMsgToActor(0x89, MR::getSensorHost(pHost->mHoldSensor));
        }
        if (pHost->mReleasedSensor != nullptr) {
            MR::hideModel(MR::getSensorHost(pHost->mReleasedSensor));
            MR::sendSimpleMsgToActor(0x89, MR::getSensorHost(pHost->mReleasedSensor));
        }
    }

    void showHoldingObj(SupportTico* pHost) {
        if (pHost->mHoldSensor != nullptr) {
            MR::showModel(MR::getSensorHost(pHost->mHoldSensor));
            MR::sendSimpleMsgToActor(0x88, MR::getSensorHost(pHost->mHoldSensor));
        }
        if (pHost->mReleasedSensor != nullptr) {
            MR::showModel(MR::getSensorHost(pHost->mReleasedSensor));
            MR::sendSimpleMsgToActor(0x88, MR::getSensorHost(pHost->mReleasedSensor));
        }
    }

    void calcFollowOffset(TVec3f* pOut) {
        fn_802DEB90(pOut, getFollowScale());
    }

    void calcFollowOffset(SupportTico* pHost) {
        fn_802DEB90(&pHost->mPosition, getFollowScale());
    }

    bool trySpinAttack(SupportTico* pHost) {
        HitSensor* pSensor = pHost->getSensor("Body");
        f32 radius = MR::getSensorRadius(pSensor);
        bool result = false;
        u32 num = Collision::checkStrikeBallToMapWithThickness(*MR::getSensorPos(pSensor), radius, radius, nullptr, nullptr);
        for (u32 i = 0; i < num; i++) {
            if (MR::sendArbitraryMsg(0x13, Collision::getStrikeInfoMap(i)->mParentTriangle.mSensor, pSensor)) {
                result = true;
            }
        }
        return result;
    }
};  // namespace SupportTicoStateFunction
