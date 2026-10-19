#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Array.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CannonShellHolder;
class HomingKiller;
class JMapInfoIter;
class Kameck;
class KoopaJr;
class ModelObj;
class NameObjArchiveListCollector;

class KoopaJrShip : public LiveActor {
public:
    KoopaJrShip(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void initShells();
    void initKoopaJr(const JMapInfoIter&);
    void initKameck(const JMapInfoIter&);
    void initKiller();
    bool receiveMsgJetTurtleAttack(HitSensor*, HitSensor*);
    s32 getNumShootShells() const;
    f32 getPropellerRotSpeed() const;
    void updateCoordSpeed();
    void calcLauncherInfo(TVec3f*, TVec3f*, s32) const NO_INLINE;
    void calcLauncherInfoKiller(TVec3f*, TVec3f*, s32) const;
    void shootShell(s32);
    bool tryShootAllKillers();
    void shootKiller(s32);
    void shootKillersAfterDamage();
    bool isExistActiveKiller() const;
    bool isExistActiveKameck() const;
    void shootMainShells();
    void emitDamageSmokeEffect();
    void emitDamageHitEffect();
    void updateKoopaJrPos();
    void updateKillerPos();
    void killAllSubModels();
    void setStateTurnFront();
    void exeAppear();
    void exeMove();
    void exeMoveFrontAttack();
    void exeShoot();
    void exeShootMain();
    void exeStopAtEnd();
    void exePowerUp();
    void exeDamage();
    void exeBreakStart();
    void exeBreak();
    void exeBreakEnd();
    void exeTurnFront();

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    inline bool isNextStateDamage(s32& x) {
        return --x <= 0;
    }

    inline bool isStateBreak() const;

    s32 get_D0() const {
        return mHealth;
    }

    /* 0x08C */ CannonShellHolder* mShellHolder;
    /* 0x090 */ CannonShellHolder* mMainShellHolder;
    /* 0x094 */ KoopaJr* mJr;
    /* 0x098 */ ModelObj* mShipBreakModel;
    /* 0x09C */ ModelObj* mPodModel;
    /* 0x0A0 */ MR::Vector< MR::FixedArray< Kameck*, 2 > > mKamecks;
    /* 0xB0 */ MR::Vector< MR::FixedArray< Kameck*, 2 > > mActiveKamecks;
    /* 0x0B8 */ MR::FixedArray< HomingKiller*, 6 > mKillers;
    /* 0xD4 */ s32 mHealth;
    /* 0xD8 */ TVec3f mDamagePos;
    /* 0xE4 */ TVec3f mDamageDirection;
    /* 0xF0 */ f32 mPropellerAngle;
    /* 0x0F0 */ f32 mPropRotateSpeed;
    /* 0x0F4 */ TPos3f mScrew00Mtx;
    /* 0x124 */ TPos3f mScrew01Mtx;
    /* 0x154 */ TRot3f mPropellerMtx;
    /* 0x188 */ f32 mRailSpeed;
    /* 0x18C */ s32 mRailSpeedTimer;
    /* 0x18C */ TMtx34f mShipMtx;
    /* 0x1BC */ TMtx34f mPodMtx;
    /* 0x1F0 */ TVec3f mJrOffset;
    /* 0x1FC */ TVec3f mStoredPosition;
};
