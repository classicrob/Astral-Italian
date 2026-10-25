#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapParts.hpp"
#include "Game/MapObj/MapPartsFunction.hpp"
#include "Game/MapObj/MapPartsRailPointPassChecker.hpp"
#include "Game/Util.hpp"


namespace NrvMapPartsRailMover {
    NEW_NERVE(HostTypeWait, MapPartsRailMover, WaitForRestart);
    NEW_NERVE(HostTypeMove, MapPartsRailMover, Move);
    NEW_NERVE(HostTypeMoveStart, MapPartsRailMover, MoveStart);
    NEW_NERVE(HostTypeStopAtPointBeforeRotate, MapPartsRailMover, StopAtPoint);
    NEW_NERVE(HostTypeStopAtPointAfterRotate, MapPartsRailMover, StopAtPoint);
    NEW_NERVE(HostTypeStopAtEndBeforeRotate, MapPartsRailMover, StopAtEnd);
    NEW_NERVE(HostTypeStopAtEndAfterRotate, MapPartsRailMover, StopAtEnd);
    NEW_NERVE(HostTypeStopAtEndWithPlayerOn, MapPartsRailMover, StopAtEndWithPlayerOn);
    NEW_NERVE(HostTypeWaitForRestartByPlayerOn, MapPartsRailMover, WaitForRestart);
    NEW_NERVE(HostTypeRotateAtPoint, MapPartsRailMover, WaitForRestart);
    NEW_NERVE(HostTypeRotateAtEndPoint, MapPartsRailMover, WaitForRestart);
    NEW_NERVE(HostTypeVanish, MapPartsRailMover, Vanish);
};  // namespace NrvMapPartsRailMover

MapPartsRailMover::MapPartsRailMover(LiveActor* pActor)
    : MapPartsFunction(pActor, "レイル移動"), mRailPointPassChecker(), mMoveConditionType(), mMoveStopType(1), mSignMotionType(), mPosition(gZeroVec), mSnapOffset(gZeroVec) {
    mInitialRailCoord = 0.0f;
    mStopTime = 0;
    mSpeed = 0.0f;
    mAccelTime = 0;
    mAcceleration = 0.0f;
    mSignMotionRailCoord = 0.0f;
    mSnapDuration = 0;
    mSnapFramesRemaining = 0;
    mHasRepeated = false;
    mSignMotionTimeOverride = -1;
    mDisableSignMotionAnimation = false;
}

// NONMATCHING: GPR allocation and paired-single subtraction scheduling in snap setup.
inline TVec3f calcRailSnapOffset(const TVec3f& position, LiveActor* host) {
    TVec3f offset(position);
    offset -= host->mPosition;
    return offset;
}

void MapPartsRailMover::init(const JMapInfoIter& rIter) {
    MapPartsFunction::init(rIter);
    TVec3f originalPosition(mHost->mPosition);
    MR::getMapPartsArgMoveConditionType(&mMoveConditionType, rIter);
    MR::getMapPartsArgMoveStopType(&mMoveStopType, mHost);
    MR::getMapPartsArgSignMotionType(&mSignMotionType, rIter);

    s32 initPostType = 0;
    MR::getMapPartsArgRailInitPosType(&initPostType, mHost);

    if (MR::isMapPartsRailInitPosTypeRailPos(initPostType)) {
        MR::moveCoordAndTransToNearestRailPos(mHost);
    } else if (MR::isMapPartsRailInitPosTypeRailPoint(initPostType)) {
        MR::moveCoordAndTransToNearestRailPoint(mHost);
    } else if (MR::isMapPartsRailInitPosTypePoint0(initPostType)) {
        MR::moveCoordAndTransToRailStartPoint(mHost);
    }

    mSnapDuration = 0;
    MR::getMapPartsArgMoveSnap(&mSnapDuration, mHost);
    if (mSnapDuration > 0) {
        mSnapFramesRemaining = mSnapDuration;
        mSnapOffset = calcRailSnapOffset(originalPosition, mHost);
        MR::setPosition(mHost, originalPosition);
    }
    updatePosition();
    mInitialRailCoord = MR::getRailCoord(mHost);
    mRailPointPassChecker = new MapPartsRailPointPassChecker(mHost);
    mRailPointPassChecker->init(rIter);
    const char* objectName = nullptr;
    MR::getObjectName(&objectName, rIter);
    if (MR::isEqualString(objectName, "FallDownWall") || MR::isEqualString(objectName, "RailMoveDisappearStep")) mSignMotionTimeOverride = 50;
    initNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
}

