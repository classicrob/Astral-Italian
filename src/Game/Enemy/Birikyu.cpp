#include "Game/Enemy/Birikyu.hpp"
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
    NEW_NERVE(HostTypeNrvKill, Birikyu, Kill);
};  // namespace NrvBirikyu

void Birikyu_FORCE_MATCH(const TVec3f& rVec) {
    TVec3f vec(rVec);
    vec * 1.0f;
    vec + TVec3f(1.0f);
}

Birikyu::Birikyu(const char* pName)
    : LiveActor(pName), mParamScale(), _8C(), _90(gZeroVec), _9C(gZeroVec), _A8(), _A9(), _AC(0.0f, 1.0f, 0.0f), _B8(0.0f, 0.0f, 1.0f), _C4(), _C8(10.0f) {
}

// SMG2 step state (48 bytes); its ctor and init are out of line in SB4E01.
class ActorStateParamScale {
public:
    ActorStateParamScale(LiveActor*);
    void init(const JMapInfoIter&);
    void calcHostVelocity();

    char mStateData[0x30 - 4];
};

void Birikyu::control() {
    mParamScale->calcHostVelocity();
}

void Birikyu::init(const JMapInfoIter& rIter) {
    MR::getObjectName(&_8C, rIter);
    MR::initActor(this, rIter, _8C, false);
    initFromJmpArgs(rIter);
    initRail(rIter);

    if (!_A8) {
        MR::invalidateShadowAll(this);
    }

    MR::joinToGroupArray(this, rIter, getName(), 64);

    mParamScale = new ActorStateParamScale(this);
    mParamScale->init(rIter);

    if (_A9) {
        initNerve(GET_NERVE(Birikyu, HostTypeMove));
    } else {
        initNerve(GET_NERVE(Birikyu, HostTypeMoveCircle));
    }

    appear();
}

void Birikyu::initAfterPlacement() {
    if (_A9) {
        MR::moveCoordAndTransToNearestRailPos(this);
    } else {
        _9C.set(mPosition);
        TPos3f matrix;
        matrix.identity();
        MR::makeMtxRotate(matrix.toMtxPtr(), mRotation);
        matrix.getYDir(_AC);
        MR::normalize(&_AC);
        matrix.getZDir(_B8);
        MR::normalize(&_B8);
        MR::setPosition(this, _9C + _B8 * 400.0f);
    }
}

void Birikyu::appear() {
    LiveActor::appear();
    MR::emitEffect(this, _8C);
}

f32 Birikyu::getHitRadius() const {
    return 120.0f;
}

char* Birikyu::getCenterJointName() const {
    return "Root";
}

void Birikyu::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) || MR::isSensorEnemy(pReceiver)) {
        if (MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
            MR::sendMsgToGroupMember(123 /* ACTMES_GROUP_MOVE_STOP */, this, getSensor("body"), "body");
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
        if (!isNerve(GET_NERVE(Birikyu, HostTypeNrvKill))) {
            setNerve(GET_NERVE(Birikyu, HostTypeNrvKill));
            return true;
        }
    }

    return false;
}

bool Birikyu::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 123 /* ACTMES_GROUP_MOVE_STOP */) {
        if (MR::getSensorHost(pSender) == this) {
            bool isMoveCircle = false;

            if (!isNerve(GET_NERVE(Birikyu, HostTypeMove))) {
                isMoveCircle = isNerve(GET_NERVE(Birikyu, HostTypeMoveCircle));
            }

            if (isMoveCircle) {
                setNerve(GET_NERVE(Birikyu, HostTypeAttackWait));
                return true;
            }
        }
    }

    if (msg == 122 /* ACTMES_GROUP_MOVE_START */) {
        if (MR::getSensorHost(pSender) == this) {
            if (!isNerve(GET_NERVE(Birikyu, HostTypeMove)) && !isNerve(GET_NERVE(Birikyu, HostTypeMoveCircle))) {
                goMove();
                return true;
            }
        }
    }

    return false;
}

void Birikyu::startClipped() {
    LiveActor::startClipped();
    MR::forceDeleteEffect(this, _8C);
}

void Birikyu::endClipped() {
    LiveActor::endClipped();
    MR::emitEffect(this, _8C);
}

void Birikyu::initFromJmpArgs(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &_C8);
    s32 num = -1;
    MR::getJMapInfoArg2NoInit(rIter, &num);
    _A8 = num == true;
}

