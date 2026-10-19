#include "Game/Enemy/Birikyu.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace NrvBirikyu {
    NEW_NERVE(HostTypeMove, Birikyu, Move);
    NEW_NERVE(HostTypeMoveCircle, Birikyu, MoveCircle);
    NEW_NERVE(HostTypeAttack, Birikyu, Attack);
    NEW_NERVE(HostTypeAttackWait, Birikyu, AttackWait);
    NEW_NERVE(HostTypeWaitAtEdge, Birikyu, WaitAtEdge);
    NEW_NERVE_ONEND(HostTypeStopPointing, Birikyu, StopPointing, StopPointing);
    class HostTypeKill : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            LiveActor* actor = reinterpret_cast<LiveActor*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::tryHitReactionDead(actor);
                actor->kill();
            }
        }
        static HostTypeKill sInstance;
    };
    HostTypeKill HostTypeKill::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBirikyu

Birikyu::Birikyu(const char* pName)
    : LiveActor(pName), mParamScale(), mObjectName(), mRailClippingCenter(gZeroVec), mOrbitCenter(gZeroVec), mIsShadowEnabled(), mIsRailMovement(), mOrbitAxis(0.0f, 1.0f, 0.0f), mOrbitFront(0.0f, 0.0f, 1.0f), mOrbitAngle(), mMoveSpeed(10.0f) {
}

void Birikyu::control() {
    mParamScale->calcHostVelocity();
}

void Birikyu::movement() {
    mParamScale->update();
    LiveActor::movement();
    mParamScale->resetHostVelocity();
}

void Birikyu::init(const JMapInfoIter& rIter) {
    MR::getObjectName(&mObjectName, rIter);
    MR::initActor(this, rIter, mObjectName, false);
    initFromJmpArgs(rIter);
    initRail(rIter);

    if (!mIsShadowEnabled) {
        MR::invalidateShadowAll(this);
    }

    MR::joinToGroupArray(this, rIter, getName(), 64);

    mParamScale = new ActorStateParamScale(this);
    mParamScale->init(rIter);

    if (mIsRailMovement) {
        initNerve(GET_NERVE(Birikyu, HostTypeMove));
    } else {
        initNerve(GET_NERVE(Birikyu, HostTypeMoveCircle));
    }

    appear();
}

void Birikyu::initAfterPlacement() {
    if (mIsRailMovement) {
        MR::moveCoordAndTransToNearestRailPos(this);
    } else {
        mOrbitCenter.set(mPosition);
        TPos3f matrix;
        matrix.identity();
        MR::makeMtxRotate(matrix.toMtxPtr(), mRotation);
        matrix.getYDir(mOrbitAxis);
        MR::normalize(&mOrbitAxis);
        matrix.getZDir(mOrbitFront);
        MR::normalize(&mOrbitFront);
        MR::setPosition(this, mOrbitCenter + mOrbitFront * 400.0f);
    }
}

void Birikyu::appear() {
    LiveActor::appear();
    MR::emitEffect(this, mObjectName);
}

f32 Birikyu::getHitRadius() const {
    return 120.0f;
}

char* Birikyu::getCenterJointName() const {
    return "Root";
}

void Birikyu::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshiOrRide(pReceiver) || MR::isSensorEnemy(pReceiver)) {
        if (MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
            if (isNerve(GET_NERVE(Birikyu, HostTypeStopPointing))) {
                return;
            }
            MR::sendMsgToGroupMember(123 /* ACTMES_GROUP_MOVE_STOP */, this, getSensor("Body"), "Body");
            setNerve(GET_NERVE(Birikyu, HostTypeAttack));
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Birikyu::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgInvincibleAttack(msg)) {
        if (!isNerve(GET_NERVE(Birikyu, HostTypeKill))) {
            setNerve(GET_NERVE(Birikyu, HostTypeKill));
            return true;
        }
    }

    return false;
}

