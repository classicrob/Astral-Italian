#include "Game/Enemy/KickStone.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"


namespace NrvKickStone {
    // Nerve 0x2948: attach the support tico to the target (custom execute).
    class KickStoneNrvAttachTico : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KickStoneNrvAttachTico sInstance;
    };
    KickStoneNrvAttachTico KickStoneNrvAttachTico::sInstance ATTRIBUTE_WEAK;

    // Nerve 0x294C: rolling move (fn_801B38B0).
    class KickStoneNrvMoveRoll : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KickStoneNrvMoveRoll sInstance;
    };
    KickStoneNrvMoveRoll KickStoneNrvMoveRoll::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KickStoneNrvBreak, KickStone, Break);
    NEW_NERVE(KickStoneNrvBreakSub, KickStone, Break);

    // Nerve 0x2958: falling / bound (fn_801B3B60).
    class KickStoneNrvDead : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KickStoneNrvDead sInstance;
    };
    KickStoneNrvDead KickStoneNrvDead::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKickStone

#define GET_KICKSTONE_NERVE(name) (&NrvKickStone::name::sInstance)


KickStone::KickStone(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mIsCoin = false;
    mLockOnTarget = nullptr;
    _9C = 0.0f;
    _A0 = false;
    mArg1 = false;
    mArg2 = false;
    mUp.set(0.0f, 1.0f, 0.0f);
    _BC.set(0.0f, 0.0f, 1.0f);
    mOffset.set(0.0f, 0.0f, 0.0f);
    mMtx.identity();
}

// NONMATCHING: MWCC removes a redundant integer conversion before getBaseMtx, changing stack allocation.
void KickStone::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mBreakModel = MR::tryCreateBreakModel(this, reinterpret_cast<MtxPtr>(&mMtx));
    mMtx.set(getBaseMtx());
    mOffset = mPosition;

    MR::getJMapInfoArg0NoInit(rIter, &mIsCoin);
    if (mIsCoin) {
        MR::declareCoin(this, 1);
    } else {
        MR::declareStarPiece(this, 1);
    }

    MR::getJMapInfoArg1NoInit(rIter, &mArg1);
    MR::getJMapInfoArg2NoInit(rIter, &mArg2);

    s32 count = 6;
    f32 rotDegree = 360.0f / count;
    s32 rnd = (s32)MR::getRandom(0L, 6L);
    MR::rotateMtxLocalYDegree(getBaseMtx(), (f32)rnd * rotDegree);

    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);

    TVec3f lockOn;
    lockOn.x = 0.0f;
    lockOn.y = 100.0f;
    lockOn.z = 0.0f;
    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), lockOn);
    MR::useStageSwitchAwake(this, rIter);
    initNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico), 0);
    MR::tryRegisterDemoCast(this, rIter);
    MR::offBind(this);
    makeActorAppeared();
}

void KickStone::initAfterPlacement() {
    if (!mArg2) {
        fn_801B3420(&mUp, this, mArg1);
    }
}

void KickStone::control() {
    if (isNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico))) {
        YoshiFunction::tryYoshiLick(mLockOnTarget);
    }
}

void KickStone::calcAndSetBaseMtx() {
    MR::fn_8000F9A0(this, &mUp, &mFront);
    mMtx.set(getBaseMtx());
    if (isNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak)) || isNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreakSub)) ||
        isNerve(GET_KICKSTONE_NERVE(KickStoneNrvDead))) {
        MR::setMtxTrans(reinterpret_cast<MtxPtr>(&mMtx), mOffset.x, mOffset.y, mOffset.z);
    }
}

