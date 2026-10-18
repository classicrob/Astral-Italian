#include "Game/Player/MarioSwim.hpp"
#include "Game/Util.hpp"
#include "Game/AreaObj/WaterArea.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/DashRing.hpp"
#include "Game/Player/MarineSnow.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

MarioSwimCollisionNode::MarioSwimCollisionNode() {
    (void)100.0f;
    (void)50.0f;
    (void)0.52359879f;
    (void)-200.0f;
    (void)0.100000001f;
    (void)0.200000003f;
    (void)10.0f;
    (void)1.74532938f;
    (void)-10.0f;
    (void)1.49599659f;
    (void)200.0f;
    (void)8.0f;
    (void)2.09439516f;
    (void)40.0f;
    (void)0.00999999978f;
    (void)0.0299999993f;
    (void)6.28318548f;
    (void)-50.0f;
    (void)0.196349546f;
    (void)30.0f;
    (void)-30.0f;
    (void)-0.100000001f;
    (void)60.0f;
    (void)1000.0f;
    (void)0.899999976f;
    (void)400.0f;
    (void)0.707000017f;
    (void)2.5f;
    (void)-400.0f;
    (void)120.0f;
    (void)0.125f;
    (void)300.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)0.150000006f;
    (void)0.87266469f;
    (void)2.26892805f;
    (void)12.0f;
    (void)25.0f;
    (void)0.25f;
    (void)0.980000019f;
    (void)0.00100000005f;
    (void)1.04719758f;
    (void)2000.0f;
    (void)-25.0f;
    (void)0.949999988f;
    (void)0.75f;
    (void)0.300000012f;
    (void)2.61799383f;
    (void)0.99000001f;
    (void)0.995000005f;
    (void)0.959999979f;
    (void)1.65346992f;
    (void)15.0f;
    (void)110.0f;
    (void)80.0f;
    (void)-500.0f;
    (void)210.0f;
    (void)170.0f;
    (void)10000.0f;
    (void)0.970000029f;
    (void)0.800000012f;
    (void)500.0f;
    (void)1.01999998f;
    (void)1.5f;
    (void)6.0f;
    (void)0.699999988f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)2.61799407f;
    (void)16.0f;
    (void)-0.5f;
    (void)18.0f;
    (void)1.25f;
    (void)1.00010002f;
}

static const f32 cAnimSpeeds[] = {1.0f, 1.0f, 1.0f, 1.0f};
namespace {
    static u32 sWaterSearchCounter;
    static u32 sSpinCooldownTime = 45;
    static u32 sDiveSideTimer = 20;
    static f32 cFrontAcc[] = {-0.2f, -0.2f, -0.2f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.0f, -0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f,  0.0f,  0.0f,  0.2f,  0.2f,  0.2f,  0.3f,  0.3f,  0.4f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
                              0.4f,  0.4f,  0.3f,  0.3f,  0.2f,  0.2f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f, 0.0f};

