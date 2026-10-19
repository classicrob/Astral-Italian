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
    NEW_NERVE(CocoNutBallNrvHitBackToOtaRockTank, CocoNutBall, HitBackToOtaRockTank);
    NEW_NERVE(CocoNutBallNrvHitBackToPlayer, CocoNutBall, HitBackToPlayer);
    NEW_NERVE(CocoNutBallNrvRebound, CocoNutBall, Rebound);
    NEW_NERVE(CocoNutBallNrvFreezeStart, CocoNutBall, FreezeStart);
    NEW_NERVE(CocoNutBallNrvFreeze, CocoNutBall, Freeze);
    NEW_NERVE(CocoNutBallNrvFreezeRelease, CocoNutBall, FreezeRelease);
};  // namespace NrvCocoNutBall

CocoNutBall::CocoNutBall(const char* pName)
    : LiveActor(pName), mHost(), mAccel(0.0f, -1.0f, 0.0f), mHitBackCount(), mFreezeStep(), mFreezePos(gZeroVec), mFreezeVelocity(gZeroVec), mHitBackRight(), mCurveRight(), mIsBoundedByHost(), mMaxHeightOffset(450.0f), mMinHeightOffset(),
      mUp(0.0f, 1.0f, 0.0f), mKillDistance(10000.0f), mIsRallyBall(), mIsFalling(), mIsTankBall(), mFlightSteps(0.0f), mHitBackLockStep(0) {
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
    mHitBackCount = 0;
    MR::onBind(this);
    setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow));

    if (mIsTankBall) {
        mHitBackLockStep = 60;
    }
}

void CocoNutBall::kill() {
    mIsFalling = false;
    MR::forceDeleteEffect(this, "CocoNutBlur");
    MR::forceDeleteEffect(this, "CocoNutLight");
    MR::startActionSound(this, "OjCoconutBallBreak");

    if (mHitBackCount > 0) {
        MR::sendArbitraryMsg(0xCC, mHost->getSensor("Body"), getSensor("Body"));
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
    mUp.negate(mGravity);
    setVelocityToPlayer(15.0f, f1, 0.0f);
    mCurveRight = MR::isHalfProbability();
    appear();
}
#pragma pop
#pragma pop

void CocoNutBall::hitBackToPlayer() {
    f32 rand = MR::getRandom(12.5f, 17.5f);

    if (mCurveRight) {
        rand = -rand;
        mCurveRight = false;
    } else {
        mCurveRight = true;
    }

    setVelocityToPlayer(3.0f * mHitBackCount + 15.0f, rand, 0.0f);
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
        if (MR::isSameDirection(velocityDirection, mUp, 0.01f)) {
            MR::makeMtxFrontNoSupportPos(&baseTRMtx, velocityDirection, mPosition);
        } else {
            MR::makeMtxFrontUpPos(&baseTRMtx, velocityDirection, mUp, mPosition);
        }
        MR::setBaseTRMtx(this, baseTRMtx);
    }
}

bool CocoNutBall::isSensorBody(HitSensor* pSensor) const {
    return pSensor == getSensor("Body");
}

void CocoNutBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isSensorBody(pSender) && MR::isSensorPlayer(pReceiver)) {
        if (isInAttackOrFreezeNerve() && MR::sendMsgEnemyAttackFlipVeryWeak(pReceiver, pSender)) {
            setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeStart));
            return;
        }
    }
    if (MR::isSensorEnemy(pReceiver) && isSensorBody(pSender)) {
        if (MR::getSensorHost(pReceiver) == mHost) {
            if ((isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost)) ||
                 isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToOtaRockTank))) &&
                MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                MR::emitEffect(this, "Hit");
                kill();
            }
        } else if ((isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow)) ||
                    isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost)) ||
                    isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer)) ||
                    isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToOtaRockTank)) ||
                    isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound))) &&
                   MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            kill();
        }
    }
}

