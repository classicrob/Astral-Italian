#include "Game/Enemy/CocoNutBall.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CocoNut.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MathUtil.hpp"

namespace {
    const Vec cReboundVelocity = {0.0f, 15.0f, 5.0f};
};  // namespace

namespace NrvCocoNutBall {
    NEW_NERVE(CocoNutBallNrvThrow, CocoNutBall, Throw);
    NEW_NERVE(CocoNutBallNrvHitBackToHost, CocoNutBall, HitBackToHost);
    NEW_NERVE(CocoNutBallNrvHitBackToPlayer, CocoNutBall, HitBackToPlayer);
    NEW_NERVE(CocoNutBallNrvRebound, CocoNutBall, Rebound);
    NEW_NERVE(CocoNutBallNrvFreeze, CocoNutBall, Freeze);
    NEW_NERVE(CocoNutBallNrvFreezeRelease, CocoNutBall, FreezeRelease);
};  // namespace NrvCocoNutBall

void CocoNutBall_FORCE_MATCH_SDATA2() {
    (void) 1.0f;
    (void) 0.0f;
    (void) MR::epsilon();
    (void) 3.0f;
    (void) -1.0f;
    (void) 2.0f;
}

CocoNutBall::CocoNutBall(const char* pName)
    : LiveActor(pName), _8C(), _90(0.0f, -1.0f, 0.0f), _9C(), _A0(), _A4(gZeroVec), _B0(gZeroVec), _BC(), _BD(), _BE(), _C0(450.0f), _C4(),
      _C8(0.0f, 1.0f, 0.0f), _D4(10000.0f), _D8(), _DD(), _DE(), _E0(0.0f), _E4(0) {
}

void CocoNutBall::initSensors() {
    initHitSensor(2);
    MR::addHitSensor(this, "Body", 0x1B, 8, 40.0f * mScale.x, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensor(this, "Bind", 0x1B, 8, 500.0f, TVec3f(0.0f, 0.0f, 0.0f));
}

void CocoNutBall::init(const JMapInfoIter& rIter) {
    MR::initActor(this, CocoNut::getModelName(), false);
    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);
    MR::initLightCtrl(this);
    initSensors();
    initBinder(60.0f, 0.0f, 0);
    initEffectKeeper(0, "CocoNut", false);
    MR::initStarPointerTarget(this, 150.0f, TVec3f(0, 0, 0));
    initSound(4, "CocoNutBall", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeCylinder(this, 60.0f);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow));
    makeActorDead();
}

void CocoNutBall::appear() {
    LiveActor::appear();
    _9C = 0;
    MR::onBind(this);
    setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow));

    if (_DE) {
        _E4 = 60;
    }
}

void CocoNutBall::kill() {
    MR::forceDeleteEffect(this, "CocoNutBlur");
    MR::forceDeleteEffect(this, "CocoNutLight");
    MR::startActionSound(this, "OjCoconutBallBreak");

    if (_9C > 0) {
        MR::sendArbitraryMsg(0xCC, _8C->getSensor("body"), getSensor("body"));
    }

    LiveActor::kill();
    MR::emitEffect(this, CocoNut::getBreakEffectName());
}

#pragma push
#pragma dont_inline on
#pragma push
#pragma dont_inline on
void CocoNutBall::appearAndThrow(const TVec3f& rAppearPos, f32 f1) {
    mPosition.set(rAppearPos);
    MR::calcGravity(this);
    _C8.negate(mGravity);
    setVelocityToPlayer(15.0f, f1);
    _BD = MR::isHalfProbability();
    appear();
}
#pragma pop
#pragma pop

void CocoNutBall::hitBackToPlayer() {
    f32 rand = MR::getRandom(12.5f, 17.5f);

    if (_BD) {
        rand = -rand;
        _BD = false;
    } else {
        _BD = true;
    }

    setVelocityToPlayer(3.0f * _9C + 15.0f, rand);
    setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer));
}

