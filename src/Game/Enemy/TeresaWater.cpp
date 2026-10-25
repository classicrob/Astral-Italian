#include "Game/Enemy/TeresaWater.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"

namespace {
    static const MR::ActorMoveParam sMoveParam = {0.5f, 0.0f, 0.9f, 1.5f};
    static const f32 sDefaultMoveLength = 800.0f;
    static const f32 sSideFriction = 0.5f;
    static const f32 sTargetRadius = 30.0f;
};  // namespace

namespace NrvTeresaWater {
    NEW_NERVE(TeresaWaterNrvMove, TeresaWater, Move);
    NEW_NERVE(TeresaWaterNrvHit, TeresaWater, Hit);
    NEW_NERVE(TeresaWaterNrvStun, TeresaWater, Stun);
    class TeresaWaterNrvDPDSwoon : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            MR::updateActorStateAndNextNerve(reinterpret_cast<LiveActor*>(spine->mExecutor), &TeresaWaterNrvMove::sInstance);
        }
        static TeresaWaterNrvDPDSwoon sInstance;
    };
    TeresaWaterNrvDPDSwoon TeresaWaterNrvDPDSwoon::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTeresaWater

TeresaWater::TeresaWater(const char* pName) : LiveActor(pName), mScaleController(), mBindStarPointer() {
    mFront.set(0.0f, 0.0f, 1.0f);
    mEndPos.set(0.0f, 0.0f, 0.0f);
    mStartPos.set(0.0f, 0.0f, 0.0f);
    mTargetPos = nullptr;
    mIsInWall = false;
}

void TeresaWater::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "TeresaWater", false);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::declareCoin(this, 1);
    MR::calcFrontVec(&mFront, this);
    f32 arg0 = ::sDefaultMoveLength;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    mStartPos.set(mPosition);
    mEndPos.scaleAdd(arg0, mFront, mStartPos);
    mTargetPos = &mEndPos;
    bool noShadow = false;
    MR::getJMapInfoArg1NoInit(rIter, &noShadow);
    if (noShadow) {
        MR::invalidateShadowAll(this);
    }
    initNerve(GET_NERVE(TeresaWater, TeresaWaterNrvMove), 1);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

// NONMATCHING: temporary pointers use fewer GPRs; wall effect scheduling differs.
void TeresaWater::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::moveAndTurnToTarget(this, &mFront, *mTargetPos, ::sMoveParam._0, ::sMoveParam._4, ::sMoveParam._8, ::sMoveParam._C);
    MR::attenuateVelocityExceptDirection(this, mFront, ::sSideFriction);

    if (mIsInWall) {
        TVec3f hit(0.0f);
        TVec3f start = mPosition - mFront * 240.0f;
        if (!MR::getFirstPolyOnLineToMap(&hit, nullptr, start, mFront * 360.0f)) {
            MR::showModel(this);
            mIsInWall = false;
            MR::emitEffectHit(this, start + mFront * 360.0f, "HideWall");
        }
    } else {
        TVec3f hit(0.0f);
        TVec3f start = mPosition - mFront * 60.0f;
        if (MR::getFirstPolyOnLineToMap(&hit, nullptr, start, mFront * 120.0f)) {
            MR::hideModelAndOnCalcAnim(this);
            mIsInWall = true;
            MR::emitEffectHit(this, hit, "HideWall");
        }
    }

    if (MR::isNear(this, *mTargetPos, ::sTargetRadius)) {
        if (mTargetPos == &mEndPos) {
            mTargetPos = &mStartPos;
        } else {
            mTargetPos = &mEndPos;
        }
    }
}

void TeresaWater::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        mVelocity.zero();
        MR::startActionSound(this, "EvTeresaAttackSuccess");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvMove));
    }
}

void TeresaWater::exeStun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvMove));
    }
}

void TeresaWater::kill() {
    MR::appearCoinPop(this, mPosition, 1);
    MR::emitEffect(this, "DeathWater");
    MR::startActionSound(this, "EmTeresaDead");
    MR::startActionSound(this, "EvTeresaDead");
    LiveActor::kill();
}

void TeresaWater::control() {
    mScaleController->updateNerve();
    MR::fn_800145C0(this);

    if (isNerve(GET_NERVE(TeresaWater, TeresaWaterNrvMove))) {
        MR::startActionSound(this, "EmLvTeresaMove");
    }

    if (tryDPDSwoon()) {
        return;
    }
}

void TeresaWater::calcAndSetBaseMtx() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    up.negate();

    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, up, mFront);
    MR::setBaseTRMtx(this, mtx);

    MR::updateBaseScale(this, mScaleController);
}

void TeresaWater::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(TeresaWater, TeresaWaterNrvHit))) {
        return;
    }

    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return;
    }

    if (!isNerve(GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon)) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
        setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvHit));
    } else {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool TeresaWater::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvStun));

        return true;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgSearchlightAttack(msg) || MR::isMsgInvincibleAttack(msg)) {
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        kill();

        return true;
    }

    return false;
}

bool TeresaWater::tryDPDSwoon() {
    if (isNerve(GET_NERVE(TeresaWater, TeresaWaterNrvStun))) {
        return false;
    }

    if (isNerve(GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon))) {
        return false;
    }

    if (!mBindStarPointer->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon));

    return true;
}

void TeresaWater::endDPDSwoon() {
    mBindStarPointer->kill();
}

void TeresaWater::exeDPDSwoon() {
    MR::updateActorStateAndNextNerve(this, mBindStarPointer, GET_NERVE(TeresaWater, TeresaWaterNrvMove));
}
