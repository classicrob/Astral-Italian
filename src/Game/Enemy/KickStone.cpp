#include "Game/Enemy/KickStone.hpp"
#include "Game/LiveActor/BreakModel.hpp"
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
    mMtx.identity();
    _BC.x = 0.0f;
    _BC.y = 0.0f;
    _BC.z = 0.0f;
}

void KickStone::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, nullptr, false);
    mBreakModel = MR::tryCreateBreakModel(this, reinterpret_cast<MtxPtr>(&mMtx));
    mMtx.set(getBaseMtx());
    mOffset.set(mPosition);

    bool isCoin = false;
    MR::getJMapInfoArg0NoInit(rIter, &isCoin);
    mIsCoin = isCoin;
    if (mIsCoin) {
        MR::declareCoin(this, 1);
    } else {
        MR::declareStarPiece(this, 1);
    }

    MR::getJMapInfoArg1NoInit(rIter, &mArg1);
    MR::getJMapInfoArg2NoInit(rIter, &mArg2);

    f32 rotDegree = 360.0f / 6.0f;
    s32 rnd = (s32)MR::getRandom(0L, 6L);
    MR::rotateMtxLocalYDegree(reinterpret_cast<MtxPtr>(&mMtx), (f32)rnd * rotDegree);

    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);

    TVec3f lockOn(0.0f, 0.0f, 0.0f);
    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), lockOn);
    MR::useStageSwitchAwake(this, rIter);
    initNerve(GET_KICKSTONE_NERVE(KickStoneNrvAttachTico), 0);
    MR::offBind(this);
    MR::tryRegisterDemoCast(this, rIter);
    makeActorAppeared();
}

// NONMATCHING: initAfterPlacement body partly recovered (12%).
void KickStone::initAfterPlacement() {
    if (!mArg2) {
        return;
    }
    // NONMATCHING: remaining body of initAfterPlacement is not recovered.
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
        MR::startActionSound(this, "KickStoneBreak", -1, -1, -1);
        MR::tryRumblePadVeryWeak(this, 0);
        if (isNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak))) {
            MR::stopScene(3);
        }
        MR::stopBck(this);
        MR::startAction(this, "Break");
        mOffset.set(mPosition);
        fn_801B3420(&mUp, this, 0);
        MR::zeroVelocity(this);
        MR::invalidateShadowAll(this);
        MR::forceDeleteEffect(this, "KickStoneBreak");
        return;
    }

    if (mBreakModel != nullptr) {
        if (mIsCoin) {
            if (!MR::isGalaxyDarkCometAppearInCurrentStage()) {
                MR::appearCoinPop(this, mPosition, 1);
            }
        } else {
            MR::appearStarPiece(this, mPosition, 1, 0.0f, 500.0f, false);
        }
        mBreakModel->kill();
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
        MR::tryHitReaction(this, "Tongue");
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

// NONMATCHING: the SMG2 rolling-move body (fn_801B38B0) is only approximated from the draft.
void KickStone::fn_801B38B0() {
    if (MR::isFirstStep(this)) {
        MR::calcGravity(this);
        TVec3f toPlayer(0.0f, 0.0f, 0.0f);
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        TVec3f cross;
        PSVECCrossProduct(&this->mUp, &toPlayer, &this->_BC);
        if (MR::isNearZero(this->_BC, 0.001f)) {
            this->_BC.set(this->mFront);
        }
        TVec3f vel = this->_BC * 0.0f;
        MR::setVelocity(this, vel);
    }

    MR::addVelocityToGravity(this, 0.0f);

    if (MR::isLessStep(this, 10)) {
        return;
    }

    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);
        MR::tryHitReaction(this, "Land");
        this->setNerve(GET_KICKSTONE_NERVE(KickStoneNrvBreak));
    }
}

// NONMATCHING: the falling/bound helper (fn_801B3B60) is approximated from the draft.
void KickStone::fn_801B3B60() {
    if (!MR::isNoBind(this)) {
        if (MR::isBinded(this)) {
            MR::zeroVelocity(this);
            MR::offBind(this);
            return;
        }
        MR::addVelocityToGravity(this, 0.0f);
    }
}
