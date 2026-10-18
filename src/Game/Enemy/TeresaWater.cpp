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

TeresaWater::TeresaWater(const char* pName) : LiveActor(pName), _8C(), _90() {
    _94.set(0.0f, 0.0f, 1.0f);
    _A0.set(0.0f, 0.0f, 0.0f);
    _AC.set(0.0f, 0.0f, 0.0f);
    _B8 = nullptr;
    _C0 = false;
}

void TeresaWater::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "TeresaWater", false);
    _8C = new AnimScaleController(nullptr);
    _90 = new WalkerStateBindStarPointer(this, _8C);
    MR::declareCoin(this, 1);
    MR::calcFrontVec(&_94, this);
    f32 arg0 = ::sDefaultMoveLength;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    _AC.set(mPosition);
    _A0.scaleAdd(arg0, _94, _AC);
    _B8 = &_A0;
    bool noShadow = false;
    MR::getJMapInfoArg1NoInit(rIter, &noShadow);
    if (noShadow) {
        MR::invalidateShadowAll(this);
    }
    initNerve(GET_NERVE(TeresaWater, TeresaWaterNrvMove), 1);
    MR::initActorState(this, _90, GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

// NONMATCHING: temporary pointers use fewer GPRs; wall effect scheduling differs.
void TeresaWater::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::moveAndTurnToTarget(this, &_94, *_B8, ::sMoveParam._0, ::sMoveParam._4, ::sMoveParam._8, ::sMoveParam._C);
    MR::attenuateVelocityExceptDirection(this, _94, ::sSideFriction);

    if (_C0) {
        TVec3f hit(0.0f);
        TVec3f start = mPosition - _94 * 240.0f;
        if (!MR::getFirstPolyOnLineToMap(&hit, nullptr, start, _94 * 360.0f)) {
            MR::showModel(this);
            _C0 = false;
            const TVec3f& effectPos = start + _94 * 360.0f;
            MR::emitEffectHit(this, effectPos, "HideWall");
        }
    } else {
        TVec3f hit(0.0f);
        TVec3f start = mPosition - _94 * 60.0f;
        if (MR::getFirstPolyOnLineToMap(&hit, nullptr, start, _94 * 120.0f)) {
            MR::hideModelAndOnCalcAnim(this);
            _C0 = true;
            MR::emitEffectHit(this, hit, "HideWall");
        }
    }

    if (MR::isNear(this, *_B8, ::sTargetRadius)) {
        if (_B8 == &_A0) {
            _B8 = &_AC;
        } else {
            _B8 = &_A0;
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
    _8C->updateNerve();
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
    MR::calcMtxFromGravityAndZAxis(&mtx, this, up, _94);
    MR::setBaseTRMtx(this, mtx);

    MR::updateBaseScale(this, _8C);
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

    if (!_90->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(TeresaWater, TeresaWaterNrvDPDSwoon));

    return true;
}

void TeresaWater::endDPDSwoon() {
    _90->kill();
}

void TeresaWater::exeDPDSwoon() {
    MR::updateActorStateAndNextNerve(this, _90, GET_NERVE(TeresaWater, TeresaWaterNrvMove));
}
