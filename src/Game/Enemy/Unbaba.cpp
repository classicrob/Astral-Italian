#include "Game/Enemy/Unbaba.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace MR {
    VolumeDrawer* createVolumeDrawer(const char*, const char*, MtxPtr);
    bool getFirstPolyOnLineToMap(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
};  // namespace MR

// Unnamed SMG2 helpers in this unit (spline / movement math, not yet ported).

namespace {
    // Static parameter vectors (sbss at 0x807294E0).
    TVec3f sParam[3] = {TVec3f(1.2f, 1.0f, 1.4f), TVec3f(400.0f, 0.0f, -300.0f), TVec3f(0.09f, 0.09f, 0.09f)};
};  // namespace

namespace NrvUnbaba {
    NEW_NERVE(UnbabaNrvWait, Unbaba, Wait);
    NEW_NERVE(UnbabaNrvAppear, Unbaba, Appear);
    NEW_NERVE(UnbabaNrvAppearStep, Unbaba, AppearStep);
    NEW_NERVE(UnbabaNrvAttackSign, Unbaba, AttackSign);
    NEW_NERVE(UnbabaNrvAttack, Unbaba, Attack);
    NEW_NERVE(UnbabaNrvLaunch, Unbaba, Launch);
    NEW_NERVE(UnbabaNrvRestart, Unbaba, Restart);
};  // namespace NrvUnbaba

#define UNBABA_NERVE(n) (&NrvUnbaba::UnbabaNrv##n::sInstance)

Unbaba::Unbaba(const char* pName) : LiveActor(pName), mVolumeDrawer(nullptr) {
    mPos.set(0.0f, 0.0f, 0.0f);
    mRotate.set(0.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 0.0f, 0.0f);
    mHeight = 0.0f;
    mDistance = 0.0f;
    mT0 = 0.0f;
    mTimer = 0.0f;
    mStepAttackSign = 30;
    mStepAttack = 20;
    mIsScaleUp = false;
    mIsAttackBlocked = false;
    mMtx.identity();
    mLaunchMtx.identity();
}

Unbaba::~Unbaba() {}

void Unbaba::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Unbaba", false);
    mVolumeDrawer = MR::createVolumeDrawer("Unbaba", "Unbaba", mMtx.toMtxPtr());
    mVolumeDrawer->setHost(this);
    f32 dist = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &dist);
    if (dist == -1.0f) {
        dist = 500.0f;
    }
    mDistance = dist;
    MR::calcUpVec(&mUpVec, this);
    initNerve(UNBABA_NERVE(Wait), 0);
    MR::getJMapInfoArg1NoInit(rIter, &mStepAttackSign);
    MR::getJMapInfoArg2NoInit(rIter, &mStepAttack);
    MR::getJMapInfoArg3NoInit(rIter, &mIsScaleUp);
    MR::getJMapInfoTrans(rIter, &mPos);
    MR::getJMapInfoRotate(rIter, &mRotate);
    calcAndSetBaseMtx();
}

void Unbaba::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(UNBABA_NERVE(Appear));
    MR::startBckNoInterpole(this, "Wait");
    MR::setPosition(this, mPos);
    MR::setRotation(this, mRotate);
    TPos3f mtx;
    mtx.identity();
    TVec3f scale(1.0f);
    MR::makeMtxTR(mtx.toMtxPtr(), scale, mRotate);
    fn_80215FB0();
}

void Unbaba::makeActorDead() {
    LiveActor::makeActorDead();
}

bool Unbaba::receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) {
    return false;
}

void Unbaba::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            setNerve(UNBABA_NERVE(AppearStep));
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(UNBABA_NERVE(AppearStep));
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
}

void Unbaba::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBckNoInterpole(this, "Wait");
        MR::validateClipping(this);
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            setNerve(UNBABA_NERVE(AppearStep));
            return;
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(UNBABA_NERVE(AppearStep));
        return;
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
    if (MR::isFirstStep(this)) {
        setNerve(UNBABA_NERVE(Wait));
    }
}

void Unbaba::exeAppearStep() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::invalidateClipping(this);
    }
    if (MR::isGreaterStep(this, mStepAttackSign)) {
        setNerve(UNBABA_NERVE(AttackSign));
    }
    fn_80215E50();
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "AttackSign");
    }
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Attack));
    }
    fn_80215E50();
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "Attack");
    }
    mIsScaleUp = !MR::isGreaterStep(this, 0x58);
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Launch));
    }
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeLaunch() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
    if (MR::isGreaterStep(this, mStepAttack)) {
        mHeight = -2500.0f * mScale.x;
        mT0 = 0.0f;
        setNerve(UNBABA_NERVE(Restart));
    }
    fn_80215EF0();
}

void Unbaba::exeRestart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Restart");
    }
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Wait));
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
}

void Unbaba::control() {
    PSMTXCopy(MR::getJointMtx(this, "Head"), mMtx.toMtxPtr());
    MR::addTransMtxLocal(mMtx.toMtxPtr(), sParam[1]);
    TVec3f scaled = sParam[0] * 10.0f;
    MR::preScaleMtx(mMtx.toMtxPtr(), scaled);
    Color8 color(0x8C, 0x28, 0x00, 0x87);
    mVolumeDrawer->setColor(color);
    mVolumeDrawer->setDrawPasses(4);
    mVolumeDrawer->setScale(sParam[2]);
    bool scaleUp = mIsScaleUp && !mIsAttackBlocked;
    if (scaleUp) {
        TVec3f pos;
        MR::extractMtxTrans(mMtx.toMtxPtr(), &pos);
        TVec3f diff(*MR::getPlayerPos());
        diff -= pos;
        if (diff.length() > 3500.0f) {
            scaleUp = false;
        }
    }
    if (scaleUp) {
        mVolumeDrawer->setScaleUpRate(60);
    } else {
        mVolumeDrawer->setScaleDownRate(60);
    }
    mIsScaleUp = false;
}

void Unbaba::calcAndSetBaseMtx() {
    TVec3f offset = mUpVec * mHeight;
    TVec3f pos = mPos;
    pos += offset;
    MR::setPosition(this, pos);
    LiveActor::calcAndSetBaseMtx();
}

void Unbaba::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            if (MR::sendMsgEnemyAttackFireStrong(pReceiver, pSender)) {
            }
        }
    } else if (MR::isSensorEnemy(pSender)) {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    }
}


// Timer-driven height update used while the Unbaba is attacking (leaf helper).
bool Unbaba::fn_80215E50() {
    this->mTimer += 1.0f / 130.0f;
    MR::clamp01(&this->mTimer);
    f32 t = fn_80215DA0(this->mTimer, 0.0f, 0.6f, 1.1f, 0.6f, 0.85f, 0.3f);
    this->mHeight = this->mDistance * t + -2500.0f * this->mScale.x;
    return 1.0f == this->mTimer;
}

bool Unbaba::fn_80215EF0() {
    this->mTimer -= 1.0f / 20.0f;
    MR::clamp01(&this->mTimer);
    f32 t = fn_80215DA0(this->mTimer, 3.0f, 0.75f, 1.0f, -0.9f, 0.92f, 0.55f);
    this->mHeight = this->mDistance * t + -2500.0f * this->mScale.x;
    return 0.0f == this->mTimer;
}
