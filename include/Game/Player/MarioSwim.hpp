#pragma once

#include "Game/Map/WaterInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioState.hpp"

class AreaInfo;
class HitInfo;
class MarioActor;
class MarineSnow;
class DashRing;

class MarioSwim;

class MarioSwimModule {
public:
    explicit MarioSwimModule(MarioSwim* swim) : mSwim(swim) {}
    virtual ~MarioSwimModule();
    MarioSwim* mSwim;
};

class MarioSwimSpin : public MarioSwimModule {
public:
    explicit MarioSwimSpin(MarioSwim*);
    virtual ~MarioSwimSpin() {}
    void reset();
    bool isActive() const NO_INLINE;
    void startAnimation(u16, u16, const char*);
    void update();
    void startDash() NO_INLINE;
    u16 getReadyTimer() const;
    u8 _8;
    u16 mSpinTimer;
    u16 mSpinDashTimer;
    u16 _E;
};

class MarioSwimJet : public MarioSwimModule {
public:
    explicit MarioSwimJet(MarioSwim*);
    virtual ~MarioSwimJet() {}
    f32 calcSpeedScale() const;
    void update(f32);
    f32 calcRotationRatio() const;
    f32 getMaxSpeed() const NO_INLINE;
    void updateVelocity(f32) NO_INLINE;
    bool isBackInput() const NO_INLINE;
    u32 _8;
    u16 mJetTimer;
    u16 mCooldownTimer;
};

class MarioSwimRingDash : public MarioSwimModule {
public:
    explicit MarioSwimRingDash(MarioSwim*);
    virtual ~MarioSwimRingDash() {}
    void reset();
    void updateBlur() NO_INLINE;
    void setRing(const DashRing*) NO_INLINE;
    u16 mTimer;
    u16 mChargeTimer;
    u16 mMaxDuration;
    f32 mSpeedScale;
    f32 mBlurOffset;
};

class MarioSwimDamage : public MarioSwimModule {
public:
    explicit MarioSwimDamage(MarioSwim*);
    virtual ~MarioSwimDamage() {}
    void update();
    void reflect(const TVec3f*);
    void reset();
    TVec3f mVelocity;
    u8 mDamageType;
    u8 _15;
    u16 mTimer;
};

class MarioSwimTilt : public MarioSwimModule {
public:
    explicit MarioSwimTilt(MarioSwim*);
    virtual ~MarioSwimTilt() {}
    void update();
    f32 mTiltX;
    f32 mTiltY;
    TMtx34f mJointMtx;
};

class MarioSwimCollisionNode {
public:
    MarioSwimCollisionNode();
    /* 0x00 */ TVec3f mOffset;
    /* 0x0C */ f32 mRadius;
    /* 0x10 */ u8 _10;
    /* 0x14 */ TVec3f _14;
    /* 0x20 */ const WaterArea* mWaterArea;
    /* 0x24 */ TVec3f _24;
    /* 0x30 */ u8 _30;
};