void MapPartsRailMover::movement() {
    bool isMove = false;

    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove)) || isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) {
        isMove = true;
    }

    if (isMove) {
        tryPassPoint();
    } else {
        tryRestartAtEnd();
    }

    mRailPointPassChecker->movement();
    MapPartsFunction::movement();
    isMove = false;
    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove)) || isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) isMove = true;
    if (isMove && isDone()) MR::startActionSoundAuto(mHost, "RailMoverStop", -1, -1, -1);
}

void MapPartsRailMover::start() {
    moveToInitPos();
    doStart();
}

void MapPartsRailMover::doStart() {
    mRailPointPassChecker->start();
    if (MR::hasMapPartsMoveStartSignMotion(mSignMotionType)) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart));
        MR::startActionSoundAuto(mHost, "RailMoverWaitStart", -1, -1, -1);
    } else {
        MR::startActionSoundAuto(mHost, "RailMoverWaitStartMove", -1, -1, -1);
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
    }
}

void MapPartsRailMover::end() {
    mRailPointPassChecker->end();
    setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
}

bool MapPartsRailMover::isWorking() const {
    bool isWork = false;

    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove)) || isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) {
        isWork = true;
    }

    return isWork;
}

bool MapPartsRailMover::receiveMsg(u32 msg) {
    if (msg == 0xD7 /* ACTMES_MAPPARTS_ON_PLAYER */ && isNerve(GET_NERVE(MapPartsRailMover, HostTypeWaitForRestartByPlayerOn))) {
        restartAtEnd();
        return true;
    }

    return false;
}

void MapPartsRailMover::moveToInitPos() {
    if (!MR::isNearZero(mInitialRailCoord - MR::getRailCoord(mHost))) {
        MR::setRailCoord(mHost, mInitialRailCoord);
        updatePosition();
        if (!MR::isRailGoingToEnd(mHost)) {
            MR::reverseRailDirection(mHost);
        }
    }
}

void MapPartsRailMover::startWithSignalMotion() {
    setNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart));
}

void MapPartsRailMover::cancelSignalMotion() {
    MR::setRailCoord(mHost, mSignMotionRailCoord);
    updatePosition();
    setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
}

bool MapPartsRailMover::tryResetPositionRepeat() {
    if (mMoveStopType != 2) {
        return false;
    }

    if (!isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove))) {
        return false;
    }

    if (getStep() != 1) {
        return false;
    }

    MR::resetPosition(mHost);
    return true;
}

void MapPartsRailMover::resetToInitPos() {
    moveToInitPos();
    MR::resetPosition(mHost, mPosition);
    setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
}

void MapPartsRailMover::passPoint() {
    mStopTime = 0;
    MR::getMapPartsArgStopTime(&mStopTime, mHost);

    if (mStopTime > 0) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtPointBeforeRotate));

    } else {
        if (!sendMsgToHost(0xDA /* ACTMES_MAPPARTS_START_ROTATE_BETWEEN_POINTS */)) {
            if (sendMsgToHost(0xD8 /* ACTMES_MAPPARTS_START_ROTATE_AT_POINT */)) {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtPoint));
            } else {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
            }
        }
    }
}

void MapPartsRailMover::reachedEnd() {
    if (mMoveStopType == 1) {
        MR::reverseRailDirection(mHost);
    }

    if (MR::isMoveStartTypePlayerOnStopEnd(mMoveConditionType)) {
        reachedEndPlayerOn();
    } else {
        setStateStopAtEndBeforeRotate();
    }
}

void MapPartsRailMover::reachedEndPlayerOn() {
    if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndWithPlayerOn));
    } else {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeWaitForRestartByPlayerOn));
    }
}

bool MapPartsRailMover::isReachedEnd() const {
    bool ret = false;

    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove)) || isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) {
        if (mRailPointPassChecker->isReachedEnd()) {
            ret = true;
        }
    }

    return ret;
}

bool MapPartsRailMover::isDone() const {
    if (mMoveStopType == 0 || mMoveStopType == 3) {
        return mRailPointPassChecker->isReachedEnd();
    }

    return false;
}

