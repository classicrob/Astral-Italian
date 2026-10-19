#pragma auto_inline off
// Nerve execute bodies fn_802A3120/fn_802A3130 are emitted as class methods (names match by bytes via namefn).
#include "Game/MapObj/FireRing.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"


namespace NrvFireRingBall {
    class FireRingBallNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static FireRingBallNrvWait sInstance;
    };
    FireRingBallNrvWait FireRingBallNrvWait::sInstance ATTRIBUTE_WEAK;

    class FireRingBallNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static FireRingBallNrvBindStarPointer sInstance;
    };
    FireRingBallNrvBindStarPointer FireRingBallNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvFireRingBall

void NrvFireRingBall::FireRingBallNrvWait::execute(Spine* pSpine) const {
    FireRingBall* pActor = reinterpret_cast< FireRingBall* >(pSpine->mExecutor);
    if (pActor->mBindStarPointerState->tryStartPointBind()) {
        pActor->setNerve(&NrvFireRingBall::FireRingBallNrvBindStarPointer::sInstance);
    }
}

void NrvFireRingBall::FireRingBallNrvBindStarPointer::execute(Spine* pSpine) const {
    LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
    MR::updateActorStateAndNextNerve(pActor, &NrvFireRingBall::FireRingBallNrvWait::sInstance);
}

FireRingBall::FireRingBall(LiveActor* pHost, const char* pName, const char* pModelName)
    : PartsModel(pHost, pName, pModelName, nullptr, -1, false) {
    mBindStarPointerState = nullptr;
    mFront.x = 0.0f;
    mFront.y = 1.0f;
    mFront.z = 0.0f;
    mUp.x = 0.0f;
    mUp.y = 0.0f;
    mUp.z = 1.0f;
    mShadowDropDir.x = 0.0f;
    mShadowDropDir.y = 0.0f;
    mShadowDropDir.z = 0.0f;
    mIsShadowDropDirPtr = false;
}

void FireRingBall::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);
    initNerve(&NrvFireRingBall::FireRingBallNrvWait::sInstance, 1);
    mBindStarPointerState = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindStarPointerState, &NrvFireRingBall::FireRingBallNrvBindStarPointer::sInstance, nullptr);
    MR::setShadowVolumeCylinderRadius(this, "Body", 30.0f * mScale.y);
}

void FireRingBall::movement() {
    f32 dist = MR::calcDistanceToPlayer(this);
    if (dist <= 1100.0f) {
        s32 pitch = (s32)((1.0f - dist / 1100.0f) * 1000.0f);
        MR::startActionSound(this, "MoveNear", pitch, -1, -1);
    }
    PartsModel::movement();
    if (!MR::isConnectToDrawTemporarily(this)) {
        MR::invalidateShadowAll(this);
    } else {
        MR::validateShadowAll(this);
    }
}

void FireRingBall::control() {}

void FireRingBall::calcAndSetBaseMtx() {
    MR::calcAndSetBaseMtxFromFrontAndUp(this, &mUp, &mFront);
}

void FireRingBall::setFrontAndUp(const TVec3f& rFront, const TVec3f& rUp) {
    mFront = rFront;
    mUp = rUp;
}

void FireRingBall::setShadowDropDirectionPtr() {
    MR::setShadowDropDirectionPtr(this, "BodySphere", &mShadowDropDir);
    mIsShadowDropDirPtr = true;
}

void FireRingBall::setShadowDropDirection(const TVec3f& rDir) {
    mShadowDropDir = rDir;
}

void FireRingBall::setShadowDropLength(f32 length) {
    MR::setShadowDropLength(this, "BodyCylinder", length);
}

bool FireRingBall::isNerveBindStarPointer() const {
    return isNerve(&NrvFireRingBall::FireRingBallNrvBindStarPointer::sInstance);
}

void FireRingBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isEqualSensor(pSender, this, "Attack")) {
            MR::sendMsgEnemyAttackFire(pReceiver, pSender);
        }
    }
}

FireRingBall::~FireRingBall() {}