void KickStone::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "Break", -1, -1, -1);
        MR::tryRumblePadVeryWeak(this, 0);
        if (isNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak))) {
            MR::stopScene(3);
        }
        MR::stopBck(this);
        MR::startAction(this, "Break");
        mOffset = mPosition;
        fn_801B3420(&mUp, this, 0);
        MR::forceDeleteEffect(this, "KickStoneLight");
        MR::invalidateShadowAll(this);
        MR::zeroVelocity(this);
        return;
    }

    if (mBreakModel != nullptr) {
        if (mIsCoin) {
            if (!MR::isGalaxyDarkCometAppearInCurrentStage()) {
                MR::appearCoinPop(this, mPosition, 1);
            }
        } else {
            MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
        }
        mBreakModel->appear();
    }

    setNerve(GET_KICKSTONE_NERVE(KickStoneNrvDead));
}

bool KickStone::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico)) && !isNerve(GET_KICKSTONE_NERVE(KickStoneNrvMoveRoll))) {
        return false;
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerSpinAttackOrSupportTico(msg) ||
        MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak));
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak));
        return true;
    }

    return false;
}

bool KickStone::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico)) && !isNerve(GET_KICKSTONE_NERVE(KickStoneNrvMoveRoll))) {
        return false;
    }

    if (!MR::isMsgExplosionAttack(msg)) {
        return false;
    }

    setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak));
    return true;
}

bool KickStone::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico)) && !isNerve(GET_KICKSTONE_NERVE(KickStoneNrvMoveRoll))) {
        return false;
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        MR::onBind(this);
        MR::tryHitReaction(this, "YoshiTongue");
        setNerve(GET_KICKSTONE_NERVE(KickStoneNrvMoveRoll));
        return true;
    }

    if (MR::isMsgYoshiReleaseEaten(msg)) {
        return true;
    }

    if (MR::isMsgPlayerKick(msg)) {
        setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak));
        return true;
    }

    return false;
}

KickStone::~KickStone() {
}

void NrvKickStone::KickStoneNrvAttachTico::execute(Spine* pSpine) const {
    MR::attachSupportTicoToTarget(reinterpret_cast<LiveActor*>(pSpine->mExecutor));
}

void NrvKickStone::KickStoneNrvMoveRoll::execute(Spine* pSpine) const {
    (reinterpret_cast<KickStone*>(pSpine->mExecutor))->fn_801B38B0();
}

void NrvKickStone::KickStoneNrvDead::execute(Spine* pSpine) const {
    (reinterpret_cast<KickStone*>(pSpine->mExecutor))->fn_801B3B60();
}

void KickStone::fn_801B38B0() {
    if (MR::isFirstStep(this)) {
        MR::calcGravity(this);
        TVec3f velocity(0.0f);
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        PSVECCrossProduct(&mUp, &toPlayer, &_BC);
        if (MR::isNearZero(_BC, 0.001f)) {
            _BC.set(mFront);
        }
        velocity = toPlayer * 4.0f - mGravity * 25.0f;
        MR::setVelocity(this, velocity);
    }
    MR::rotateVecDegree(&mUp, _BC, 4.0f);
    MR::addVelocityToGravity(this, 1.4f);
    if (!MR::isLessStep(this, 10) && (MR::isBinded(this) || MR::isStep(this, 120))) {
        MR::zeroVelocity(this);
        MR::tryHitReaction(this, "PullBreak");
        setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreakSub));
    }
}

void KickStone::fn_801B3B60() {
    if (!MR::isNoBind(this)) {
        if (MR::isBinded(this)) {
            MR::zeroVelocity(this);
            MR::offBind(this);
            return;
        }
        MR::addVelocityToGravity(this, 1.0f);
    }
}

bool fn_801B3420(TVec3f* pUp, KickStone* pStone, s32 useCurrentUp) {
    MR::calcGravity(pStone);
    TVec3f gravity(pStone->mGravity);
    if (useCurrentUp) {
        gravity.set(-*pUp);
    }
    TVec3f line = gravity * 300.0f;
    TVec3f start = pStone->mPosition - gravity * 50.0f;
    TVec3f hit(0.0f);
    Triangle triangle;
    if (MR::getFirstPolyOnLineToMapExceptActor(&hit, &triangle, start, line, pStone)) {
        MR::setPosition(pStone, hit);
        pUp->set(*triangle.getFaceNormal());
        return true;
    }
    return false;
}
