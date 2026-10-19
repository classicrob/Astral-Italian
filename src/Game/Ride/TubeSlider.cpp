#include "Game/Ride/TubeSlider.hpp"
#include "Game/Ride/TubeSliderFunction.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"


namespace {
    extern const f32 sCeilingReflectRate;
    extern const f32 sWallReflectRate;
    extern const volatile f32 sInitialRiderOffset;
    f32 one = 1.0f;
    f32 thirty = 30.0f;
    f32 onefifty = 150.0f;

    f32 onehundred = 100.0f;
    f32 twohundred = 200.0f;
}  // namespace

TubeSliderInfo::TubeSliderInfo(f32 f) {
    mRadius = f;
    mPrimaryFloorType = 0;
    mPrimaryFloorAngle = 0.0f;
    mPrimaryFloorWidth = 0.0f;
    mSecondaryFloorType = 0;
    mSecondaryFloorAngle = 0.0f;
    mSecondaryFloorWidth = 0.0f;
}

bool TubeSliderInfo::hasWall(bool s) const {  // TRUE for Left side, FALSE for right side.
    if ((mSecondaryFloorType == 0x32) || (s && mSecondaryFloorType == 0x33) || (!s && mSecondaryFloorType == 0x34))
        return true;
    return false;
}

namespace NrvTubeSlider {
    NEW_NERVE(TubeSliderNrvWait, TubeSlider, Wait);
    NEW_NERVE(TubeSliderNrvEnd, TubeSlider, End);
    NEW_NERVE(TubeSliderNrvRideJump, TubeSlider, RideAir);
    NEW_NERVE(TubeSliderNrvRideSpin, TubeSlider, RideAir);
    NEW_NERVE(TubeSliderNrvRideFall, TubeSlider, RideAir);
    NEW_NERVE(TubeSliderNrvRide, TubeSlider, Ride);
    NEW_NERVE(TubeSliderNrvRideWallHit, TubeSlider, RideWallHit);
    NEW_NERVE(TubeSliderNrvRideLand, TubeSlider, RideLand);
    NEW_NERVE(TubeSliderNrvRideDamage, TubeSlider, RideAir);
    NEW_NERVE(TubeSliderNrvRideDamageLand, TubeSlider, RideLand);
    NEW_NERVE(TubeSliderNrvRideDamageDeath, TubeSlider, DamageDeath);
    NEW_NERVE(TubeSliderNrvRideSlamCeiling, TubeSlider, KillPlayer);
    NEW_NERVE(TubeSliderNrvRideSlamWall, TubeSlider, KillPlayer);
}  // namespace NrvTubeSlider

TubeSlider::TubeSlider(const char* pName)
    : LiveActor(pName), mRider(nullptr), _94(0.0f), _98(0.0f, 0.0f, 1.0f), _A4(0.0f, 1.0f, 0.0f), mForwardSpeed(0.0f), mCameraInfo(nullptr),
      mPlayerTheta(0.0f), _EC(0.0f), _F0(0.0f), _F4(0.0f), _F8(sInitialRiderOffset), _FC(0.0f), _100(0.0f, 0.0f, 0.0f), mJumpVelocity(0.0f, 0.0f, 0.0f), _118(0),
      _11C(0.0f), _120(0), mInfo(500.0f), mRadiusDefault(500.0f), mFloorCodeDefault(0), _148(0.0f, 0.0f, 0.0f),
      mShadowDropPos(0.0f, 0.0f, 0.0f), mShadowDropDir(0.0f, 1.0f, 0.0f), _16C(0.0f, 0.0f, 0.0f), _178(0.0f, 0.0f, 0.0f),
      _184(0.0f, 0.0f, 1.0f), _190(0.0f, 1.0f, 0.0f), _19C(150.0f), _1A0(&thirty), _1A4(&onefifty), _1A8(false) {
    mRiderMtx.identity();
}