void MapPartsRailMover::setStateStopAtEndBeforeRotate() {
    mSpeed = 0.0f;
    mAcceleration = 0.0f;
    MR::setRailCoordSpeed(mHost, 0.0f);
    mStopTime = 0;
    MR::getMapPartsArgStopTime(&mStopTime, mHost);

    if (mStopTime <= 0) {
        if (sendMsgToHost(0xD8 /* ACTMES_MAPPARTS_START_ROTATE_AT_POINT */)) {
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtEndPoint));
        } else {
            restartAtEnd();
        }
    } else {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndBeforeRotate));
    }
}

void MapPartsRailMover::calcTimeToNextRailPoint(f32* pTime) const {
    f32 distance = 0.0f;
    MR::calcDistanceToNextRailPoint(mHost, &distance);

    bool res = MR::isNearZero(mAcceleration, 0.000099999997f) == false;

    if (!res) {
        *pTime = MR::abs(distance / mSpeed);
        return;
    }

    f32 accelDistance = mSpeed * mAccelTime + 0.5f * mAcceleration * mAccelTime * mAccelTime;

    if (accelDistance < distance) {
        *pTime = mAccelTime + (distance - accelDistance) / (mAcceleration * mAccelTime);
    } else {
        f32 sqrtDiscriminant = MR::sqrt(mSpeed * mSpeed + 2.0f * mAcceleration * distance);

        f32 t0 = (-mSpeed + sqrtDiscriminant) / mAcceleration;
        f32 t1 = (-mSpeed - sqrtDiscriminant) / mAcceleration;

        *pTime = MR::max(t0, t1);
    }
}

void MapPartsRailMover::endRotateAtPoint() {
    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtPoint))) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtPointAfterRotate));
    } else if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtEndPoint))) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndAfterRotate));
    }
}

void MapPartsRailMover::calcMoveSpeed(f32* pMoveSpeed) const {
    bool isNotNear = MR::isNearZero(mAcceleration, 0.0001f) == false;

    if (isNotNear) {
        *pMoveSpeed = MR::getRailCoordSpeed(mHost);
    } else {
        s32 calcType = 0;
        MR::getMapPartsArgSpeedCalcType(&calcType, mHost);

        if (MR::isMapPartsRailSpeedCalcTypeTime(calcType)) {
            calcMoveSpeedTime(pMoveSpeed);
        } else {
            calcMoveSpeedDirect(pMoveSpeed);
        }
    }
}

void MapPartsRailMover::calcMoveSpeedDirect(f32* pSpeed) const {
    f32 speed = -1.0f;
    MR::getMapPartsArgMoveSpeed(&speed, mHost);

    if (speed < 0.0f) {
        return;
    }

    *pSpeed = speed;
}

void MapPartsRailMover::calcMoveSpeedTime(f32* pTime) const {
    s32 moveTime = -1;
    MR::getMapPartsArgMoveTimeToNextPoint(&moveTime, mHost);

    if (moveTime >= 0) {
        s32 curPnt = MR::getCurrentRailPointNo(mHost);

        if (!MR::isRailGoingToEnd(mHost)) {
            curPnt--;
        }

        *pTime = MR::getRailPartLength(mHost, curPnt) / moveTime;
    }
}

void MapPartsRailMover::updateAccel() {
    mAccelTime = 0;
    MR::getMapPartsArgAccelTime(&mAccelTime, mHost);

    if (mAccelTime <= 0) {
        mAcceleration = 0.0f;
    } else {
        f32 moveSpeed = -1.0f;
        MR::getMapPartsArgMoveSpeed(&moveSpeed, mHost);
        if (moveSpeed < 0.0f) {
            mAcceleration = 0.0f;
        } else {
            mAcceleration = (moveSpeed - mSpeed) / mAccelTime;
        }
    }
}

bool MapPartsRailMover::tryPassPoint() {
    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) return false;
    if (mRailPointPassChecker->isReachedEnd()) {
        if (mMoveStopType == 0) {
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
            return true;
        } else {
            reachedEnd();
            return true;
        }
    } else if (mRailPointPassChecker->isPassed()) {
        passPoint();
        return true;
    }

    return false;
}

