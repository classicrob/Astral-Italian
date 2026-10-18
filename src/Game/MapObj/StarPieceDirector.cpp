#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/StarPointerDirector.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

namespace NrvStarPieceShooter {
    NEW_NERVE(HostTypeNrvWait, StarPieceShooter, Wait);
    NEW_NERVE(HostTypeNrvLockOn, StarPieceShooter, LockOn);
};  // namespace NrvStarPieceShooter

StarPieceDirector::~StarPieceDirector() {
}
StarPieceShooter::~StarPieceShooter() {
}

StarPieceDirector* MR::getStarPieceDirector() {
    return MR::getSceneObj< StarPieceDirector >(SceneObj_StarPieceDirector);
}

void MR::createStarPiece() {
    int max = 70;

    if (!MR::isEqualStageName("AstroGalaxy")) {
        max = 30;
    }

    for (int i = 0; i < max; i++) {
        StarPiece* starPiece = new StarPiece("スターピースディレクターピース");
        starPiece->initWithoutIter();
        starPiece->makeActorDead();
        getStarPieceDirector()->registerActor(starPiece);
        registerDemoSimpleCastAll(starPiece);
    }
}

StarPiece* MR::getDeadStarPiece() {
    return getStarPieceDirector()->getDeadStarPiece();
}

void MR::resetChasingStarPiece() {
    getStarPieceDirector()->mResetChasingStarPiece = true;
}

bool MR::isResetChasingStarPiece() {
    return getStarPieceDirector()->mResetChasingStarPiece;
}

bool MR::gotByPlayer() {
    return getStarPieceDirector()->gotByPlayer();
}

bool MR::giftStarPieceToTarget(HitSensor* pGiftSensor, u32 numGift) {
    return getStarPieceDirector()->giftToTarget(pGiftSensor, numGift);
}

void MR::incNumStarPieceGettable(s32 inc) {
    getStarPieceDirector()->mNumStarPieceGettable += inc;
}

void MR::incNumStarPieceNewed() {
    getStarPieceDirector()->mNumStarPieceNewed++;
}

u32 StarPieceDirector::getRandomColor() {
    const u32 colors[6] = {0x800099FF, 0xE6A000FF, 0x46A108FF, 0x375AA0FF, 0xBE330BFF, 0x808080FF};
    return colors[MR::getRandom(0L, 6L)];
}

StarPieceDirector::StarPieceDirector(const char* pName)
    : LiveActorGroup(pName, 128), mNumStarPieceNewed(), mNumStarPieceGettable(), mColorIndex(), mNumStarPiecesLaunched(), mNewHostInfoIndex(),
      mNewReceiverInfoIndex(), mResetChasingStarPiece(), mSoundIndex(), mQueueNewGetSound() {
    MR::connectToSceneMapObjMovement(this);

    for (int i = 0; i < ARRAY_SIZE(mStarPieceShooters); i++) {
        mStarPieceShooters[i] = new StarPieceShooter(i, "発射用アクター");
        mStarPieceShooters[i]->initWithoutIter();
    }

    for (int i = 0; i < ARRAY_SIZE(mHostInfoArray); i++) {
        mHostInfoArray[i] = new StarPieceHostInfo(nullptr, 0, 0, 0);
    }

    for (int i = 0; i < ARRAY_SIZE(mReceiverInfoArray); i++) {
        mReceiverInfoArray[i] = new StarPieceReceiverInfo(nullptr, 0, 0, 0);
    }

    initCSDelay();
}

void StarPieceDirector::declare(const NameObj* pNameObj, s32 num) {
    if (num > 0) {
        StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);

        if (hostInfo == nullptr) {
            hostInfo = mHostInfoArray[mNewHostInfoIndex];
            hostInfo->mObj = const_cast< NameObj* >(pNameObj);
            hostInfo = mHostInfoArray[mNewHostInfoIndex];
            hostInfo->mObj = const_cast< NameObj* >(pNameObj);
            mNewHostInfoIndex++;
        }

        hostInfo->_4 += num;
        MR::incNumStarPieceGettable(num);
    }
}

void StarPieceDirector::declareReceiveNum(const NameObj* pNameObj, s32 num) {
    if (num > 0) {
        StarPieceReceiverInfo* receiverInfo = findReceiverInfo(pNameObj);

        if (receiverInfo == nullptr) {
            receiverInfo = mReceiverInfoArray[mNewReceiverInfoIndex];
            receiverInfo->mObj = const_cast< NameObj* >(pNameObj);
            receiverInfo = mReceiverInfoArray[mNewReceiverInfoIndex];
            receiverInfo->mObj = const_cast< NameObj* >(pNameObj);
            mNewReceiverInfoIndex++;
        }

        receiverInfo->_4 += num;
    }
}