void TubeSlider::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mRadiusDefault);
    MR::getJMapInfoArg1NoInit(rIter, &mFloorCodeDefault);
    s32 arg6 = -1;
    MR::getJMapInfoArg6NoInit(rIter, &arg6);
    if (arg6 == 1) {
        _1A0 = &onehundred;
        _1A4 = &twohundred;
    }
    initRailRider(rIter);
    MR::connectToScene(this, 0x28, -1, -1, -1);
    initSound(8, "TubeSlider", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(TubeSlider, TubeSliderNrvWait), 0);
    MR::initAndSetRailClipping(&_148, this, 100.0f, 500.0f);
    MR::calcRailStartPointPos(&mPosition, this);
    initHitSensor(2);
    MR::addHitSensorCallbackBinder(this, "Bind", 8, 50.0f);
    MR::addHitSensorMtxRide(this, "Ride", 8, 50.0f, mRiderMtx, TVec3f(0.0f, 50.0f, 0.0f));
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "滑り");
    MR::initActorCameraProgrammable(this);
    MR::initShadowVolumeCylinder(this, 40.0f);
    MR::setShadowDropPositionPtr(this, nullptr, &mShadowDropPos);
    MR::setShadowDropDirectionPtr(this, nullptr, &mShadowDropDir);
    TubeSliderFunction::initChildObjs(this, rIter);
    makeActorAppeared();
}

void TubeSlider::tryCalcInfo(TVec3f* a, TVec3f* b, f32* c, TubeSlider* slider, bool check) {
    f32 flt = MR::getRailCoord(slider);
    MR::moveCoordAndTransToNearestRailPos(slider, *MR::getPlayerCenterPos());
    TubeSliderFunction::updateTubeSliderInfo(slider, true);
    if (a != nullptr)
        a->set(MR::getRailPos(slider));
    if (b != nullptr) {
        b->set(*MR::getPlayerCenterPos() - MR::getRailPos(slider));
        MR::vecKillElement(*b, MR::getRailDirection(slider), b);
        MR::normalizeOrZero(b);
    }
    if (c != nullptr)
        *c = TubeSliderFunction::getRadius(slider);

    if (!check) {
        MR::setRailCoord(slider, flt);
        TubeSliderFunction::updateTubeSliderInfo(slider, true);
    }
}

void TubeSlider::exeWait() {
    if (MR::isFirstStep(this))
        MR::validateClipping(this);
}

void TubeSlider::exeEnd() {
    if (MR::isStep(this, 15) || MR::isOnGroundPlayer())
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvWait));
}