void CocoNutBall::demoBreak(const TVec3f& rPos) {
    mPosition.set(rPos);
    mRotation.zero();
    mVelocity.zero();

    TPos3f TRMtx;
    MR::makeMtxTR(TRMtx.toMtxPtr(), this);
    MR::setBaseTRMtx(this, TRMtx);

    MR::emitEffect(this, "Hit");
    MR::startActionSound(this, "OjCoconutBallBreak");
    MR::emitEffect(this, CocoNut::getBreakEffectName());
}

void CocoNutBall::calcAndSetBaseMtx() {
    if (!MR::isNearZero(mVelocity)) {
        TVec3f velocityDirection;
        MR::normalize(&velocityDirection, mVelocity);
        TPos3f baseTRMtx;
        if (MR::isSameDirection(velocityDirection, _C8, 0.01f)) {
            MR::makeMtxFrontNoSupportPos(&baseTRMtx, velocityDirection, mPosition);
        } else {
            MR::makeMtxFrontUpPos(&baseTRMtx, velocityDirection, _C8, mPosition);
        }
        MR::setBaseTRMtx(this, baseTRMtx);
    }
}

bool CocoNutBall::isSensorBody(HitSensor* pSensor) const {
    return pSensor == getSensor("body");
}

void CocoNutBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isSensorBody(pSender) && MR::isSensorPlayer(pReceiver)) {
        if (isNerveTrowToOrFreeze() && MR::sendMsgEnemyAttackFlipVeryWeak(pReceiver, pSender)) {
            setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound));
            return;
        }
    }

    if (MR::isSensorEnemy(pReceiver) && isSensorBody(pSender)) {
        if (pReceiver->mHost == _8C) {
            if (isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost))) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    MR::emitEffect(this, "Hit");
                    kill();
                }
            }

            return;
        }

        if (isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow)) || isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost)) ||
            isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer))) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                kill();
            }
        }
    }
}

bool CocoNutBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (isValidReceivePunch()) {
            setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost));
            return true;
        }
    } else if (isSensorBody(pReceiver) && MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    return false;
}

HitSensor* CocoNutBall::isBindedAny() const {
    if (MR::isBindedGround(this)) {
        return MR::getGroundSensor(this);
    }

    if (MR::isBindedWall(this)) {
        return MR::getWallSensor(this);
    }

    if (MR::isBindedRoof(this)) {
        return MR::getRoofSensor(this);
    }

    return nullptr;
}

bool CocoNutBall::isNerveTrowToOrFreeze() const {
    return isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow)) || isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer)) ||
           isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreeze)) || isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeRelease));
}

// SMG2 unnamed helper fn_80178940: Throw / HitBackToPlayer / Freeze / FreezeRelease / Rebound.
bool CocoNutBall::fn_80178940() const {
    return this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow)) || this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer)) ||
           this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreeze)) || this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeRelease)) ||
           this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound));
}

// NONMATCHING: SMG2 also accepts CocoNutBallNrvFreezeStart here (nerve not ported yet).
bool CocoNutBall::isValidReceivePunch() const {
    if (MR::isDead(this) || !this->fn_80178940()) {
        return false;
    }

    return mPosition.distance(*MR::getPlayerCenterPos()) < 400.0f;
}

