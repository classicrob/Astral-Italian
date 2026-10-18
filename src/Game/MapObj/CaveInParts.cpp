#include "Game/MapObj/CaveInParts.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace MR {
    void calcGravity(LiveActor*);
    bool isExistMapCollision(const TVec3f&, const TVec3f&);
    bool isGreaterStep(const LiveActor*, s32);
    bool isEqualSensor(const HitSensor*, LiveActor*, const char*);
};  // namespace MR



namespace NrvCaveInParts {
    NEW_NERVE(CaveInPartsNrvFall, CaveInParts, Fall);

    class CaveInPartsNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &CaveInPartsNrvFall::sInstance);
        };

        static CaveInPartsNrvFreeze sInstance;
    };
    CaveInPartsNrvFreeze CaveInPartsNrvFreeze::sInstance ATTRIBUTE_WEAK;

    class CaveInPartsNrvExplode : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            CaveInParts* actor = reinterpret_cast< CaveInParts* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::emitEffect(actor, "Explosion");
                actor->kill();
            }
        };

        static CaveInPartsNrvExplode sInstance ATTRIBUTE_WEAK;
    };
    CaveInPartsNrvExplode CaveInPartsNrvExplode::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvCaveInParts

CaveInParts::CaveInParts(CaveInCube* pCube, const char* pName, const char* pModelName) : LiveActor(pName) {
    mModelName = pModelName;
    mCube = pCube;
    mLife = 0;
    mMaxFallSpeed = 0.0f;
    mGravityAccel = 0.0f;
    mRotAxis.set(0.0f, 1.0f, 0.0f);
    mBindState = nullptr;
}

CaveInParts::~CaveInParts() {}

void CaveInParts::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, mModelName, false);
    MR::invalidateClipping(this);
    initNerve(&NrvCaveInParts::CaveInPartsNrvFall::sInstance, 1);
    MR::calcSideVec(&mRotAxis, this);
    mBindState = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindState, &NrvCaveInParts::CaveInPartsNrvFreeze::sInstance, nullptr);
    makeActorDead();
}

void CaveInParts::appear() {
    LiveActor::appear();
    MR::calcGravity(this);
    setNerve(&NrvCaveInParts::CaveInPartsNrvFall::sInstance);
}

void CaveInParts::control() {
    if (isNerve(&NrvCaveInParts::CaveInPartsNrvFall::sInstance)) {
        if (mBindState->tryStartPointBind()) {
            setNerve(&NrvCaveInParts::CaveInPartsNrvFreeze::sInstance);
        }
    }
}

void CaveInParts::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvCaveInParts::CaveInPartsNrvExplode::sInstance)) {
        return;
    }

    if (MR::isEqualSensor(pSender, this, "Attack")) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            setNerve(&NrvCaveInParts::CaveInPartsNrvExplode::sInstance);
        }
    }
}

bool CaveInParts::receiveMsgEnemyAttack(u32 msg, HitSensor*, HitSensor*) {
    if (isNerve(&NrvCaveInParts::CaveInPartsNrvExplode::sInstance)) {
        return false;
    }

    if (MR::isMsgEnemyAttack(msg)) {
        setNerve(&NrvCaveInParts::CaveInPartsNrvExplode::sInstance);
        return true;
    }

    return false;
}

void CaveInParts::setLifeAndSpeed(s32 life, f32 maxFallSpeed, f32 gravityAccel, f32 rotSpeed) {
    mLife = life;
    mMaxFallSpeed = maxFallSpeed;
    mGravityAccel = gravityAccel;
    mRotSpeed = rotSpeed;
}

void CaveInParts::exeFall() {
    const char* pBase = "Attack";
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::tryEmitEffect(this, pBase + 0x8);
    }

    MR::addVelocityToGravity(this, mGravityAccel);
    MR::restrictVelocity(this, mMaxFallSpeed);

    TVec3f rotVec = mRotAxis * mRotSpeed;
    mRotation += rotVec;
    MR::normalizeAngleDegree(mRotation.x);
    MR::normalizeAngleDegree(mRotation.y);
    MR::normalizeAngleDegree(mRotation.z);

    f32 radius = MR::getSensorRadius(this, pBase);
    TVec3f checkVec = mGravity * radius;
    if (MR::isExistMapCollision(mPosition, checkVec)) {
        MR::emitEffect(this, pBase + 0x10);
        kill();
    } else if (MR::isGreaterStep(this, mLife)) {
        kill();
    }
}