// NONMATCHING: f-reg allocation in wind block
void TubeSlider::exeRideAir() {
    if (MR::isFirstStep(this))
        _118 = 0;

    bool isReachedGoal = MR::isRailReachedNearGoal(this, 50.0f);
    if (!isReachedGoal) {
        TVec3f railEndPos(0.0f);
        MR::calcRailEndPos(&railEndPos, this);
        f32 distToEnd = PSVECDistance(railEndPos, MR::getPlayerCenterPos());
        TVec3f* center = MR::getPlayerCenterPos();
        f32 distToRailPos = PSVECDistance(MR::getRailPos(this), center);
        if (distToEnd <= distToRailPos)
            isReachedGoal = true;
    }

    if (isReachedGoal) {
        MR::stopPlayerBlink();
        MR::endActorCameraProgrammable(this, -1, true);
        MR::endBindAndPlayerJump(this, mJumpVelocity, 0);
        MR::invalidateShadow(this, nullptr);
        if (MR::isValidSwitchA(this))
            MR::offSwitchA(this);
        MR::tryDeleteEffect(mRider, "共通スリップ坂");
        mRider = nullptr;
        MR::resetSupportTicoAttachRangeTubeSlider();
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvEnd));

        if (isReachedGoal && !_1A8) {
            if (MR::isEqualStageName("BigTree2Galaxy")) {
                if (MR::getCurrentScenarioNo() == 3) {
                    if (MR::isPurpleCoinNum100()) {
                        MR::startLevelSoundPlayer("JumpObj", -1, -1);
                        MR::startSubBGM("BGM_RACE_WIN", false);
                    } else {
                        MR::startSubBGM("BGM_RACE_LOSE", false);
                    }
                } else {
                    MR::startLevelSoundPlayer("JumpObj", -1, -1);
                    MR::startSubBGM("BGM_RACE_WIN", false);
                }
            }

            _1A8 = true;
            return;
        }
    } else {
        {
        s32 pitchBase = 50;
        s32 scale = 10;
        f32 spd = mForwardSpeed;
        s32 windPitch = (s32)((f32)pitchBase + __fabsf(spd) * (f32)scale);
        s32 windVol = MR::clamp((s32)(spd * (f32)scale) + 600, 0, 1000);
        MR::startActionSound(this, "Wind", windPitch, windVol, -1);
    }

        if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideFall)) && MR::isLessEqualStep(this, 10) && tryPlayerJump(false))
            return;

        if (_118 > 0)
            _118--;

        if (MR::testCorePadTriggerA(0))
            _118 = 5;

        TVec3f fro(1.0f, 0.0f, 0.0f);
        TVec3f up(_98);
        MR::makeAxisUpFront(&fro, &up, _A4, up);

        if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage))) {
            mJumpVelocity.scale(0.98f);
        } else {
            mJumpVelocity.scale(0.995f);
        }
        TVec3f unk(0.0f, 1.0f, 0.0f);
        TubeSlider::tryCalcInfo(nullptr, &unk, nullptr, this, false);
        if (unk.dot(_A4) < 0.0f) {
            MR::turnVecToVecDegree(&_A4, _A4, -unk, 0.85f, TVec3f((int)0, (int)1, (int)0));
        }
        if (MR::getSubPadStickX(0) != 0.0f) {
            if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSpin))) {
                mJumpVelocity.sub(fro * MR::getSubPadStickX(0) * 0.2f);
            } else if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage))) {
                mJumpVelocity.sub(fro * MR::getSubPadStickX(0) * 0.3f);
            } else {
                mJumpVelocity.sub(fro * MR::getSubPadStickX(0) * 0.03f);
            }
        }
        mJumpVelocity.sub(_A4 * 0.88f);
        _100.add(mJumpVelocity);
        mRiderMtx.setXYZDir(fro, _A4, up);
        mRiderMtx.setTrans(_100);
        TVec3f igh(_98);
        if (MR::isGreaterEqualStep(this, 10)) {
            TubeSlider::tryCalcInfo(nullptr, nullptr, nullptr, this, true);
            igh.set(MR::getRailDirection(this));
        }

        updateCameraAndShadow(MR::getRailDirection(this));

        if (!isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSpin)) && !isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage)) &&
            mJumpVelocity.dot(_A4) < 0.0f && tryPlayerJumpSpin())
            return;

        if (MR::isGreaterStep(this, 120))
            MR::forceKillPlayerByAbyss();
    }
}

void TubeSlider::exeRide() {
    if (MR::isFirstStep(this))
        MR::startBckPlayerJ("チューブスライダー");
    updateOnGround();
}

void TubeSlider::exeRideWallHit() {
    if (MR::isFirstStep(this)) {
        MR::startLevelSoundPlayer("WallHitSlider", -1, -1);
        MR::startBckPlayerJ("チューブスライダー壁ヒット");
    }
    if (!updateOnGround() && MR::isActionEnd(mRider))
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRide));
}

void TubeSlider::exeDamageDeath() {
    if (MR::isFirstStep(this))
        MR::startBckPlayerJ("チューブスライダーダメージ着地");
}

void TubeSlider::exeRideLand() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand))) {
            MR::startBckPlayerJ("チューブスライダーダメージ着地");
            MR::startLevelSoundPlayer("SliderDamageLand", -1, -1);
        } else {
            MR::startBckPlayerJ("チューブスライダー着地");
            if (TubeSliderFunction::isGroundIce(this)) {
                MR::startLevelSoundPlayer("LandSkate", -1, -1);
            } else {
                MR::startLevelSoundPlayer("Land", -1, -1);
            }
        }
        MR::emitEffect(mRider, "共通スリップ坂");
    }

    bool isDamaging = isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand));
    if (isDamaging && MR::isStep(this, 40)) {
        MR::startLevelSoundPlayer("SliderDamageLand2nd", -1, -1);
    }

    if (tryPlayerJump(true)) {
        if (isDamaging)
            MR::stopPlayerBlink();
    } else if (updateOnGround()) {
        if (isDamaging)
            MR::stopPlayerBlink();
    } else if (MR::isActionEnd(mRider)) {
        if (isDamaging)
            MR::stopPlayerBlink();
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRide));
    }
}