void Birikyu::initRail(const JMapInfoIter& rIter) {
    bool result = MR::isConnectedWithRail(rIter);
    _A9 = result;
    if (_A9) {
        MR::initAndSetRailClipping(&_90, this, 100.0f, 500.0f);
    }
}

void Birikyu::initCollision() {
    initHitSensor(1);
    TVec3f offset(0.0f, 0.0f, 0.0f);
    f32 radius = getHitRadius();
    radius *= mScale.x;
    MR::addHitSensorAtJointEnemy(this, "body", getCenterJointName(), 16, radius, offset);
}

void Birikyu::initShadow() {
    if (_A8) {
        f32 num = getHitRadius();
        MR::initShadowVolumeSphere(this, (0.8f * (num * mScale.x)));
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::setShadowDropPositionPtr(this, nullptr, &(*MR::getSensorPos(this, "body")));
    }
}

bool Birikyu::tryStopPointing() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        MR::sendMsgToGroupMember(123 /* ACTMES_GROUP_MOVE_STOP */, this, getSensor("body"), "body");
        setNerve(GET_NERVE(Birikyu, HostTypeStopPointing));

        return true;
    }

    return false;
}

void Birikyu::goMove() {
    if (_A9) {
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

        MR::moveCoordAndFollowTrans(this, _C8);
    }
}

void Birikyu::exeMoveCircle() {
    MR::startActionSound(this, "OjLvBirikyuMove");
    if (!tryStopPointing()) {
        _C4 = MR::repeat(_C4 + (_C8 / 400.0f), 0.0f, TWO_PI);
        TPos3f matrix;
        matrix.identity();
        matrix.makeRotate(_AC, _C4);
        TVec3f temp = _B8 * 400.0f;
        matrix.mult(temp, temp);
        TVec3f matrix2 = (_9C + temp);
        MR::setPosition(this, matrix2);
    }
}

void Birikyu::exeWaitAtEdge() {
    MR::startActionSound(this, "OjLvBirikyuMove");
    s32 arg = 0;
    MR::getCurrentRailPointArg0NoInit(this, &arg);

    if (MR::isStep(this, arg)) {
        setNerve(GET_NERVE(Birikyu, HostTypeMove));
    }
}

void Birikyu::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::emitEffect(this, "Hit");
    }

    MR::startActionSound(this, "OjLvBirikyuMove");

    if (MR::isStep(this, 90)) {
        MR::sendMsgToGroupMember(122 /* ACTMES_GROUP_MOVE_START */, this, getSensor("body"), "body");
        goMove();
    }
}

void Birikyu::exeAttackWait() {
}

void Birikyu::exeStopPointing() {
    if (MR::isFirstStep(this)) {
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

    MR::sendMsgToGroupMember(122 /* ACTMES_GROUP_MOVE_START */, this, getSensor("body"), "body");
}

BirikyuWithFace::BirikyuWithFace(const char* pName) : Birikyu(pName) {
    _CC = false;
}

void BirikyuWithFace::init(const JMapInfoIter& rIter) {
    Birikyu::init(rIter);
    s32 num = -1;
    MR::getJMapInfoArg1NoInit(rIter, &num);
    _CC = num == 1;
    MR::getJMapInfoArg3NoInit(rIter, &_D8);

    if (static_cast<u32>(_D8) <= 1) {
        MR::onCalcGravity(this);
    }
}

void BirikyuWithFace::initAfterPlacement() {
    Birikyu::initAfterPlacement();

    if (_D8 == 1) {
        MR::calcGravity(this);
        MR::offCalcGravity(this);
    }
}

void BirikyuWithFace::calcAndSetBaseMtx() {
    TPos3f vec3;
    vec3.identity();
    if (_A9) {
        TVec3f vec(MR::getRailDirection(this));
        if (_CC) {
            vec.negate();
        }

        if (!MR::isRailGoingToEnd(this)) {
            vec.negate();
        }

        TVec3f vec2(_AC);
        if (MR::isSameDirection(vec2, vec)) {
            MR::makeMtxSideFront(&vec3, vec, _B8);
        } else {
            MR::makeMtxSideUp(&vec3, vec, vec2);
        }
    } else {
        vec3.makeRotate(_AC, _C4);
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