bool CocoNutBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (MR::isMsgSupportTicoSpin(msg)) {
            if (canHitBackFromSensor(pSender)) {
                calcPlayerHitBackVelocity(pSender);
                setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound));
                return true;
            }
            return false;
        }
        if (isValidReceivePunch()) {
            if (mIsTankBall) setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToOtaRockTank));
            else setNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToHost));
            return true;
        }
    } else if (isSensorBody(pReceiver) && MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    return false;
}

bool CocoNutBall::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!mHost) return false;
    if (MR::getSensorHost(pSender) != mHost) return false;
    if (!mIsTankBall) return false;
    if (mHitBackLockStep > 0) return false;
    if (!MR::isEqualSensor(pReceiver, this, "Body")) return false;
    if (isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToOtaRockTank))) return false;
    kill();
    return true;
}

void CocoNutBall::initSensors() {
    initHitSensor(2);
    MR::addHitSensor(this, "Body", 0x1B, 8, 40.0f * mScale.x, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensor(this, "Bind", 0x1B, 8, 500.0f, TVec3f(0.0f, 0.0f, 0.0f));
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

// SMG2 unnamed helper isInAttackOrFreezeNerve: Throw / HitBackToPlayer / Freeze / FreezeRelease / Rebound.
bool CocoNutBall::isInAttackOrFreezeNerve() const {
    return this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvThrow)) || this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvHitBackToPlayer)) ||
           this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreeze)) || this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeRelease)) ||
           this->isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound));
}

bool CocoNutBall::isValidReceivePunch() const {
    if (MR::isDead(this) || (!isInAttackOrFreezeNerve() && !isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeStart)))) {
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
    scaled.scale(mUp.dot(dir), mUp);
    MR::vecKillElement(dir, mUp, &dir);
    f32 f1 = dir.length() / 42.0f;
    MR::normalize(&dir);
    cross.cross(mUp, dir);
    MR::normalize(&cross);
    mAccel.scale(2.2f, mGravity);
    TVec3f scaled2;
    scaled2.scale(42.0f, dir);
    TVec3f verticalVelocity;
    verticalVelocity = (scaled.scaleInline(2.0f) - mAccel.scaleInline(f1).scaleInline(f1)).scaleInline(1.0f / (2.0f * f1));
    mVelocity.add(scaled2, verticalVelocity);

    if (!hitBackFront) {
        f32 scaleFactor = (hitBackRight ? 1.2f : -1.2f);
        TVec3f scaled4;
        scaled4.scale(scaleFactor * f1 * f1 / (f1 * 2.0f), cross);
        mVelocity.add(scaled4);
        scaled4.scale(scaleFactor, cross);
        mAccel.sub(scaled4);
    }

    mHitBackRight = hitBackRight;
}

bool CocoNutBall::isHitBackRight() const {
    TVec3f vec1;
    vec1.sub(mHost->mPosition, *MR::getPlayerPos());

    TVec3f vec2;
    vec2.sub(mPosition, *MR::getPlayerPos());

    MR::vecKillElement(vec1, mUp, &vec1);
    MR::vecKillElement(vec2, mUp, &vec2);

    MR::normalize(&vec1);
    MR::normalize(&vec2);

    TVec3f cross = vec1.cross(vec2);

    return 0.0f < mUp.dot(cross);
}

bool CocoNutBall::isHitBackFront() const {
    TVec3f vec1;
    vec1.sub(*MR::getPlayerPos(), mHost->mPosition);

    TVec3f vec2;
    vec2.sub(mPosition, *MR::getPlayerPos());

    MR::vecKillElement(vec1, mUp, &vec1);
    MR::vecKillElement(vec2, mUp, &vec2);

    MR::normalize(&vec1);
    MR::normalize(&vec2);

    return vec1.dot(vec2) < -0.95f;
}

void CocoNutBall::calcHitBackDstPos(TVec3f* pOut, bool a1, bool a2) {
    TVec3f vec1(0.0f, 0.0f, 0.0f);
    if (!a2) {
        TVec3f cross;
        TVec3f vec2;

        vec2.sub(mHost->mPosition, *MR::getPlayerPos());

        MR::vecKillElement(vec2, mUp, &vec2);
        MR::normalize(&vec2);

        cross.cross(mUp, vec2);
        MR::vecKillElement(cross, mUp, &cross);
        MR::normalize(&cross);

        vec1.scale(a1 ? 150.0f : -150.0f, cross);
    }

    TVec3f scaled;

    scaled.scale(100.0f, mUp);
    vec1.add(scaled);

    pOut->add(*MR::getSensorPos(mHost, "Body"), vec1);
}