void TubeSlider::exeKillPlayer() {
    if (MR::isFirstStep(this) && isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSlamWall))) {
        MR::startBckPlayerJ("壁ヒット");
    }

    mJumpVelocity.sub(_A4 * 0.88f);
    _100.add(mJumpVelocity);

    TVec3f front(1.0f, 0.0f, 0.0f);
    TVec3f up(_98);
    MR::makeAxisUpFront(&front, &up, _A4, up);
    mRiderMtx.setXYZDir(front, _A4, up);
    mRiderMtx.setTrans(_100);
    updateCameraAndShadow(_98);

    if (MR::isGreaterStep(this, 120))
        MR::forceKillPlayerByAbyss();
}

void TubeSlider::updateHitSensor(HitSensor* sensor) {
    if (!mRider || (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideJump)) || isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSpin)) ||
                 isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideFall)) || isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage)))) {
        TVec3f A(0.0f);
        TVec3f B(0.0f);
        f32 C = 0.0f;
        TubeSlider::tryCalcInfo(&A, &B, &C, this, false);
        B.scale(C + MR::getSensorRadius(sensor));
        A.add(B);
        MR::setSensorPos(sensor, A);
    }
}

void TubeSlider::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!mRider)
        return;

    if (MR::isSensorBinder(pSender) &&
        (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideJump)) || isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSpin)) ||
         (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideFall)) && MR::isGreaterEqualStep(this, 5)) ||
         isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage))) &&
        MR::isSensorPlayer(pReceiver)) {
        const TVec3f* senderPos = MR::getSensorPos(pSender);
        const TVec3f* receiverPos = MR::getSensorPos(pReceiver);
        TVec3f scratch(*receiverPos);
        scratch.sub(*senderPos);
        TVec3f scratch2(0.0f);
        TubeSlider::tryCalcInfo(nullptr, &scratch2, nullptr, this, true);
        scratch2.scale(-1.0f);
        f32 dotA = scratch.dot(scratch2);
        f32 dotB = mJumpVelocity.dot(scratch2);

        f32 wallDist = 0.0f;
        if (dotB < 0.0f && (50.0f <= dotA || TubeSliderFunction::isHitLeftWall(this, &wallDist) ||
                            TubeSliderFunction::isHitRightWall(this, &wallDist)) &&
            isOffSlide()) {
            MR::tryRumblePadMiddle(this, 0);
            MR::shakeCameraNormal();
            if (MR::isPlayerDead())
                setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageDeath));
            else if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage)))
                setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand));
            else {
                MR::stopPlayerBlink();
                setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideLand));
            }

            return;
        }

        if (TubeSliderFunction::isUnknown2(this))
            return;

        if (dotB > 0.0f && -50.0f < dotA && dotA < -30.0f) {
            MR::tryRumblePadMiddle(this, 0);
            MR::shakeCameraNormal();
            mJumpVelocity.sub(scratch2.scaleInline(dotB).scaleInline(1.0f + sCeilingReflectRate));
            MR::stopPlayerBlink();
            setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSlamCeiling));
            return;
        }
        if (-30.0f <= dotA && dotA < 50.0f) {
            MR::tryRumblePadMiddle(this, 0);
            MR::shakeCameraNormal();
            f32 along = mJumpVelocity.dot(_98);
            mJumpVelocity.sub(_98.scaleInline(along).scaleInline(1.0f + sWallReflectRate));
            MR::stopPlayerBlink();
            setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSlamWall));
            return;
        }
    }
    if (MR::tryGetItem(pSender, pReceiver)) return;
}

bool TubeSlider::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }
    return false;
}

bool TubeSlider::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!mRider)
        return false;

    if (!isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage)) && !isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand)) &&
        !isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageDeath))) {
        damagePlayer();
        return true;
    }
    return false;
}

