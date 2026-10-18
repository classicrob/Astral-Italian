#pragma once

#include <revolution.h>

class Mario;
class MarioActor;

/// @brief The table of player character constants.
struct MarioConstTable {
    /* 0x000 */ f32 mJumpHeight[9];  // SMG2 offset guess
    /* 0x024 */ f32 mGravityJumping[12];  // SMG2 offset guess
    /* 0x054 */ f32 mGravityAirWalk;  // SMG2 offset guess
    /* 0x058 */ f32 mGravityRatioA;  // SMG2 offset guess
    /* 0x05C */ f32 mJumpFrontSpeed;
    /* 0x060 */ f32 mJumpTornadoSpeed;  // SMG2 offset inh
    /* 0x064 */ f32 mDropFrontSpeed;  // SMG2 offset inh
    /* 0x068 */ s16 mJumpConnectTime;  // SMG2 offset inh
    /* 0x06A */ u8 _pad6A[0x2];
    /* 0x06C */ f32 mJumpConnectSpeed;  // SMG2 offset inh
    /* 0x070 */ f32 mSquatJumpFrontSpeed;
    /* 0x074 */ f32 mSquatJumpBackSpeed;
    /* 0x078 */ f32 mTurnJumpFrontSpeed;  // SMG2 offset inh
    /* 0x07C */ f32 mMaxDropSpeed[5];  // SMG2 offset inh
    /* 0x090 */ s16 mGrReductionDropTimer;  // SMG2 offset inh
    /* 0x092 */ u16 mDropWaitTime;  // SMG2 offset inh
    /* 0x094 */ f32 mJumpFrontReduction;  // SMG2 offset inh
    /* 0x098 */ s16 mJumpFrontReductionBeginTime;  // SMG2 offset inh
    /* 0x09A */ u8 _pad9A[0x2];
    /* 0x09C */ f32 mSquatJumpFrontReduction;  // SMG2 offset inh
    /* 0x0A0 */ s16 mSquatJumpFrontReductionBTime;  // SMG2 offset inh
    /* 0x0A2 */ u8 _padA2[0x2];
    /* 0x0A4 */ f32 mMudFloorJumpWeakRatio;  // SMG2 offset inh
    /* 0x0A8 */ f32 mGravityHipDrop;  // SMG2 offset inh
    /* 0x0AC */ f32 mLimitSpeedHipDrop;  // SMG2 offset inh
    /* 0x0B0 */ f32 mHipDropLimitHeight;  // SMG2 offset inh
    /* 0x0B4 */ u16 mHipDropZeroGrTime;  // SMG2 offset inh
    /* 0x0B6 */ s16 mHipDropLimitTimerAfterTornado;  // SMG2 offset inh
    /* 0x0B8 */ s16 mSlipUpSpdCtrlTimer;  // SMG2 offset inh
    /* 0x0BA */ u8 _padBA[0x2];
    /* 0x0BC */ f32 mSlipUpSpdRatio;  // SMG2 offset inh
    /* 0x0C0 */ f32 mSlipUpHeight;  // SMG2 offset inh
    /* 0x0C4 */ f32 mSlipUpFront;  // SMG2 offset inh
    /* 0x0C8 */ f32 mSlipUpFrontWeak;  // SMG2 offset inh
    /* 0x0CC */ f32 mSlipUpHeightHang;
    /* 0x0D0 */ f32 mSlipUpFrontHang;
    /* 0x0D4 */ f32 mSlipUpContinueHeight;  // SMG2 offset inh
    /* 0x0D8 */ u16 mAirWalkTime;
    /* 0x0DA */ u8 _padDA[0x2];
    /* 0x0DC */ f32 mAirWalkTimerFact1;  // SMG2 offset inh
    /* 0x0E0 */ s16 mAirWalkTimerFact2;  // SMG2 offset inh
    /* 0x0E2 */ u8 _padE2[0x2];
    /* 0x0E4 */ f32 mAirWalkTimerFact3;  // SMG2 offset inh
    /* 0x0E8 */ f32 mMaxJumpSpeed;  // SMG2 offset inh
    /* 0x0EC */ f32 mAirWalkBackBonus;  // SMG2 offset inh
    /* 0x0F0 */ f32 mAirWalkSpeedKiller;  // SMG2 offset inh
    /* 0x0F4 */ f32 mMaxBackJumpSpeed;  // SMG2 offset inh
    /* 0x0F8 */ f32 mAirWalkBonus;  // SMG2 offset inh
    /* 0x0FC */ s16 mBackJumpLimitFrame;  // SMG2 offset inh
    /* 0x0FE */ u8 _padFE[0x2];
    /* 0x100 */ f32 mBackJumpRatio;  // SMG2 offset inh
    /* 0x104 */ s16 mWaitNeutralTimer;  // SMG2 offset inh
    /* 0x106 */ u8 _pad106[0x2];
    /* 0x108 */ f32 mHardLandingHeight;  // SMG2 offset inh
    /* 0x10C */ u16 mHardLandStunTimer;  // SMG2 offset inh
    /* 0x10E */ u8 _pad10E[0x2];
    /* 0x110 */ f32 mCodeJumpPower[4];  // SMG2 offset inh
    /* 0x120 */ f32 mTrampleNormal;  // SMG2 offset inh
    /* 0x124 */ f32 mTrampleLong;  // SMG2 offset inh
    /* 0x128 */ f32 mTrampleNormalTaco;  // SMG2 offset inh
    /* 0x12C */ f32 mTrampleBegoma;  // SMG2 offset inh
    /* 0x130 */ f32 mTrampleBegomaRotRise;
    /* 0x134 */ f32 mTrampleBegomaRotFall;
    /* 0x138 */ u16 mTrampleBegomaOpenTime;  // SMG2 offset inh
    /* 0x13A */ u8 _pad13A[0x2];
    /* 0x13C */ f32 mDropUnderPowerMin;  // SMG2 offset inh
    /* 0x140 */ f32 mDropUnderPowerMax;  // SMG2 offset inh
    /* 0x144 */ f32 mMsgJumpGroundPowerXZ;  // SMG2 offset inh
    /* 0x148 */ f32 mMsgJumpGroundPowerY;  // SMG2 offset inh
    /* 0x14C */ f32 mMsgJumpAirPower;  // SMG2 offset inh
    /* 0x150 */ f32 mBilliardJumpExtra;  // SMG2 offset inh
    /* 0x154 */ s16 mFutureJumpReqLimitTime;
    /* 0x156 */ u8 _pad156[0x2];
    /* 0x158 */ f32 mWarpPodJumpX;  // SMG2 offset inh
    /* 0x15C */ f32 mWarpPodJumpY;  // SMG2 offset inh
    /* 0x160 */ f32 mJumpHeightDamage;  // SMG2 offset inh
    /* 0x164 */ f32 mJumpDistDamage;
    /* 0x168 */ f32 mGravityDamage;  // SMG2 offset inh
    /* 0x16C */ f32 mJumpHeightLargeDamage;  // SMG2 offset inh
    /* 0x170 */ f32 mJumpDistLargeDamage;  // SMG2 offset inh
    /* 0x174 */ f32 mSlideDistFaint;
    /* 0x178 */ s16 mFaintTimer1;
    /* 0x17A */ s16 mFaintTimer2;
    /* 0x17C */ f32 mFaintFriction1;
    /* 0x180 */ f32 mFaintFriction2;
    /* 0x184 */ f32 mSlideDistFlip;  // SMG2 offset inh
    /* 0x188 */ f32 mSlideDistFlipSpin;
    /* 0x18C */ s16 mFlipTimer1;
    /* 0x18E */ s16 mFlipTimer2;
    /* 0x190 */ f32 mFlipFriction1;
    /* 0x194 */ f32 mFlipFriction2;
    /* 0x198 */ f32 mFlipFriction3;
    /* 0x19C */ u16 mFireRunTimer1;
    /* 0x19E */ u16 mFireRunTimer2;
    /* 0x1A0 */ u16 mFireRunTimer3;
    /* 0x1A2 */ u8 _pad1A2[0x2];
    /* 0x1A4 */ f32 mFireRunSpeed;
    /* 0x1A8 */ f32 mFireRunTurnRatio;  // SMG2 offset inh
    /* 0x1AC */ f32 mFireRunGravity;
    /* 0x1B0 */ f32 mFireRunFirstJump;
    /* 0x1B4 */ f32 mFireDanceTurnRatio;  // SMG2 offset inh
    /* 0x1B8 */ f32 mFireDanceGravityRise;  // SMG2 offset inh
    /* 0x1BC */ f32 mFireDanceGravityDrop;  // SMG2 offset inh
    /* 0x1C0 */ f32 mFireDanceFirstJump;  // SMG2 offset inh
    /* 0x1C4 */ f32 mFireDanceSecondJump;  // SMG2 offset inh
    /* 0x1C8 */ f32 mFireDanceMoveSpeed;
    /* 0x1CC */ f32 mFireDanceMoveAcc;  // SMG2 offset inh
    /* 0x1D0 */ f32 mBlackHoleRadiusRatio;  // SMG2 offset inh
    /* 0x1D4 */ f32 mBlackHoleScaleSpeed;  // SMG2 offset inh
    /* 0x1D8 */ f32 mBlackHoleScaleLimit;  // SMG2 offset inh
    /* 0x1DC */ f32 mBlackHoleRotateSpeed;  // SMG2 offset inh
    /* 0x1E0 */ f32 mBlackHoleRotateLimit;  // SMG2 offset inh
    /* 0x1E4 */ f32 mBlackHoleFirstRadius;
    /* 0x1E8 */ u16 mBlackHoleHideTime;  // SMG2 offset inh
    /* 0x1EA */ u16 mWaterInnerFaintTime;  // SMG2 offset inh
    /* 0x1EC */ f32 mSwimFaintSpeed;  // SMG2 offset inh
    /* 0x1F0 */ u16 mWaterInnerDamageTime;  // SMG2 offset inh
    /* 0x1F2 */ u8 _pad1F2[0x2];
    /* 0x1F4 */ f32 mSwimDamageSpeed;  // SMG2 offset inh
    /* 0x1F8 */ f32 mSwimSurfaceDamageSpeed;  // SMG2 offset inh
    /* 0x1FC */ f32 mWaterSurfaceDamageJump;  // SMG2 offset inh
    /* 0x200 */ u16 mSwimDamageTimerEx1;  // SMG2 offset inh
    /* 0x202 */ u8 _pad202[0x2];
    /* 0x204 */ f32 mSwimDamageSpeedEx1;  // SMG2 offset inh
    /* 0x208 */ f32 mJumpHeightFlow;  // SMG2 offset inh
    /* 0x20C */ f32 mJumpDistFlow;  // SMG2 offset inh
    /* 0x210 */ f32 mGravityFlow;  // SMG2 offset inh
    /* 0x214 */ f32 mFlatAngle;
    /* 0x218 */ f32 mSlipAngle;
    /* 0x21C */ f32 mForceWallAngle;
    /* 0x220 */ f32 mDebugMoveSpeed;  // SMG2 offset guess
    /* 0x224 */ f32 mWalkSpeed;  // SMG2 offset guess
    /* 0x228 */ f32 mInertiaStandardStop;  // SMG2 offset guess
    /* 0x22C */ f32 mInertiaStandardMax;  // SMG2 offset guess
    /* 0x230 */ f32 mInertiaBdash;  // SMG2 offset guess
    /* 0x234 */ f32 mInertiaBdashAfter;  // SMG2 offset guess
    /* 0x238 */ f32 mInertiaStop;  // SMG2 offset guess
    /* 0x23C */ f32 mInertiaSquat;  // SMG2 offset guess
    /* 0x240 */ f32 mInertiaTurnSlip;  // SMG2 offset guess
    /* 0x244 */ f32 mInertiaTurning;  // SMG2 offset guess
    /* 0x248 */ f32 mInertiaJumpFinish;  // SMG2 offset guess
    /* 0x24C */ f32 mInertiaReflectSlip;  // SMG2 offset guess
    /* 0x250 */ f32 mInertiaStartSpin;  // SMG2 offset guess
    /* 0x254 */ f32 mInertiaOverSpeed;  // SMG2 offset guess
    /* 0x258 */ f32 mTurnAngleSpeed;  // SMG2 offset guess
    /* 0x25C */ f32 mTurnAngleSpeed2;  // SMG2 offset guess
    /* 0x260 */ f32 mTurnAngleSpeed3;  // SMG2 offset guess
    /* 0x264 */ f32 mTurnAngleSpeedSlowWalk;  // SMG2 offset guess
    /* 0x268 */ f32 mGravityStandard;  // SMG2 offset guess
    /* 0x26C */ f32 mGravityGrounding;  // SMG2 offset guess
    /* 0x270 */ f32 mDashMultiply;  // SMG2 offset guess
    /* 0x274 */ f32 mCameraAngleLimit;  // SMG2 offset guess
    /* 0x278 */ u16 mStartSpinTime;  // SMG2 offset guess
    /* 0x27A */ u16 mDashAfterTime;  // SMG2 offset guess
    /* 0x27C */ u16 mAutoBdashTime;  // SMG2 offset guess
    /* 0x27E */ u16 mSlowStartTime;  // SMG2 offset guess
    /* 0x280 */ f32 mStartSpinAnimeRatio;  // SMG2 offset guess
    /* 0x284 */ f32 mSlopeAnimeRatio;  // SMG2 offset guess
    /* 0x288 */ f32 mSlopeSpinAnimeRatio;  // SMG2 offset guess
    /* 0x28C */ u8 _pad28C[0x4];
    /* 0x290 */ f32 mItemDashRatio;
    /* 0x294 */ u16 mItemDashTimer;  // SMG2 offset inh
    /* 0x296 */ u8 _pad296[0x2];
    /* 0x298 */ f32 mInertiaIceStandardStop;
    /* 0x29C */ f32 mInertiaIceStandardMax;
    /* 0x2A0 */ f32 mInertiaIceStartSpin;
    /* 0x2A4 */ f32 mInertiaIceStop;
    /* 0x2A8 */ f32 mInertiaIceTurn;  // SMG2 offset inh
    /* 0x2AC */ f32 mInertiaSlipStandardStop;
    /* 0x2B0 */ f32 mInertiaSlipStandardMax;
    /* 0x2B4 */ f32 mInertiaSlipStartSpin;
    /* 0x2B8 */ f32 mInertiaSlipStop;
    /* 0x2BC */ f32 mInertiaSlipTurn;
    /* 0x2C0 */ f32 mTurnSlipAngle;  // SMG2 offset inh
    /* 0x2C4 */ u16 mTurnReadyTime;
    /* 0x2C6 */ u8 _pad2C6[0x2];
    /* 0x2C8 */ f32 mTurnSlipNeutral;  // SMG2 offset inh
    /* 0x2CC */ f32 mSlipSpeed;  // SMG2 offset inh
    /* 0x2D0 */ f32 mFastTurnSpeed;  // SMG2 offset inh
    /* 0x2D4 */ s16 mTurnSlipTime;  // SMG2 offset inh
    /* 0x2D6 */ s16 mTurnSlipTimeB;  // SMG2 offset inh
    /* 0x2D8 */ s16 mTurnJumpInhibitTime;  // SMG2 offset inh
    /* 0x2DA */ u8 _pad2DA[0x2];
    /* 0x2DC */ f32 mLandTurnHeight;  // SMG2 offset inh
    /* 0x2E0 */ f32 mInertiaBrake;  // SMG2 offset inh
    /* 0x2E4 */ u16 mStandingTurnTime;  // SMG2 offset inh
    /* 0x2E6 */ u16 mWeakTurnTime;  // SMG2 offset inh
    /* 0x2E8 */ u16 mBrakeFirstTimer;  // SMG2 offset inh
    /* 0x2EA */ u16 mBrakeSecondTimer;  // SMG2 offset inh
    /* 0x2EC */ f32 mSlopeSpeedMax;  // SMG2 offset inh
    /* 0x2F0 */ f32 mSlopeAccel;  // SMG2 offset inh
    /* 0x2F4 */ f32 mSlopeSpeedMaxBraking;  // SMG2 offset inh
    /* 0x2F8 */ f32 mSlopeFinishInertia;  // SMG2 offset inh
    /* 0x2FC */ f32 mSlopeCancelInertia;  // SMG2 offset inh
    /* 0x300 */ f32 mSlopeAnimeFinishSpeed;  // SMG2 offset inh
    /* 0x304 */ f32 mSlopeDashAngleFactor;  // SMG2 offset inh
    /* 0x308 */ f32 mSlopeDashSpeedFactor;  // SMG2 offset inh
    /* 0x30C */ f32 mSlopeDashAccelFactor;  // SMG2 offset inh
    /* 0x310 */ u16 mSlopeDashAccelTime;  // SMG2 offset inh
    /* 0x312 */ u8 _pad312[0x2];
    /* 0x314 */ f32 mSlopeSideMoveInertia;
    /* 0x318 */ f32 mSlopeSideStopInertia;
    /* 0x31C */ f32 mSlopeSideMoveSpeed;
    /* 0x320 */ f32 mSlopeCurveAssist;  // SMG2 offset inh
    /* 0x324 */ f32 mSlipMoveTurnAngleRad;  // SMG2 offset inh
    /* 0x328 */ f32 mWalkStepHeight;  // SMG2 offset inh
    /* 0x32C */ f32 mTiltRatio;  // SMG2 offset inh
    /* 0x330 */ f32 mLookDownRatio;  // SMG2 offset inh
    /* 0x334 */ f32 mSpeedStep[3];  // SMG2 offset inh
    /* 0x340 */ f32 mMaxAnmSpeedA;  // SMG2 offset inh
    /* 0x344 */ f32 mMinAnmSpeedA;  // SMG2 offset inh
    /* 0x348 */ f32 mMaxAnmSpeedB;  // SMG2 offset inh
    /* 0x34C */ f32 mMinAnmSpeedB;  // SMG2 offset inh
    /* 0x350 */ f32 mMaxAnmSpeedC;  // SMG2 offset inh
    /* 0x354 */ f32 mMinAnmSpeedC;  // SMG2 offset inh
    /* 0x358 */ f32 mWeightBlendRatio;  // SMG2 offset inh
    /* 0x35C */ f32 mMudFloorSlipRatio;  // SMG2 offset inh
    /* 0x360 */ f32 mStickAngleMargin;
    /* 0x364 */ f32 mStickMarginX;  // SMG2 offset inh
    /* 0x368 */ f32 mStickMarginY;  // SMG2 offset inh
    /* 0x36C */ f32 mStickMarginYstart;  // SMG2 offset inh
    /* 0x370 */ f32 mStickMarginXstart;  // SMG2 offset inh
    /* 0x374 */ f32 mStickHeavyMinRatio;  // SMG2 offset inh
    /* 0x378 */ f32 mStickHeavyMinAngle;  // SMG2 offset inh
    /* 0x37C */ f32 mStickHeavyMaxAngle;  // SMG2 offset inh
    /* 0x380 */ f32 mSpeedSquatWalkLower;  // SMG2 offset inh
    /* 0x384 */ f32 mSquatWalkStep;  // SMG2 offset inh
    /* 0x388 */ f32 mSquatWalkMaxSpeed;  // SMG2 offset inh
    /* 0x38C */ f32 mSquatWalkMinSpeed;  // SMG2 offset inh
    /* 0x390 */ f32 mWindSlideLimit;
    /* 0x394 */ f32 mWindSlideFriction;
    /* 0x398 */ f32 mWindForwardFriction;
    /* 0x39C */ f32 mWindSlideFrictionAgainst;
    /* 0x3A0 */ f32 mWindJumpingFriction;
    /* 0x3A4 */ u16 mTornadoTime;  // SMG2 offset inh
    /* 0x3A6 */ u16 mTornadoTimeAir;  // SMG2 offset inh
    /* 0x3A8 */ u16 mAirWalkTimeTornado;  // SMG2 offset inh
    /* 0x3AA */ u16 mTornadoRestartTime;
    /* 0x3AC */ f32 mSpinDifferMargin;  // SMG2 offset inh
    /* 0x3B0 */ f32 mSpinGoalAngle;  // SMG2 offset inh
    /* 0x3B4 */ s16 mSpinInputLimitTime;  // SMG2 offset inh
    /* 0x3B6 */ s16 mSpinTime;  // SMG2 offset inh
    /* 0x3B8 */ f32 mSpinSpeed;  // SMG2 offset inh
    /* 0x3BC */ f32 mSpinWalkRatio;  // SMG2 offset inh
    /* 0x3C0 */ f32 mSpinJumpRatio;  // SMG2 offset inh
    /* 0x3C4 */ f32 mSpinWallReboundSpeed;  // SMG2 offset inh
    /* 0x3C8 */ u16 mSpinFinishTime;  // SMG2 offset inh
    /* 0x3CA */ u8 _pad3CA[0x2];
    /* 0x3CC */ f32 mSpinFinishRate;  // SMG2 offset inh
    /* 0x3D0 */ f32 mTornadoMultiply;  // SMG2 offset inh
    /* 0x3D4 */ f32 mTurnAngleSpeedTornado;  // SMG2 offset inh
    /* 0x3D8 */ f32 mInertiaTornadoAccel;  // SMG2 offset inh
    /* 0x3DC */ f32 mInertiaTornadoBrake;  // SMG2 offset inh
    /* 0x3E0 */ u16 mTornadoChargeTime;  // SMG2 offset inh
    /* 0x3E2 */ u16 mTornadoChargeUpTime;  // SMG2 offset inh
    /* 0x3E4 */ u16 mTornadoZeroGravityTimer;  // SMG2 offset inh
    /* 0x3E6 */ u8 _pad3E6[0x2];
    /* 0x3E8 */ f32 mTornadoBoostPower;  // SMG2 offset inh
    /* 0x3EC */ f32 mTornadoBoostAttn;  // SMG2 offset inh
    /* 0x3F0 */ u16 mTornadoBoostTimer;  // SMG2 offset inh
    /* 0x3F2 */ u8 _pad3F2[0x2];
    /* 0x3F4 */ f32 mTornadoBoostAttnMini;  // SMG2 offset inh
    /* 0x3F8 */ u16 mTornadoBoostTimerMini;  // SMG2 offset inh
    /* 0x3FA */ u8 _pad3FA[0x2];
    /* 0x3FC */ f32 mTornadoTiltCancel;  // SMG2 offset inh
    /* 0x400 */ f32 mTornadoTiltAngle;  // SMG2 offset inh
    /* 0x404 */ f32 mTornadoTiltSpeed;  // SMG2 offset inh
    /* 0x408 */ f32 mTornadoTiltOffSpeed;  // SMG2 offset inh
    /* 0x40C */ f32 mTornadoTiltNear;  // SMG2 offset inh
    /* 0x410 */ u16 mTornadoReflectTimer;  // SMG2 offset inh
    /* 0x412 */ u8 _pad412[0x2];
    /* 0x414 */ f32 mTornadoReflectSpeed;  // SMG2 offset inh
    /* 0x418 */ u16 mAirWalkTimeSpin;  // SMG2 offset inh
    /* 0x41A */ u8 _pad41A[0x2];
    /* 0x41C */ f32 mSpinCoinPullRadius;  // SMG2 offset inh
    /* 0x420 */ f32 mCoinPullAngleSpeedRatio;  // SMG2 offset inh
    /* 0x424 */ f32 mCoinPullDistSpeedRatio;  // SMG2 offset inh
    /* 0x428 */ u16 mSpinJumpCount;  // SMG2 offset inh
    /* 0x42A */ u16 mSpinIntervalTime;  // SMG2 offset inh
    /* 0x42C */ f32 mSpinJumpHeight;  // SMG2 offset inh
    /* 0x430 */ f32 mSpinJumpGravity;  // SMG2 offset inh
    /* 0x434 */ f32 mWallStickGrHeight;  // SMG2 offset inh
    /* 0x438 */ f32 mWallStickFrHeight;  // SMG2 offset inh
    /* 0x43C */ f32 mWallStickStepHeight;  // SMG2 offset inh
    /* 0x440 */ f32 mWallDropSpeedStop;  // SMG2 offset inh
    /* 0x444 */ f32 mWallDropSpeedNormal;  // SMG2 offset inh
    /* 0x448 */ f32 mWallJumpPowerXZ;
    /* 0x44C */ f32 mWallJumpPowerY;
    /* 0x450 */ f32 mWallBackJumpPowerXZ;  // SMG2 offset inh
    /* 0x454 */ f32 mWallBackJumpPowerY;  // SMG2 offset inh
    /* 0x458 */ s16 mWallStickTime;  // SMG2 offset inh
    /* 0x45A */ s16 mWallStickTimeIce;  // SMG2 offset inh
    /* 0x45C */ s16 mWallReleaseTime;  // SMG2 offset inh
    /* 0x45E */ u8 _pad45E[0x2];
    /* 0x460 */ f32 mWallSideMoveRatio;  // SMG2 offset inh
    /* 0x464 */ f32 mJumpHeightBlown;  // SMG2 offset inh
    /* 0x468 */ f32 mGravityBlown;  // SMG2 offset inh
    /* 0x46C */ f32 mWallHangGrHeight;  // SMG2 offset inh
    /* 0x470 */ f32 mWallHangMyHeight;  // SMG2 offset inh
    /* 0x474 */ s16 mHangBlendTime;
    /* 0x476 */ u8 _pad476[0x2];
    /* 0x478 */ f32 mWallPushAngleRange;  // SMG2 offset inh
    /* 0x47C */ f32 mWallFrontAngleRange;  // SMG2 offset inh
    /* 0x480 */ f32 mWallBackAngleRange;  // SMG2 offset inh
    /* 0x484 */ f32 mWallStickCancelAngle;
    /* 0x488 */ f32 mWallBackHangStickPower;  // SMG2 offset inh
    /* 0x48C */ f32 mWallBackHangWalkSpeed;  // SMG2 offset inh
    /* 0x490 */ f32 mWallTriJumpMargin;
    /* 0x494 */ f32 mWallSpinFlipGround;
    /* 0x498 */ f32 mWallSpinHopGround;
    /* 0x49C */ f32 mWallSpinFlipAirRatio;
    /* 0x4A0 */ u16 mClapCoinTime;  // SMG2 offset inh
    /* 0x4A2 */ u8 _pad4A2[0x2];
    /* 0x4A4 */ f32 mClapCoinPullRate;  // SMG2 offset inh
    /* 0x4A8 */ f32 mClapJumpBonusRising;  // SMG2 offset inh
    /* 0x4AC */ f32 mClapJumpBonusFalling;  // SMG2 offset inh
    /* 0x4B0 */ f32 mHopLimit;  // SMG2 offset inh
    /* 0x4B4 */ f32 mSwimFrontAcc;  // SMG2 offset inh
    /* 0x4B8 */ f32 mSwimFrontMaxSpeed;
    /* 0x4BC */ f32 mSwimFrontSpinSpeed;  // SMG2 offset guess
    /* 0x4C0 */ f32 mSwimFrontNormalSpeed;  // SMG2 offset guess
    /* 0x4C4 */ f32 mSwimFrontOnWaterSpeed;  // SMG2 offset guess
    /* 0x4C8 */ f32 mSwimFrontJetSpeed;  // SMG2 offset guess
    /* 0x4CC */ f32 mSwimFrontJetSpeedSlow;  // SMG2 offset guess
    /* 0x4D0 */ f32 mSwimFrontAccYoshi;
    /* 0x4D4 */ f32 mSwimFrontMaxSpeedYoshi;
    /* 0x4D8 */ f32 mSwimFrontSpinSpeedYoshi;
    /* 0x4DC */ f32 mSwimFrontNormalSpeedYoshi;
    /* 0x4E0 */ f32 mSwimFrontOnWaterSpeedYoshi;
    /* 0x4E4 */ f32 mSwimFrontJetSpeedYoshi;
    /* 0x4E8 */ f32 mSwimFrontJetSpeedSlowYoshi;
    /* 0x4EC */ f32 mSwimAccRatio;  // SMG2 offset guess
    /* 0x4F0 */ f32 mSwimAccMinRatio;  // SMG2 offset guess
    /* 0x4F4 */ f32 mSwimFrontIne;  // SMG2 offset guess
    /* 0x4F8 */ f32 mSwimStopIne;  // SMG2 offset guess
    /* 0x4FC */ f32 mSwimStopIneSurface;  // SMG2 offset guess
    /* 0x500 */ f32 mSwimRotSpeedX;  // SMG2 offset guess
    /* 0x504 */ f32 mSwimRotSpeedZ;  // SMG2 offset guess
    /* 0x508 */ f32 mSwimRotSpeedZStop;  // SMG2 offset guess
    /* 0x50C */ f32 mSwimRotXIne;  // SMG2 offset guess
    /* 0x510 */ f32 mSwimRotZIne;  // SMG2 offset guess
    /* 0x514 */ f32 mSwimRotXIneT;  // SMG2 offset guess
    /* 0x518 */ f32 mSwimRotZIneT;  // SMG2 offset guess
    /* 0x51C */ f32 mSwimUpAcc;  // SMG2 offset guess
    /* 0x520 */ f32 mSwimUpMaxSpeed;  // SMG2 offset guess
    /* 0x524 */ f32 mSwimUpAccYoshi;
    /* 0x528 */ f32 mSwimUpMaxSpeedYoshi;
    /* 0x52C */ f32 mWeightBlendRatioSwim;  // SMG2 offset guess
    /* 0x530 */ f32 mWeightBlendRatioSwimB;  // SMG2 offset guess
    /* 0x534 */ f32 mSwimFreeXback;  // SMG2 offset guess
    /* 0x538 */ f32 mSwimFreeXback2;  // SMG2 offset guess
    /* 0x53C */ f32 mSwimSpdYratio;  // SMG2 offset guess
    /* 0x540 */ f32 mSwimRotSpeedRatioSurface;  // SMG2 offset guess
    /* 0x544 */ u16 mJumpSinkTimer;  // SMG2 offset guess
    /* 0x546 */ u16 mJumpDamageSinkTimer;  // SMG2 offset guess
    /* 0x548 */ f32 mSwimToWalkSpd;  // SMG2 offset guess
    /* 0x54C */ u16 mWaterDecInterval;  // SMG2 offset guess
    /* 0x54E */ u16 mWaterDecSpinAcc;  // SMG2 offset guess
    /* 0x550 */ u16 mOxygenMax;  // SMG2 offset guess
    /* 0x552 */ u8 _pad52E[0x2];
    /* 0x554 */ f32 mSwimXJetRotRatio;  // SMG2 offset guess
    /* 0x558 */ f32 mSwimXJetRotRatioRed;  // SMG2 offset guess
    /* 0x55C */ f32 mTurboReductionStream;  // SMG2 offset guess
    /* 0x560 */ f32 mTurboReductionHitWall;  // SMG2 offset guess
    /* 0x564 */ f32 mTurboReductionHitFloor;  // SMG2 offset guess
    /* 0x568 */ u16 mSwimJetPeriod1;  // SMG2 offset guess
    /* 0x56A */ u16 mSwimJetPeriod2;  // SMG2 offset guess
    /* 0x56C */ f32 mSwimJetPeriod1Ratio;  // SMG2 offset guess
    /* 0x570 */ f32 mSwimJetPeriod3Ratio;  // SMG2 offset guess
    /* 0x574 */ f32 mZsinkAngleX;  // SMG2 offset guess
    /* 0x578 */ u16 mZsinkMoveTimer;  // SMG2 offset guess
    /* 0x57A */ u16 mZsinkSinkTimer;  // SMG2 offset guess
    /* 0x57C */ u16 mZsinkStartTimer;  // SMG2 offset guess
    /* 0x57E */ u8 _pad55A[0x2];
    /* 0x580 */ f32 mZsinkFrontSpeed;  // SMG2 offset guess
    /* 0x584 */ u16 mSwimSpecStartTimerA[3];  // SMG2 offset guess
    /* 0x58A */ u16 mSwimSpecStartTimerB[3];  // SMG2 offset guess
    /* 0x590 */ u16 mSwimSpinReadyTime;  // SMG2 offset guess
    /* 0x592 */ u16 mSwimSpinTime;  // SMG2 offset guess
    /* 0x594 */ u16 mSwimDashTime;  // SMG2 offset guess
    /* 0x596 */ u16 mSwimSpinFrontTime;  // SMG2 offset guess
    /* 0x598 */ u16 mSwimSpinYspeedTime;  // SMG2 offset guess
    /* 0x59A */ u16 mSwimSpinDownStep;  // SMG2 offset guess
    /* 0x59C */ f32 mSwimSpinSpeed;  // SMG2 offset guess
    /* 0x5A0 */ f32 mSwimSpinAcc1;  // SMG2 offset guess
    /* 0x5A4 */ f32 mSwimSpinAcc2;  // SMG2 offset guess
    /* 0x5A8 */ u16 mSwimSpinMissOfsTime;  // SMG2 offset guess
    /* 0x5AA */ u16 mSwimSpinMissOfsTimeYoshi;
    /* 0x5AC */ f32 mSwimSpinAccRatio;  // SMG2 offset guess
    /* 0x5B0 */ f32 mSwimSpinSurfaceAccRatio;  // SMG2 offset guess
    /* 0x5B4 */ f32 mSwimSpinAccRatioR;  // SMG2 offset guess
    /* 0x5B8 */ f32 mSwimSpinSurfaceAccRatioR;  // SMG2 offset guess
    /* 0x5BC */ u16 mSwimRingDashChargeTime;  // SMG2 offset guess
    /* 0x5BE */ u16 mSwimRingDashTime;  // SMG2 offset guess
    /* 0x5C0 */ u16 mSwimRingDashFinishTime;  // SMG2 offset guess
    /* 0x5C2 */ u8 _pad59E[0x2];
    /* 0x5C4 */ f32 mSwimRingDashSpeedRatio;  // SMG2 offset guess
    /* 0x5C8 */ f32 mSwimSurfaceSpeed;  // SMG2 offset guess
    /* 0x5CC */ f32 mSwimWaterSpeed;  // SMG2 offset guess
    /* 0x5D0 */ f32 mSwimDriftSpeed;  // SMG2 offset guess
    /* 0x5D4 */ u16 mSwimConnectIn;  // SMG2 offset guess
    /* 0x5D6 */ u16 mSwimConnectOut;  // SMG2 offset guess
    /* 0x5D8 */ u8 _pad5D8[8];
    /* 0x5E0 */ f32 mSwimTiltZup;
    /* 0x5E4 */ f32 mSwimTiltZdown;
    /* 0x5E8 */ f32 mSwimTiltReal;  // SMG2 offset inh
    /* 0x5EC */ f32 mSwimTiltSpd;  // SMG2 offset inh
    /* 0x5F0 */ f32 mSwimUpSurfaceDist;  // SMG2 offset inh
    /* 0x5F4 */ f32 mSwimUpStrongDist;  // SMG2 offset inh
    /* 0x5F8 */ f32 mSwimUpStrongRatio;  // SMG2 offset inh
    /* 0x5FC */ f32 mSwimUpWeakDist;  // SMG2 offset inh
    /* 0x600 */ f32 mSwimUpBottomDist;  // SMG2 offset inh
    /* 0x604 */ f32 mSwimUpWeakRatio;  // SMG2 offset inh
    /* 0x608 */ f32 mSwimReverseSinkRatio;  // SMG2 offset inh
    /* 0x60C */ f32 mSliderBrakeIne;
    /* 0x610 */ f32 mSliderWeightIne;
    /* 0x614 */ f32 mSliderSlopePow;
    /* 0x618 */ f32 mSliderWeightPow;
    /* 0x61C */ f32 mSliderMaxSpeed;
    /* 0x620 */ f32 mSliderBrakePow;
    /* 0x624 */ f32 mSliderFrontTurnRatio;
    /* 0x628 */ f32 mSliderHeadRotateRatio;  // SMG2 offset inh
    /* 0x62C */ f32 mSliderTiltRatio;  // SMG2 offset inh
    /* 0x630 */ f32 mBeeFlyRandomFactor;  // SMG2 offset inh
    /* 0x634 */ f32 mBeeFlyConstantFactor;  // SMG2 offset inh
    /* 0x638 */ f32 mBeeAirWalkAcc;  // SMG2 offset inh
    /* 0x63C */ f32 mBeeAirWalkLimit;  // SMG2 offset inh
    /* 0x640 */ f32 mBeeAirWalkTurnSpd;  // SMG2 offset inh
    /* 0x644 */ f32 mBeeSpeedRotateRatio;  // SMG2 offset inh
    /* 0x648 */ f32 mBeeFreeDropAcc;  // SMG2 offset inh
    /* 0x64C */ f32 mBeeFreeDropMaxSpd;  // SMG2 offset inh
    /* 0x650 */ f32 mBeePushRiseGravityEraser;  // SMG2 offset inh
    /* 0x654 */ f32 mBeeStickJumpPower;  // SMG2 offset inh
    /* 0x658 */ f32 mBeeStickJumpBonus;  // SMG2 offset inh
    /* 0x65C */ f32 mBeeStickAngleLimit;  // SMG2 offset inh
    /* 0x660 */ f32 mBeeUpSpeedMax;  // SMG2 offset inh
    /* 0x664 */ f32 mBeeUpAccelRatio;  // SMG2 offset inh
    /* 0x668 */ f32 mBeeAccelRatio;  // SMG2 offset inh
    /* 0x66C */ f32 mBeeUpDownKiller;  // SMG2 offset inh
    /* 0x670 */ u16 mBeeGravityReviveTime;  // SMG2 offset inh
    /* 0x672 */ u16 mBeeGravityPowerTime;  // SMG2 offset inh
    /* 0x674 */ u16 mBeeAirWalkInhibitTime;  // SMG2 offset inh
    /* 0x676 */ u16 mBeeGravityPowerTimeV;  // SMG2 offset inh
    /* 0x678 */ u16 mBeeAirWalkInhibitTimeV;  // SMG2 offset inh
    /* 0x67A */ u16 mBeeGravityPowerTimeD;
    /* 0x67C */ u16 mBeeAirWalkInhibitTimeD;
    /* 0x67E */ u8 _pad67E[0x2];
    /* 0x680 */ f32 mBeeWallWalkCancelRadius;  // SMG2 offset inh
    /* 0x684 */ f32 mBeeWallWalkCancelRadiusShort;  // SMG2 offset inh
    /* 0x688 */ f32 mBeePoseDelayAngleAir;  // SMG2 offset inh
    /* 0x68C */ f32 mBeePoseDelayAngleGround;  // SMG2 offset inh
    /* 0x690 */ f32 mBeePoseDelayAccel;  // SMG2 offset inh
    /* 0x694 */ f32 mBeePoseLimitAngleAir;  // SMG2 offset inh
    /* 0x698 */ f32 mBeePoseLimitAngleGround;  // SMG2 offset inh
    /* 0x69C */ f32 mBeePoseHeadToFootLength;  // SMG2 offset inh
    /* 0x6A0 */ f32 mBeePoseDelaySpeedLimit;  // SMG2 offset inh
    /* 0x6A4 */ f32 mBeePoseFrictionStop;  // SMG2 offset inh
    /* 0x6A8 */ f32 mBeePoseTransBlendingRatioStop;  // SMG2 offset inh
    /* 0x6AC */ f32 mBeePoseFrictionMove;  // SMG2 offset inh
    /* 0x6B0 */ f32 mBeePoseTransBlendingRatioMove;  // SMG2 offset inh
    /* 0x6B4 */ f32 mRabbitTurnRatio;  // SMG2 offset inh
    /* 0x6B8 */ f32 mRabbitTurnRatio2;  // SMG2 offset inh
    /* 0x6BC */ f32 mRabbitGravityRise;  // SMG2 offset inh
    /* 0x6C0 */ f32 mRabbitGravityDrop;  // SMG2 offset inh
    /* 0x6C4 */ f32 mRabbitFirstJump;
    /* 0x6C8 */ f32 mRabbitMoveSpeed;  // SMG2 offset inh
    /* 0x6CC */ f32 mRabbitMoveAcc;  // SMG2 offset inh
    /* 0x6D0 */ f32 mRabbitMoveAcc2;  // SMG2 offset inh
    /* 0x6D4 */ f32 mRabbitMoveAcc3;  // SMG2 offset inh
    /* 0x6D8 */ f32 mRabbitFirstJump2;  // SMG2 offset inh
    /* 0x6DC */ f32 mRabbitGravityRise2;  // SMG2 offset inh
    /* 0x6E0 */ f32 mRabbitGravityDrop2;  // SMG2 offset inh
    /* 0x6E4 */ u16 mRabbitChargeTime2;  // SMG2 offset inh
    /* 0x6E6 */ u16 mHopperLandingTime;  // SMG2 offset inh
    /* 0x6E8 */ f32 mTeresaHorizontalSpeedMax;
    /* 0x6EC */ f32 mTeresaAlphaLevelMax;  // SMG2 offset inh
    /* 0x6F0 */ f32 mTeresaAlphaLevelInc;  // SMG2 offset inh
    /* 0x6F4 */ f32 mTeresaAlphaLevelDec;  // SMG2 offset inh
    /* 0x6F8 */ u16 mTeresaWallThroughTime;
    /* 0x6FA */ u8 _pad6FA[0x2];
    /* 0x6FC */ f32 mTeresaAngleUp;  // SMG2 offset inh
    /* 0x700 */ f32 mTeresaAngleDown;  // SMG2 offset inh
    /* 0x704 */ f32 mTeresaWallReflectPower;  // SMG2 offset inh
    /* 0x708 */ f32 mTeresaWallReflectReduction;
    /* 0x70C */ u16 mTeresaWallReflectTime;  // SMG2 offset inh
    /* 0x70E */ u8 _pad70E[0x2];
    /* 0x710 */ f32 mTeresaWaitHeight;
    /* 0x714 */ f32 mTeresaDropDownHeight;  // SMG2 offset inh
    /* 0x718 */ f32 mTeresaBodyRadius;  // SMG2 offset inh
    /* 0x71C */ f32 mTeresaAirWalkTurnSpd;  // SMG2 offset inh
    /* 0x720 */ f32 mTeresaDropSpeedMax;
    /* 0x724 */ f32 mTeresaRiseSpeedMax;
    /* 0x728 */ f32 mTeresaDropBase;  // SMG2 offset inh
    /* 0x72C */ f32 mTeresaUpInertia;  // SMG2 offset inh
    /* 0x730 */ u16 mTeresaAccelTime;
    /* 0x732 */ u16 mTeresaTrgOnPushTime1;
    /* 0x734 */ u16 mTeresaTrgOnPushTime2;
    /* 0x736 */ u8 _pad736[0x2];
    /* 0x738 */ f32 mTeresaOnePushAccel1;  // SMG2 offset inh
    /* 0x73C */ f32 mTeresaOnePushAccel2;  // SMG2 offset inh
    /* 0x740 */ f32 mTeresaNoPushDownRatio;  // SMG2 offset inh
    /* 0x744 */ f32 mTeresaRisingBrake;  // SMG2 offset inh
    /* 0x748 */ f32 mTeresaWindMovingBrake1;  // SMG2 offset inh
    /* 0x74C */ f32 mTeresaWindMovingBrake2;  // SMG2 offset inh
    /* 0x750 */ f32 mTeresaWindMovingBrake3;  // SMG2 offset inh
    /* 0x754 */ f32 mSilhouetteZoffset;  // SMG2 offset inh
    /* 0x758 */ u8 mRibbonColor[4];  // SMG2 offset inh
    /* 0x75C */ u8 mDamageFogLow;  // SMG2 offset inh
    /* 0x75D */ u8 mDamageFogHigh;  // SMG2 offset inh
    /* 0x75E */ u8 _pad75E[0x2];
    /* 0x760 */ f32 mStarPieceFogLevel;  // SMG2 offset inh
    /* 0x764 */ u8 mStarPieceFogTime;
    /* 0x765 */ u8 mSearchLightAlphaLevel;  // SMG2 offset guess
    /* 0x766 */ u8 mSearchLightBlurAlpha[3];  // SMG2 offset guess
    /* 0x769 */ u8 _pad769[0x3];
    /* 0x76C */ f32 mSearchLightBlurScale[3];  // SMG2 offset guess
    /* 0x778 */ u8 mSearchLightColor[4];  // SMG2 offset guess
    /* 0x77C */ f32 mRasterV;  // SMG2 offset guess
    /* 0x780 */ s32 mRasterSpeed;  // SMG2 offset guess
    /* 0x784 */ f32 mRasterRange;  // SMG2 offset guess
    /* 0x788 */ u16 mRacketHoldTime;  // SMG2 offset guess
    /* 0x78A */ u16 mTornadoHoldTime;  // SMG2 offset guess
    /* 0x78C */ u16 mMetalHoldTime;  // SMG2 offset guess
    /* 0x78E */ u16 mFireModeTime;  // SMG2 offset guess
    /* 0x790 */ u16 mIceModeTime;  // SMG2 offset guess
    /* 0x792 */ u8 _pad792[0x6];
    /* 0x798 */ f32 mHeadAngleFixMargin;
    /* 0x79C */ f32 mFrontAngleFixMargin;
    /* 0x7A0 */ u16 mCameraHeadRotationTimer;  // SMG2 offset inh
    /* 0x7A2 */ u8 _pad7A2[0x2];
    /* 0x7A4 */ f32 mHeadRotateRatio;
    /* 0x7A8 */ f32 mVibrationAbsorbAngleF;  // SMG2 offset guess
    /* 0x7AC */ f32 mVibrationAbsorbAngleH;  // SMG2 offset guess
    /* 0x7B0 */ f32 mVibrationAbsorbAngleS;  // SMG2 offset guess
    /* 0x7B4 */ f32 mSlopeAnimBlendRatio;  // SMG2 offset guess
    /* 0x7B8 */ f32 mRotateHeadVecSpeedByGravityL;  // SMG2 offset guess
    /* 0x7BC */ f32 mRotateHeadVecSpeedByGravityM;  // SMG2 offset guess
    /* 0x7C0 */ f32 mRotateHeadVecSpeedByGravityS;  // SMG2 offset guess
    /* 0x7C4 */ f32 mLookMaxAngleH;  // SMG2 offset guess
    /* 0x7C8 */ f32 mLookMaxAngleVP;  // SMG2 offset guess
    /* 0x7CC */ f32 mLookMaxAngleVM;  // SMG2 offset guess
    /* 0x7D0 */ f32 mLookShoulderMoveMax;  // SMG2 offset guess
    /* 0x7D4 */ u16 mRushInBlendTimer;  // SMG2 offset guess
    /* 0x7D6 */ u8 _pad7D6[0x2];
};

/// @brief The accessor for player character constants.
class MarioConst {
    friend class Mario;
    friend class MarioActor;

public:
    /// @brief Creates a new `MarioConst`.
    MarioConst();

    /// @brief Returns the working table of player character constants.
    MarioConstTable* getTable() const {
        return mTable[mCurrentTable];
    }

    /// @brief Switches the working table of player character constants.
    /// @param tbl The new table index.
    void changeTable(u32 tbl) {
        mCurrentTable = tbl;
    }

private:
    /// @brief The array of player character constant tables.
    /* 0x00 */ MarioConstTable* mTable[3];  // SMG2: 3 tables (mCurrentTable read at 0xC)

    /// @brief The index of the working player character constant table.
    /* 0x0C */ u32 mCurrentTable;
};