bool CocoNutBall::tryToKill(bool alwaysKill) {
    HitSensor* bindedSensor = isBindedAny();

    if (bindedSensor != nullptr) {
        if (MR::isSensorType(bindedSensor, 0x4F) && !isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvFreezeStart))) {
            MR::sendMsgEnemyAttack(bindedSensor, getSensor("Body"));
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

void CocoNutBall::setVelocityToPlayer(f32 f1, f32 f2, f32 height) {
    TVec3f vec1;

    if (!mIsBoundedByHost) {
        if (height == 0.0f) {
            vec1.scale(120.0f, mUp);
            vec1.add(*MR::getPlayerPos());
        } else {
            vec1.scale(height, mUp);
            vec1.add(*MR::getPlayerPos());
        }
    } else {
        vec1.set(*MR::getPlayerPos());
    }

    TPos3f rotate;
    rotate.makeRotate(mUp, MR::toRadian(f2));
    TVec3f vec2;
    vec2.sub(vec1, mPosition);
    rotate.mult33(vec2);
    vec1.add(mPosition, vec2);

    if (mIsBoundedByHost) {
        f32 minY = mMinHeightOffset + mHost->mPosition.y;
        f32 val2 = mMaxHeightOffset + mHost->mPosition.y;

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
    mFreezeStep = 0;
    mFreezePos.set(mPosition);
    mFreezeVelocity.set(mVelocity);
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

    if (tryToKill(!MR::isNear(this, mHost, mKillDistance))) {
        return;
    }
}

void CocoNutBall::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinX");
        MR::emitEffect(this, "CocoNutLight");

        if (mIsRallyBall) {
            MR::startActionSound(this, "BmOtakingSpitRallyBall");
        } else {
            MR::startActionSound(this, "OjCoconutBallSpitOut");
        }

    }

    if (mIsFalling) {
            MR::startActionSound(this, "Fall");
        } else {
            MR::startActionSound(this, "Fly");
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

        if (mIsRallyBall && MR::isPlayingStageBgm()) {
            if (mHitBackCount == 0) {
                MR::startSystemME("ME_RALLY_COMBO_FIRST");
            } else {
                MR::startSystemME("ME_RALLY_COMBO_SECOND");
            }
        }

        mHitBackCount++;
        calcHitBackVelocitAndGravity();
    }

    if (MR::isStep(this, 1)) {
        MR::stopScene(6);
    }

    if (MR::isStep(this, 2)) {
        MR::shakeCameraWeak();
    }

    if (MR::isGreaterEqualStep(this, 1)) {
        mVelocity.add(mAccel);
    }

    if (tryToKill(!MR::isNearPlayer(this, 5000.0f))) {
        return;
    }
}

void CocoNutBall::exeHitBackToPlayer() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Hit");
        MR::startActionSound(this, "OjCoconutBallVolley");

        if (mIsRallyBall && MR::isPlayingStageBgm()) {
            MR::startSystemME("ME_RALLY_COMBO_SECOND");
        }

        MR::onBind(this);
    }

    processApproachToPlayer();
}