bool MapPartsRailMover::tryRestartAtEnd() {
    bool isMove = false;

    if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeMove)) || isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart))) {
        isMove = true;
    }

    if (isMove) {
        return false;
    }

    if (!MR::isMoveStartTypePlayerOnStopEnd(mMoveConditionType)) {
        return false;
    }

    if (!MR::isOnPlayer(MR::getBodySensor(mHost))) {
        return false;
    }

    if (!isNerve(GET_NERVE(MapPartsRailMover, HostTypeWaitForRestartByPlayerOn))) {
        return false;
    }

    restartAtEnd();
    return true;
}

void MapPartsRailMover::restartAtEnd() {
    MR::startActionSoundAuto(mHost, "RailMoverRestartAtEnd", -1, -1, -1);
    if (mMoveStopType == 0) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
    } else if (mMoveStopType == 1) {
        if (MR::hasMapPartsMoveStartSignMotion(mSignMotionType)) {
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart));
        } else {
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
        }
    } else if (mMoveStopType == 2) {
        mHasRepeated = true;
        MR::moveCoordToStartPos(mHost);
        updatePosition();
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
    } else if (mMoveStopType == 3) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeVanish));
    }
}

void MapPartsRailMover::exeWaitForRestart() {
    if (isFirstStep()) {
        MR::startActionSoundAuto(mHost, "RailMoverWaitStart", -1, -1, -1);
    }
    MR::startActionSoundAuto(mHost, "RailMoverWaiting", -1, -1, -1);
}

void MapPartsRailMover::exeMove() {
    if (isFirstStep()) {
        MR::startActionSoundAuto(mHost, "RailMoverMoveStart", -1, -1, -1);
        updateAccel();
        calcMoveSpeed(&mSpeed);
        sendMsgToHost(0xDA /* ACTMES_MAPPARTS_START_ROTATE_BETWEEN_POINTS */);
    }

    MR::startActionSoundAuto(mHost, "RailMoverMoveLoop", -1, -1, -1);
    bool isNotNear = !MR::isNearZero(mAcceleration, 0.0001f);

    if (isNotNear && getStep() < mAccelTime) {
        mSpeed += mAcceleration;
    }

    if (mSnapFramesRemaining > 0) --mSnapFramesRemaining;
    LiveActor* host = mHost;
    f32 speed = getSpeed();
    MR::moveCoord(host, mSpeed * speed);
    updatePosition();
}

void MapPartsRailMover::exeMoveStart() {
    if (isFirstStep()) {
        MR::startActionSoundAuto(mHost, "RailMoverSignStart", -1, -1, -1);
        mSignMotionRailCoord = MR::getRailCoord(mHost);
        if (MR::isExistCollisionParts(mHost)) MR::offUpdateCollisionParts(mHost);
        if (!mDisableSignMotionAnimation) MR::tryStartAllAnim(mHost, "RailMoveSign");
    }
    MR::startActionSoundAuto(mHost, "RailMoverSignLoop", -1, -1, -1);
    if (!mDisableSignMotionAnimation && MR::isExistAnimInKeeper(mHost, "RailMoveSign")) {
        if (MR::isActionEnd(mHost)) {
            MR::setRailCoord(mHost, mSignMotionRailCoord);
            if (MR::isExistCollisionParts(mHost)) MR::onUpdateCollisionParts(mHost);
            updatePosition();
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
        }
        return;
    }

    f32 v3;

    if ((getStep() / 3) % 2 == 0) {
        v3 = 1.0f;
    } else {
        v3 = -1.0f;
    }

    if (MR::isRailGoingToEnd(mHost)) {
        v3 *= -1.0f;
    }

    MR::setRailCoord(mHost, mSignMotionRailCoord + (7.0f * (v3 * ((getStep() % 3)))));
    updatePosition();
    s32 signalTime = MapParts::getMoveStartSignalTimeForRail();
    if (mSignMotionTimeOverride != -1) signalTime = mSignMotionTimeOverride;
    if (isStep(signalTime)) {
        MR::setRailCoord(mHost, mSignMotionRailCoord);
        if (MR::isExistCollisionParts(mHost)) MR::onUpdateCollisionParts(mHost);
        updatePosition();
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
    }
}