bool TubeSlider::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvEnd)) || MR::isOnGroundPlayer())
            return false;
        mRider = MR::getSensorHost(pSender);
        MR::invalidateClipping(this);
        if (!isOffSlide()) {
            MR::validateClipping(this);
            mRider = nullptr;
            return false;
        }
        MR::tryRumblePadMiddle(this, 0);
        if (MR::isValidSwitchA(this))
            MR::onSwitchA(this);
        MR::startActorCameraProgrammable(this, -1);
        _184.set(_98);
        _190.set(_A4);
        _120 = 0;
        _11C = 0.0f;
        MR::tryRumblePadMiddle(this, 0);
        MR::shakeCameraNormal();
        MR::setSupportTicoAttachRangeTubeSlider();
        MR::validateShadow(this, nullptr);
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideLand));
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg)) {
        _120++;
        MR::setBaseTRMtx(mRider, mRiderMtx);
        return true;
    }
    if (MR::isMsgRushCancel(msg)) {
        MR::endActorCameraProgrammable(this, -1, true);
        if (MR::isValidSwitchA(this))
            MR::offSwitchA(this);
        MR::tryDeleteEffect(mRider, "共通スリップ坂");
        mRider = nullptr;
        MR::resetSupportTicoAttachRangeTubeSlider();
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvEnd));
        return true;
    }
    return false;
}

bool TubeSlider::isOffSlide() {
    TVec3f temp;
    TubeSlider::tryCalcInfo(nullptr, &temp, nullptr, this, true);
    TVec3f temp2;
    TubeSliderFunction::getRailUpVec(&temp2, this);
    _94 = MR::getRailCoord(this);
    mPlayerTheta = MR::diffAngleSigned(temp2, temp, MR::getRailDirection(this)) * 57.29578f;
    if (TubeSliderFunction::isUnknown2(this) && !TubeSliderFunction::isHitLeftWall(this, &mPlayerTheta) &&
        !TubeSliderFunction::isHitRightWall(this, &mPlayerTheta)) {
        return false;
    }

    _98.set(MR::getRailDirection(this));
    _A4.set(-temp);
    MR::repeatDegree(&mPlayerTheta);
    MR::setRailDirectionToEnd(this);
    mForwardSpeed = MR::getPlayerVelocity()->dot(MR::getRailDirection(this));
    mForwardSpeed = MR::clamp(mForwardSpeed, 20.0f, 45.0f);
    _EC = 0.0f;
    _F4 = 0.0f;
    _FC = 0.0f;
    _19C = *_1A4;
    updateMarioPos();
    return true;
}

bool TubeSlider::tryEndSlide() {
    if (!MR::isRailReachedGoal(this))
        return false;

    MR::moveCoordToEndPos(this);
    TVec3f vec;
    mRiderMtx.getTrans(vec);
    MR::setPlayerPos(vec);
    TVec3f vec2;
    mRiderMtx.getZDir(vec2);
    MR::setPlayerFrontTargetVec(vec2, 1);
    TVec3f vec3(vec2);
    vec3.scale(20.0f);
    vec3.add(TVec3f(0.0f, 1.0f, 0.0f) * 10.0f);
    MR::startBckPlayerJ("チューブスライダージャンプ");
    MR::stopPlayerBlink();
    MR::endActorCameraProgrammable(this, -1, true);
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, vec3);
    MR::invalidateShadow(this, nullptr);
    if (MR::isValidSwitchA(this))
        MR::offSwitchA(this);
    MR::tryDeleteEffect(mRider, "共通スリップ坂");
    mRider = nullptr;
    MR::resetSupportTicoAttachRangeTubeSlider();
    setNerve(GET_NERVE(TubeSlider, TubeSliderNrvEnd));
    return true;
}

bool TubeSlider::tryPlayerJump(bool b) {
    if ((!b && MR::testCorePadTriggerA(0)) || (b && _118 > 0)) {
        if (TubeSliderFunction::isGroundIce(this))
            MR::startLevelSoundPlayer("JumpSkate", -1, -1);
        else
            MR::startLevelSoundPlayer("JumpS", -1, -1);
        MR::tryDeleteEffect(mRider, "共通スリップ坂");
        startJump(25.0f);
        MR::startBckPlayerJ("チューブスライダージャンプ");
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideJump));
        return true;
    }

    return false;
}

