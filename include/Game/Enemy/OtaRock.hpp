#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class CocoNutBall;
class FireBall;
class FixedPosition;
class NameObjArchiveListCollector;

class OtaRock : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    void initOtaRockPlacement(const JMapInfoIter& rIter);
    bool tryStartOpeningDemo();
    void chooseWaitNerve();
    void exeOtaRockOpeningDemo();
    /// @brief Creates a new `OtaRock`.
    /// @param pName A pointer to the null-terminated name of the object.
    OtaRock(const char* pName);

    virtual ~OtaRock();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    void initModel();
    void initSensor();
    void updateBaseMtx();
    CocoNutBall* getDisappearedCocoNut();
    FireBall* getDisappearedFireBall();
    s32 getDisappearedCocoNutNum() const;
    s32 getDisappearedFireBallNum() const;
    bool isValidThrowCocoNut() const;
    bool isValidThrowFireBall() const;
    void throwCocoNut();
    void throwFireBall();
    bool tryToHide();

    void exeWait();
    void exeThrowCocoNut();
    void exeThrowFireBall();
    void exeDown();
    void exeHide();
    void exeHideWait();
    void exeShow();

public:
    /* 0x90 */ bool mNoThrowCocoNut;
    /* 0x94 */ CocoNutBall* mCocoNutArray;
    /* 0x98 */ FireBall* mFireBallArray;
    /* 0x9C */ FixedPosition* mFixedPosition;
    /* 0xA0 */ s32 mThrowCocoNutCounter;
    /* 0xA4 */ u32 mWaitFrame;
    /* 0xA8 */ TPos3f mBaseMtx;
    /* 0xD8 */ TVec3f mFront;
    /* 0xE4 */ AnimScaleController* mAnimScaleController;
};