s32 StarPieceDirector::getDeclareRemnantStarPieceCount(const NameObj* pNameObj) {
    StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);
    return hostInfo->_4 - hostInfo->_C;
}

s32 StarPieceDirector::getDeclareRemnantStarPieceCountNotExist(const NameObj* pNameObj) {
    StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);
    return hostInfo->_4 - hostInfo->_C - hostInfo->_8;
}

bool StarPieceDirector::hopPiece(const NameObj* pNameObj, const TVec3f& rVec1, const TVec3f& rVec2) {
    StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);

    if (hostInfo == nullptr) {
        return false;
    }

    if (!hostInfo->isAppearable()) {
        return false;
    }

    StarPiece* hopPiece = getDeadStarPiece();

    if (hopPiece == nullptr) {
        return false;
    }

    hopPiece->setHostInfo(hostInfo);
    hostInfo->_8++;
    hopPiece->appearHop(rVec1, rVec2);
    if (MR::isInWater(rVec1)) MR::startActionSound(hopPiece, "OjStarPieceBurstW", -1, -1, -1);
    else MR::startActionSound(hopPiece, "OjStarPieceBurst", -1, -1, -1);
    return true;
}

bool StarPieceDirector::appearPiece(const NameObj* pNameObj, const TVec3f& rVec, s32 a1, f32 f1, f32 f2, bool a2, bool a3, bool a4) {
    StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);

    if (hostInfo == nullptr) {
        return false;
    }

    bool appearedStarPiece = false;

    for (int i = 0; i < a1; i++) {
        if (!hostInfo->isAppearable()) {
            break;
        }

        StarPiece* appearPiece = getDeadStarPiece();

        if (appearPiece != nullptr) {
            appearPiece->setHostInfo(hostInfo);
            hostInfo->_8++;
            appearPiece->launch(rVec, f1, f2, a3, a2, a4);
            if (!appearedStarPiece) {
                if (MR::isInWater(rVec)) MR::startActionSound(appearPiece, "OjStarPieceBurstW", -1, -1, -1);
                else MR::startActionSound(appearPiece, "OjStarPieceBurst", -1, -1, -1);
            }
            appearedStarPiece = true;
        }
    }

    return appearedStarPiece;
}

bool StarPieceDirector::appearPieceToDirection(const NameObj* pNameObj, const TVec3f& rVec1, const TVec3f& rVec2, s32 a1, f32 f1, f32 f2, bool a2,
                                               bool a3, bool a4) {
    StarPieceHostInfo* hostInfo = findHostInfo(pNameObj);

    if (hostInfo == nullptr) {
        return false;
    }

    bool appearedStarPiece = false;

    for (int i = 0; i < a1; i++) {
        if (!hostInfo->isAppearable()) {
            break;
        }

        StarPiece* appearPiece = getDeadStarPiece();

        if (appearPiece == nullptr) {
            break;
        }

        appearPiece->setHostInfo(hostInfo);
        hostInfo->_8++;
        appearPiece->launch(rVec1, rVec2, f1, f2, a3, a2, a4);
        if (!appearedStarPiece) {
            if (MR::isInWater(rVec1)) MR::startActionSound(appearPiece, "OjStarPieceBurstW", -1, -1, -1);
            else MR::startActionSound(appearPiece, "OjStarPieceBurst", -1, -1, -1);
        }
        appearedStarPiece = true;
    }

    return appearedStarPiece;
}