bool TubeSlider::tryPlayerJumpSpin() {
    if (MR::isPadSwing(0)) {
        MR::startLevelSoundPlayer("JumpSpin", -1, -1);
        MR::tryRumblePadMiddle(this, 0);
        MR::vecKillElement(mJumpVelocity, _A4, &mJumpVelocity);
        mJumpVelocity.add(_A4 * 22.0f);
        MR::startBckPlayer("Spin", nullptr);
        MR::emitEffect(mRider, "SliderSpinLight");
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideSpin));
        return true;
    }
    return false;
}

bool TubeSlider::tryPlayerFall() {
    if (TubeSliderFunction::isUnknown2(this)) {
        MR::tryDeleteEffect(mRider, "共通スリップ坂");
        MR::startBckPlayerJ("落下");
        startJump(0.0f);
        setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideFall));
        return true;
    }
    return false;
}

void TubeSlider::damagePlayer() {
    MR::startPlayerBlink();
    MR::decrementPlayerSwimLife();
    MR::tryRumblePadMiddle(this, 0);
    MR::startLevelSoundPlayer("DamageS", -1, -1);
    MR::startBckPlayerJ("チューブスライダーダメージ");
    MR::emitEffect(mRider, "TubeSlideDamage");
    MR::tryDeleteEffect(mRider, "共通スリップ坂");
    startJump(25.0f);
    setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamage));
}

void TubeSlider::startJump(f32 power) {
    TVec3f a(1.0f, 0.0f, 0.0f);
    TVec3f b(_98);
    MR::makeAxisUpFront(&a, &b, _A4, b);
    _100.set(*MR::getPlayerPos());
    mJumpVelocity.set(_98 * mForwardSpeed + _A4 * power - a * _EC * 2.0f);
}

// NONMATCHING: fmuls operand order on int-to-float conversions
bool TubeSlider::updateOnGround() {
    if (updatePlayer())
        return true;

    if (tryPlayerFall())
        return true;

    if (TubeSliderFunction::isGroundDamage(this) && !isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand))) {
        damagePlayer();
        return true;
    }

    if (tryPlayerJump(false))
        return true;

    s32 curveScale = 300;
    s32 volScale = 10;
    s32 pitchScale = 23;
    s32 curveVol = MR::clamp((s32)(__fabsf(MR::getSubPadStickX(0)) * (f32)curveScale) + 60, 0, 1000);
    s32 slipPitch = (s32)(mForwardSpeed * (f32)pitchScale);
    s32 slipVol = MR::clamp((s32)(mForwardSpeed * (f32)volScale) + 600, 0, 1000);
    if (curveVol > 170)
        slipPitch -= (s32)(0.4f * (f32)curveVol);

    if (!isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRide)) && !isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideLand))) {
        slipPitch = (s32)(0.4f * (f32)slipPitch);
        curveVol = (s32)(0.4f * (f32)slipPitch);
    }

    if (curveVol > 170)
        MR::startActionSound(this, "SlipCurve", curveVol, -1, -1);

    if (TubeSliderFunction::isGroundWood(this)) {
        MR::startActionSound(this, "SlipWood", slipPitch, slipVol, -1);
    } else if (TubeSliderFunction::isGroundIce(this)) {
        MR::startActionSound(this, "SlipIce", slipPitch, slipVol, -1);
    } else {
        if (!TubeSliderFunction::isGroundNormal(this))
            TubeSliderFunction::isGroundNormal2(this);
        MR::startActionSound(this, "SlipNormal", slipPitch, slipVol, -1);
    }
    return false;
}