    static f32 cFrontAccSpin[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f,  6.0f,  4.0f,  2.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

    static f32 cFrontAccSpinSurface[] = {0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  8.0f,  6.0f,  4.0f,  2.0f,  1.0f,
                                         -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    static f32 cWeightTable[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    static f32 cSwimWeightTable[] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    static f32 cWeightTableSP[] = {0.0f, 0.0f, 0.75f, 0.25f};
    static f32 cLimitAngleSink = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    static f32 cNeutralAngleWait = (JGeometry::TUtil< f32 >::PI() / 2.0f) - (JGeometry::TUtil< f32 >::PI() / 6.0f);
    static f32 cLimitAngleWait = JGeometry::TUtil< f32 >::PI() / 1.5f;
    static f32 cUpperAngleWait = JGeometry::TUtil< f32 >::PI() / 100.0f;
    static f32 cWaterSurfaceRange = 160.0f;
    static f32 cTurnMotionSpeed = 5.0f;
};  // namespace

f32 getSwimValue(f32 stick, u32 index, const MarioConstTable* pTable) {
    if (index == 2) {
        return pTable->mSwimRotXIne;
    }

    if (index == 3) {
        return pTable->mSwimRotZIne;
    }

    return 0.0f;
}

void MarioSwim::setDamage(const TVec3f& rKnockbackVec, u16 damage) {
    mDamage->mVelocity.set(rKnockbackVec);
    mDamage->mTimer = damage;
    mStickInputYInertia = 0.0f;
    mStickInputXInertia = 0.0f;
    mForwardSpeed = 0.0f;
    mSpin->reset();
    mJumpDiveTimer = 0;
    mSinkTimer = 0;
    mZSinkTimer = 0;
    mActionLockTimer = damage;
    mSwimYAccTimer = 0;
    mStateTimer = 0x12C;
    if (mIsOnSurface) {
        mIsOnSurface = false;
    }
}

MarioSwim::MarioSwim(MarioActor* pActor) : MarioState(pActor, MarioStatus_Swim), mWaterInfo() {
    sWaterSearchCounter = 0;
    _18 = 0;
    mIsOnSurface = false;
    mEnteredWater = false;
    _1B = false;
    _1C = false;
    _1D = false;
    _1E = 0;
    _1F = false;
    mIsSwimmingAtSurface = false;
    mIsSinking = false;
    _22 = false;
    mStateTimer = 0;
    mSurfacePaddleHoldTimer = 0;
    mActionLockTimer = 0;
    mSinkTimer = 0;
    mSwimYAccTimer = 0;
    mZSinkTimer = 0;
    mJumpDiveTimer = 0;
    mDashTimer = 0;
    _38 = 0;
    mBuoyancyDelayTimer = 0;
    mIdleWaitTimer = 0;
    mWaveFlowTimer = 0;
    _40 = 0;
    mFloorContactTimer = 0;
    _44 = 0;
    mSurfaceOffset = 0.0f;
    mStickInputYInertia = 0.0f;
    mStickInputXInertia = 0.0f;
    mForwardSpeed = 0.0f;
    mBuoyancySpeed = 0.0f;
    mVerticalAngle = 0.0f;

    mFrontVec.x = 1.0f;
    mFrontVec.y = 0.0f;
    mFrontVec.z = 0.0f;

    mUpVec.x = 0.0f;
    mUpVec.y = 1.0f;
    mUpVec.z = 0.0f;

    mPoseVec.zero();
    mSpin = new MarioSwimSpin(this);
    mJet = new MarioSwimJet(this);
    mRingDash = new MarioSwimRingDash(this);
    _9C = false;
    _9D = 0;
    mNextAction = EXIT_ACTION_NONE;
    mEffectState = 0xFF;
    _9CFlag = 0;
    mDamage = new MarioSwimDamage(this);
    mTilt = new MarioSwimTilt(this);
    mDrownTimer = 0;
    mOxygenDecreaseDelay = 120;
    mColdWaterDamageInterval = 0;
    mSwimState = SWIM_STATE_NONE;
    _148.zero();
    mAreaFollowMtxPos.zero();
    _118 = nullptr;
    mSurfacePos.set(0.0f, 1.0f, 0.0f);
    mSurfaceNorm.zero();
    _178.zero();
    mWaterStreamVel.zero();
    mWaterStreamDir.zero();
    mWaterDepth = 0.0f;
    _1A0 = 0.0f;
    mDistToFloor = 0.0f;
    _1A8 = 0.0f;
    mDistanceToWaterSurface = 0.0f;
    mWallStickTimer = 0;
    _1B2 = 0;
    mWaterDistanceTarget = 0.0f;
    _1B8 = 0.0f;
    mMarineSnow = new MarineSnow();
    u32 rows = 5;
    u32 columns = 4;
    f32 columnStep = 80.0f / static_cast<f32>(columns - 1);
    f32 rowStep = 120.0f / static_cast<f32>(rows - 1);
    mCollisionNodeCount = rows * columns;
    mCollisionNodes = new MarioSwimCollisionNode[mCollisionNodeCount];
    _180 = 0;
    mPreviousPos.zero();
    mCollisionNodeCenter.zero();
    _1F0.zero();
    _1FC = 0.0f;
    _200 = 0.0f;
    _204 = 0;
    MarioSwimCollisionNode* node;
    u32 index = 0;
    for (u32 row = 0; row < rows; row++) {
        for (u32 column = 0; column < columns; column++, index++) {
            node = &mCollisionNodes[index];
            node->mOffset.x = 0.0f;
            node->mOffset.y = -40.8f + (rowStep * row);
            node->mOffset.z = -40.0f + (columnStep * column);
            node->mRadius = node->mOffset.length();
            node->_10 = 0;
            node->_14.zero();
            node->mWaterArea = nullptr;
            node->_24.zero();
            node->_30 = 0;
            mCollisionNodeCenter.add(mCollisionNodes[index].mOffset);
        }
    }
    mCollisionNodeCenter.scale(1.0f / mCollisionNodeCount);
}

void MarioSwim::init() {
    mOxygen = mActor->getConst().getTable()->mOxygenMax;
}

bool MarioSwim::start() {
    fn_80401160();
    fn_80401270();
    fn_804013C0();
    _1B2 = 0;
    mWaterDistanceTarget = 0.0f;
    fn_804013E0();
    _9C = !checkLvlA();
    fn_803FB300();
    fn_80401440();
    mUpVec = -getPlayer()->getAirGravityVec();
    fn_803FA820();
    stopAnimationUpper(nullptr, nullptr);
    changeAnimation(nullptr, "水泳基本");
    changeAnimationNonStop("水泳基本");
    fn_80401510();
    fn_804015C0();
    fn_80401640();
    fn_80401010();
    mNextAction = EXIT_ACTION_NONE;
    getPlayer()->_10.digitalJump = false;
    if (isPlayerModeYoshi()) {
        getPlayer()->_10._2A = false;
        mActor->fn_803B9A30();
        mActor->fn_803B9830()->fn_8041BBA0();
        getPlayer()->mMovementStates._13 = false;
    }
    return true;
}

f32 MarioSwim::getSurface() const {
    f32 fr1f = getTrans().y - mWaterDepth;
    f32 fr1e = 40.0f;
    f32 wave;
    f32 fr1d = 0.01f;
    f32 fr1c = 0.03f;
    u16 tmp = 1800;
    wave = 1.0f + fr1d * MR::sin((mStateTimer * JMath::TAngleConstant_< f32 >::RADIAN_DEG360() / tmp));
    tmp = 500;
    f32 ftmp = fr1c * MR::sin((mStateTimer * JMath::TAngleConstant_< f32 >::RADIAN_DEG360() / tmp));
    tmp = 180;

    return fr1f + wave * (fr1e * (MR::sin(ftmp + (mStateTimer * JMath::TAngleConstant_< f32 >::RADIAN_DEG360() / tmp)) - 1.0f));
}

bool MarioSwim::update() {
    mStateTimer++;
    if (getAnimator()->isAnimationRunSimple() && MR::isEqualString(getAnimator()->fn_803DF0D0(), "SpinDriverWait"))
        stopAnimation(nullptr, static_cast<const char*>(nullptr));
    mUpVec = -getPlayer()->getAirGravityVec();
    if (!fn_803F7D90())
        return false;
    updateLifeByTime();
    fn_803F8430();
    mUpVec = -getPlayer()->getAirGravityVec();
    if (mWallStickTimer)
        mWallStickTimer--;
    if (!fn_80401690())
        return false;
    fn_803F8DA0();
    bool turn = fn_803F8E10();
    fn_80401710();
    spin();
    MR::vecKillElement(mFrontVec, mUpVec, &mFrontVec);
    MR::normalizeOrZero(&mFrontVec);
    fn_804017C0(turn);
    Mario* mario = getPlayer();
    fn_803F9AC0(&mPoseVec, getFrontVec(), mario->mSideVec);
    fn_803F9B90();
    fn_803F9D50();
    if (mJumpDiveTimer)
        mJumpDiveTimer--;
    fn_803F9DF0();
    fn_803F9E90();
    fn_803F9FC0();
    fn_803FA020();
    decideAnimation();
    decideEffect(false);
    if (mActionLockTimer)
        mActionLockTimer--;
    if (mSwimYAccTimer)
        mSwimYAccTimer--;
    if (mFloorContactTimer)
        mFloorContactTimer--;
    mTilt->update();
    jet();
    pushedByWaterWall();
    return true;
}

void MarioSwim::procBuoyancy() {
    if (mNextAction != EXIT_ACTION_NONE) {
        return;
    }

    if (mDamage->mTimer != 0) {
        mBuoyancySpeed = 0.0f;
    } else {
        mIsSinking = false;
        if (mEnteredWater) {
            mEnteredWater = false;
            mBuoyancyDelayTimer = 120;
        }

        f32 waterDepthNeg = -mWaterDepth;
        f32 buoyancyScale = 1.0f;
        f32 accelerationScale = 1.0f;
        const MarioConstTable* table = mActor->getConst().getTable();
        bool isBelowStrongSwimDepth = table->mSwimUpStrongDist > waterDepthNeg;

        f32 swimUpWeakDist = table->mSwimUpWeakDist;
        f32 swimUpBottomDist = table->mSwimUpBottomDist;
        f32 floorDepthSum = waterDepthNeg + mDistToFloor;

        if (floorDepthSum < 2000.0f) {
            f32 ratio = floorDepthSum / 2000.0f;
            swimUpWeakDist = table->mSwimUpWeakDist * ratio;
            swimUpBottomDist = table->mSwimUpBottomDist * ratio;
        }

        bool shouldSink = (swimUpWeakDist > mDistToFloor);
        if (mForwardSpeed > 2.0f && mVerticalAngle > PI / 2) {
            shouldSink = false;
            mIsSinking = false;
        }

        if (!(mDistToFloor < 300.0f) || !(-mWaterDepth < 100.0f)) {
            if (shouldSink) {
                if (mStateTimer > 300) {
                    f32 distRatio = mDistToFloor / swimUpWeakDist;
                    if (distRatio < 0.0f) {
                        distRatio = 0.0f;
                    }

                    if (swimUpBottomDist > mDistToFloor) {
                        distRatio = 0.0f;
                    }

                    buoyancyScale =
                        mActor->getConst().getTable()->mSwimUpWeakRatio + (distRatio * (1.0f - mActor->getConst().getTable()->mSwimUpWeakRatio));
                } else {
                    buoyancyScale = 1.0f + (1.0f - (static_cast< f32 >(mStateTimer) / 300.0f));
                }

                mIsSinking = true;
            } else if (isBelowStrongSwimDepth) {
                const MarioConst& marioConst = mActor->getConst();
                MarioConstTable* table = mActor->getConst().getTable();

                if (table->mSwimUpSurfaceDist > waterDepthNeg) {
                    buoyancyScale = table->mSwimUpStrongRatio;
                } else {
                    f32 strongRatio = (waterDepthNeg - table->mSwimUpSurfaceDist) / (table->mSwimUpStrongDist - table->mSwimUpSurfaceDist);
                    if (strongRatio < 0.0f) {
                        strongRatio = 0.0f;
                    }

                    buoyancyScale = strongRatio + ((1.0f - strongRatio) * marioConst.getTable()->mSwimUpStrongRatio);
                }
            }
        }

        if (mBuoyancyDelayTimer != 0) {
            mBuoyancyDelayTimer--;
        } else {
            f32 ratio = 1.0f;
            mBuoyancySpeed += (accelerationScale * (ratio * mActor->getConst().getTable()->mSwimUpAcc));
        }

        if (mBuoyancySpeed > mActor->getConst().getTable()->mSwimUpMaxSpeed) {
            mBuoyancySpeed = mActor->getConst().getTable()->mSwimUpMaxSpeed;
        }

        if (!MR::isNearZero(mWaterStreamVel)) {
            buoyancyScale = 1.0f;
        }

        if (mIsSinking) {
            if (mDistToFloor < mActor->getConst().getTable()->mSwimUpBottomDist) {
                f32 clampRatio = MR::clamp(1.0f - (static_cast< f32 >(mActionLockTimer) / 50.0f), 0.0f, 1.0f);
                f32 sinkFactor = ((1.0f - clampRatio) + (clampRatio * mActor->getConst().getTable()->mSwimReverseSinkRatio));
                sinkFactor *= 0.1f;

                if (!MR::isNearZero(mWaterStreamVel)) {
                    sinkFactor = 1.0f;
                }

                addVelocity(-mUpVec, (sinkFactor * (mBuoyancySpeed * buoyancyScale)));
            } else {
                addVelocity(-mUpVec, mBuoyancySpeed * buoyancyScale);
            }
        } else {
            addVelocity(mUpVec, mBuoyancySpeed * buoyancyScale);
        }
    }
}

void MarioSwim::flowOnWave(f32 unused) {
    if (mActionLockTimer != 0) {
        return;
    }

    if (mWaterDepth < -::cWaterSurfaceRange) {
        if (mIsOnSurface) {
            mIsOnSurface = false;
        }

        return;
    } else if (mIsOnSurface) {
        if (mWaterDepth < -25.0f) {
            addVelocity(mSurfacePos, mBuoyancySpeed);
        }

        if (mWaterDepth > 0.0f) {
            TVec3f inverseFlow = -mSurfacePos;
            addVelocity(inverseFlow, 5.0f + mWaterDepth);
        }
    }

    mWaveFlowTimer++;
}

void MarioSwim::decideVelocity() {
    if (mRingDash->mTimer != 0) {
        return;
    }

    if (MarioModule::checkLvlZ() || (MarioModule::checkLvlA() && _9C)) {
        mIsSwimmingAtSurface = false;
    }

    if (mWaterDepth < -400.0f && mSinkTimer == 0) {
        mIsSwimmingAtSurface = false;
    }

    if ((mIsOnSurface && (MarioModule::isStickOn() || mActionLockTimer != 0)) || (mIsSwimmingAtSurface && MarioModule::isStickOn())) {
        if (mActionLockTimer == 0 && mJet->mJetTimer == 0) {
            f32 angleDiff = MR::diffAngleAbs(mFrontVec, getWorldPadDir());
            f32 angleFactor = (PI - angleDiff) / PI;
            angleFactor *= MarioModule::getStickP();
            const MarioConst& marioConst = mActor->getConst();
            MarioConstTable* table = mActor->getConst().getTable();
            f32 inertia = table->mSwimFrontIne;
            if (mSpin->mSpinDashTimer == 0 && mForwardSpeed >= (angleFactor * table->mSwimFrontOnWaterSpeed)) {
                inertia = 0.95f;
            }

            mForwardSpeed = mForwardSpeed * inertia + (1.0f - inertia) * (angleFactor * marioConst.getTable()->mSwimFrontOnWaterSpeed);
        }
    } else if (mIsOnSurface && mDashTimer != 0) {
        f32 inertia = mActor->getConst().getTable()->mSwimFrontIne;
        mForwardSpeed = (mForwardSpeed * inertia) + ((1.0f - inertia) * mActor->getConst().getTable()->mSwimFrontOnWaterSpeed);
    } else {
        if (!mIsOnSurface && checkTrgA() && mDamage->mTimer == 0 && !check7Aand7C()) {
            f32 stickX = MR::abs(MarioModule::getStickX());
            f32 stickFactor = 1.0f - (0.3f * stickX);
            f32 threshold = 0.75f * stickFactor;

            if (mVerticalAngle < PI / 2 && getStickY() > threshold && mActionLockTimer == 0 && _1E != 0) {
                f32 stickY = getStickY();
                f32 normalizedY = (stickY - threshold) / (1.0f - threshold);
                mVerticalAngle =
                    ((1.0f - normalizedY) * (JGeometry::TUtil< f32 >::PI() * 2.0f / 3.0f)) + (normalizedY * (JGeometry::TUtil< f32 >::PI() / 1.2f));
            } else if (mVerticalAngle < PI / 2 && MR::abs(getStickY()) < 0.1f && mActionLockTimer == 0 && _1E != 0) {
                mVerticalAngle = PI / 2;
            }

            _1E = 0;

            if (!MarioModule::isAnimationTerminate("水泳一掻き")) {
                if (_1D) {
                    return;
                }

                const MarioConst& marioConst = mActor->getConst();
                f32 frame = getAnimator()->getFrame();
                MarioConstTable* table = marioConst.getTable();
                if (!MR::isInRange(frame, static_cast< f32 >(table->mSwimConnectIn), static_cast< f32 >(table->mSwimConnectOut))) {
                    _1D = true;
                    return;
                }
            }

            _1D = false;
            mActionLockTimer = 60;
            mSwimYAccTimer = 50;
            if (!MarioModule::isAnimationRun("水泳潜り") && !MarioModule::isAnimationRun("水泳ジャンプダイブ回転")) {
                getAnimator()->forceSetBlendWeight(::cWeightTableSP);
                stopAnimation(nullptr);
                changeAnimationNonStop("水泳一掻き");
                playSound("水中一掻き");
                mDashTimer = 40;
            }
        } else {
            if (!mIsOnSurface && MarioModule::checkLvlA()) {
                mForwardSpeed = (mForwardSpeed * mActor->getConst().getTable()->mSwimFrontIne) +
                                (mActor->getConst().getTable()->mSwimFrontNormalSpeed * (1.0f - mActor->getConst().getTable()->mSwimFrontIne));
                mBuoyancySpeed = 0.0f;
            } else {
                if (mIsOnSurface) {
                    mForwardSpeed *= mActor->getConst().getTable()->mSwimStopIneSurface;
                } else {
                    mForwardSpeed *= mActor->getConst().getTable()->mSwimStopIne;
                }

                if (MarioModule::isAnimationRun("水泳上昇呼吸")) {
                    mForwardSpeed *= 0.95f;
                }

                if (mActor->getConst().getTable()->mSwimUpBottomDist > mDistToFloor) {
                    mForwardSpeed *= 0.99f;
                } else if (mActor->getConst().getTable()->mSwimUpWeakDist > mDistToFloor) {
                    mForwardSpeed *= 0.995f;
                }
            }
        }
    }

    if (mDashTimer != 0) {
        bool isSpinning = (mSpin->mSpinTimer != 0 || mSpin->mSpinDashTimer != 0);

        if (isSpinning) {
            if (mIsOnSurface) {
                if (mDashTimer < 20) {
                    f32 acc = ::cFrontAccSpinSurface[20 - mDashTimer];

                    if (acc < 0.0f) {
                        mForwardSpeed += acc * mActor->getConst().getTable()->mSwimSpinSurfaceAccRatioR;
                    } else {
                        mForwardSpeed += acc * mActor->getConst().getTable()->mSwimSpinSurfaceAccRatio;
                    }

                    playSound("水面スピン");
                } else {
                    mForwardSpeed *= 0.96f;
                }
            } else {
                if (mDashTimer < 30) {
                    f32 acc = ::cFrontAccSpin[30 - mDashTimer];
                    if (acc < 0.0f) {
                        mForwardSpeed += acc * mActor->getConst().getTable()->mSwimSpinAccRatioR;
                    } else {
                        mForwardSpeed += acc * mActor->getConst().getTable()->mSwimSpinAccRatio;
                    }

                    playSound("水中スピン");
                } else {
                    mForwardSpeed *= 0.96f;
                }
            }
        } else if (mDashTimer < 40) {
            f32 minRatio;
            f32 speedRatio;
            f32 speed;
            f32 acc;
            acc = ::cFrontAcc[40 - mDashTimer];
            const MarioConstTable* table = mActor->getConst().getTable();
            speed = mForwardSpeed;
            speedRatio = speed / table->mSwimFrontMaxSpeed;
            minRatio = table->mSwimAccMinRatio;
            mForwardSpeed += (minRatio + speedRatio * (1.0f - minRatio)) * (acc * table->mSwimAccRatio);
        }

        mDashTimer--;
        bool canClamp = check7Aand7C();

        if (canClamp) {
            if (mForwardSpeed > mActor->getConst().getTable()->mSwimFrontSpinSpeed) {
                mForwardSpeed = mActor->getConst().getTable()->mSwimFrontSpinSpeed;
            }
        } else {
            if (mForwardSpeed > mActor->getConst().getTable()->mSwimFrontMaxSpeed) {
                mForwardSpeed = mActor->getConst().getTable()->mSwimFrontMaxSpeed;
            }
        }
    }

    if (mZSinkTimer != 0) {
        mForwardSpeed *= 0.95f;
    }
}

void MarioSwim::decideAnimation() {
    f32 animSpeeds[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    animSpeeds[1] = mActor->mConst->getTable()->mSwimSurfaceSpeed;
    animSpeeds[2] = mActor->getConst().getTable()->mSwimWaterSpeed;
    animSpeeds[3] = mActor->getConst().getTable()->mSwimDriftSpeed;

    if (mJet->mJetTimer != 0) {
        return;
    }

    if (isAnimationRun("水泳水面初期移動")) {
        if (getStickP() < 0.1f) {
            stopAnimationUpper(nullptr);
        }
    }

    if (isAnimationRun(nullptr) && !isAnimationRun("水泳上昇呼吸")) {
        if (MR::abs(getStickX()) < 0.1f) {
            stopAnimation("水泳ターン左");
            stopAnimation("水泳ターン右");
        }

        return;
    }

    if (mActionLockTimer != 0) {
        getAnimator()->setBlendWeight(&::cWeightTable[4], mActor->getConst().getTable()->mWeightBlendRatioSwim);
        return;
    }

    u32 animIndex = 0;

    if (!mIsOnSurface) {
        if (checkLvlA()) {
            playSound("水中バタ足");
            animIndex = 2;
        } else {
            animIndex = 3;
            if (mIsSwimmingAtSurface != false) {
                if (getStickP() > 0.1f) {
                    animIndex = 1;
                }
            } else if (MR::abs(getStickX()) > 0.1f && mVerticalAngle < 1.6534699f && mForwardSpeed < ::cTurnMotionSpeed) {
                if (getStickX() < 0.0f) {
                    changeAnimation("水泳ターン左");
                } else {
                    changeAnimation("水泳ターン右");
                }

                return;
            }
        }
    } else {
        // Surface logic
        if (getStickP() > 0.1f) {
            playSound("水面バタ足");

            if (isAnimationRun("水泳上昇呼吸")) {
                stopAnimation("水泳上昇呼吸");
                if (mIsSwimmingAtSurface == false) {
                    _1B = true;
                }
            }

            animIndex = 1;

            if (_1B) {
                getAnimator()->forceSetBlendWeight(&::cWeightTable[4]);
                changeAnimationUpper("水泳水面初期移動");
                _1B = false;
                return;
            }
        } else {
            if (mDashTimer == 0 && mForwardSpeed < 2.0f) {
                _1B = true;
            }

            animIndex = 0;
            if (isAnimationRun(nullptr)) {
                return;
            }
        }
    }

    if (animIndex == 1) {
        getAnimator()->setBlendWeight(&::cWeightTable[animIndex * 4], mActor->getConst().getTable()->mWeightBlendRatioSwimB);
    } else {
        getAnimator()->setBlendWeight(&::cWeightTable[animIndex * 4], mActor->getConst().getTable()->mWeightBlendRatioSwim);
    }

    getAnimator()->setSpeed(animSpeeds[animIndex]);
}

void MarioSwim::decideEffect(bool isReset) {
    u8 oldState = mEffectState;
    if (mIsOnSurface) {
        if (isAnimationRun("水泳水面初期移動") || mForwardSpeed > 2.5f) {
            mEffectState = 1;
        } else {
            mEffectState = 0;
        }

    } else {
        if (checkLvlZ() || checkLvlA() || mActionLockTimer != 0 || isAnimationRun(nullptr)) {
            mEffectState = 2;
        } else {
            mEffectState = 3;
        }
    }

    if (isReset == true) {
        mEffectState = 0xFF;
    }

    u8 newState = mEffectState;
    if (oldState != mEffectState) {
        switch (oldState) {
        case 0:
            if (mEffectState != 1) {
                stopEffect("水面ウエイト波紋");
            }

            break;

        case 1:
            stopEffect("水面泳ぎ");
            if (mEffectState != 0) {
                stopEffect("水面ウエイト波紋");
            }

            break;

        case 2:
            stopEffect("水中身体泡");
            break;
        }

        if ((mEffectState < 2 && oldState >= 2) || isReset) {
            stopEffect("水中口泡");
        }

        switch (mEffectState) {
        case 0: {
            TVec3f copyPos((getTrans() - mSurfacePos * mWaterDepth) + mSurfacePos * 30.0f);

            break;
        }

        case 1:
            if (mSurfaceOffset > -10.0f) {
                TVec3f projected;
                MR::vecKillElement(mActor->_2AC - mSurfaceNorm, mSurfacePos, &projected);

                const TVec3f& lastMove = mActor->getLastMove();

                playEffectRTW("水面泳ぎ", mSurfacePos, mSurfaceNorm + projected + lastMove);
            } else {
                stopEffect("水面泳ぎ");
            }

            break;
        case 2:
            playEffect("水中身体泡");

        case 3:
            playEffect("水中口泡");
            break;
        }
    }

    if (mEffectState <= 1) {
        TVec3f projected;
        MR::vecKillElement(mActor->_2AC - mSurfaceNorm, mSurfacePos, &projected);

        const TVec3f& lastMove = mActor->getLastMove();

        playEffectRTW("水面ウエイト波紋", mSurfacePos, mSurfaceNorm + projected + lastMove);

        if (MR::getRandom() < 0.03f) {
            playSound("水面ウエイト");
        }
    }
}

bool MarioSwim::notice() {
    if (MarioState::getNoticedStatus() == MarioStatus_FpView) {
        return true;
    }

    return getNoticedStatus() == MarioStatus_Paralyze;
}

bool MarioSwim::close() {
    if (mIsOnSurface) {
        mIsOnSurface = false;
    }

    stopAnimationUpper(nullptr);
    stopAnimation("水上スピン");        // Surface Spin
    stopAnimation("水泳スピン");        // Swim Spin
    stopAnimation("水泳ジェット開始");  // Swim Jet Start
    stopEffect("水底接触");             // Water Bottom Contact
    stopEffect("水泳スピン");           // Swim Spin Effect

    if (mActor->getCarrySensor()) {
        LiveActor* shell = mActor->getCarrySensor()->mHost;
        MR::forceDeleteEffect(shell, "BrakeLamp");
    }

    setYangleOffset(0.0f);
    u16 jointID = getAnimator()->getUpperJointID();
    setJointGlobalMtx(jointID, nullptr);

    decideEffect(true);

    if (mNextAction != EXIT_ACTION_SURFACE) {
        // If not surfacing, decide between Fall or Basic based on vertical state
        if (getPlayer()->mMovementStates.jumping) {
            stopAnimation("水泳基本", "落下");
        } else {
            stopAnimation("水泳基本", "基本");
        }
    } else {
        if (isDefaultAnimationRun("水泳基本")) {
            changeAnimation(nullptr, "基本");
        }
    }

    mWaterInfo.clear();

    if (mNextAction == EXIT_ACTION_JUMP || mNextAction == EXIT_ACTION_SLIDE) {
        mActor->setBlendMtxTimer(16);
        changeAnimation("水泳陸うちあげ");  // Landing from water
        playSound("水歩行脱出");

        f32 stickP = getStickP();
        getPlayer()->mWalkSpeed = stickP;

        getPlayer()->mMovementStates._14 = false;
        getPlayer()->mMovementStates._1 = true;
        getPlayer()->updateGroundInfo();
        getPlayer()->mDrawStates._0 = true;
        getPlayer()->mMovementStates._14 = false;
        getPlayer()->mMovementStates._1 = true;

        TVec3f groundPos;
        mActor->getGroundPos(&groundPos);

        if ((groundPos - getTrans()).length() >= 100.0f || isSlipPolygon(getGroundPolygon())) {
            mNextAction = EXIT_ACTION_POWER_JUMP;

            getPlayer()->tryForcePowerJump(mFrontVec * 5.0f - getAirGravityVec() * 15.0f, false);
            Mario* player = getPlayer();
            player->_43A = 120;
        } else {
            // Snap to ground
            addTrans(groundPos - getTrans(), "Module");
            Mario* player = getPlayer();
            player->_43A = 60;
        }
    } else {
        getPlayer()->forceSetHeadVecKeepSide(-getGravityVec());
        changeAnimationInterpoleFrame(6);
        mActor->setBlendMtxTimer(4);
    }

    if (mNextAction == EXIT_ACTION_POWER_JUMP) {
        TVec3f spawnPos = getTrans() - mUpVec * mWaterDepth + mUpVec * 10.0f;

        playEffectRT("水面ジャンプ水柱", mSurfacePos, mSurfaceNorm);
        playSound("水ジャンプ脱出");

        _1B2 = true;
        mWaterDistanceTarget = 0.0f;
        _1B8 = 0.0f;

        getPlayer()->setGroundNorm(-getAirGravityVec());
    }

    if (mNextAction == EXIT_ACTION_FALL) {
        TVec3f trans = getTrans();

        playEffectRT("水面ジャンプ水柱", -mSurfacePos, trans + mSurfacePos * mDistToFloor);

        getPlayer()->setGroundNorm(-getAirGravityVec());

        if (mJet->mJetTimer != 0) {
            changeAnimationNonStop("水泳ジェット");
        }
    }

    if (mNextAction == EXIT_ACTION_SLIDE) {
        getPlayer()->connectToClimb();
    }

    if (mNextAction == EXIT_ACTION_NONE) {
        getPlayer()->tryDrop();

        MR::vecKillElement(mWaterStreamVel, getGravityVec(), &mWaterStreamVel);

        getPlayer()->mJumpVec = getPlayer()->getJumpVec() + mWaterStreamVel * 0.3f;
    }

    getPlayer()->unlockGroundCheck(this);
    mActor->_F44 = true;
    mSwimState = SWIM_STATE_NONE;

    mActor->resetWaterLife();
    MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();

    mRingDash->mBlurOffset = 0.0f;
    if (mJet->mJetTimer != 0) {
        getPlayer()->_10._7 = true;
    }

    getPlayer()->forceSetHeadVecKeepSide(-getAirGravityVec());

    return true;
}

bool MarioSwim::checkWaterCube(bool checkOnly) {
    mUpVec = -getPlayer()->getAirGravityVec();
    WaterInfo bodyWaterInfo;
    MR::getWaterAreaObj(&bodyWaterInfo, getTrans());
    if ((getPlayer()->mMovementStates.jumping & 1) && getPlayer()->isRising()) {
        bodyWaterInfo.clear();
    }

    WaterInfo headWaterInfo;
    if (mSwimState == SWIM_STATE_NONE) {
        if (bodyWaterInfo.isInWater()) {
            getPlayer()->mDrawStates.mIsUnderwater = true;
            getPlayer()->touchWater();

            MR::getWaterAreaInfo(&bodyWaterInfo, getTrans(), getPlayer()->getAirGravityVec(), false);

            getPlayer()->_738 = bodyWaterInfo.mCamWaterDepth;
            getPlayer()->_73C = bodyWaterInfo.mSurfaceNormal;
        }

        MR::getWaterAreaObj(&headWaterInfo, getTrans() + mUpVec * 110.0f);
        if (!bodyWaterInfo.isInWater() || !headWaterInfo.isInWater()) {
            return false;
        }
    } else {
        MR::getWaterAreaObj(&headWaterInfo, getTrans() + mUpVec * 80.0f);
    }

    if (bodyWaterInfo.isInWater()) {
        if (headWaterInfo.isInWater()) {
            mSwimState = SWIM_STATE_UNDERWATER;
            mWaterInfo = bodyWaterInfo;
        } else {
            if (getPlayer()->mVerticalSpeed - mWaterDepth < 80.0f) {
                if (mSwimState != SWIM_STATE_NONE) {
                    const TVec3f& shadowNorm = getPlayer()->getShadowNorm();
                    if (calcAngleD(shadowNorm) >= 30.0f) {
                        mNextAction = EXIT_ACTION_SLIDE;
                    } else {
                        mNextAction = EXIT_ACTION_JUMP;
                    }
                }

                mSwimState = SWIM_STATE_NONE;
                return false;
            }

            mSwimState = SWIM_STATE_ENTERING;
            mWaterInfo = bodyWaterInfo;
        }

        TVec2f depthVec;
        getWaterAreaInfo(&bodyWaterInfo, getTrans(), &depthVec);
        mWaterDepth = depthVec.x;
        mDistToFloor = depthVec.y;
        mSurfacePos = bodyWaterInfo.mSurfaceNormal;
        mSurfaceNorm = bodyWaterInfo.mSurfacePos;
        return true;
    } else {
        if ((getPlayer()->mMovementStates.jumping & 1) && !getPlayer()->isRising()) {
            headWaterInfo.clear();
        }

        if (headWaterInfo.isInWater()) {
            TVec2f depthVec;
            getWaterAreaInfo(&headWaterInfo, getTrans(), &depthVec);
            mSwimState = SWIM_STATE_SURFACE;
            mWaterDepth = depthVec.x;
            mDistToFloor = depthVec.y;
            mWaterInfo = headWaterInfo;

            mSurfacePos = bodyWaterInfo.mSurfaceNormal;
            mSurfaceNorm = bodyWaterInfo.mSurfacePos;

            getPlayer()->touchWater();
            return true;
        }
    }

    if (!checkOnly) {
        if (mWaterDepth > 100.0f && getPlayer()->mVerticalSpeed < 200.0f) {
            return true;
        }
    }

    mSwimState = SWIM_STATE_NONE;
    mNextAction = EXIT_ACTION_FALL;

    if (mWaterInfo.isInWater()) {
        TVec2f depthVec;
        getWaterAreaInfo(&mWaterInfo, getTrans(), &depthVec);
        mWaterDepth = depthVec.x;
    }

    if (!(mWaterDepth < -500.0f)) {
        if (mIsOnSurface && getPlayer()->mVerticalSpeed < 30.0f) {
            mNextAction = EXIT_ACTION_SLIDE;
        } else if (!MR::isStageSwimAngleLimit()) {
            mNextAction = EXIT_ACTION_SLIDE;
        }
    }

    return false;
}

void MarioSwim::updateUnderWater() {
    if (MarioModule::getPlayerMode() != 4 && MarioModule::getStickP() > 0.1f) {
        mWaterDistanceTarget = 0.0f;
    }

    MarioActor* actor;
    u32 searchLimit = 2;
    if (MarioModule::getPlayerMode() == 3) {
        searchLimit = 10;
    }

    TVec3f searchDir = MarioModule::getFrontVec();
    if (!MR::isNearZero(mActor->getLastMove())) {
        searchDir = mActor->getLastMove();
        MR::vecKillElement(searchDir, getGravityVec(), &searchDir);
        MR::normalizeOrZero(&searchDir);
    }

    const f32 kOffset = 100.0f;

    for (u32 i = 0; i < searchLimit; i++) {
        TVec3f checkPos = MarioModule::getTrans() + searchDir * kOffset + getGravityVec() * (mWaterDistanceTarget - kOffset);

        WaterInfo waterInfo;
        MR::getWaterAreaObj(&waterInfo, checkPos);

        if (waterInfo.isInWater()) {
            _1B8 = mWaterDistanceTarget;

            if (mWaterDistanceTarget > 300.0f) {
                getWaterAreaInfo(&waterInfo, checkPos, nullptr);
                TVec3f surfacePos = waterInfo.mSurfacePos;
                _1B8 = (surfacePos - getTrans()).dot(getGravityVec());
                _1B2 = true;
            }

            else if (mWaterDistanceTarget < 210.0f && MarioModule::getPlayerMode() == 3) {
                if (!MR::checkStrikePointToMap(checkPos, nullptr)) {
                    getWaterAreaInfo(&waterInfo, getTrans(), nullptr);
                    TVec3f surfacePos = waterInfo.mSurfacePos;

                    if (waterInfo.mCamWaterDepth < 100.0f) {
                        if (!(getPlayer()->mMovementStates._1) || getPlayer()->mTargetWalkSpeedIndex) {
                            actor = mActor;
                            actor->createIceFloor(surfacePos + searchDir * 170.0f);
                        }

                        mWaterDistanceTarget = 0.0f;
                    }

                    return;
                }

                mWaterDistanceTarget = 0.0f;
            }

            mWaterDistanceTarget = 0.0f;
            return;
        }

        if (MR::checkStrikePointToMap(checkPos, nullptr)) {
            _1B2 = false;
            mWaterDistanceTarget = 0.0f;
            return;
        }

        mWaterDistanceTarget += 40.0f;
    }
}

f32 MarioSwim::checkUnderWaterFull(const TVec3f& rVec) {
    for (f32 currentDist = 0.0f; currentDist < 10000.0f; currentDist += 40.0f) {
        TVec3f checkPos = getTrans() + rVec * 100.0f + getGravityVec() * (currentDist - 100.0f);
        if (MR::getAreaObj("ForbidWaterSearchCube", checkPos) != nullptr) {
            return -1.0f;
        }

        WaterInfo waterInfo;
        MR::getWaterAreaObj(&waterInfo, checkPos);
        if (waterInfo.isInWater()) {
            return currentDist;
        }

        if (MR::checkStrikePointToMap(checkPos, nullptr)) {
            return -1.0f;
        }
    }

    return -1.0f;
}

bool MarioSwim::checkWaterBottom() {
    TVec3f rayStart(getTrans() + mUpVec * 50.0f);

    u32 hitCount = MR::getNearPolyOnLineSort(rayStart, rayStart, -mUpVec * 2000.0f, nullptr);
    for (u32 i = 0; i < hitCount; i++) {
        Triangle tri;
        TVec3f hitPos;

        MR::getSortedPoly(&hitPos, &tri, i);

        if (!MR::isWaterPolygon(&tri)) {
            mDistToFloor = (getTrans() - hitPos).dot(mUpVec);

            return mDistToFloor > 200.0f;
        }
    }

    return true;
}

bool MarioSwim::getWaterAreaInfo(WaterInfo* pInfo, const TVec3f& rPos, TVec2f* pOutVec) {
    bool isInWater = MR::getWaterAreaInfo(pInfo, rPos, getPlayer()->getAirGravityVec(), false);

    if (pOutVec != nullptr) {
        pOutVec->x = -pInfo->mCamWaterDepth;
        pOutVec->y = pInfo->_4;
    }

    if (mIsOnSurface) {
        if (mSurfaceOffset > 0.0f) {
            f32 waveHeight = pInfo->mWaveHeight;
            mSurfaceOffset = 0.97f * mSurfaceOffset + 0.03f * waveHeight;
        } else {
            f32 waveHeight = pInfo->mWaveHeight;
            mSurfaceOffset = 0.99f * mSurfaceOffset + 0.01f * (waveHeight * 0.5f);
        }
    } else {
        mSurfaceOffset = 0.8f * mSurfaceOffset;
    }

    _178 = pInfo->mEdgePos;
    mDistanceToWaterSurface = pInfo->mEdgeDistance;
    TVec3f currentFlow(pInfo->mStreamVec);

    f32 depthFactor = 1.0f;
    if (mWaterDepth > -500.0f) {
        depthFactor = -mWaterDepth / 500.0f;
    }

    f32 finalScale = MR::clamp(depthFactor, 0.0f, 1.0f);
    if (mIsOnSurface) {
        finalScale = 0.0f;
    }

    currentFlow.scale(finalScale);
    mWaterStreamVel = mWaterStreamVel * 0.95f + currentFlow * 0.05f;
    mWaterStreamDir = currentFlow;

    return isInWater;
}

const TVec3f& MarioSwim::getGravityVec() const {
    return MarioModule::getGravityVec();
}

bool MarioSwim::surfacePaddle() {
    if (mDashTimer != 0) {
        return false;
    }

    stopAnimation(nullptr);
    changeAnimation("水上一掻き");
    // Surface Paddle
    playSound("水面一掻き");
    mDashTimer = 40;

    return true;
}

// NONMATCHING: equivalent branch folding and register allocation.
void MarioSwim::spin() {
    if (mJet->mJetTimer != 0) {
        if (mActor->isRequestSpin()) {
            bool active = false;
            if (mSpin->mSpinTimer || mSpin->mSpinDashTimer) {
                active = true;
            }
            if (!active) {
                dropJet(false);
            }
        }
    } else {
        if (mDamage->mTimer == 0) {
            if (mZSinkTimer != 0) {
                return;
            } else {
                mSpin->update();
            }
        }
    }
}

void MarioSwim::hitPunch(const TVec3f& rPunchDir) {
    if (mSpin->mSpinTimer != 0) {
        mDamage->mTimer = 30;
        mForwardSpeed = 0.0f;
        mDamage->mVelocity = rPunchDir;
        mSpin->_8 = true;

        stopAnimation("アッパーパンチ");
    }
}

void MarioSwim::jet() {
    if (mActor->getCarrySensor() == nullptr) {
        mJet->mJetTimer = 0;
        mJet->mCooldownTimer = 0;
    }
    f32 factor = calcRingAcc();
    if (mJet->mJetTimer != 0) {
        mJet->update(factor);
    } else {
        ringDash(factor);
    }
}

void MarioSwim::resetJet() {
    if (mJet->mJetTimer != 0) {
        if (isStatusActiveS(this)) {
            getAnimator()->changeDefault("水泳基本");
        }
    }

    mJet->mJetTimer = 0;
}

void MarioSwim::dropJet(bool isDamage) {
    if (mJet->mJetTimer == 0) {
        return;
    }

    mJet->mJetTimer = 0;

    if (mForwardSpeed > mActor->getConst().getTable()->mSwimFrontMaxSpeed) {
        mForwardSpeed = mActor->getConst().getTable()->mSwimFrontMaxSpeed;
    }

    if (isDamage) {
        changeAnimation("水泳ジェット終了", "水泳基本");
    } else {
        changeAnimation("水泳亀投げ", "水泳基本");
    }

    if (mActor->getCarrySensor()) {
        LiveActor* shell = mActor->getCarrySensor()->mHost;
        MR::forceDeleteEffect(shell, "BrakeLamp");
    }

    if (isDamage) {
        mActor->damageDropThrowMemoSensor();
    } else {
        mActor->releaseThrowMemoSensor();
    }

    mDamage->mTimer = 15;
    mDamage->mVelocity.zero();

    startPadVib(static_cast< u32 >(0));
}

bool MarioSwim::startJet(const HitSensor* sensor) {
    if (mJet->mJetTimer != 0) {
        return false;
    }

    mJet->mJetTimer = 1;
    mJet->_8 = sensor->isType(0x13);

    forceStopSpin();
    updateJetAnimation();

    mJet->mCooldownTimer = 20;
    return true;
}

void MarioSwim::forceStopSpin() {
    mSpin->mSpinDashTimer = 0;
    mSpin->mSpinTimer = 0;
    setYangleOffset(0.0f);



    // Swim Spin
    stopAnimation("水泳スピン");
    stopAnimation("水泳スピン移動");
    // Surface Spin
    stopAnimation("水上スピン");
    stopAnimation("水上スピン移動");

    stopEffect("水泳スピン");
}

void MarioSwim::addFaint(const TVec3f& rFaintDir) {
    if (getPlayer()->getDamageAfterTimer() != 0) {
        return;
    }

    if (mDamage->mDamageType == 0) {
        Mario* player = getPlayer();
        player->_41E = 150;
    }

    dropJet(false);
    mDamage->mTimer = mActor->getConst().getTable()->mWaterInnerFaintTime;

    changeAnimation("水泳ダメージ");
    playSound("水中ダメージ");
    playSound("声水中ダメージ");
    playSound("ダメージ");

    startPadVib(2);
    mDamage->mVelocity = rFaintDir;
    mDamage->mVelocity.setLength(mActor->getConst().getTable()->mSwimFaintSpeed);
    mActionLockTimer = 0;
    forceStopSpin();
    mForwardSpeed = 0.0f;

    if (mDamage->mDamageType == 0) {
        mActor->decLife(0);
        if (mActor->mHealth == 0) {
            mActor->forceGameOverNonStop();
        }
    }

    mDamage->mDamageType = 0;
}

void MarioSwim::addDamage(const TVec3f& rDamageDir) {
    TVec3f surfaceKnockback;
    TVec3f postureDir;
    if (getPlayer()->getDamageAfterTimer() != 0) {
        return;
    }

    if (mDamage->mDamageType == 0) {
        Mario* player = getPlayer();
        player->_41E = 180;
    }

    dropJet(false);
    mForwardSpeed = 0.0f;

    if (mDamage->mDamageType == 0) {
        playSound("ダメージ");
    } else {
        playSound("壁衝突");
    }

    if (mIsOnSurface) {
        MR::vecKillElement(rDamageDir, mUpVec, &surfaceKnockback);
        surfaceKnockback.setLength(mActor->getConst().getTable()->mSwimSurfaceDamageSpeed);

        TVec3f jumpVel = mUpVec * mActor->getConst().getTable()->mWaterSurfaceDamageJump + surfaceKnockback;
        getPlayer()->tryForcePowerJump(jumpVel, false);
        changeAnimationNonStop("水上ダメージ中");
        playSound("水面ダメージ");
        playSound("声小ダメージ");

        mFrontVec = -surfaceKnockback;
        mNextAction = EXIT_ACTION_POWER_JUMP;
        getPlayer()->closeStatus(this);

        if (mDamage->mDamageType == 0) {
            mActor->decLife(0);
            if (mActor->mHealth == 0) {
                mActor->forceGameOverNonStop();
            }
        }

        mDamage->mDamageType = 0;
    } else {
        mDamage->mTimer = mActor->getConst().getTable()->mWaterInnerDamageTime;
        changeAnimation("水泳ダメージ中");

        mDamage->mVelocity = rDamageDir;
        mDamage->mVelocity.setLength(mActor->getConst().getTable()->mSwimDamageSpeed);

        playSound("水中ダメージ");
        playSound("声水中ダメージ");

        if (mDamage->mDamageType == 0) {
            decLife();
        }

        mDamage->mDamageType = 0;
    }

    if (mDamage->_15 != 0) {
        mDamage->mVelocity.setLength(mActor->getConst().getTable()->mSwimDamageSpeedEx1);
        mDamage->mTimer = mActor->getConst().getTable()->mSwimDamageTimerEx1;
        mDamage->_15 = 0;
    }

    startPadVib(3);
    playEffect("ダメージ");
    mActionLockTimer = 0;
    forceStopSpin();
    mForwardSpeed = 0.0f;

    MR::vecKillElement(rDamageDir, getGravityVec(), &postureDir);

    if (MR::normalizeOrZero(&postureDir) == 0) {
        mFrontVec = -postureDir;
    }
}


void MarioSwim::pushedByWaterWall() {
    if (!(mDistanceToWaterSurface < 0.0f)) {
        f32 wallDistanceLimit = 80.0f;
        if (mDistanceToWaterSurface < wallDistanceLimit) {
            TVec3f pushDir = getTrans() - _178;
            pushDir.setLength(wallDistanceLimit - mDistanceToWaterSurface);
            getPlayer()->push(pushDir);

            if (MR::normalizeOrZero(&pushDir) == 0) {
                const TVec3f& headVec = getPlayer()->mHeadVec;
                f32 dot = pushDir.dot(-headVec);

                if (_44 != 0) {
                    dot = 1.0f;
                }

                if (!(dot < 0.0f)) {
                    if (dot < 0.995f) {
                        TVec3f newHead;
                        MR::vecKillElement(headVec, pushDir, &newHead);
                        MR::normalize(&newHead);

                        MR::vecBlendSphere(getPlayer()->mHeadVec, newHead, &newHead, 0.2f);
                        getPlayer()->forceSetHeadVecKeepSide(newHead);
                        MR::vecKillElement(newHead, getGravityVec(), &mFrontVec);

                        Mario* player = getPlayer();
                        mVerticalAngle = MR::diffAngleAbs(player->mHeadVec, -getGravityVec());

                    } else {
                        MR::vecBlendSphere(mFrontVec, pushDir, &mFrontVec, 0.2f);
                        _44 = 30;
                    }
                }
            }
        }
    }
}

bool MarioSwim::tryJetAttack(HitSensor* pHitSensor) {
    if (mActor->tryJetAttack(pHitSensor) == true) {
        dropJet(1);

        const TVec3f& trans = getTrans();
        TVec3f damageDir = trans - pHitSensor->mPosition;
        addDamage(damageDir);

        return true;
    } else {
        return false;
    }
}

void MarioSwim::hitHead(const HitInfo* pHit) {
    if (mJet->mJetTimer != 0) {
        if (sendPunch(pHit->mParentTriangle.mSensor, false) == 1) {
            startCamVib(0);
            return;
        }

        if (tryJetAttack(pHit->mParentTriangle.mSensor)) {
            return;
        }

        const TVec3f* normal = pHit->mParentTriangle.getNormal(0);
        if (normal->dot(getGravityVec()) > 0.0f) {
            Mario* player = getPlayer();
            TVec3f direction;
            TVec3f velocityPart;

            f32 element = MR::vecKillElement(player->_1FC, -*normal, &velocityPart);
            velocityPart += *normal * element * 1.5f;

            if (velocityPart.length() < 10.0f) {
                velocityPart.setLength(10.0f);
            }

            MR::vecKillElement(velocityPart, getGravityVec(), &direction);

            direction.length();
            if (MR::normalizeOrZero(&direction) == false) {
                mFrontVec = direction;
            }
        }
    }

    TVec3f pushBack = getTrans() - pHit->_64;
    MR::vecKillElement(pushBack, *pHit->mParentTriangle.getNormal(0), &pushBack);
    pushBack.setLength(5.0f);
    addVelocity(pushBack);
}

void MarioSwim::hitPoly(u8 type, const TVec3f& rNormal, HitSensor* pSensor) {
    if (mStateTimer <= 120) {
        return;
    }

    if (!mIsOnSurface) {
        return;
    }

    const TVec3f& gravity = getPlayer()->getAirGravityVec();
    f32 dot = rNormal.dot(gravity);
    f32 invDot = -dot;

    const TVec3f& swimGravity = getAirGravityVec();
    TVec3f negNormal = -rNormal;

    f32 angle = MR::diffAngleAbsHorizontal(negNormal, mFrontVec, swimGravity);
    if (invDot >= 0.5f && angle <= PI / 4) {
        _22 = true;
    }
}

void MarioSwim::hitWall(const TVec3f& rNormal, HitSensor* pSensor) {
    if (mActor->_424 == 0) {
        f32 floorDot = -rNormal.dot(getPlayer()->getAirGravityVec());

        if (!mIsOnSurface && floorDot > 0.70700002f) {
            _1F = true;
            return;
        } else if (mIsOnSurface) {
            if (MR::diffAngleAbsHorizontal(-rNormal, mFrontVec, getAirGravityVec()) < DEG_TO_RAD(45)) {
                mWallStickTimer = 6;
            }

            if (mStateTimer > 0x78) {
                // If on surface and hitting something slope-like, treat as slide/floor interaction
                if (floorDot >= 0.5f) {
                    _22 = true;
                    return;
                }
            } else {
                return;
            }
        }

        bool isHit = false;

        if (!mIsOnSurface && mForwardSpeed > 1.0f) {
            TVec3f projectedVelocity;
            MR::vecKillElement(getPlayer()->_1FC, rNormal, &projectedVelocity);

            if (projectedVelocity.dot(mUpVec) > 0.0f) {
                if (getStickY() <= 0.0f) {
                    mVerticalAngle -= 0.1f;
                }
            } else {
                if (getStickY() >= 0.0f) {
                    mVerticalAngle += 0.1f;
                }
            }
        }

        mWallStickTimer = 6;
        mDashTimer = 0;

        if (mJet->mJetTimer != 0 || (!MR::isNearZero(mWaterStreamDir) && mActionLockTimer != 0)) {
            bool punched = false;
            if (mJet->mJetTimer != 0) {
                punched = sendPunch(pSensor, false);
            }

            if (!punched) {
                if (mJet->mJetTimer != 0) {
                    if (tryJetAttack(pSensor)) {
                        return;
                    }
                }

                TVec3f velocityAfterHit;
                TVec3f tangent;
                Mario* player = getPlayer();
                f32 speedIntoWall = MR::vecKillElement(player->_1FC, -rNormal, &tangent);

                tangent += rNormal * speedIntoWall * 1.5f;
                MR::vecKillElement(tangent, getGravityVec(), &velocityAfterHit);
                f32 velocityAfterHitLength = velocityAfterHit.length();

                bool isReflect = true;
                f32 speedReduction = mActor->getConst().getTable()->mTurboReductionHitWall;

                if (!MR::normalizeOrZero(&velocityAfterHit)) {
                    if (rNormal.dot(getGravityVec()) > 0.1f && mVerticalAngle < 1.5707964f) {
                        mVerticalAngle = PI - mVerticalAngle;
                    }

                    mDamage->mTimer = 20;
                    mDamage->mVelocity = rNormal * 6.0f;

                    if (MR::isNearZero(mWaterStreamDir)) {
                        f32 impactAngle = MR::diffAngleAbsHorizontal(mFrontVec, rNormal, getAirGravityVec());

                        if (impactAngle >= 2.6179941f) {
                            f32 headAngle = MR::diffAngleAbs(getPlayer()->_1FC, rNormal);

                            if (headAngle <= 2.6179941f) {
                                if (headAngle >= 2 * PI) {
                                    Mario* player = getPlayer();
                                    TVec3f cross1 = rNormal.cross(player->_1FC);
                                    MR::normalizeOrZero(&cross1);

                                    TVec3f cross2 = getPlayer()->_1FC.cross(cross1);
                                    MR::normalizeOrZero(&cross2);

                                    getPlayer()->forceSetHeadVecKeepSide(cross2);

                                    player = getPlayer();
                                    mVerticalAngle = MR::diffAngleAbs(player->mHeadVec, -getAirGravityVec());
                                }

                                isReflect = false;

                            } else {
                                mDamage->mTimer = 60;
                                mDamage->mVelocity = rNormal * 16.0f;
                            }
                        } else {
                            TVec3f glanceDir;
                            MR::vecKillElement(mFrontVec, rNormal, &glanceDir);
                            if (!MR::normalizeOrZero(&glanceDir)) {
                                bool isTurn = false;
                                mFrontVec = glanceDir;
                                speedReduction = 0.8f;
                                isReflect = false;
                            }
                        }
                    } else {
                        if (velocityAfterHit.dot(mWaterStreamDir) > 0.0f) {
                            mFrontVec = velocityAfterHit;
                        } else {
                            mDamage->mTimer = 60;
                            mDamage->mVelocity = rNormal * 16.0f;
                        }
                    }
                }

                if (mJet->mJetTimer != 0 && isReflect) {
                    playEffectTrans("壁ヒット", getPlayer()->getWallPos());
                    playSound("亀壁ヒット");
                    changeAnimation("水泳ジェット壁ターン");
                }

                mForwardSpeed *= speedReduction;
            } else {
                startCamVib(0);
            }

            isHit = true;
        }

        // Spin Attack Collision
        if (mSpin->mSpinDashTimer != 0) {
            if (!sendPunch(pSensor, true)) {
                mForwardSpeed = 0.0f;
                forceStopSpin();
                changeAnimation("水泳壁ヒット");
                playEffectTrans("壁ヒット", getPlayer()->getWallPos());
                startCamVib(0);
                mDamage->mTimer = 60;
                mDamage->mVelocity = rNormal * 6.0f;
            }

            isHit = true;
        } else if (mSpin->mSpinTimer != 0) {
            if (!sendPunch(pSensor, false)) {
                mSpin->mSpinTimer = 0;
                mForwardSpeed = 0.0f;
                playEffectTrans("壁ヒット", getPlayer()->getWallPos());
                startCamVib(2);
                mDamage->mTimer = 30;
                mDamage->mVelocity = rNormal * 6.0f;
            }

            isHit = true;
        }

        // Ring Dash Collision
        if (mRingDash->mTimer != 0) {
            if (mJet->mJetTimer != 0) {
                dropJet(true);
            }

            mForwardSpeed = 0.0f;
            changeAnimation("水泳壁ヒット");
            playEffectTrans("壁ヒット", getPlayer()->getWallPos());
            startCamVib(0);
            mDamage->mTimer = 60;
            mDamage->mVelocity = rNormal * 6.0f;
            isHit = true;
        }

        mRingDash->mTimer = 0;
        mRingDash->mChargeTimer = 0;

        if (isHit) {
            startPadVib(2);
        }

        // Wall Push / Kick Logic check
        if (rNormal.dot(getGravityVec()) > 0.8f) {
            TVec3f* worldPadDir = &getWorldPadDir();
            if (getStickP() != 0.0f) {
                if (checkTrgA()) {
                    if (mDamage->mTimer == 0) {
                        mDamage->mTimer = 60;
                    }

                    mDamage->mVelocity = *worldPadDir * 10.0f + getGravityVec() * 10.0f;
                }
            }
        }

        stopEffect("水泳スピン");
    }
}

void MarioSwim::decLife() {
    mDrownTimer = 0;
    mActor->decLife(0);
    if (mActor->mHealth == 0) {
        mActor->forceGameOver();

        TVec3f gravityVec = -getGravityVec();
        getPlayer()->forceSetHeadVecKeepSide(gravityVec);

        u16 upperJoint = getAnimator()->getUpperJointID();
        setJointGlobalMtx(upperJoint, nullptr);
    }
}

void MarioSwim::incLife() {
    mDrownTimer = 0;
    mActor->incLife(1);
    incOxygen();
}

void MarioSwim::incOxygen() {
    u32 oxygenMax;
    if (mOxygen == (oxygenMax = mActor->getConst().getTable()->mOxygenMax)) {
        return;
    }

    mOxygen += (oxygenMax / 8);

    if (mOxygen > (oxygenMax = mActor->getConst().getTable()->mOxygenMax)) {
        // 0x64: sth r0, 0xea(r3)
        mOxygen = oxygenMax;
    }

    if (mOxygen == mActor->getConst().getTable()->mOxygenMax) {
        // Play "Full" sound
        playSound("酸素完全回復");
    } else {
        // Play "Recovering" sound
        playSound("酸素回復");
    }

    mOxygenDecreaseDelay = 120;
}

void MarioSwim::decOxygen(u16 amount) {
    if (mOxygen == 0) {
        return;
    }

    MarioActor* actor = mActor;

    if (actor->_934 != 0) {
        return;
    }

    if (actor->_EA4 != 0) {
        return;
    }

    if (isStatusActiveID(MarioStatus_Talk)) {
        return;
    }

    if (mOxygen < amount) {
        mOxygen = 0;
    } else {
        mOxygen -= amount;
    }

    if (mOxygen == 0) {
        playSound("無酸素警告");
    }
}

void MarioSwim::draw3D() const {
    WaterInfo camWaterInfo;
    MR::getWaterAreaObj(&camWaterInfo, getCamPos());
    if (camWaterInfo.isInWater()) {
        mMarineSnow->view();
    } else {
        mMarineSnow->clear();
    }

    mMarineSnow->draw(getTrans(), mUpVec, -mWaterDepth);
}

void MarioSwim::onSurface() {
    if (!mIsOnSurface) {
        mIsOnSurface = true;
        if (getPlayer()->mMovementStates._37) {
            mDashTimer = 0;
            mForwardSpeed = 0.0f;
        }
    }
}

void MarioSwim::updateLifeByTime() {
    updateOxygenWatch();
    if (mActor->_EA4) {
        mColdWaterDamageInterval = 120;
    } else {
        if (isInColdWater()) {
            updateColdWaterDamage();
        } else {
            mColdWaterDamageInterval = 0;
            if (mIsOnSurface) {
                mDrownTimer = 0;
                recoverOxygenAtSurface();
                return;
            }
        }
        updateOxygenByTime();
    }
}

bool MarioSwim::isInColdWater() const {
    const WaterArea* area = mWaterInfo.mWaterArea;
    if (area && MR::getAreaObjArg(area, 1) == 1) {
        return true;
    }
    return false;
}

void MarioSwim::updateColdWaterDamage() {
    if (getPlayer()->isDamaging()) {
        mColdWaterDamageInterval = 120;
    }
    getPlayer()->_1C._3 = 1;
    if (mColdWaterDamageInterval == 0) {
        mColdWaterDamageInterval = 120;
        return;
    }
    mColdWaterDamageInterval--;
    if (mColdWaterDamageInterval == 0) {
        doDecLifeByCold();
        mColdWaterDamageInterval = 120;
    }
}

void MarioSwim::recoverOxygenAtSurface() {
    mOxygenDecreaseDelay = 120;
    MarioActor* actor = mActor;
    u16 oxygen = mOxygen;
    if (oxygen != actor->getConst().getTable()->mOxygenMax) {
        mOxygen = oxygen + 32;
        if (mOxygen > actor->getConst().getTable()->mOxygenMax) {
            playSound("水面酸素完全回復");
            mOxygen = mActor->getConst().getTable()->mOxygenMax;
            return;
        }
        playSound("水面酸素回復");
    }
}

void MarioSwim::updateOxygenByTime() {
    if (mOxygen) {
        if (mOxygenDecreaseDelay) {
            mOxygenDecreaseDelay--;
            return;
        }
        decOxygen(1);
    } else {
        mDrownTimer++;
        if (mDrownTimer >= mActor->getConst().getTable()->mWaterDecInterval) {
            doDecLifeByCold();
        }
    }
}

void MarioSwim::fn_803F8430() {
    if (_18) {
        onSurface();
        mSinkTimer = 0;
        mJumpDiveTimer = 0;
        mZSinkTimer = 0;
        mActionLockTimer = 0;
        mStickInputYInertia = 0.0f;
    }
}

void MarioSwim::doDecLifeByCold() {
    if (mActor->mHealth != 0) {
        decLife();

        if (mJet->mJetTimer != 0) {
            changeAnimation("水泳ダメージ亀");
        } else {
            changeAnimation("水泳ダメージ");
        }

        playSound("水中ダメージ");
        playSound("声冷水ダメージ");
        playSound("ダメージ");
        startPadVib(2);

        mActor->_BC4 = 16;
    }
}

// NONMATCHING: pooled string offsets differ; live arguments resolve equally.
bool MarioSwim::passRing(const HitSensor* sensor) {
    const TVec3f& ringPos = *MR::getSensorPos(sensor);
    if (mRingDash->mTimer == 0) {
        mRingDash->mChargeTimer = mActor->getConst().getTable()->mSwimRingDashChargeTime;
        if (mJet->mJetTimer == 0)
            changeAnimation("リングダッシュ準備");
        else
            changeAnimation("カメ持ちリング準備");
        playSound("亀加速");
    }
    if (mRingDash->mChargeTimer != 0)
        getPlayer()->push((ringPos - getTrans()) * 0.25f);
    mRingDash->setRing(static_cast<const DashRing*>(MR::getSensorHost(sensor)));
    return true;
}

void MarioSwimRingDash::setRing(const DashRing* ring) {
    mMaxDuration = ring->mMaxDuration;
    mSpeedScale = ring->mSpeedScale;
    mTimer = ring->mBoostTime;
}

// NONMATCHING: timer-load reuse and register allocation.
f32 MarioSwim::calcRingAcc() {
    mRingDash->updateBlur();
    MarioSwimRingDash* ring = mRingDash;
    u16 timer = ring->mTimer;
    if (timer != 0) {
        if (ring->mChargeTimer != 0) {
            ring->mChargeTimer--;
            if (mRingDash->mChargeTimer == 1) {
                MarioActor* actor = mActor;
                actor->_1AA = actor->mConst->getTable()->mStarPieceFogTime;
                actor->_1AC = 1.0f;
                actor->mSwimFogColor.r = 255;
                actor->mSwimFogColor.g = 255;
                actor->mSwimFogColor.b = 255;
                actor->mSwimFogColor.a = 0;
                actor->mSwimFogDisabled = false;
            }
            if (mRingDash->mChargeTimer == 0) {
                startPadVib(3);
                if (mJet->mJetTimer == 0) {
                    changeAnimation("リングダッシュ");
                } else {
                    changeAnimation("カメ持ちリング");
                }
            }
            return 1.0f;
        }
        u16 maxDuration = ring->mMaxDuration;
        f32 result = ring->mSpeedScale;
        if (timer < maxDuration) {
            f32 ratio = static_cast<f32>(timer) / static_cast<f32>(maxDuration);
            result = (1.0f - ratio) + (result * ratio);
        }
        ring->mTimer--;
        return result;
    }
    return 1.0f;
}

void MarioSwim::ringDash(f32 accFactor) {

    if (mRingDash->mTimer != 0) {
        const MarioConstTable* table = mActor->getConst().getTable();
        f32 targetSpeed = accFactor * fn_803FA8E0();

        if (mForwardSpeed < targetSpeed) {
            if (mForwardSpeed < 1.0f) {
                mForwardSpeed = 1.0f;
            }

            if (targetSpeed / mForwardSpeed > 1.5f) {
                mForwardSpeed *= 1.5f;
            } else {
                mForwardSpeed = targetSpeed;
            }
        } else {
            if (mForwardSpeed > targetSpeed) {
                if (targetSpeed / mForwardSpeed < 0.98f) {
                    mForwardSpeed *= 0.98f;
                } else {
                    mForwardSpeed = targetSpeed;
                }
            } else {
                mForwardSpeed = targetSpeed;
            }
        }
    }
}

void MarioSwim::resetAndFixPose() {
    mActionLockTimer = 0;
    mSinkTimer = 0;
    mSwimYAccTimer = 0;
    mZSinkTimer = 0;
    mIdleWaitTimer = 0;
    mBuoyancyDelayTimer = 0;
    mDashTimer = 0;
    mSpin->mSpinDashTimer = 0;
    mSpin->mSpinTimer = 0;

    mForwardSpeed = 0.0f;
    mStickInputYInertia = 0.0f;
    mStickInputXInertia = 0.0f;
    mBuoyancySpeed = 0.0f;

    MR::vecKillElement(getFrontVec(), getGravityVec(), &mFrontVec);

    Mario* player = getPlayer();
    mVerticalAngle = MR::diffAngleAbs(player->mHeadVec, -getGravityVec());
}

f32 MarioSwim::getStickY() const {
    f32 stickY = MarioModule::getStickY();

    if (mIsSinking && stickY < 0.0f) {
        if (stickY > -0.5f) {
            stickY = 0.0f;
        } else {
            stickY = (0.5f + stickY) / 0.5f;
        }
    }

    return stickY;
}

void MarioSwim::updateOxygenWatch() {
    if (mOxygen == (mActor->getConst().getTable()->mOxygenMax >> 2)) {
        playSound("酸素減少警告");
    }
    if (mOxygen == (mActor->getConst().getTable()->mOxygenMax >> 1)) {
        playSound("酸素減少警告");
    }
}

void MarioSwim::addVelocity(const TVec3f& rVelocity) {
    if (mIsOnSurface && mNextAction != EXIT_ACTION_NONE) {
        const TVec3f& gravity = getAirGravityVec();
        TVec3f horizontalVel;
        f32 verticalMag = MR::vecKillElement(rVelocity, gravity, &horizontalVel);
        if (verticalMag < 0.0f) {
            MarioModule::addVelocity(horizontalVel);
        } else {
            MarioModule::addVelocity(rVelocity);
        }
    } else {
        MarioModule::addVelocity(rVelocity);
    }
}

void MarioSwim::addVelocity(const TVec3f& rVelocity, f32 scale) {
    if (mIsOnSurface && mNextAction != EXIT_ACTION_NONE && mWaterDepth >= 0.0f) {
        const TVec3f& gravity = getAirGravityVec();
        TVec3f horizontalVel;
        f32 verticalMag = MR::vecKillElement(rVelocity, gravity, &horizontalVel);

        if (verticalMag < 0.0f) {
            MarioModule::addVelocity(horizontalVel, scale);
        } else {
            MarioModule::addVelocity(rVelocity, scale);
        }
    } else {
        MarioModule::addVelocity(rVelocity, scale);
    }
}

void MarioSwim::doJetJump(u8 type) {
    if (!getPlayer()->_1C._7 && mRingDash->mChargeTimer == 0) {
        f32 verticalScale;
        f32 horizontalSpeed = 12.0f;
        f32 verticalBoost = 15.0f;

        if (checkLvlA()) {
            horizontalSpeed = 18.0f;
            verticalBoost = 30.0f;
        } else if (type == 0) {
            getPlayer()->_10.digitalJump = true;
        }

        const MarioConstTable* table = mActor->getConst().getTable();
        f32 speedRatio = MR::clamp(mForwardSpeed / table->mSwimToWalkSpd, 0.0f, 1.0f);

        mUpVec.dot(getPlayer()->mHeadVec);
        f32 horizontalScale = 1.25f;
        verticalScale = 1.0f;

        TVec3f jumpDir;
        MR::vecKillElement(mActor->getLastMove(), mUpVec, &jumpDir);
        MR::normalizeOrZero(&jumpDir);

        getPlayer()->mMovementStates._5 = 0;
        getPlayer()->mMovementStates._2F = 0;
        getPlayer()->mWalkSpeed = speedRatio;

        TVec3f finalVelocity = jumpDir * horizontalSpeed * horizontalScale + mUpVec * verticalBoost * verticalScale;

        getPlayer()->tryForcePowerJump(finalVelocity, false);

        mNextAction = EXIT_ACTION_POWER_JUMP;
        changeAnimationNonStop("水泳ジェット");
    }
}
MarioSwimModule::~MarioSwimModule() {}

MarioSwimSpin::MarioSwimSpin(MarioSwim* swim) : MarioSwimModule(swim) {
    _8 = 0;
    mSpinTimer = 0;
    mSpinDashTimer = 0;
    _E = 0;
}

void MarioSwimSpin::reset() {
    mSpinTimer = 0;
    mSpinDashTimer = 0;
    _E = 0;
}

bool MarioSwimSpin::isActive() const {
    return mSpinTimer || mSpinDashTimer;
}

MarioSwimJet::MarioSwimJet(MarioSwim* swim) : MarioSwimModule(swim) {
    _8 = 0;
    mJetTimer = 0;
    mCooldownTimer = 0;
}

f32 MarioSwimJet::calcSpeedScale() const {
    if (mJetTimer) {
        return 3.0f;
    }
    return 1.0f;
}

MarioSwimRingDash::MarioSwimRingDash(MarioSwim* swim) : MarioSwimModule(swim) {
    mTimer = 0;
    mChargeTimer = 0;
    mMaxDuration = 0;
    mSpeedScale = 0.0f;
    mBlurOffset = 0.0f;
}

void MarioSwimRingDash::reset() {
    mTimer = 0;
    mChargeTimer = 0;
    mBlurOffset = 0.0f;
}

void MarioSwimRingDash::updateBlur() {
    if (mChargeTimer == 0) {
        if (mTimer != 0) {
            mBlurOffset += 0.5f;
            if (mBlurOffset > 30.0f) {
                mBlurOffset = 30.0f;
            }
        }
        if (mBlurOffset > 0.0f) {
            mBlurOffset -= 0.5f;
            return;
        }
        mBlurOffset = 0.0f;
    }
}

MarioSwimDamage::MarioSwimDamage(MarioSwim* swim) : MarioSwimModule(swim) {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mDamageType = 0;
    _15 = 0;
    mTimer = 0;
}

void MarioSwimDamage::update() {
    mTimer--;
    if (mTimer < 120) {
        mVelocity.x *= 0.98f;
        mVelocity.y *= 0.98f;
        mVelocity.z *= 0.98f;
    }
}

void MarioSwimDamage::reflect(const TVec3f* normal) {
    f32 projection = MR::vecKillElement(mVelocity, *normal, &mVelocity);
    mVelocity.add((*normal * projection) * 0.5f);
}

void MarioSwimDamage::reset() {
    mTimer = 0;
    mDamageType = 0;
    mVelocity.zero();
}

MarioSwimTilt::MarioSwimTilt(MarioSwim* swim) : MarioSwimModule(swim) {
    mTiltX = 0.0f;
    mTiltY = 0.0f;
    PSMTXIdentity(mJointMtx.toMtxPtr());
}

void MarioSwimSpin::startAnimation(u16 frontTime, u16 lockTime, const char* animation) {
    mSwim->mDashTimer = frontTime;
    mSwim->mActionLockTimer = lockTime;
    mSwim->mSwimYAccTimer = 50;
    mSwim->changeAnimation(animation);
}

void MarioSwim::fn_803F9D50() {
    u16 timer = mSinkTimer;
    if (timer != 0) {
        addVelocity(getGravityVec(), 0.25f * timer);
        mSinkTimer--;
    }
}

bool MarioSwim::fn_80401EC0() const {
    return !mIsSwimmingAtSurface && mIdleWaitTimer == 0 && mActionLockTimer != 0;
}

void MarioSwim::fn_80402040() {
    if (mVerticalAngle <= 1.57079637051f && mIdleWaitTimer > 30) {
        _1E = 1;
    }
}

void MarioSwim::fn_804020F0() {
    mIdleWaitTimer++;
    if (mIdleWaitTimer >= 120) {
        mIdleWaitTimer = 120;
    }
}

f32 MarioSwim::fn_80402160() const {
    f32 ratio = mIdleWaitTimer / 120.0f;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    }
    return ratio;
}

// NONMATCHING: equivalent multiply operand order.
f32 MarioSwim::fn_80402200() const {
    f32 speed = mForwardSpeed;
    if (speed < 2.0f) {
        return 1.0f;
    }
    if (speed > 10.0f) {
        return 0.0f;
    }
    return 1.0f - (0.125f * (speed - 2.0f));
}

void MarioSwim::fn_803F7C00() {
    mDamage->_15 = 1;
}

void MarioSwim::fn_803F7C10() {
    mDamage->mDamageType = 1;
}

void MarioSwim::fn_803F7C20(const TVec3f* normal) {
    mDamage->reflect(normal);
}

bool MarioSwim::fn_803FE9F0() const {
    return mSpin->isActive();
}

bool MarioSwim::fn_80400910() const {
    return mJet->mJetTimer != 0;
}

bool MarioSwim::fn_80400930() const {
    return mJet->mCooldownTimer != 0;
}

f32 MarioSwim::fn_803FA8E0() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontMaxSpeed;
    }
    return mActor->getConst().getTable()->mSwimFrontMaxSpeedYoshi;
}

f32 MarioSwim::fn_803FA950() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontSpinSpeed;
    }
    return mActor->getConst().getTable()->mSwimFrontSpinSpeedYoshi;
}