// NONMATCHING: quaternion/vector stack slots and f30/f31 allocation differ.
bool StarPieceDirector::appearPieceToDirectionRandom(const NameObj* host, const TVec3f& pos, const TVec3f& velocity, s32 count, f32 angle, bool a2, bool a3, bool a4) {
    StarPieceHostInfo* hostInfo = findHostInfo(host);
    if (!hostInfo) return false;
    bool appeared = false;
    for (s32 i = 0; i < count; i++) {
        if (!hostInfo->isAppearable()) break;
        StarPiece* piece = getDeadStarPiece();
        TVec3f direction(velocity);
        TVec3f axis(direction);
        f32 speed = axis.length();
        if (!MR::normalizeOrZero(&axis)) {
            TQuat4f tilt;
            f32 tiltAngle = MR::getRandom(0.0f, angle);
            tilt.setRotate(TVec3f(0.0f, 1.0f, 0.0f), tiltAngle);
            TQuat4f spin;
            f32 spinAngle = MR::getRandom(-3.1415927f, 3.1415927f);
            spin.setRotate(TVec3f(0.0f, 0.0f, 1.0f), spinAngle);
            TQuat4f align;
            align.setRotate(TVec3f(0.0f, 0.0f, 1.0f), axis);
            TVec3f rotated(0.0f, 0.0f, 1.0f);
            tilt.transform(rotated);
            spin.transform(rotated);
            align.transform(rotated);
            direction.set(rotated);
            direction.setLength(speed);
        }
        if (!piece) break;
        piece->setHostInfo(hostInfo);
        hostInfo->_8++;
        piece->launch(pos, direction, a3, a2, a4);
        if (!appeared) {
            if (MR::isInWater(pos)) MR::startActionSound(piece, "OjStarPieceBurstW", -1, -1, -1);
            else MR::startActionSound(piece, "OjStarPieceBurst", -1, -1, -1);
        }
        appeared = true;
    }
    return appeared;
}

bool StarPieceDirector::gotByPlayer() {
    if (MR::getStarPieceNum() >= 10000) {
        return false;
    }

    MR::addStarPiece(1);
    MR::startSystemSE("SE_SY_STAR_PIECE_GET");

    mQueueNewGetSound = true;
    MR::getStarPiecePlayer();
    return true;
}

void StarPieceDirector::movement() {
    mResetChasingStarPiece = false;
    updateSound();
}

void StarPieceDirector::updateSound() {
    if (mGetSoundArray[mSoundIndex]) {
        MR::startCSSound("CS_STARDUST_GET", nullptr, -1, 0, 1.0f);
    }

    mGetSoundArray[mSoundIndex] = false;

    if (mQueueNewGetSound) {
        mGetSoundArray[mSoundIndex] = true;
    }

    mQueueNewGetSound = false;
    mSoundIndex++;
    mSoundIndex %= 16;
}

void StarPieceDirector::initCSDelay() {
    for (int i = 0; i < 16; i++) {
        mGetSoundArray[i] = false;
    }
}

void StarPieceDirector::initCSSound() {
    mQueueNewGetSound = false;
    initCSDelay();
    mSoundIndex = 0;
}

bool StarPieceDirector::giftToTarget(HitSensor* pGiftSensor, u32 numGift) {
    if (MR::getStarPieceNum() < numGift) {
        return false;
    }

    StarPieceReceiverInfo* receiverInfo = findReceiverInfo(MR::getSensorHost(pGiftSensor));

    if (receiverInfo == nullptr) {
        return false;
    }

    if (receiverInfo->_4 < receiverInfo->mGotCount + receiverInfo->_8 + numGift) {
        return false;
    }

    StarPiece* giftPiece = getDeadStarPiece();

    if (giftPiece == nullptr) {
        return false;
    }

    MR::requestMovementOn(giftPiece);
    receiverInfo->_8 += numGift;
    MR::startCSSound("CS_FEED_SPIECE", "SE_SY_CS_FEED_PIECE", 0, 0, 1.0f);
    giftPiece->giftToTarget(receiverInfo, numGift, pGiftSensor, calcPosCameraShoot(0));
    return true;
}

StarPiece* StarPieceDirector::getDeadStarPiece() {
    LiveActor* deadActor = getDeadActor();

    if (deadActor == nullptr) {
        return nullptr;
    }

    StarPiece* deadStarPiece = static_cast< StarPiece* >(deadActor);

    deadStarPiece->setColor(mColorIndex);
    mColorIndex += 1;

    if (StarPiece::getNumColor() <= mColorIndex) {
        mColorIndex = 0;
    }

    return deadStarPiece;
}

TVec3f StarPieceDirector::calcPosCameraShoot(s32 addOrSubXDir) {
    TVec3f returnVec(MR::getCamPos());
    TVec3f camXDir(MR::getCamXdir());
    TVec3f camYDir(MR::getCamYdir());
    TVec3f camZDir(MR::getCamZdir());

    if (addOrSubXDir == 0) {
        returnVec.add(camXDir * 300.0f);
    } else {
        returnVec.sub(camXDir * 300.0f);
    }

    returnVec.add(camYDir * 0.0f);
    returnVec.add(camZDir * 30.0f);
    return returnVec;
}