class MarioSwim : public MarioState {
public:
    bool fn_803F8750(u32) NO_INLINE;
    bool fn_803FE7E0() NO_INLINE;
    void fn_80400A40(TVec3f*) NO_INLINE;
    void fn_80401850(u32) NO_INLINE;
    void fn_80402720() NO_INLINE;
    bool fn_803FB8E0() NO_INLINE;
    bool fn_803FB9C0() NO_INLINE;
    bool fn_803FBB90() NO_INLINE;
    bool fn_803FBC70() NO_INLINE;
    f32 fn_803FBDA0() NO_INLINE;
    f32 fn_803FBE10() NO_INLINE;
    void fn_803FBE80() NO_INLINE;
    void fn_803FBF20() NO_INLINE;
    bool fn_803FC020() NO_INLINE;
    void fn_80401960() NO_INLINE;
    void fn_804019D0() NO_INLINE;
    void fn_80401AE0() NO_INLINE;
    void fn_80401B70() NO_INLINE;
    bool fn_80401BE0() NO_INLINE;
    void fn_80401CD0() NO_INLINE;
    void fn_80401D60() NO_INLINE;
    void fn_80401E10() NO_INLINE;
    f32 fn_80402120() NO_INLINE;
    f32 fn_804021B0() NO_INLINE;
    f32 fn_80402250() NO_INLINE;
    f32 fn_804022D0() NO_INLINE;
    f32 fn_80402370() NO_INLINE;
    f32 fn_80402400() NO_INLINE;
    void fn_80402070() NO_INLINE;
    void fn_803F8D10() NO_INLINE;
    f32 fn_803FA2C0() NO_INLINE;
    void fn_803FD140() NO_INLINE;
    void fn_803FE4A0(WaterInfo*, const TVec3f&, const TVec3f&, u32) NO_INLINE;
    void fn_80400FC0(const TVec3f&) NO_INLINE;
    bool fn_803F7D90();
    void fn_803F8DA0();
    bool fn_803F8E10();
    void fn_803F9AC0(TVec3f*, const TVec3f&, const TVec3f&);
    void fn_803F9B90();
    void fn_803F9DF0();
    void fn_803F9E90();
    void fn_803F9FC0();
    void fn_803FA020();
    bool fn_80401690();
    void fn_80401710();
    void fn_804017C0(bool);
    void fn_803F8060() NO_INLINE;
    bool fn_803F8490();
    bool fn_803F88C0();
    bool fn_803F8930() NO_INLINE;
    f32 fn_803F8EF0();
    void fn_803F8F60(f32) NO_INLINE;
    void fn_803F9350();
    void fn_803F95A0(f32);
    void fn_803F9750(const TVec3f&, const TVec3f&, bool);
    void fn_803FB570();
    void fn_80402470(f32);
    void fn_80401F00(f32);
    void fn_80401F90();
    void fn_804025C0();
    f32 fn_803FB250();
    void fn_80401160();
    void fn_80401270();
    void fn_804013C0();
    void fn_804013E0();
    void fn_80401440();
    void fn_804014A0();
    void fn_80401510();
    void fn_804015C0();
    void fn_80401640();
    void fn_80401010();
    void fn_803FA820();
    void fn_803FB300();
    void fn_803FA450();
    bool fn_803FA2E0() const;
    bool fn_803FA350() const;
    bool fn_803FA3C0() const;
    bool fn_803FB3D0() const;
    bool fn_804026A0() const;

    enum WaterExitAction {
        EXIT_ACTION_NONE = 0,
        EXIT_ACTION_JUMP = 1,        // Shallow water jump
        EXIT_ACTION_POWER_JUMP = 2,  // Spin jump or paddle jump
        EXIT_ACTION_FALL = 3,

        EXIT_ACTION_SLIDE = 5,
        EXIT_ACTION_SURFACE = 6
    };
    enum SwimState { SWIM_STATE_NONE = 0, SWIM_STATE_ENTERING = 1, SWIM_STATE_UNDERWATER = 2, SWIM_STATE_SURFACE = 3 };

    MarioSwim(MarioActor*);

    // SMG2-only (original names unknown; named by address)
    bool isSwimTurnAnimation() const NO_INLINE;
    void updateJetAnimation() NO_INLINE;
    void fn_803F9D50();
    f32 fn_803FA8E0() const;
    f32 fn_803FA950() const;
    f32 fn_803FA9C0() const;
    f32 fn_803FAA30() const;
    f32 fn_803FAAA0() const;
    f32 fn_803FAB10() const;
    f32 fn_803FAB80() const;
    f32 fn_803FABF0() const;

    bool fn_80401EC0() const;
    void fn_80402040();
    void fn_804020F0();
    f32 fn_80402160() const NO_INLINE;
    f32 fn_80402200() const;
    void fn_803F7C10();
    void fn_803F7C00();
    void fn_803F7C20(const TVec3f*);
    bool fn_803FD5E0(s32) NO_INLINE;
    bool fn_803FE9F0() const;
    bool fn_80400910() const;
    bool fn_80400930() const;

    virtual void init();
    virtual bool start();
    virtual bool close();
    virtual bool update();
    virtual bool notice();
    virtual void hitWall(const TVec3f&, HitSensor*);
    virtual void hitPoly(const TVec3f&, HitSensor*);
    virtual bool passRing(const HitSensor*);
    virtual f32 getBlurOffset() const {
        return mRingDash->mBlurOffset;
    }