// NONMATCHING: stack temp slot order, f-reg allocation in wind block and wall-hit frsp ordering
bool TubeSlider::updatePlayer() {
    TVec3f railDir(MR::getRailDirection(this));
    mForwardSpeed += 1.0f;
    if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRide))) {
        if (TubeSliderFunction::isGroundSlow(this)) {
            mForwardSpeed *= 0.95f;
        } else if (TubeSliderFunction::isGroundIce(this)) {
            mForwardSpeed *= 0.995f;
        } else {
            mForwardSpeed *= 0.99f;
        }
    } else if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideDamageLand))) {
        mForwardSpeed *= 0.95f;
    } else {
        mForwardSpeed *= one;
    }
    mForwardSpeed = MR::clamp(mForwardSpeed, 20.0f, 45.0f);
    MR::moveCoordAndFollowTrans(this, mForwardSpeed);
    TubeSliderFunction::updateTubeSliderInfo(this, true);
    TVec3f cross(1.0f, 0.0f, 0.0f);
    PSVECCrossProduct(railDir, _A4, &cross);
    MR::normalizeOrZero(&cross);
    _EC -= cross.dot(MR::getRailDirection(this)) * 7.0f;
    TVec3f vec(0.0f);
    mRiderMtx.getTrans(vec);
    TVec3f vec2(0.0f);
    TVec3f vec3(_A4);
    if (!TubeSliderFunction::isUnknown2(this)) {
        TVec3f upVec(0.0f, 1.0f, 0.0f);
        TubeSliderFunction::getRailUpVec(&upVec, this);
        TPos3f mtx;
        mtx.identity();
        f32 rot = 0.017453292f * (180.0f + mPlayerTheta);
        mtx.setRotate(-MR::getRailDirection(this), rot);
        mtx.mult(upVec, upVec);
        MR::turnVecToVecDegree(&vec3, vec3, upVec, 10.0f, TVec3f((int)0, (int)1, (int)0));
    }
    f32 dist = TubeSliderFunction::getRadius(this) - _F4 - _F8;
    vec2.set(mPosition - vec3.scaleInline(dist));
    TVec3f diff(vec2);
    diff.sub(vec);
    _11C += mForwardSpeed - diff.dot(railDir);
    MR::turnVecToVecDegree(&_98, _98, MR::getRailDirection(this), 1.0f, TVec3f((int)0, (int)1, (int)0));
    if (!TubeSliderFunction::isUnknown2(this) && MR::getSubPadStickX(0) != 0.0f) {
        _EC += 0.215f * MR::getSubPadStickX(0);
        f32 range = 1.45f;
        _EC = MR::clamp(_EC, -range, range);
        f32 stick = MR::getSubPadStickX(0);
        if (stick < 0.0f) {
            _F0 = MR::getInterpolateValue(0.3f, _F0, MR::clamp(stick, -1.0f, 0.0f));
        } else if (stick > 0.0f) {
            _F0 = MR::getInterpolateValue(0.3f, _F0, MR::clamp(stick, 0.0f, 1.0f));
        }
    } else {
        _EC *= 0.95f;
        _F0 = MR::getInterpolateValue(0.3f, _F0, 0.0f);
    }
    mPlayerTheta += _EC;

    if (isNerve(GET_NERVE(TubeSlider, TubeSliderNrvRide))) {
        f32 blend = MR::clamp(_F0 * one, -1.0f, 1.0f);
        if (blend > 0.0f) {
            MR::setBckBlendWeight(1.0f - blend, 0.0f, blend);
        } else {
            MR::setBckBlendWeight(1.0f + blend, -blend, 0.0f);
        }
    }

    bool hasHitWall = false;
    if (_EC <= 0.0f && TubeSliderFunction::isHitLeftWall(this, &mPlayerTheta)) {
        _EC = 2.0f;
        mPlayerTheta += _EC;
        MR::repeatDegree(&mPlayerTheta);
        mForwardSpeed *= 0.2f;
        MR::tryRumblePadMiddle(this, 0);
        if (_F4 <= 0.0f) {
            hasHitWall = true;
            MR::emitEffect(mRider, "SliderWallHitL");
            setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideWallHit));
        }
    }
    if (_EC >= 0.0f && TubeSliderFunction::isHitRightWall(this, &mPlayerTheta)) {
        f32 lim = 2.0f;
        mPlayerTheta += (f32)(-(f64)lim);
        _EC = -lim;
        MR::repeatDegree(&mPlayerTheta);
        mForwardSpeed *= 0.2f;
        MR::tryRumblePadMiddle(this, 0);
        if (_F4 <= 0.0f) {
            hasHitWall = true;
            MR::emitEffect(mRider, "SliderWallHitR");
            setNerve(GET_NERVE(TubeSlider, TubeSliderNrvRideWallHit));
        }
    }

    updateMarioPos();

    {
        s32 pitchBase = 50;
        s32 scale = 10;
        f32 spd = mForwardSpeed;
        s32 windPitch = (s32)((f32)pitchBase + __fabsf(spd) * (f32)scale);
        s32 windVol = MR::clamp((s32)(spd * (f32)scale) + 600, 0, 1000);
        MR::startActionSound(this, "Wind", windPitch, windVol, -1);
    }

    if (hasHitWall)
        return true;

    if (tryEndSlide()) {
        if (!_1A8) {
            if (MR::isEqualStageName("BigTree2Galaxy")) {
                if (MR::getCurrentScenarioNo() == 3) {
                    if (MR::isPurpleCoinNum100()) {
                        MR::startLevelSoundPlayer("JumpObj", -1, -1);
                        MR::startSubBGM("BGM_RACE_WIN", false);
                    } else {
                        MR::startSubBGM("BGM_RACE_LOSE", false);
                    }
                } else {
                    MR::startLevelSoundPlayer("JumpObj", -1, -1);
                    MR::startSubBGM("BGM_RACE_WIN", false);
                }
            }
            _1A8 = true;
        }
        return true;
    }

    return false;
}