f32 MarioSwim::fn_803FA9C0() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontNormalSpeed;
    }
    return mActor->getConst().getTable()->mSwimFrontNormalSpeedYoshi;
}

f32 MarioSwim::fn_803FAA30() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontOnWaterSpeed;
    }
    return mActor->getConst().getTable()->mSwimFrontOnWaterSpeedYoshi;
}

f32 MarioSwim::fn_803FAAA0() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontJetSpeed;
    }
    return mActor->getConst().getTable()->mSwimFrontJetSpeedYoshi;
}

f32 MarioSwim::fn_803FAB10() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimFrontJetSpeedSlow;
    }
    return mActor->getConst().getTable()->mSwimFrontJetSpeedSlowYoshi;
}

f32 MarioSwim::fn_803FAB80() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimUpAcc;
    }
    return mActor->getConst().getTable()->mSwimUpAccYoshi;
}

f32 MarioSwim::fn_803FABF0() const {
    if (!getPlayer()->mMovementStates._37) {
        return mActor->getConst().getTable()->mSwimUpMaxSpeed;
    }
    return mActor->getConst().getTable()->mSwimUpMaxSpeedYoshi;
}

// NONMATCHING: target reloads the dash timer; remaining pooled offsets resolve equally.
void MarioSwimSpin::update() {
    if (mSpinTimer != 0) {
        if (mSwim->checkTrgA()) {
            mSpinTimer = 0;
            startDash();
            return;
        }
        if (mSpinTimer != 0) {
            mSwim->mForwardSpeed *= 0.95f;
            mSpinTimer--;
            if (mSpinTimer == 0) {
                startDash();
            }
        }
    }
    if (mSpinDashTimer != 0) {
        mSpinDashTimer = mSpinDashTimer - 1;
    }
    if (_E != 0) {
        bool active = mSpinTimer || mSpinDashTimer;
        if (!active) {
            _E--;
            mSwim->mActor->fn_803BE650(_E);
        }
    }
    if (!mSwim->isPlayerModeYoshi() && mSwim->mActor->isRequestSpin()) {
        bool active = mSpinTimer || mSpinDashTimer;
        if (!active && _E == 0) {
            if (mSpinTimer != 0) {
                mSpinTimer = getReadyTimer();
                mSwim->mForwardSpeed *= 0.5f;
            } else {
                mSpinTimer = 1;
            }
            mSwim->mDashTimer = 0;
            mSpinDashTimer = 0;
            _8 = 0;
            mSwim->stopAnimationUpper(nullptr);
            if (!mSwim->isOnWaterSurface()) {
                mSwim->changeAnimation("水泳スピン");
                mSwim->playSound("水中スピン開始");
            } else {
                mSwim->changeAnimation("水上スピン");
                mSwim->playSound("水面スピン開始");
            }
            mSwim->mActor->setPunchHitTimer(40);
            mSwim->playEffect("水泳スピン");
            _E = sSpinCooldownTime;
        }
    }
}