    virtual void draw3D() const;
    virtual f32 getStickY() const;
    virtual const TVec3f& getGravityVec() const;
    virtual void addVelocity(const TVec3f& rVelocity);
    virtual void addVelocity(const TVec3f& rVelocity, f32);

    bool checkWaterCube(bool);
    void onSurface() NO_INLINE;
    void doJetJump(u8);
    void updateLifeByTime();
    bool isInColdWater() const NO_INLINE;
    void updateColdWaterDamage() NO_INLINE;
    void recoverOxygenAtSurface() NO_INLINE;
    void updateOxygenByTime() NO_INLINE;
    void fn_803F8430();
    bool surfacePaddle();
    void flowOnWave(f32);
    bool checkWaterBottom();
    void spin();
    void decideVelocity();
    void procBuoyancy();
    void decideAnimation();
    void decideEffect(bool);
    void jet();
    void pushedByWaterWall();
    void setDamage(const TVec3f&, u16);
    void updateUnderWater();
    bool startJet(const HitSensor*);
    void addDamage(const TVec3f&);
    void addFaint(const TVec3f&);
    bool getWaterAreaInfo(WaterInfo*, const TVec3f&, TVec2f*);
    void decOxygen(u16 amount);
    void incOxygen();
    void incLife();
    void decLife();
    f32 calcRingAcc();
    void hitPunch(const TVec3f& rPunchDir);
    f32 getSurface() const;
    f32 getWaterEdgeDist() const NO_INLINE {
        if (isOnWaterSurface()) {
            return -1.0f;
        }

        return mDistanceToWaterSurface;
    }

    bool tryJetAttack(HitSensor*);
    void dropJet(bool);
    void updateOxygenWatch();
    void doDecLifeByCold();
    void ringDash(f32);
    void resetJet();
    void forceStopSpin();
    void resetAndFixPose();
    f32 checkUnderWaterFull(const TVec3f&);
    void hitHead(const Triangle*, const TVec3f&);

    f32 calcWaterEdgeDist() const {
        if (isOnWaterSurface()) {
            return -1.0f;
        }

        return mDistanceToWaterSurface;
    };

    static inline f32 getWorthlessNumber() {
        return 0.523598790169f;
    }

    /* The worthlesser variable, as its name suggests, has no purpose.
       It achieves nothing in life. Its existence is completely devoid of value.
       Nevertheless, the compiler left evidence of this forgettable variable behind.
       As a result, we know that it existed, but we do not know how precisely it achieved nothing.

       Summary:
       worthlesser does not impact the behavior of this function.

     */

    inline void funReferenceTime(bool& worthlesser) {
        if (mVerticalAngle > 1.57079637051f) {
            worthlesser = true;
        } else {
            if (mIdleWaitTimer > 0x1E) {
                _1E = 1;
            }

            worthlesser = false;
            if (getStickY() > 0.0f) {
                f32 stickY = getStickY();
                // Note: The binary does not tell us whether this comparison is > or <.
                // It is not == because that generates an fcmpu instruction, not fcmpo.
                // It is not <= or >= because those generate cror instructions.
                if (getWorthlessNumber() + getWorthlessNumber() * stickY < mVerticalAngle

                    // This comparison needs to be present for the compiler to optimize the condition
                    // accurately.
                    && worthlesser) {
                    return;
                }
            }
        }

        worthlesser = true;
    }

    inline TVec3f getPlayer380() const {
        return getPlayer()->_380;
    }

    inline bool check7Aand7C() const {
        return mSpin->mSpinTimer || mSpin->mSpinDashTimer;
    }

    inline bool isOnWaterSurface() const {
        return mIsOnSurface || mIsSwimmingAtSurface;
    }

