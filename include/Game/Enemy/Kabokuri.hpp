#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class Kuribo;
class ModelObj;
class WalkerStateBindStarPointer;

/// @brief Jack O'Goomba
class Kabokuri : public LiveActor {
public:
    Kabokuri(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void initSensor();
    void updatePose();
    void addVelocityBase();
    // existence is implied by sdata2 ordering
    void addVelocityHitAttacked();
    void addVelocityToRailPoint(f32);
    bool requestTrampled();
    bool requestStarPieceHitted();
    bool requestHipDropped();
    bool requestHitAttacked(HitSensor*, HitSensor*);
    bool tryPointBind();
    void exeWait();
    void exeWalk();
    void exeDropFire();
    void exeAttacksuccess();
    void exeTrampled();
    void exeStarPieceHitted();
    void exeHipDropped();
    void exeHitAttacked();
    void exeBreak();
    void exeBindStarPointer();
    bool isEnableAttack() const;
    bool isEnablePointBind() const;
    bool isEnableTrampled() const;
    bool isEnablePush() const;
    void endBindStarPointer() const;

    /* 0x8C */ Kuribo* mKuribo;
    /* 0x90 */ ModelObj* mBreakModel;
    /* 0x94 */ AnimScaleController* mAnimeScale;
    /* 0x9C */ TMtx34f _9C;
    /* 0xCC */ TMtx34f _CC;
    /* 0xFC */ WalkerStateBindStarPointer* mStateBindStartPointer;
    /* 0x100 */ s32 _100;
    /* 0x104 */ s32 _104;
    /* 0x108 */ TQuat4f mRotationQuat;
    /* 0x118 */ TVec3f mFrontVec;
    /* 0x124 */ s32 _B8;
    /* 0x128 */ s32 mCounter;
    /* 0x12C */ bool mIsFloating;
    /// @brief Drops fire on every point of the rail reached, except the first one.
    /* 0x12D */ bool mIsValidDropFire;
};