// NONMATCHING: Remaining FP register allocation and vector temporary stack slots differ.
void CocoNutBall::exeRebound() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(this, 1.0f);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutBlur");
        MR::emitEffect(this, "CocoNutLight");
        MR::emitEffect(this, "SpinHitMark");
        MR::onBind(this);
    }
    mVelocity.add(mAccel);

    if (tryToKill(!MR::isNearPlayer(this, 5000.0f))) {
        return;
    }

    TVec3f playerPos(*MR::getPlayerCenterPos());
    playerPos.add(mUp * 100.0f);

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

    f32 rate = MR::calcNerveRate(this, 1, (s32)(1.0f + mFlightSteps));
    if (velDotTangent < 0.0f) {
        if (mGravity.dot(diff) < 0.0f) {
            tangent.set<f32>(0.0f, 0.0f, 0.0f);
        } else {
            diffDotTangent = mGravity.dot(diff);
            TVec3f gravityScaled(mGravity);
            gravityScaled.scale(diffDotTangent);
            tangent.set(gravityScaled);
        }

    diffDotTangent = mVelocity.dot(mGravity);
    if (diffDotTangent < 0.0f) {
        diffDotTangent = 0.0f;
    }

    f32 mag = diff.length();
    diffDotTangent += rate * (rate * mag);

    mVelocity.set(vel + mGravity * diffDotTangent);
    }
    TVec3f pos2 = mPosition + dirScaled * rate + tangent * rate;
    mPosition = mPosition * (1.0f - rate) + pos2 * rate;
}

#pragma push
#pragma global_optimizer off
void CocoNutBall::exeFreeze() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();

        MR::deleteEffect(this, "CocoNutBlur");
        MR::deleteEffect(this, "CocoNutLight");
        MR::emitEffect(this, "Touch");

        if (mFreezeStep == 0) {
            MR::startDPDHitSound();
        }
    }

    mFreezeStep++;
    MR::startDPDFreezeLevelSound(this);

    f32 cos = MR::cosDegree(MR::repeatDegree(mFreezeStep * 75.0f));

    f32 scaleFactor = ((7.5f * cos) * (20 - getNerveStep())) / 20.0f;

    TVec3f camXDirScaled;
    camXDirScaled.set(MR::getCamXdir());
    camXDirScaled.scale(scaleFactor);
    mPosition.add(mFreezePos, camXDirScaled);

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
        mPosition.set(mFreezePos);
        mVelocity.set(mFreezeVelocity);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutLight");
    }

    processApproachToPlayer();
}

void CocoNutBall::calcTankHitBackVelocity() {
    bool hitBackRight = isHitBackRight();
    bool hitBackFront = isHitBackFront();
    TVec3f hitBackDstPos;
    calcHitBackDstPos(&hitBackDstPos, hitBackRight, hitBackFront);

    TVec3f dir;
    dir.sub(hitBackDstPos, mPosition);
    TVec3f cross;
    TVec3f scaled;
    scaled.scale(mUp.dot(dir), mUp);
    MR::vecKillElement(dir, mUp, &dir);
    f32 f1 = dir.length() / 42.0f;
    mFlightSteps = f1;
    MR::normalize(&dir);
    cross.cross(mUp, dir);
    MR::normalize(&cross);
    mAccel.scale(2.2f, mGravity);
    TVec3f scaled2;
    scaled2.scale(42.0f, dir);
    TVec3f verticalVelocity;
    verticalVelocity = (scaled.scaleInline(2.0f) - mAccel.scaleInline(f1).scaleInline(f1)).scaleInline(1.0f / (2.0f * f1));
    mVelocity.add(scaled2, verticalVelocity);

    if (!hitBackFront) {
        f32 scaleFactor = (hitBackRight ? 1.2f : -1.2f);
        TVec3f scaled4;
        scaled4.scale(scaleFactor * f1 * f1 / (f1 * 2.0f), cross);
        mVelocity.add(scaled4);
        scaled4.scale(scaleFactor, cross);
        mAccel.sub(scaled4);
    }

    mHitBackRight = hitBackRight;
}


void CocoNutBall::control() {
    if (mIsTankBall && mHitBackLockStep > 0) mHitBackLockStep--;
}

void CocoNutBall::launchCoconutTowardPlayer(const TVec3f& pos, f32 speed, f32 angle, f32 height) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUp.negate(mGravity);
    setVelocityToPlayer(speed, angle, height);
    mCurveRight = MR::isHalfProbability();
    appear();
}

void CocoNutBall::launchCoconutWithVelocity(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUp.negate(mGravity);
    mVelocity.set(velocity);
    mCurveRight = MR::isHalfProbability();
    appear();
}