void TubeSlider::updateMarioPos() {
    MR::repeatDegree(&mPlayerTheta);
    TPos3f pos;
    pos.identity();
    f32 rot = 0.017453292f * (180.0f + mPlayerTheta);
    pos.setRotate(-MR::getRailDirection(this), rot);

    if (!TubeSliderFunction::isUnknown2(this)) {
        TVec3f railup(0.0f, 1.0f, 0.0f);
        TubeSliderFunction::getRailUpVec(&railup, this);
        pos.mult(railup, railup);
        MR::turnVecToVecDegree(&_A4, _A4, railup, 10.0f, TVec3f((int)0, (int)1, (int)0));
    }
    TVec3f translation(0.0f);
    mRiderMtx.getTrans(translation);
    TVec3f front(1.0f, 0.0f, 0.0f);
    TVec3f up(_98);
    MR::makeAxisUpFront(&front, &up, _A4, up);
    mRiderMtx.setXYZDir(front, _A4, up);
    _F8 = MR::lerp(_F8, sInitialRiderOffset, 0.5f);
    f32 dist = TubeSliderFunction::getRadius(this) - _F4 - _F8;
    TVec3f p = mPosition - _A4.scaleInline(dist);
    mRiderMtx.setTrans(p);
    updateCameraAndShadow(MR::getRailDirection(this));
}

void TubeSlider::updateCameraAndShadow(const TVec3f& rDir) {
    mShadowDropDir.set(-_A4);
    mRiderMtx.getTrans(mShadowDropPos);
    mShadowDropPos.add(_A4 * 10.0f);
    MR::turnVecToVecDegree(&_184, _184, rDir, 0.3f, TVec3f((int)0, (int)1, (int)0));
    MR::vecBlend(&_190, _190, _A4, 0.03f);
    _19C = MR::lerp(_19C, *_1A4, 0.03f);
    _178 = *MR::getPlayerPos() + (_190 * _19C);
    _16C = _178 - _184 * 550.0f + _190 * (*_1A0);
    f32 railCoord = MR::getRailCoord(this);
    MR::moveCoordAndTransToNearestRailPos(this, _16C);
    f32 rad = TubeSliderFunction::getRadius(this) - 100.0f;
    if (PSVECDistance(mPosition, _16C) > rad) {
        _16C.sub(mPosition);
        MR::normalize(&_16C);
        _16C.scale(rad);
        _16C.add(mPosition);
    }
    TubeSliderFunction::updateTubeSliderInfo(this, true);
    MR::setRailCoord(this, railCoord);
    TubeSliderFunction::updateTubeSliderInfo(this, true);
    MR::setProgrammableCameraParam(this, _178, _16C, _190);
}

namespace {
    const f32 sCeilingReflectRate = 0.6f;
    const f32 sWallReflectRate = 0.2f;
    extern const volatile f32 sInitialRiderOffset = 0.0f;
}