bool Birikyu::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 123 /* ACTMES_GROUP_MOVE_STOP */) {
        if (MR::getSensorHost(pSender) != this) {
            bool isMoveCircle = false;
            if (isNerve(GET_NERVE(Birikyu, HostTypeMove)) || isNerve(GET_NERVE(Birikyu, HostTypeMoveCircle))) {
                isMoveCircle = true;
            }

            if (isMoveCircle) {
                setNerve(GET_NERVE(Birikyu, HostTypeAttackWait));
                return true;
            }
        }
    }

    if (msg == 122 /* ACTMES_GROUP_MOVE_START */) {
        if (MR::getSensorHost(pSender) != this) {
            bool isMoving = false;
            if (isNerve(GET_NERVE(Birikyu, HostTypeMove)) || isNerve(GET_NERVE(Birikyu, HostTypeMoveCircle))) {
                isMoving = true;
            }
            if (!isMoving) {
                goMove();
                return true;
            }
        }
    }

    return false;
}

void Birikyu::startClipped() {
    LiveActor::startClipped();
    MR::forceDeleteEffect(this, mObjectName);
}

void Birikyu::endClipped() {
    LiveActor::endClipped();
    MR::emitEffect(this, mObjectName);
}

void Birikyu::initFromJmpArgs(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mMoveSpeed);
    s32 num = -1;
    MR::getJMapInfoArg2NoInit(rIter, &num);
    mIsShadowEnabled = num == true;
}

void Birikyu::initRail(const JMapInfoIter& rIter) {
    bool result = MR::isConnectedWithRail(rIter);
    mIsRailMovement = result;
    if (mIsRailMovement) {
        MR::initAndSetRailClipping(&mRailClippingCenter, this, 100.0f, 500.0f);
    }
}

void Birikyu::initCollision() {
    initHitSensor(1);
    TVec3f offset(0.0f, 0.0f, 0.0f);
    f32 radius = getHitRadius();
    radius *= mScale.x;
    MR::addHitSensorAtJointEnemy(this, "Body", getCenterJointName(), 16, radius, offset);
}

void Birikyu::initShadow() {
    if (mIsShadowEnabled) {
        f32 num = getHitRadius();
        MR::initShadowVolumeSphere(this, (0.8f * (num * mScale.x)));
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::setShadowDropPositionPtr(this, nullptr, &(*MR::getSensorPos(this, "Body")));
    }
}

bool Birikyu::tryStopPointing() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        MR::sendMsgToGroupMember(123 /* ACTMES_GROUP_MOVE_STOP */, this, getSensor("Body"), "Body");
        setNerve(GET_NERVE(Birikyu, HostTypeStopPointing));

        return true;
    }

    return false;
}

void Birikyu::goMove() {
    if (mIsRailMovement) {
        setNerve(GET_NERVE(Birikyu, HostTypeMove));
    } else {
        setNerve(GET_NERVE(Birikyu, HostTypeMoveCircle));
    }
}

void Birikyu::exeMove() {
    MR::startActionSound(this, "OjLvBirikyuMove");

    if (!tryStopPointing()) {
        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
            s32 arg = 0;
            MR::getCurrentRailPointArg0NoInit(this, &arg);

            if (arg > 0) {
                setNerve(GET_NERVE(Birikyu, HostTypeWaitAtEdge));
                return;
            } else {
                MR::emitEffect(this, "Clash");
            }
        }

        MR::moveCoordAndFollowTrans(this, mMoveSpeed * mParamScale->getSpeed());
    }
}

void Birikyu::exeMoveCircle() {
    MR::startActionSound(this, "OjLvBirikyuMove");
    if (!tryStopPointing()) {
        mOrbitAngle = MR::repeat(mOrbitAngle + ((mMoveSpeed / 400.0f) * mParamScale->getSpeed()), 0.0f, TWO_PI);
        TPos3f matrix;
        matrix.identity();
        matrix.makeRotate(mOrbitAxis, mOrbitAngle);
        TVec3f temp = mOrbitFront * 400.0f;
        matrix.mult(temp, temp);
        TVec3f matrix2 = (mOrbitCenter + temp);
        MR::setPosition(this, matrix2);
    }
}

