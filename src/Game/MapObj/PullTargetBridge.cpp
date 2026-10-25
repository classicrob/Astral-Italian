#pragma auto_inline off
#include "Game/MapObj/PullTargetBridge.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Map/CollisionParts.hpp"

#include "Game/Player/YoshiFunction.hpp"

PullTargetAnim::PullTargetAnim(LiveActor* pHost) {
    mHost = pHost;
    mPrevDist = 0.0f;
    mStillCount = 0;
    mIsPulling = false;
}





PullTarget::PullTarget(LiveActor* pHost) : PartsModel(pHost, "引っ張りターゲット", "PullTarget", nullptr, -1, false) {
    mIsEnable = true;
    mLockOnTarget = nullptr;
    initWithoutIter();
    initHitSensor(1);
    MR::addHitSensorMapObjSimple(this, "Body", 32, 1.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initFixedPosition("PullTargetPos");
    MR::initStarPointerTarget(this, 300.0f, TVec3f(0.0f, 0.0f, 0.0f));
    mLockOnTarget = MR::createYoshiLockOnTarget(this, (MtxPtr)nullptr, TVec3f(0.0f));
}

void PullTargetAnim::reset() {
    MR::startAction(mHost, "Wait");
    mPrevDist = 0.0f;
    mStillCount = 0;
    mIsPulling = false;
}

void PullTargetAnim::update(f32 dist) {
    if (dist - mPrevDist > 1.0f) {
        if (!mIsPulling) {
            MR::startAction(mHost, "PullStart");
            mIsPulling = true;
        }
        mStillCount = 0;
        mPrevDist = dist;
    } else {
        mStillCount++;
        if (mIsPulling && mStillCount > 5) {
            MR::startAction(mHost, "Wait");
            mIsPulling = false;
        }
    }
}

// Target inlines the matrix copy; the current shared helper stays out of line.
bool PullTarget::isFacingPlayer() {
    TVec3f toPlayer;
    MR::calcVecToPlayer(&toPlayer, this);
    TPos3f mtx;
    mtx.set(getBaseMtx());
    TVec3f front;
    f32 z = mtx.mMtx[2][2];
    f32 y = mtx.mMtx[1][2];
    f32 x = mtx.mMtx[0][2];
    front.set< f32 >(x, y, z);
    return toPlayer.dot(front) > 0.1f;
}

void PullTarget::control() {
    if (mIsEnable && isFacingPlayer()) {
        YoshiFunction::tryYoshiPull(mLockOnTarget);
    }
}

bool PullTarget::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    LiveActor* host = mHost;
    return host->receiveMessage(msg, pSender, host->getSensor("Body"));
}

PullTargetBridge::PullTargetBridge(const char* pName) : LiveActor(pName) {
    mPullTarget = nullptr;
    mPlayerPos.x = 0.0f;
    mPlayerPos.y = 0.0f;
    mPlayerPos.z = 0.0f;
    mPullDist = 0.0f;
    mAnim = nullptr;
}

namespace NrvPullTargetBridge {
    class PullTargetBridgeNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            PullTargetBridge* actor = reinterpret_cast< PullTargetBridge* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Wait");
            }
        }
        static PullTargetBridgeNrvWait sInstance;
    };
    PullTargetBridgeNrvWait PullTargetBridgeNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PullTargetBridgeNrvPull, PullTargetBridge, Pull);
    class PullTargetBridgeNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            PullTargetBridge* actor = reinterpret_cast< PullTargetBridge* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Fall");
            }
        }
        static PullTargetBridgeNrvFall sInstance;
    };
    PullTargetBridgeNrvFall PullTargetBridgeNrvFall::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvPullTargetBridge

void PullTargetBridge::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "PullTargetBridge", false);
    initHitSensor(1);
    MR::addHitSensorMapObjSimple(this, "Body", 32, 1.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initCollisionParts(this, "PullTargetBridge", getSensor("Body"), MR::getJointMtx(this, "PullTargetBridge"));
    MR::connectToSceneMapObj(this);
    initNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance, 0);
    mClippingCenter.set< f32 >(mPosition);
    MR::setClippingTypeSphere(this, 1500.0f, &mClippingCenter);
    mPullTarget = new PullTarget(this);
    mAnim = new PullTargetAnim(this);
    makeActorAppeared();
}

void PullTargetBridge::exePull() {
    if (MR::isFirstStep(this)) {
        mAnim->reset();
    }
    mAnim->update(mPullDist);
    bool isPulled = mPullDist > 250.0f;
    if (isPulled) {
        PullTarget* target = mPullTarget;
        target->mIsEnable = false;
        MR::fn_804222D0();
        setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvFall::sInstance);
    }
}

// Target inlines vector addition; the combined TU currently emits the shared operator call.
void PullTargetBridge::control() {
    TVec3f posA;
    MR::copyJointPos(this, "PullTargetBridge", &posA);
    TVec3f posB;
    MR::copyJointPos(this, "PullTargetPos", &posB);
    mClippingCenter.set< f32 >(posA * 0.5f + posB * 0.5f);
}

bool PullTargetBridge::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance)) {
            mPlayerPos.set< f32 >(*MR::getPlayerPos());
            mPullDist = 0.0f;
            setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvPull::sInstance);
        }
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return tryPull();
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (isNerve(&NrvPullTargetBridge::PullTargetBridgeNrvPull::sInstance)) {
            setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance);
        }
        return true;
    }
    return false;
}

// Target inlines vector subtraction; the combined TU currently emits the shared operator call.
bool PullTargetBridge::tryPull() {
    TVec3f move(*MR::getPlayerPos());
    move.sub(move, mPlayerPos);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir(move);
    MR::normalizeOrZero(&dir);
    MtxPtr mtx = mPullTarget->getBaseMtx();
    TVec3f front(mtx[0][2], mtx[1][2], mtx[2][2]);
    if (front.dot(dir) < 0.0f) {
        return false;
    }
    mPullDist += PSVECMag(&move);
    return true;
}