void StarPieceDirector::clearGotCountReceiver(const NameObj* pNameObj) {
    StarPieceReceiverInfo* receiverInfo = findReceiverInfo(pNameObj);
    receiverInfo->mGotCount = 0;
}

StarPieceHostInfo* StarPieceDirector::findHostInfo(const NameObj* pNameObj) {
    for (StarPieceHostInfo** current = mHostInfoArray; current < &mHostInfoArray[ARRAY_SIZE(mHostInfoArray)]; current++) {
        if (pNameObj == (*current)->mObj) {
            return *current;
        }
    }

    return nullptr;
}

StarPieceReceiverInfo* StarPieceDirector::findReceiverInfo(const NameObj* pNameObj) {
    for (StarPieceReceiverInfo** current = mReceiverInfoArray; current < &mReceiverInfoArray[ARRAY_SIZE(mReceiverInfoArray)]; current++) {
        if (pNameObj == (*current)->mObj) {
            return *current;
        }
    }

    return nullptr;
}

StarPieceShooter::StarPieceShooter(s32 a1, const char* pName)
    : LiveActor(pName), _90(), _8C(), _94(999999.0f), _98(), _9C(999999.0f), _A0(a1), _A4(), _A8(0, 0, 0), _B4(0, 0, 0) {
}

void StarPieceShooter::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovementCalcAnim(this);
    MR::invalidateClipping(this);
    MR::offCalcGravity(this);

    initNerve(GET_NERVE(StarPieceShooter, HostTypeNrvWait));

    initEffectKeeper(0, "StarDust", false);
    MR::setEffectHostSRT(this, "Charge", &_A8, &_B4, nullptr);

    initHitSensor(1);
    MR::addHitSensorEye(this, "search", 64, 5.0f, TVec3f(0.0f, 0.0f, 0.0f));
    makeActorAppeared();
}

void StarPieceShooter::exeWait() {
    if (_90 != nullptr) {
        _98 = _90;
        _9C = _94;
        setNerve(GET_NERVE(StarPieceShooter, HostTypeNrvLockOn));
    }
}

void StarPieceShooter::exeLockOn() {
    if (_90 != nullptr) {
        _98 = _90;
        _9C = _94;
        setNerve(GET_NERVE(StarPieceShooter, HostTypeNrvLockOn));
        return;
    }

    if (MR::isGreaterStep(this, 0)) {
        _98 = nullptr;
        _9C = 999999.0f;
        setNerve(GET_NERVE(StarPieceShooter, HostTypeNrvWait));
    }
}

void StarPieceShooter::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorMapObj(pReceiver) && !MR::isSensorEnemy(pReceiver)) {
        return;
    }

    if (MR::sendMsgLockOnStarPieceShoot(pReceiver, pSender)) {
        TVec3f diff(*MR::getSensorPos(pReceiver));
        diff.sub(*MR::getSensorPos(pSender));
        f32 distBetweenSensors = diff.length();

        if (distBetweenSensors < _94) {
            _90 = pReceiver;
            _94 = distBetweenSensors;
        }
    }
}

void StarPieceShooter::control() {
    tryShoot();

    TVec3f playerCenterPos(*MR::getPlayerCenterPos());
    TVec3f playerUpVec;
    MR::getPlayerUpVec(&playerUpVec);

    _A8.set(playerCenterPos);
    _A8.sub(playerUpVec * -160.0f);

    f32 scale = _A4 / 7.0f;
    scale *= 0.25f;
    scale += 0.125f;
    MR::setEffectBaseScale(this, "Charge", scale);
}

bool StarPieceShooter::shoot() {
    HitSensor* pSensor;
    StarPiece* const deadStarPiece = MR::getDeadStarPiece();

    if (deadStarPiece == nullptr) {
        return false;
    }

    pSensor = _90;

    if (pSensor == nullptr) {
        pSensor = _98;
    }

    TVec3f position = mPosition;
    TVec3f vec;
    TVec3f posCameraShoot = MR::getStarPieceDirector()->calcPosCameraShoot(_A0);
    TVec3f negCameraPos = -MR::getCamPos();

    if (pSensor != nullptr) {
        negCameraPos.add(*MR::getSensorPos(pSensor));

        if (MR::getFirstPolyOnLineToMap(&vec, nullptr, MR::getCamPos(), negCameraPos) && vec.squared(MR::getCamPos()) < negCameraPos.squared()) {
            deadStarPiece->throwToTargetCore(vec, posCameraShoot, mGravity, _8C, true);
        } else {
            deadStarPiece->throwToTarget(pSensor, posCameraShoot, mGravity, _8C);
        }
    } else {
        negCameraPos.add(position);

        if (MR::getFirstPolyOnLineToMap(&vec, nullptr, MR::getCamPos(), negCameraPos) && vec.squared(MR::getCamPos()) < negCameraPos.squared()) {
            deadStarPiece->throwToTargetCore(vec, posCameraShoot, mGravity, _8C, true);
        } else {
            deadStarPiece->throwToTargetCore(position, posCameraShoot, mGravity, _8C, true);
        }
    }

    return true;
}