void Birikyu::exeWaitAtEdge() {
    MR::startActionSound(this, "OjLvBirikyuMove");
    s32 arg = 0;
    MR::getCurrentRailPointArg0NoInit(this, &arg);

    if (mParamScale->isStep(arg)) {
        setNerve(GET_NERVE(Birikyu, HostTypeMove));
    }
}

void Birikyu::exeAttack() {
    if (mParamScale->isFirstStep()) {
        mVelocity.zero();
        MR::emitEffect(this, "Hit");
    }

    MR::startActionSound(this, "OjLvBirikyuMove");

    if (mParamScale->isStep(90)) {
        MR::sendMsgToGroupMember(122 /* ACTMES_GROUP_MOVE_START */, this, getSensor("Body"), "Body");
        goMove();
    }
}

void Birikyu::exeAttackWait() {
}

void Birikyu::exeStopPointing() {
    if (mParamScale->isFirstStep()) {
        MR::startDPDHitSound();
        if (MR::isRegisteredEffect(this, "Touch")) {
            MR::emitEffect(this, "Touch");
        }
    }

    MR::startDPDFreezeLevelSound(this);

    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        goMove();
    }
}

void Birikyu::endStopPointing() {
    if (MR::isRegisteredEffect(this, "Touch")) {
        MR::deleteEffect(this, "Touch");
    }

    MR::sendMsgToGroupMember(122 /* ACTMES_GROUP_MOVE_START */, this, getSensor("Body"), "Body");
}

BirikyuWithFace::BirikyuWithFace(const char* pName) : Birikyu(pName) {
    mIsFaceReversed = false;
    mOrientationMode = -1;
}

void BirikyuWithFace::init(const JMapInfoIter& rIter) {
    Birikyu::init(rIter);
    s32 num = -1;
    MR::getJMapInfoArg1NoInit(rIter, &num);
    mIsFaceReversed = num == 1;
    MR::getJMapInfoArg3NoInit(rIter, &mOrientationMode);

    if (static_cast<u32>(mOrientationMode) <= 1) {
        MR::onCalcGravity(this);
    }
}

void BirikyuWithFace::initAfterPlacement() {
    Birikyu::initAfterPlacement();

    if (mOrientationMode == 1) {
        MR::calcGravity(this);
        MR::offCalcGravity(this);
    }
}

void BirikyuWithFace::calcAndSetBaseMtx() {
    if (mOrientationMode == -1) {
        LiveActor::calcAndSetBaseMtx();
        return;
    }

    TPos3f vec3;
    vec3.identity();
    if (mIsRailMovement) {
        TVec3f vec(MR::getRailDirection(this));
        if (mIsFaceReversed) {
            vec.negate();
        }

        if (!MR::isRailGoingToEnd(this)) {
            vec.negate();
        }

        TVec3f vec2(mOrbitAxis);
        if (static_cast<u32>(mOrientationMode) <= 1) {
            vec2.set(-mGravity);
        }
        if (MR::isSameDirection(vec2, vec)) {
            MR::makeMtxSideFront(&vec3, vec, mOrbitFront);
        } else {
            MR::makeMtxSideUp(&vec3, vec, vec2);
        }
    } else {
        vec3.makeRotate(mOrbitAxis, mOrbitAngle);
    }

    vec3.mMtx[0][3] = mPosition.x;
    vec3.mMtx[1][3] = mPosition.y;
    vec3.mMtx[2][3] = mPosition.z;
    MR::setBaseTRMtx(this, vec3);
}

f32 BirikyuWithFace::getHitRadius() const {
    return 50.0f;
}

char* BirikyuWithFace::getCenterJointName() const {
    return "Center";
}