// NONMATCHING: pooled string offsets differ; all live arguments resolve equally.
void MarioSwimSpin::startDash() {
    mSwim->stopEffect("水泳スピン");
    if (_8 != 0) {
        mSwim->stopAnimation("水泳スピン");
        mSwim->stopAnimation("水上スピン");
        return;
    }
    if (mSwim->isOnWaterSurface()) {
        startAnimation(20, 30, "水上スピン移動");
        mSpinDashTimer = 45;
    } else {
        const MarioConstTable* table = mSwim->mActor->getConst().getTable();
        startAnimation(table->mSwimSpinFrontTime, table->mSwimSpinTime, "水泳スピン移動");
        mSpinDashTimer = mSwim->mActor->getConst().getTable()->mSwimDashTime;
    }
    mSwim->mActor->setPunchHitTimer(mSpinDashTimer);
    if (gIsLuigi) {
        mSwim->decOxygen(static_cast<u16>(static_cast<u32>(mSwim->mActor->getConst().getTable()->mOxygenMax) >> 4));
    }
}

u16 MarioSwimSpin::getReadyTimer() const {
    if (!mSwim->getPlayer()->mMovementStates._37) {
        return mSwim->mActor->getConst().getTable()->mSwimSpinMissOfsTime;
    }
    return mSwim->mActor->getConst().getTable()->mSwimSpinMissOfsTimeYoshi;
}