void CocoNutBall::calcHitBackVelocitAndGravity() {
    bool hitBackRight = isHitBackRight();
    bool hitBackFront = isHitBackFront();
    TVec3f hitBackDstPos;
    calcHitBackDstPos(&hitBackDstPos, hitBackRight, hitBackFront);

    TVec3f dir;
    dir.sub(hitBackDstPos, mPosition);
    TVec3f cross;
    TVec3f scaled;
    scaled.scale(_C8.dot(dir), _C8);
    MR::vecKillElement(dir, _C8, &dir);
    f32 f1 = dir.length() / 42.0f;
    MR::normalize(&dir);
    cross.cross(_C8, dir);
    MR::normalize(&cross);
    _90.scale(2.2f, mGravity);
    TVec3f scaled2;
    scaled2.scale(42.0f, dir);
    TVec3f verticalVelocity;
    verticalVelocity = (scaled.scaleInline(2.0f) - _90.scaleInline(f1).scaleInline(f1)).scaleInline(1.0f / (2.0f * f1));
    mVelocity.add(scaled2, verticalVelocity);

    if (!hitBackFront) {
        f32 scaleFactor = (hitBackRight ? 1.2f : -1.2f);
        TVec3f scaled4;
        scaled4.scale(scaleFactor * f1 * f1 / (f1 * 2.0f), cross);
        mVelocity.add(scaled4);
        scaled4.scale(scaleFactor, cross);
        _90.sub(scaled4);
    }

    _BC = hitBackRight;
}

bool CocoNutBall::isHitBackRight() const {
    TVec3f vec1;
    vec1.sub(_8C->mPosition, *MR::getPlayerPos());

    TVec3f vec2;
    vec2.sub(mPosition, *MR::getPlayerPos());

    MR::vecKillElement(vec1, _C8, &vec1);
    MR::vecKillElement(vec2, _C8, &vec2);

    MR::normalize(&vec1);
    MR::normalize(&vec2);

    TVec3f cross = vec1.cross(vec2);

    return 0.0f < _C8.dot(cross);
}

bool CocoNutBall::isHitBackFront() const {
    TVec3f vec1;
    vec1.sub(*MR::getPlayerPos(), _8C->mPosition);

    TVec3f vec2;
    vec2.sub(mPosition, *MR::getPlayerPos());

    MR::vecKillElement(vec1, _C8, &vec1);
    MR::vecKillElement(vec2, _C8, &vec2);

    MR::normalize(&vec1);
    MR::normalize(&vec2);

    return vec1.dot(vec2) < -0.95f;
}

void CocoNutBall::calcHitBackDstPos(TVec3f* pOut, bool a1, bool a2) {
    TVec3f vec1(0.0f, 0.0f, 0.0f);
    if (!a2) {
        TVec3f cross;
        TVec3f vec2;

        vec2.sub(_8C->mPosition, *MR::getPlayerPos());

        MR::vecKillElement(vec2, _C8, &vec2);
        MR::normalize(&vec2);

        cross.cross(_C8, vec2);
        MR::vecKillElement(cross, _C8, &cross);
        MR::normalize(&cross);

        vec1.scale(a1 ? 150.0f : -150.0f, cross);
    }

    TVec3f scaled;

    scaled.scale(100.0f, _C8);
    vec1.add(scaled);

    pOut->add(_8C->getSensor("body")->mPosition, vec1);
}

bool CocoNutBall::tryToKill(bool alwaysKill) {
    HitSensor* bindedSensor = isBindedAny();

    if (bindedSensor != nullptr) {
        if (MR::isSensorType(bindedSensor, 0x4F) && !isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound))) {
            MR::sendMsgEnemyAttack(bindedSensor, getSensor("body"));
        }

        kill();
        return true;
    }

    if (alwaysKill) {
        kill();
        return true;
    }

    return false;
}

void CocoNutBall::setVelocityToPlayer(f32 f1, f32 f2) {
    TVec3f vec1;

    if (!_BE) {
        vec1.scale(120.0f, _C8);
        vec1.add(*MR::getPlayerPos());
    } else {
        vec1.set(*MR::getPlayerPos());
    }

    TPos3f rotate;
    rotate.makeRotate(_C8, MR::toRadian(f2));
    TVec3f vec2;
    vec2.sub(vec1, mPosition);
    rotate.mult33(vec2);
    vec1.add(mPosition, vec2);

    if (_BE) {
        f32 minY = _C4 + _8C->mPosition.y;
        f32 val2 = _C0 + _8C->mPosition.y;

        vec1.y = MR::max(vec1.y, minY);

        f32 flt = 120.0f;
        bool v1 = false;
        while (vec1.y < val2 - flt) {
            vec2.sub(vec1, mPosition);
            vec2.scale(1.5f);

            if (!MR::getFirstPolyOnLineToMap(nullptr, nullptr, mPosition, vec2)) {
                v1 = true;
                vec1.y += flt;
                break;
            }

            vec1.y += 60.0f;
        }

        if (!v1) {
            vec1.y = val2;
        }
    }

    vec2.sub(vec1, mPosition);
    mVelocity.setLength(vec2, f1);
}

