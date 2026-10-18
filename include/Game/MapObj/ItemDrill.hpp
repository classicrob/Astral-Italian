#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class ActorCameraInfo;
class CameraTargetMtx;
class HitSensor;
class ItemDrillMovementLimitPos;
class Triangle;

// SMG2-only unit (no Petari source). Layout from the ctor/init stores; size 0x228 (NameObjFactory).
class ItemDrill : public LiveActor {
public:
    ItemDrill(const char*);

    static bool isTargetCollisionParts(const CollisionParts*);

    virtual ~ItemDrill();
    virtual void init(const JMapInfoIter&);
    virtual void draw() const;
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool receiveMsgItemGet(HitSensor*, HitSensor*);
    bool startDrill();
    void cancelDrill(bool);
    void emitInvalidEffect();
    void emitReflectEffect();
    bool isTakenByPlayer() const;
    void incDrillingTimeCheckKillPlayer();
    void endCamera();
    void endFireDamage();
    s32 calcDiveStep();
    bool isOtherTriangleOnLine(Triangle*, const TVec3f&);
    bool tryAlignToTriangle(Triangle*, const TVec3f&);
    bool updateDrillMove(const TVec3f&);
    void alignDrillMtx(const TPos3f&, const TVec3f&);
    void reflectDrillAt(TMtx34f*);
    bool checkDrillStart();
    bool checkAimDirection(HitSensor*, HitSensor*);
    bool checkAttackFromEnemy(HitSensor*, HitSensor*);
    bool checkAttackFromDiver(HitSensor*, HitSensor*);

    void exeTake();
    void exeTaken();
    void exeDrillDiveStart();
    void exeDrillDiveLoop();
    void exeDrillling();
    void exeReverseDrillling();
    void exeDrillDiveEnd();
    void exeDrop();
    void exeHide();
    void exeReappear();
    void exeGroupHide();

    /* 0x090 */ TPos3f mBaseTRMtx;
    /* 0x0C0 */ TMtx34f mMtxC0;
    /* 0x0F0 */ TVec3f mVecF0;
    /* 0x0FC */ TMtx34f mMtxFC;
    /* 0x12C */ TPos3f mMtx12C;
    /* 0x15C */ TPos3f mMtx15C;
    /* 0x18C */ ActorCameraInfo* mCameraInfo;
    /* 0x190 */ bool mIsArg0;
    /* 0x191 */ bool mIsArg2;
    /* 0x194 */ Triangle* mTriangle;
    /* 0x198 */ CameraTargetMtx* mCameraTargetMtx;
    /* 0x19C */ TVec3f mEffectTrans;
    /* 0x1A8 */ TMtx34f mMtx1A8;
    /* 0x1D8 */ TMtx34f mMtx1D8;
    /* 0x208 */ ItemDrillMovementLimitPos* mLimitPos;
    /* 0x20C */ s32 mUnk20C;
    /* 0x210 */ HitSensor* mUnk210;
    /* 0x214 */ f32 mDrillDepth;
    /* 0x218 */ s32 mUnk218;
    /* 0x21C */ s32 mDrillingTime;
    /* 0x220 */ bool mUnk220;
    /* 0x224 */ TVec3f* mDirs;  // 15 entries, initialised to (0, -1, 0)
};

class ItemDrillMovementLimitPos : public LiveActor {
public:
    ItemDrillMovementLimitPos(const char*);

    virtual ~ItemDrillMovementLimitPos();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();

    bool adjustPositionByCollision(const TVec3f&, const TVec3f*);
    bool checkMapStrikePushback(TVec3f*, const TVec3f&, const TVec3f*);

    /* 0x090 */ CollisionParts* mCollisionParts;
    /* 0x094 */ void* mCollisionPartsFilter;
};