// NONMATCHING: equivalent multiply operands and pooled string offsets.
void MarioSwimJet::update(f32 factor) {
    MR::setStageBGMStateBit(4);
    if (mCooldownTimer != 0) {
        mCooldownTimer--;
    }
    f32 targetSpeed;
    if (factor != 1.0f) {
        targetSpeed = factor * mSwim->fn_803FA8E0();
        if (getMaxSpeed() > targetSpeed) {
            targetSpeed = getMaxSpeed();
        }
    } else {
        targetSpeed = getMaxSpeed();
    }
    if (mSwim->checkLvlZ() || isBackInput()) {
        if (mJetTimer == 1) {
            MR::emitEffect(mSwim->mActor->getCarrySensor()->mHost, "BrakeLamp");
        } else if (mSwim->checkTrgZ() || mSwim->mStateTimer < 2) {
            MR::emitEffect(mSwim->mActor->getCarrySensor()->mHost, "BrakeLamp");
            mSwim->playSound("亀ブレーキ");
        }
        mSwim->getPlayer()->_1C._9 = 1;
        f32 speed = mSwim->mForwardSpeed;
        if (speed > targetSpeed) {
            mSwim->mForwardSpeed = 0.95f * speed;
        } else if (speed > (0.1f * targetSpeed)) {
            mSwim->mForwardSpeed = 0.96f * speed;
        } else {
            updateVelocity(targetSpeed);
        }
    } else {
        mSwim->playSound("亀ジェット泳ぎ");
        MR::forceDeleteEffect(mSwim->mActor->getCarrySensor()->mHost, "BrakeLamp");
        updateVelocity(targetSpeed);
    }
    mJetTimer++;
}