void CocoNutBall::freeze() {
    _A0 = 0;
    _A4.set(mPosition);
    _B0.set(mVelocity);
    setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreeze));
}

bool CocoNutBall::isFreezable() {
    return !MR::isHiddenModel(this) && MR::isStarPointerPointing2PAttachTarget(this);
}

void CocoNutBall::processApproachToPlayer() {
    if (MR::changeShowModelFlagSyncNearClipping(this, 300.0f)) {
        MR::emitEffect(this, "CocoNutLight");
    } else {
        MR::deleteEffect(this, "CocoNutLight");
    }

    if (isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeRelease)) || MR::isGreaterStep(this, 20)) {
        if (isFreezable()) {
            freeze();
            return;
        }
    }

    if (tryToKill(!MR::isNear(this, _8C, _D4))) {
        return;
    }
}

void CocoNutBall::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinX");
        MR::emitEffect(this, "CocoNutLight");

        if (_D8) {
            MR::startActionSound(this, "BmOtakingSpitRallyBall");
        } else {
            MR::startActionSound(this, "OjCoconutBallSpitOut");
        }

        if (_DD) {
            MR::startActionSound(this, "Fall");
        } else {
            MR::startActionSound(this, "Fly");
        }
    }

    processApproachToPlayer();
}

void CocoNutBall::exeHitBackToHost() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(this, 1.0f);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutBlur");
        MR::emitEffect(this, "CocoNutLight");
        MR::emitEffect(this, "SpinHitMark");
        MR::offBind(this);
        MR::startActionSound(this, "OjCoconutBallVolley");
        MR::startSpinHitSound(this);

        if (_D8 && MR::isPlayingStageBgm()) {
            if (_9C == 0) {
                MR::startSystemME("ME_RALLY_COMBO_FIRST");
            } else {
                MR::startSystemME("ME_RALLY_COMBO_SECOND");
            }
        }

        _9C++;
        calcHitBackVelocitAndGravity();
    }

    if (MR::isStep(this, 1)) {
        MR::stopScene(6);
    }

    if (MR::isStep(this, 2)) {
        MR::shakeCameraWeak();
    }

    if (MR::isGreaterEqualStep(this, 1)) {
        mVelocity.add(_90);
    }

    if (tryToKill(!MR::isNearPlayer(this, 5000.0f))) {
        return;
    }
}

void CocoNutBall::exeHitBackToPlayer() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Hit");
        MR::startActionSound(this, "OjCoconutBallVolley");

        if (_D8 && MR::isPlayingStageBgm()) {
            MR::startSystemME("ME_RALLY_COMBO_SECOND");
        }

        MR::onBind(this);
    }

    processApproachToPlayer();
}