void MapPartsRailMover::exeStopAtPoint() {
    if (isFirstStep()) {
        mSpeed = 0.0f;
        mAcceleration = 0.0f;
        MR::setRailCoordSpeed(mHost, 0.0f);
        MR::startActionSoundAuto(mHost, "RailMoverStopAtPoint", -1, -1, -1);
        if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtPointBeforeRotate)))
            MR::startActionSoundAuto(mHost, "RailMoverStopAtPoint", -1, -1, -1);
        else MR::startActionSoundAuto(mHost, "RailMoverStopAtPointAfter", -1, -1, -1);
    }
    MR::startActionSoundAuto(mHost, "RailMoverStopAtPointLoop", -1, -1, -1);
    if (isStep(mStopTime)) {
        if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtPointBeforeRotate))) {
            if (sendMsgToHost(0xD8 /* ACTMES_MAPPARTS_START_ROTATE_AT_POINT */)) {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtPoint));
            } else {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtPointAfterRotate));
            }
        } else {
            setNerve(GET_NERVE(MapPartsRailMover, HostTypeMove));
        }
    }
}

void MapPartsRailMover::exeStopAtEnd() {
    if (isFirstStep()) {
        if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndBeforeRotate)))
            MR::startActionSoundAuto(mHost, "RailMoverStopAtEnd", -1, -1, -1);
        else MR::startActionSoundAuto(mHost, "RailMoverStopAtEndAfter", -1, -1, -1);
    }
    if (tryRestartAtEnd()) return;
    MR::startActionSoundAuto(mHost, "RailMoverStopAtEndLoop", -1, -1, -1);
    if (isStep(mStopTime)) {
        if (isNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndBeforeRotate))) {
            if (sendMsgToHost(0xD8 /* ACTMES_MAPPARTS_START_ROTATE_AT_POINT */)) {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeRotateAtEndPoint));
            } else {
                setNerve(GET_NERVE(MapPartsRailMover, HostTypeStopAtEndAfterRotate));
            }

        } else {
            restartAtEnd();
        }
    }
}

MapPartsRailMover::~MapPartsRailMover() {
    return;
}

void MapPartsRailMover::exeVanish() {
    if (MapPartsFunction::isFirstStep()) {
        MapPartsFunction::sendMsgToHost(0xDC /* ACTMES_MAPPARTS_DISAPPEAR_WITH_BLINK */);
    }
}

void MapPartsRailMover::exeStopAtEndWithPlayerOn() NO_INLINE {
    if (isFirstStep()) {
        MR::startActionSoundAuto(mHost, "RailMoverStopAtEndWithPlayerOn", -1, -1, -1);
    }

    if (!MR::isOnPlayer(MR::getBodySensor(mHost))) {
        setNerve(GET_NERVE(MapPartsRailMover, HostTypeWaitForRestartByPlayerOn));
    }
}

void MapPartsRailMover::exeRotateAtEndPoint() NO_INLINE {
    return;
}

void MapPartsRailMover::exeRotateAtPoint() NO_INLINE {
    return;
}

void MapPartsRailMover::exeWaitForRestartByPlayerOn() NO_INLINE {
    return;
}

void MapPartsRailMover::setStopTypeStopAtEnd() {
    mMoveStopType = 0;
}

void MapPartsRailMover::setSignMotionTypeShake() {
    mSignMotionType = 1;
}

bool MapPartsRailMover::isStarting() const {
    return isNerve(GET_NERVE(MapPartsRailMover, HostTypeMoveStart));
}


void MapPartsRailMover::exeWait() {
    return;
}

void MapPartsRailMover::moveToEndPos() {
    MR::moveCoordToEndPos(mHost);
    updatePosition();
    if (!MR::isRailGoingToEnd(mHost)) MR::reverseRailDirection(mHost);
    setNerve(GET_NERVE(MapPartsRailMover, HostTypeWait));
}

#pragma auto_inline off
// NONMATCHING: the two vector temporaries and their FPR values use reversed slots/registers.
void MapPartsRailMover::updatePosition() {
    const TVec3f& pos = MR::getRailPos(mHost);
    mPosition.x = pos.x;
    mPosition.y = pos.y;
    mPosition.z = pos.z;
    if (mSnapFramesRemaining > 0) {
        f32 remaining;
        f32 total = mSnapDuration;
        remaining = mSnapFramesRemaining;
        TVec3f offset(mSnapOffset);
        offset.scale(remaining);
        TVec3f scaled(offset);
        scaled.scale(1.0f / total);
        mPosition += scaled;
    }
}
#pragma auto_inline reset