f32 MarioSwimJet::calcRotationRatio() const {
    if (mJetTimer != 0) {
        if (_8 == 0) {
            return mSwim->mActor->getConst().getTable()->mSwimXJetRotRatio;
        }
        return mSwim->mActor->getConst().getTable()->mSwimXJetRotRatioRed;
    }
    return 1.0f;
}

f32 MarioSwimJet::getMaxSpeed() const {
    if (_8 == 0) {
        return mSwim->fn_803FAAA0();
    }
    return mSwim->fn_803FAB10();
}

// NONMATCHING: one equivalent multiply operand order.
void MarioSwimJet::updateVelocity(f32 targetSpeed) {
    f32 speed = mSwim->mForwardSpeed;
    if (speed < targetSpeed) {
        if (speed < 1.0f) {
            mSwim->mForwardSpeed = 1.0f;
        }
        mSwim->mForwardSpeed *= 1.02f;
        if (mSwim->mRingDash->mTimer != 0) {
            mSwim->mForwardSpeed *= 1.5f;
        }
        return;
    }
    if (speed > targetSpeed) {
        mSwim->mForwardSpeed = 0.98f * speed;
        return;
    }
    mSwim->mForwardSpeed = targetSpeed;
}

bool MarioSwimJet::isBackInput() const {
    if (!mSwim->getPlayer()->mMovementStates._37) {
        return false;
    }
    const TVec3f* upPtr;
    const TVec3f& pad = mSwim->getWorldPadDir();
    upPtr = &mSwim->mUpVec;
    const TVec3f& up = *upPtr;
    TVec3f direction;
    JMAVECScaleAdd(&up, &pad, &direction, -up.dot(pad));
    MR::normalizeOrZero(&direction);
    return mSwim->mFrontVec.dot(direction) < 0.0f;
}