    /* 0x014 */ MarineSnow* mMarineSnow;
    /* 0x018 */ u8 _18;
    /* 0x019 */ bool mIsOnSurface;
    /* 0x01A */ bool mEnteredWater;
    /* 0x01B */ bool _1B;
    /* 0x01C */ bool _1C;
    /* 0x01D */ bool _1D;
    /* 0x01E */ bool _1E;
    /* 0x01F */ bool _1F;
    /* 0x020 */ bool mIsSwimmingAtSurface;
    /* 0x021 */ bool mIsSinking;
    /* 0x022 */ bool _22;
    /* 0x024 */ u32 mStateTimer;
    /* 0x028 */ u32 mSurfacePaddleHoldTimer;
    /* 0x02C */ u16 mActionLockTimer;
    /* 0x02E */ u16 mSinkTimer;
    /* 0x030 */ u16 mSwimYAccTimer;
    /* 0x032 */ u16 mZSinkTimer;
    /* 0x034 */ u16 mJumpDiveTimer;
    /* 0x036 */ u16 mDashTimer;
    /* 0x038 */ u16 _38;
    /* 0x03A */ u16 mBuoyancyDelayTimer;
    /* 0x03C */ u16 mIdleWaitTimer;
    /* 0x03E */ u16 mWaveFlowTimer;
    /* 0x040 */ u16 _40;
    /* 0x042 */ u16 mFloorContactTimer;
    /* 0x044 */ u32 _44;
    /* 0x048 */ f32 mSurfaceOffset;
    /* 0x04C */ f32 mStickInputYInertia;
    /* 0x050 */ f32 mStickInputXInertia;
    /* 0x054 */ f32 mForwardSpeed;
    /* 0x058 */ f32 mBuoyancySpeed;
    /* 0x05C */ f32 mVerticalAngle;
    /* 0x060 */ TVec3f mFrontVec;
    /* 0x06C */ TVec3f mUpVec;
    /* 0x078 */ TVec3f mPoseVec;
    /* 0x084 */ MarioSwimSpin* mSpin;
    /* 0x088 */ MarioSwimJet* mJet;
    /* 0x08C */ MarioSwimRingDash* mRingDash;
    /* 0x090 */ MarioSwimDamage* mDamage;
    /* 0x094 */ MarioSwimTilt* mTilt;
    /* 0x098 */ bool _9C;
    /* 0x099 */ u8 _9D;
    /* 0x09A */ u8 mNextAction;
    /* 0x09B */ u8 mEffectState;
    /* 0x09C */ u8 _9CFlag;
    /* 0x09E */ u16 mDrownTimer;
    /* 0x0A0 */ u16 mOxygen;
    /* 0x0A2 */ u16 mOxygenDecreaseDelay;
    /* 0x0A4 */ u16 mColdWaterDamageInterval;
    /* 0x0A8 */ WaterInfo mWaterInfo;
    /* 0x0FC */ s32 mSwimState;
    /* 0x100 */ TVec3f _148;
    /* 0x10C */ TVec3f mAreaFollowMtxPos;
    /* 0x118 */ const WaterArea* _118;
    /* 0x11C */ TVec3f mSurfacePos;
    /* 0x128 */ TVec3f mSurfaceNorm;
    /* 0x134 */ TVec3f _178;
    /* 0x140 */ TVec3f mWaterStreamVel;
    /* 0x14C */ TVec3f mWaterStreamDir;
    /* 0x158 */ f32 mWaterDepth;
    /* 0x15C */ f32 _1A0;
    /* 0x160 */ f32 mDistToFloor;
    /* 0x164 */ f32 _1A8;
    /* 0x168 */ f32 mDistanceToWaterSurface;
    /* 0x16C */ u16 mWallStickTimer;
    /* 0x16E */ u8 _1B2;
    /* 0x170 */ f32 mWaterDistanceTarget;
    /* 0x174 */ f32 _1B8;
    /* 0x178 */ MarioSwimCollisionNode* mCollisionNodes;
    /* 0x17C */ u32 mCollisionNodeCount;
    /* 0x180 */ u32 _180;
    /* 0x184 */ WaterInfo mPreviousWaterInfo;
    /* 0x1D8 */ TVec3f mPreviousPos;
    /* 0x1E4 */ TVec3f mCollisionNodeCenter;
    /* 0x1F0 */ TVec3f _1F0;
    /* 0x1FC */ f32 _1FC;
    /* 0x200 */ f32 _200;
    /* 0x204 */ s32 _204;
};
