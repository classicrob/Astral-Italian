#pragma once

#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioModule.hpp"

class HashSortTable;
class MarioActor;
class XanimeResourceTable;
class XanimePlayer;
class HitSensor;

class MarioAnimator {
public:
    MarioAnimator(MarioActor*);
    inline Mario* getPlayer() const;

    void setAnimationFlag(u32);
    void resetAnimationFlag(u32);
    bool isAnimationFlag(u32) const;
    void enableExtraAnimation();
    void disableExtraAnimation();
    void changeExtraAnimation(const char*);
    const char* getUpperAnimationName() const;
    const char* getAnimationStringPointer(const char*) const;
    bool isUpperAnimationTerminated(const char*) const;
    f32 getWalkWeight(u32) const;
    f32 getAnimationInterpoleRatio() const;
    bool isFrameCtrlAttribute(u32) const;
    void changeTrackAnim(u8, const char*);
    XanimePlayer* getYoshiAnimationPlayer() const;
    XanimePlayer* getYoshiUpperAnimationPlayer() const;
    void setAnimationFrameCtrlAttribute(u8);
    void setAnimationPaused(bool);
    void setAnimationInterpoleFrame(s32);
    void setAnimationFrame(f32);
    void resetAnimationUpdateFlags();
    f32 getAnimationTrackWeight(u32);
    void setMainJointMatrix(u32, MtxPtr);
    void setYoshiJointMatrix(const char*, MtxPtr);
    void setMainJointOffset(u32, const TVec3f&);
    void setYoshiJointOffset(const char*, const TVec3f&);
    void setAnimationModel(J3DModel*);
    void setAnimationInterpoleRatio(f32);
    bool isAnimationRun(const char*) const;
    bool isMainAnimationRun(const char*) const;
    bool isAnimationTerminate(const char*) const;
    bool isUpperAnimationActive() const;
    bool isAnimationRun(const char*, u32);
    void init();
    void update();
    void fn_803E0A60();
    void fn_803E0AF0();
    void fn_803E0C20();
    void fn_803E0C90();
    void fn_803E0CE0();
    void fn_803E0D30();
    void fn_803E0DC0();
    void fn_803E0E90();
    void fn_803E0F40();
    void fn_803E0F80();
    void fn_803E1020();
    void fn_803E1060();
    void fn_803E1160();
    void fn_803E11F0();
    void fn_803E1230();
    void fn_803E12B0();
    bool fn_803E1110() const;
    bool fn_803E11D0() const;

    void calc();

    void setHoming();
    bool isAnimationStop() const;
    bool isDefaultAnimationRun(const char*) const;
    void setSpeed(f32);
    void forceSetBlendWeight(const f32*);
    void waterToGround();
    void initCallbackTable();
    void change(const char*);
    void fn_803E0E40(const char*);
    void changeDefault(const char*);
    void stopAnimation(const char*, const char*);  // SMG2 (fn_803DF490)
    void changeUpper(const char*);
    void changeDefaultUpper(const char*);
    void stopUpper(const char*);
    void setUpperRotateY(f32);
    void entryCallback(const char*);
    f32 getFrame() const;
    f32 getUpperFrame() const;
    u16 getUpperJointID() const;
    void setBlendWeight(const f32*, f32);
    void targetWeight(f32*, f32, f32);
    void setWalkWeight(const f32*);
    void initWalkWeight();
    bool isLandingAnimationRun() const;
    bool isCancelableAnimationRun() const;
    bool isWalkOrWaitingMotion() const;
    void updateWalkBas(const char*, f32);
    void setHand();
    void setTilt();
    void resetTilt();
    void setHipSlidingTilt(f32, f32);
    void setHipSliderTilt();
    void setHipSlipTilt();
    void setWalkMode();
    void updateJointRumble();
    void addRumblePower(f32, u32);
    void clearAllJointTransform();
    bool isMirrorAnimation();
    void switchMirrorMode();
    void changePickupAnimation(const HitSensor*);
    void updateTakingAnimation(const HitSensor*);
    void changeThrowAnimation(const HitSensor*);
    void stopWaitAnimation();
    void controlWaitAnimation();
    void runningCallback();
    void closeCallback();
    void spinEntry();
    void spinUpdate();
    void spinClose();
    void stageInCheck();
    void throwCheck();
    void throwEntry();
    void throwClose();
    void squatSpinCheck();
    void walkinClose();

    // SMG2-only (original names unknown; named by address)
    void fn_803DC1B0(const char*);
    void fn_803DC330();
    f32 fn_803DD170(const char*, f32);
    f32 fn_803DD460();
    f32 fn_803DD530();
    void fn_803DDD80(const HitSensor*);
    void fn_803DDF60();
    const char* fn_803DF0D0() const;
    f32 fn_803DF0E0() const;
    void fn_803DF0F0();
    void fn_803DF1B0();
    void fn_803DF2D0();
    void fn_803DF8E0();
    s32 fn_803DF9D0();
    bool fn_803DCB80();
    XanimePlayer* fn_803DFBF0() const;

    inline void f1(const char* name) {
        getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);

        mXanimePlayer->setDefaultAnimation(name);
    }

    inline bool isTeresaClear() const {
        return !getPlayer()->isPlayerModeTeresa();
    }

    inline XanimePlayer* getXanimePlayer() {
        return mXanimePlayer;
    }

    /* 0x000 */ MarioActor* mActor;
    /* 0x004 */ XanimeResourceTable* mResourceTable;
    /* 0x008 */ XanimePlayer* mXanimePlayer;
    /* 0x00C */ XanimePlayer* mXanimePlayerUpper;
    /* 0x010 */ XanimePlayer* mXanimePlayerExtra;
    /* 0x014 */ u8 _14;
    /* 0x015 */ u8 _15;
    /* 0x016 */ u8 _16;
    /* 0x018 */ f32 _18;
    /* 0x01C */ f32 _1C;
    /* 0x020 */ f32 _20;
    /* 0x024 */ f32 _24;
    /* 0x028 */ TMtx34f _28;
    /* 0x058 */ f32 _58;
    /* 0x05C */ f32 _5C;
    /* 0x060 */ TVec3f _60;
    /* 0x06C */ bool _6C;
    /* 0x070 */ f32 _70;
    /* 0x074 */ u32 _74;
    /* 0x078 */ u16 _78;
    /* 0x07C */ TMtx34f _7C;
    /* 0x0AC */ TMtx34f _AC;
    /* 0x0DC */ TMtx34f _DC;
    /* 0x10C */ bool _10C;
    /* 0x10D */ bool _10D;
    /* 0x10E */ bool mUpperDefaultSet;
    /* 0x10F */ bool mCallbackEnded;
    /* 0x110 */ f32 _110;
    /* 0x114 */ const char* mCurrBck;
    /* 0x118 */ f32 _118;

    u32 _11C;
    u32 _120;
    u32 _124;
    bool mExtraAnimationEnabled;
    u8 mAnimationFlags[32];
    /* 0x14C */ s32 mCallbackId;
    /* 0x150 */ HashSortTable* mCallbackTable;
};