void MarioSwim::updateJetAnimation() {
    if (!isAnimationRun("水泳ジェット")) {
        changeAnimation("水泳ジェット開始", "水泳ジェット");
        return;
    }
    changeAnimation(nullptr, "水泳ジェット");
}

// NONMATCHING: boolean register allocation and pooled string offsets.
bool MarioSwim::isSwimTurnAnimation() const {
    bool result = false;
    if (isAnimationRun("水泳ターン左") || isAnimationRun("水泳ターン右") || isAnimationRun("水泳ターン下")) {
        result = true;
    }
    return result;
}

// NONMATCHING: floating-point register allocation and operation scheduling.
void MarioSwimTilt::update() {
    f32 targetTiltX = 0.0f;
    f32 blendX = 0.1f;
    if (mSwim->isOnWaterSurface()) {
        if (mSwim->getStickP() > 0.1f) {
            MarioSwim* swim = mSwim;
            f32 dot = swim->mFrontVec.dot(swim->getWorldPadDir());
            f32 scale = MR::clamp(1.0f - dot, 0.0f, 1.0f);
            targetTiltX = scale * MR::abs((mSwim->getStickX() * JGeometry::TUtil<f32>::PI()) / 6.0f);
            TVec3f cross;
            PSVECCrossProduct(&mSwim->mFrontVec, &mSwim->getWorldPadDir(), &cross);
            if (cross.dot(mSwim->mUpVec) > 0.0f) {
                targetTiltX = -targetTiltX;
            }
            if (MR::abs(mTiltX) < MR::abs(targetTiltX)) {
                blendX = 0.1f;
            } else {
                blendX = 0.03f;
            }
        }
    } else {
        targetTiltX = (mSwim->getStickX() * JGeometry::TUtil<f32>::PI()) / 5.0f;
    }
    if (mSwim->isSwimTurnAnimation()) {
        targetTiltX = 0.125f * (MR::clamp(mSwim->getStickX() / 0.7f, -1.0f, 1.0f) * JGeometry::TUtil<f32>::PI());
    }
    f32 targetTiltY = 0.0f;
    if (!mSwim->isOnWaterSurface()) {
        if (mSwim->mIdleWaitTimer == 0) {
            targetTiltY = (mSwim->getStickY() * JGeometry::TUtil<f32>::PI()) / 12.0f;
            if (mSwim->getStickY() > 0.0f && mSwim->mVerticalAngle <= 2.09439516068f) {
                targetTiltY = 0.125f * (mSwim->getStickY() * JGeometry::TUtil<f32>::PI());
            }
        } else if (mSwim->getStickY() < 0.0f) {
            const MarioConstTable* table = mSwim->mActor->getConst().getTable();
            targetTiltY = (mSwim->getStickY() * JGeometry::TUtil<f32>::PI()) / table->mSwimTiltZup;
        } else if (mSwim->getStickY() > 0.0f) {
            const MarioConstTable* table = mSwim->mActor->getConst().getTable();
            targetTiltY = (mSwim->getStickY() * JGeometry::TUtil<f32>::PI()) / table->mSwimTiltZdown;
        } else {
            targetTiltY = targetTiltY - (0.314159274101f * mSwim->fn_80402160());
        }
    }
    mTiltX = (blendX * targetTiltX) + ((1.0f - blendX) * mTiltX);
    mTiltY = (0.1f * targetTiltY) + (0.9f * mTiltY);
    if (mSwim->getPlayer()->mMovementStates._37) {
        mTiltX = 0.0f;
        mTiltY = 0.0f;
    }
    Mtx rotationX;
    Mtx rotationZ;
    PSMTXRotRad(rotationX, 'X', mTiltX);
    PSMTXRotRad(rotationZ, 'Z', mTiltY);
    PSMTXConcat(rotationX, rotationZ, mJointMtx.toMtxPtr());
    MarioSwim* swim = mSwim;
    u16 joint = swim->getAnimator()->getUpperJointID();
    swim->setJointGlobalMtx(joint, mJointMtx.toMtxPtr());
}

bool MarioSwim::fn_803FA2E0() const {
    bool result = true;
    if (getPlayer()->mMovementStates.jumping) {
        if (!(getPlayer()->mVerticalSpeed <= 100.0f))
            result = false;
    }
    return result;
}

bool MarioSwim::fn_803FA350() const {
    bool result = false;
    if (mJet->mJetTimer && getPlayer()->mMovementStates.jumping && getPlayer()->_10.digitalJump)
        result = true;
    return result;
}

// NONMATCHING: equivalent early-return register assignment.
bool MarioSwim::fn_803FA3C0() const {
    if (_44 != 0)
        return true;
    if (fn_803FA350())
        return false;
    bool result = false;
    if (getPlayer()->mMovementStates.jumping && getPlayer()->mVerticalSpeed > 100.0f)
        result = true;
    return result;
}

// NONMATCHING: boolean register allocation.
bool MarioSwim::fn_803FB3D0() const {
    bool result = false;
    bool carried = false;
    if (mActor->_468 && mJet->mJetTimer == 0)
        carried = true;
    if (carried) {
        bool turtle = false;
        if (mActor->getCarrySensor()->isType(ATYPE_JET_TURTLE) || mActor->getCarrySensor()->isType(ATYPE_JET_TURTLE_SLOW))
            turtle = true;
        if (turtle)
            result = true;
    }
    return result;
}

void MarioSwim::fn_80401270() {
    mStickInputYInertia = 0.0f;
    mStickInputXInertia = 0.0f;
    mForwardSpeed = 0.0f;
    mSpin->reset();
    mDrownTimer = 0;
    mOxygen = mActor->getConst().getTable()->mOxygenMax;
    mOxygenDecreaseDelay = 0x78;
    mColdWaterDamageInterval = 0;
    mFloorContactTimer = 0;
    _40 = 0;
    mNextAction = EXIT_ACTION_NONE;
    mRingDash->reset();
    mIdleWaitTimer = 0;
    mEffectState = 0xFF;
    _9CFlag = 0;
    mDamage->reset();
    MarioSwimTilt* tilt = mTilt;
    tilt->mTiltY = 0.0f;
    tilt->mTiltX = 0.0f;
    mSurfacePaddleHoldTimer = 0;
    mWallStickTimer = 0;
    mStateTimer = 0;
    mSurfaceOffset = 0.0f;
    mWaveFlowTimer = 0;
    mWaterStreamVel.zero();
    mWaterStreamDir.zero();
    _118 = nullptr;
    _1F = false;
    _22 = false;
    if (isPlayerModeYoshi())
        mVerticalAngle = 0.785398185f;
    else
        mVerticalAngle = 0.52359879f;
    _1B = false;
    _1D = false;
    _1E = false;
    getPlayer()->mMovementStates._1 = false;
    mActionLockTimer = 0;
    mZSinkTimer = 0;
    mSinkTimer = 0;
    mJumpDiveTimer = 0;
    mDashTimer = 0;
    _38 = 0;
    mBuoyancyDelayTimer = 0;
    _1C = false;
}

void MarioSwim::fn_804013C0() {
    if (!mActor->_468)
        mJet->mJetTimer = 0;
}

void MarioSwim::fn_804013E0() {
    if (fn_803FA2E0() || isPlayerModeYoshi()) {
        mBuoyancySpeed = fn_803FABF0();
        return;
    }
    mBuoyancySpeed = 0.0f;
}

void MarioSwim::fn_80401440() {
    if (isAnimationRun("後方飛び込みジャンプ"))
        getPlayer()->setFrontVecKeepUp(-getFrontVec(), 1.0f);
}

void MarioSwim::fn_804014A0() {
    mDamage->mTimer = 0x5A;
    TVec3f velocity;
    MR::vecKillElement(mActor->getLastMove(), mUpVec, &velocity);
    velocity.scale(0.5f);
    mDamage->mVelocity.set(velocity);
}

void MarioSwim::fn_804015C0() {
    if (fn_803FB3D0()) {
        mIsSwimmingAtSurface = false;
        mVerticalAngle = 2.09439516f;
        startJet(mActor->getCarrySensor());
        return;
    }
    if (_44 == 2 || mJet->mJetTimer)
        updateJetAnimation();
}

void MarioSwim::fn_80401640() {
    updateLifeByTime();
    if (!getPlayer()->_1C._3)
        MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
}

bool MarioSwim::fn_804026A0() const {
    bool result = false;
    bool hemisphere = false;
    if (mWaterInfo.mWaterArea != nullptr && mWaterInfo.mWaterArea->isNotHemisphere())
        hemisphere = true;
    if (hemisphere && mWaterInfo.mWaterArea->getFollowMtx())
        result = true;
    return result;
}

// NONMATCHING: pooled string offsets differ; all live arguments resolve equally.
void MarioSwim::fn_80401160() {
    _44 = 0;
    if (isAnimationRun("飛び込みジャンプ"))
        _44 = 1;
    if (isAnimationRun("後方飛び込みジャンプ"))
        _44 = 1;
    if (mJet->mJetTimer && getPlayer()->mMovementStates.jumping && !getPlayer()->_10.digitalJump)
        _44 = 2;
    if (isAnimationRun("リングダッシュ"))
        _44 = 2;
    if (isAnimationRun("水泳スピン移動"))
        _44 = 3;
    if (isAnimationRun("水泳スピンジャンプ"))
        _44 = 3;
}

void MarioSwim::fn_803FA820() {
    if (getPlayer()->mMovementStates._37) {
        Mario* mario = getPlayer();
        TVec3f up(-getAirGravityVec());
        PSVECCrossProduct(&up, &mario->mSwimSideVec694, &mFrontVec);
        if (mFrontVec.dot(getFrontVec()) < 0.0f)
            mFrontVec.negate();
    } else {
        mFrontVec = getFrontVec();
        MR::normalize(&mFrontVec);
    }
}

// NONMATCHING: equivalent final multiply register and omitted move.
f32 MarioSwim::fn_803FB250() {
    TVec3f velocity;
    Mario* mario = getPlayer();
    MR::vecKillElement(mario->mJumpVec, getGravityVec(), &velocity);
    f32 speed = PSVECMag(&velocity);
    if (getFrontVec().dot(velocity) < 0.0f)
        speed = 0.0f;
    return MR::clamp(speed * 0.1f, 0.0f, 1.0f);
}