void CocoNutBall::launchFallingCoconut(const TVec3f& pos, const TVec3f& velocity) {
    mPosition.set(pos);
    MR::calcGravity(this);
    mUp.negate(mGravity);
    mVelocity.set(velocity);
    mCurveRight = MR::isHalfProbability();
    mIsFalling = true;
    appear();
}

bool CocoNutBall::canHitBackFromSensor(HitSensor* sender) {
    if (MR::isDead(this)) return false;
    if (isNerve(GET_NERVE(CocoNutBall, CocoNutBallNrvRebound))) return false;
    if (!isInAttackOrFreezeNerve()) return false;
    return mPosition.distance(*MR::getSensorPos(sender)) < 400.0f;
}

void CocoNutBall::calcPlayerHitBackVelocity(HitSensor* sender) {
    MR::getSensorHost(sender);
    f32 time;
    TVec3f dst;
    TVec3f dir;
    TVec3f cross;
    TVec3f scaled;
    TVec3f offset;
    dst.set(*MR::getPlayerCenterPos());
    offset.scale(100.0f, mUp);
    dst.add(offset);
    dir.sub(dst, mPosition);
    scaled.scale(mUp.dot(dir), mUp);
    MR::vecKillElement(dir, mUp, &dir);
    f32 speed = 25.0f;
    time = dir.length() / speed;
    mFlightSteps = time;
    MR::normalize(&dir);
    cross.cross(mUp, dir);
    MR::normalize(&cross);
    mAccel.scale(1.5f, mGravity);
    TVec3f horizontal;
    horizontal.scale(speed, dir);
    TVec3f vertical;
    vertical = (scaled.scaleInline(2.0f) - mAccel.scaleInline(time).scaleInline(time)).scaleInline(1.0f / (2.0f * time));
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mVelocity.add(horizontal, vertical);
}

// NONMATCHING: The epilogue restores the saved GPR and LR in a different order.
void CocoNutBall::exeFreezeStart() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(this, 0.25f);
        MR::deleteEffect(this, "CocoNutBlur");
        MR::deleteEffect(this, "CocoNutLight");
        TVec3f front;
        front.sub(mPosition, *MR::getPlayerPos());
        TPos3f matrix;
        matrix.identity();
        MR::makeMtxUpFront(&matrix, mUp, front);
        mVelocity.set(cReboundVelocity);
        matrix.mult33(mVelocity);
    }
    TVec3f gravity;
    gravity.scale(2.0f, mGravity);
    mVelocity.add(gravity);
    tryToKill(!MR::isNearPlayer(this, 5000.0f));
}

void CocoNutBall::exeHitBackToOtaRockTank() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(this, 1.0f);
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "CocoNutBlur");
        MR::emitEffect(this, "CocoNutLight");
        MR::emitEffect(this, "SpinHitMark");
        MR::offBind(this);
        MR::startActionSound(this, "OjCoconutBallVolley");
        MR::startSpinHitSound(this);

        if (mIsRallyBall && MR::isPlayingStageBgm()) {
            if (mHitBackCount == 0) {
                MR::startSystemME("ME_RALLY_COMBO_FIRST");
            } else {
                MR::startSystemME("ME_RALLY_COMBO_SECOND");
            }
        }

        mHitBackCount++;
        calcTankHitBackVelocity();
    }

    if (MR::isStep(this, 1)) MR::stopScene(6);
    if (MR::isStep(this, 2)) MR::shakeCameraWeak();
    if (MR::isGreaterEqualStep(this, 1)) mVelocity.add(mAccel);
    mVelocity.x *= 0.99f;
    mVelocity.y *= 0.99f;
    mVelocity.z *= 0.99f;
    TVec3f dst;
    calcHitBackDstPos(&dst, true, false);
    f32 rate = MR::calcNerveRate(this, 1, (s32)(1.0f + mFlightSteps));
    TVec3f delta(dst);
    delta.sub(mPosition);
    TVec3f intermediate = mPosition + delta * rate;
    mPosition = mPosition * (1.0f - rate) + intermediate * rate;
}