bool StarPieceShooter::tryShoot() {
    if (!((MR::isStarPointerInScreen(_A0)) &&
          MR::isStarPointerValid(_A0) && MR::getStarPieceNum() > 0 && MR::isEnableStarPointerShootStarPiece(_A0) &&
          (_A0 != 0 || !MR::isStarPointer1PInvalid2PValidMode()))) {
        _90 = nullptr;
        _94 = 999999.9f;
        _A4 = -1;
        MR::deleteEffect(this, "Charge");

        if ((MR::getStarPieceNum() <= 0) && MR::isStarPointerInScreen(_A0) && MR::isStarPointerValid(_A0) && MR::testCorePadTriggerB(_A0) &&
            (MR::isEnableStarPointerShootStarPiece(_A0) || MR::isStarPointerModeStarPieceTarget())) {
            MR::startCSSound("CS_NO_SPIECE", "SE_SY_CS_NO_PIECE", _A0, 0, 1.0f);
        }

        return false;
    }

    calcShootGoalUsingPointingDepth();

    if (_A4 < 0) {
        if (!MR::testCorePadButtonB(_A0)) {
            _A4 = 0;
        }
    } else if (!MR::testCorePadButtonB(_A0)) {
        if (_A4 > 0) {
            MR::deleteEffect(this, "Charge");
        }

        _A4 = 0;
    } else {
        if (_A4 == 0) {
            if (MR::testCorePadTriggerB(_A0)) {
                if (StarPointerFunction::canShoot(_A0)) {
                    MR::emitEffect(this, "Charge");
                    _A4++;
                } else {
                    _A4 = -1;
                }
            }
        } else if (MR::testCorePadButtonB(_A0)) {
            _A4++;
        }

        s32 shootStep;

        for (int i = 0; i < 2; i++) {
            shootStep = 7;

            if (i == 0) {
                shootStep = 1;
            }

            shootStep += i * 7;

            if (shootStep == _A4) {
                shoot();
                MR::deleteEffect(this, "Charge");
                MR::tryRumblePadMiddle(this, _A0);
                MR::startCSSound("CS_STARDUST_SHOOT", "SE_SY_CS_STAR_PIECE_SHOOT", _A0, 0, 1.0f);

                if (i == 1) {
                    _A4 = 1;
                }

                break;
            }
        }
    }

    _90 = nullptr;
    _94 = 999999.9f;
    return true;
}

void StarPieceShooter::calcShootGoalUsingPointingDepth() {
    TVec3f playerPos(*MR::getPlayerPos());
    TVec3f starPointerWorldPos(*MR::getStarPointerWorldPosUsingDepth(_A0));

    mGravity.set(*MR::getPlayerGravity());

    TVec3f dir(starPointerWorldPos);
    dir.sub(playerPos);

    f32 dirsquared = dir.squared();
    f32 new_8C = 400.0f;

    if (16000000.0f < dirsquared) {
        TVec3f camPos(MR::getCamPos());
        TVec3f dirToPlayer(playerPos);
        dirToPlayer.sub(camPos);

        TVec2f pointingPos;
        MR::getCorePadPointingPosBasedOnScreen(&pointingPos, _A0);

        TVec3f worldPos;
        MR::calcWorldPositionFromScreen(&worldPos, pointingPos, -1.0f);
        worldPos.sub(camPos);
        MR::normalize(&worldPos);

        starPointerWorldPos.set(camPos);

        starPointerWorldPos.add(worldPos * (3000.0f + dirToPlayer.length()));
        new_8C = 400.0f;
    } else if (9000000.0f < dirsquared) {
        dir.setLength(3000.0f);
        starPointerWorldPos.set(playerPos);
        starPointerWorldPos.add(dir);
    }

    MR::setPosition(this, starPointerWorldPos);
    _8C = new_8C;
}