// NONMATCHING: FP rounding schedule and register allocation; live strings resolve equally.
void MarioSwim::fn_803FA450() {
    mStateTimer = 0;
    f32 factor = fn_803FB250();
    if (mSwimState == SWIM_STATE_SURFACE) {
        Mario* mario = getPlayer();
        f32 projection = -getAirGravityVec().dot(mario->mJumpVec);
        f32 speed = projection * 0.5f;
        mForwardSpeed = speed;
        if (speed < 0.1f)
            mForwardSpeed = 0.1f;
        else if (speed > 5.0f)
            mForwardSpeed = 5.0f;
        mVerticalAngle = 0.0f;
        mJumpDiveTimer = 0x3C;
        return;
    }
    if (getPlayer()->mMovementStates._B || _44 != 0) {
        mForwardSpeed = 10.0f;
        if (getPlayer()->mMovementStates._37)
            _38 = sDiveSideTimer;
        if (mJet->mJetTimer)
            changeAnimationNonStop("水泳ジェット");
        else if (!isPlayerModeYoshi())
            changeAnimation("水泳ジャンプダイブ回転");
        playEffect("水面Ｚ沈降");
        switch (_44) {
        case 0:
        case 1:
            mActionLockTimer = mActor->getConst().getTable()->mSwimSpecStartTimerA[0];
            mSinkTimer = mActor->getConst().getTable()->mSwimSpecStartTimerB[0];
            break;
        case 2:
            mActionLockTimer = mActor->getConst().getTable()->mSwimSpecStartTimerA[1];
            mSinkTimer = mActor->getConst().getTable()->mSwimSpecStartTimerB[1];
            break;
        case 3:
            mActionLockTimer = mActor->getConst().getTable()->mSwimSpecStartTimerA[2];
            mSinkTimer = mActor->getConst().getTable()->mSwimSpecStartTimerB[2];
            break;
        }
        mVerticalAngle = cLimitAngleSink * (1.0f - factor) + factor * 1.74532938f;
        const TVec3f& gravity = getAirGravityVec();
        getPlayer()->forceSetHeadVecKeepSide(gravity);
        return;
    }
    if (_9D == 4) {
        changeAnimation("水上ダメージ着水");
        mSinkTimer = mActor->getConst().getTable()->mJumpDamageSinkTimer;
        mDamage->mTimer = 0x78;
        return;
    }
    if (!isPlayerModeYoshi()) {
        mForwardSpeed = (0.2f + 0.1f * factor) * fn_803FA8E0();
        changeAnimation("水泳ジャンプダイブ");
        mActor->setBlendMtxTimer(8);
        mActionLockTimer = 0xF;
        mSinkTimer = mActor->getConst().getTable()->mJumpSinkTimer;
        mIsSwimmingAtSurface = true;
        mVerticalAngle = 1.49599659f;
        if (getPlayer()->mVerticalSpeed < 200.0f) {
            f32 ratio = getPlayer()->mVerticalSpeed / 200.0f;
            mActionLockTimer = mActionLockTimer * ratio;
            mSinkTimer = mSinkTimer * ratio;
            mActor->setBlendMtxTimer(8.0f * ratio);
        }
    }
}

void MarioSwim::fn_80401510() {
    if (fn_803FA350() || isPlayerModeYoshi())
        getPlayer()->mMovementStates.jumping = false;
    if (fn_803FA3C0() && !isPlayerModeYoshi()) {
        fn_803FA450();
        return;
    }
    if (mIsOnSurface)
        fn_804014A0();
}

void MarioSwim::fn_803FB300() {
    if (isPlayerModeYoshi()) {
        onSurface();
        return;
    }
    if (fn_803FB3D0()) {
        if (mIsOnSurface)
            mIsOnSurface = false;
    } else if (fn_803FA3C0()) {
        if (mIsOnSurface)
            mIsOnSurface = false;
    } else if (mWaterDepth < -200.0f || mActor->mSwimSurfaceState1010) {
        if (mIsOnSurface)
            mIsOnSurface = false;
    } else {
        onSurface();
    }
}

// NONMATCHING: temporary vectors occupy exchanged stack slots.
void MarioSwim::fn_80401010() {
    if (fn_804026A0()) {
        playEffect("水エリア飛び込み飛沫");
        return;
    }
    TVec3f velocity(fn_803EF300());
    velocity.scale(10.0f);
    TVec3f previous(mPreviousPos);
    previous.sub(velocity);
    if (MR::func_80062020(&mWaterInfo, &mPreviousPos, &previous)) {
        mActor->emitEffectWaterColumn(mSurfacePos, mSurfaceNorm);
        return;
    }
    if (MR::func_800621A0(&mWaterInfo, &mPreviousPos, &previous)) {
        MarioActor* actor = mActor;
        TVec3f normal(-mSurfacePos);
        f32 distance = mDistToFloor;
        TVec3f offset(mSurfacePos);
        offset.scale(distance);
        TVec3f position(mPreviousPos);
        position.sub(offset);
        actor->emitEffectWaterColumn(normal, position);
        return;
    }
    playEffect("水エリア飛び込み飛沫");
}

void MarioSwim::fn_803F8DA0() {
    if (_40) {
        playSound("水底接触", -1);
        if (--_40 == 0)
            stopEffect("水底接触");
    }
}

// NONMATCHING: constant-table register allocation.
bool MarioSwim::fn_803F8E10() {
    if (isPlayerModeYoshi())
        return false;
    if (mZSinkTimer == 0)
        return false;
    if (--mZSinkTimer != 0)
        return false;
    MarioActor* actor = mActor;
    mVerticalAngle = actor->getConst().getTable()->mZsinkAngleX;
    mActionLockTimer = actor->getConst().getTable()->mZsinkMoveTimer;
    mSinkTimer = actor->getConst().getTable()->mZsinkSinkTimer;
    mForwardSpeed = actor->getConst().getTable()->mZsinkFrontSpeed;
    _38 = 20;
    return true;
}

void MarioSwim::fn_803F9AC0(TVec3f* pose, const TVec3f& front, const TVec3f& side) {
    pose->set(getPlayer()->_1FC);
    if (mJumpDiveTimer || mSinkTimer || isAnimationRun("水泳ジャンプダイブ回転") || isAnimationRun("水泳潜り")) {
        PSVECCrossProduct(&front, &side, pose);
        const TVec3f& head = getPlayer()->_1FC;
        if (MR::normalizeOrZero(pose))
            pose->set(head);
    }
}

// NONMATCHING: temporary vector stack placement.
void MarioSwim::fn_803F9B90() {
    TVec3f* pose = &mPoseVec;
    if (!mJet->mJetTimer)
        fn_803FB570();
    f32 speed = mForwardSpeed;
    TVec3f velocity(*pose);
    velocity.scale(speed);
    if (mIsOnSurface && mUpVec.dot(*pose) > 0.0f) {
        f32 length = PSVECMag(&velocity);
        MR::vecKillElement(velocity, mUpVec, &velocity);
        velocity.setLength(length);
    }
    f32 verticalSpeed = MR::vecKillElement(velocity, mUpVec, &velocity);
    u16 timer = mSwimYAccTimer;
    f32 ratio;
    if (timer > 25)
        ratio = (50 - timer) / 25.0f;
    else
        ratio = timer / 25.0f;
    f32 scale = ratio + (1.0f - ratio) * mActor->getConst().getTable()->mSwimSpdYratio;
    TVec3f vertical(mUpVec);
    vertical.scale(verticalSpeed);
    TVec3f scaled(vertical);
    scaled.scale(scale);
    velocity.add(scaled);
    addVelocity(velocity);
}

void MarioSwim::fn_803F9DF0() {
    if (!mActionLockTimer && !mJet->mJetTimer) {
        procBuoyancy();
        return;
    }
    if (!_18 && !getPlayer()->mMovementStates._37 && mUpVec.dot(mPoseVec) < 0.0f && mIsOnSurface)
        mIsOnSurface = false;
}

// NONMATCHING: temporary vector stack placement.
void MarioSwim::fn_803F9E90() {
    if (!mIsOnSurface || mWaterInfo.mWaterArea == nullptr || !mWaterInfo.mWaterArea->isNotHemisphere() || !mWaterInfo.mWaterArea->getFollowMtx()) {
        _118 = nullptr;
        return;
    }
    TPos3f follow = *mWaterInfo.mWaterArea->getFollowMtx();
    TVec3f translation;
    MR::extractMtxTrans(follow, &translation);
    if (_118 == mWaterInfo.mWaterArea) {
        TVec3f movement(translation);
        movement.sub(mAreaFollowMtxPos);
        f32 distance = getAirGravityVec().dot(movement);
        TVec3f velocity(getAirGravityVec());
        velocity.scale(distance);
        addVelocity(velocity);
    }
    mAreaFollowMtxPos = translation;
    _118 = mWaterInfo.mWaterArea;
}

void MarioSwim::fn_803F9FC0() {
    MarioSwimDamage* damage = mDamage;
    if (damage->mTimer) {
        addVelocity(damage->mVelocity);
        mDamage->update();
    }
}

bool MarioSwim::fn_80401690() {
    if (mIsOnSurface) {
        if (!fn_803F8490())
            return false;
    } else if (mWaterDepth > -30.0f) {
        if (!fn_803F8930())
            return false;
    } else if (!fn_803F88C0()) {
        return false;
    }
    return true;
}

void MarioSwim::fn_80401710() {
    f32 stick = fn_803F8EF0();
    if (getPlayer()->mMovementStates._37) {
        fn_803F8F60(stick);
        return;
    }
    if (_38)
        _38--;
    else {
        fn_80402470(stick);
        fn_80401F00(stick);
        fn_80401F90();
        fn_804025C0();
        fn_803F9350();
    }
    fn_803F95A0(stick);
}

// NONMATCHING: temporary vector stack placement.
void MarioSwim::fn_804017C0(bool turn) {
    if (!MR::isNearZero(mFrontVec, 0.001f)) {
        TVec3f side;
        PSVECCrossProduct(&mUpVec, &mFrontVec, &side);
        MR::normalize(&side);
        TVec3f front;
        MR::rotAxisVecRad(mFrontVec, side, &front, mVerticalAngle);
        fn_803F9750(front, side, turn);
    }
}

// NONMATCHING: negative angle bound loaded as literal instead of negated register.
void MarioSwim::fn_803FA020() {
    if (mJet->mJetTimer)
        addVelocity(mWaterStreamVel, mActor->getConst().getTable()->mTurboReductionStream);
    else
        addVelocity(mWaterStreamVel);
    TVec3f stream(mWaterStreamVel);
    f32 speed = PSVECMag(&stream);
    if (!MR::normalizeOrZero(&stream)) {
        if (MR::diffAngleAbs(stream, mFrontVec) < 1.57079637f)
            MR::vecBlendSphere(&mFrontVec, mFrontVec, stream, 0.001f * speed);
        else
            MR::vecBlendSphere(&mFrontVec, mFrontVec, -stream, 0.001f * speed);
    }
    TVec3f whirlpool;
    if (MR::calcWhirlPoolAccelInfo(getTrans(), &whirlpool) && !MR::isNearZero(whirlpool, 0.001f)) {
        mWaterStreamVel.add(whirlpool);
        TVec3f direction(whirlpool);
        MR::normalizeOrZero(&direction);
        MR::vecBlendSphere(&mFrontVec, mFrontVec, direction, 0.1f);
    }
    if (!mJet->mJetTimer && !mIsOnSurface && !MR::isNearZero(mWaterStreamDir, 0.001f)) {
        f32 angle = MR::diffAngleSignedHorizontal(mWaterStreamDir, mFrontVec, getPlayer()->getAirGravityVec());
        if (angle > 1.04719758f) {
            Mtx rotation;
            PSMTXRotAxisRad(rotation, &mUpVec, angle - 1.04719758f);
            PSMTXMultVecSR(rotation, &mFrontVec, &mFrontVec);
            return;
        }
        if (angle < -1.04719758f) {
            Mtx rotation;
            PSMTXRotAxisRad(rotation, &mUpVec, -(-angle - 1.04719758f));
            PSMTXMultVecSR(rotation, &mFrontVec, &mFrontVec);
        }
    }
}

// NONMATCHING: equivalent branch folding and temporary vector stack placement.
bool MarioSwim::fn_803F7D90() {
    if (_22) {
        mNextAction = EXIT_ACTION_SLIDE;
        return false;
    }
    if (fn_803FD5E0(0))
        return true;
    if (mNextAction == EXIT_ACTION_JUMP || mNextAction == EXIT_ACTION_SLIDE)
        return false;
    fn_803F8060();
    switch (_204) {
    case 0:
        mBuoyancySpeed = 0.0f;
        mWaterDepth = 0.0f;
        break;
    case 1:
    case 2:
        mNextAction = EXIT_ACTION_FALL;
        return false;
    }
    if (mRingDash->mTimer && getPlayer()->mVerticalSpeed - mWaterDepth > 200.0f) {
        f32 speed = MR::clamp(mForwardSpeed / mActor->getConst().getTable()->mSwimToWalkSpd, 0.0f, 1.0f);
        getPlayer()->mMovementStates._5 = false;
        getPlayer()->mWalkSpeed = speed;
        TVec3f up(mUpVec);
        up.scale(10.0f);
        TVec3f velocity(mActor->getLastMove());
        velocity.scale(2.0f);
        TVec3f jump(velocity);
        jump.add(up);
        getPlayer()->tryForcePowerJump(jump, false);
        mNextAction = EXIT_ACTION_POWER_JUMP;
        changeAnimationNonStop("リングダッシュ");
        return false;
    }
    const TVec3f& translation = getTrans();
    TVec3f delta(mSurfaceNorm);
    delta.sub(translation);
    TVec3f horizontal;
    MR::vecKillElement(delta, getAirGravityVec(), &horizontal);
    if ((PSVECMag(&horizontal) > 100.0f || -mWaterDepth >= getPlayer()->mVerticalSpeed || mDamage->mTimer || mWallStickTimer)
        && !isAnimationRun("水泳ジャンプダイブ")) {
        if (mJet->mJetTimer) {
            doJetJump(1);
            return false;
        }
        mNextAction = EXIT_ACTION_FALL;
        return false;
    }
    return true;
}