void CocoNutBall::exeRebound() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(this, 1.0f);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutBlur");
        MR::emitEffect(this, "CocoNutLight");
        MR::emitEffect(this, "SpinHitMark");
        MR::onBind(this);
        mVelocity.add(_90);
    }

    if (tryToKill(!MR::isNearPlayer(this, 5000.0f))) {
        return;
    }

    TVec3f playerPos(*MR::getPlayerCenterPos());
    TVec3f offset(_C8);
    offset.scale(100.0f);
    playerPos.add(offset);

    TVec3f diff(playerPos);
    diff.sub(mPosition);

    TVec3f dir(diff);
    if (MR::normalizeOrZero(&dir)) {
        return;
    }

    MR::vecKillElement(dir, mGravity, &dir);
    if (MR::normalizeOrZero(&dir)) {
        return;
    }

    TVec3f vel(mVelocity);
    if (MR::normalizeOrZero(&vel)) {
        return;
    }

    MR::vecKillElement(vel, mGravity, &vel);
    if (MR::normalizeOrZero(&vel)) {
        return;
    }

    MR::vecKillElement(dir, vel, &dir);
    if (MR::normalizeOrZero(&dir)) {
        return;
    }

    f32 diffDotDir = diff.dot(dir);
    TVec3f dirScaled(dir);
    dirScaled.scale(diffDotDir);

    TVec3f tangent(diff);
    if (MR::normalizeOrZero(&tangent)) {
        return;
    }

    MR::vecKillElement(tangent, dir, &tangent);
    if (MR::normalizeOrZero(&tangent)) {
        return;
    }

    f32 velDotTangent = tangent.dot(vel);
    f32 diffDotTangent = diff.dot(tangent);
    TVec3f tangentScaled(tangent);
    tangentScaled.scale(diffDotTangent);
    tangent = tangentScaled;

    f32 rate = MR::calcNerveRate(this, 1, (s32)(1.0f + _E0));
    if (velDotTangent < 0.0f) {
        if (mGravity.dot(diff) < 0.0f) {
            tangent.set(0.0f, 0.0f, 0.0f);
        } else {
            f32 gravDot = mGravity.dot(diff);
            TVec3f gravityScaled(mGravity);
            gravityScaled.scale(gravDot);
            tangent.set(gravityScaled);
        }
    }

    f32 velDotGravity = mVelocity.dot(mGravity);
    if (velDotGravity < 0.0f) {
        velDotGravity = 0.0f;
    }

    f32 mag = diff.length();
    velDotGravity += rate * (rate * mag);

    TVec3f gravityScaled2(mGravity);
    gravityScaled2.scale(velDotGravity);
    TVec3f newVel(vel);
    newVel.add(gravityScaled2);
    mVelocity.set(newVel);

    TVec3f tangentRate(tangent);
    tangentRate.scale(rate);
    TVec3f dirRate(dirScaled);
    dirRate.scale(rate);
    TVec3f pos(mPosition);
    pos.add(dirRate);
    TVec3f pos2(pos);
    pos2.add(tangentRate);
    TVec3f pos3(pos2);
    pos3.scale(rate);
    TVec3f keep(mPosition);
    keep.scale(1.0f - rate);
    keep.add(pos3);
    mPosition = keep;
}

#pragma push
#pragma global_optimizer off
void CocoNutBall::exeFreeze() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();

        MR::deleteEffect(this, "CocoNutBlur");
        MR::deleteEffect(this, "CocoNutLight");
        MR::emitEffect(this, "Touch");

        if (_A0 == 0) {
            MR::startDPDHitSound();
        }
    }

    _A0++;
    MR::startDPDFreezeLevelSound(this);

    f32 cos = MR::cosDegree(MR::repeatDegree(_A0 * 75.0f));

    f32 scaleFactor = ((7.5f * cos) * (20 - getNerveStep())) / 20.0f;

    TVec3f camXDirScaled;
    camXDirScaled.set(MR::getCamXdir());
    camXDirScaled.scale(scaleFactor);
    mPosition.add(_A4, camXDirScaled);

    if (MR::changeShowModelFlagSyncNearClipping(this, 300.0f)) {
        MR::emitEffect(this, "Touch");
    } else {
        MR::deleteEffect(this, "Touch");
    }

    if (isFreezable()) {
        // resets nerve step every frame i guess
        setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreeze));
    } else if (MR::isStep(this, 20)) {
        setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeRelease));
    }
}
#pragma pop

void CocoNutBall::exeFreezeRelease() {
    if (MR::isFirstStep(this)) {
        mPosition.set(_A4);
        mVelocity.set(_B0);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutLight");
    }

    processApproachToPlayer();
}
