#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"

class HitSensor;
class StarPiece;
class StarPieceDirector;
class StarPieceHostInfo;
class StarPieceReceiverInfo;
class StarPieceShooter;

namespace MR {
    StarPieceDirector* getStarPieceDirector();

    void createStarPiece();
    StarPiece* getDeadStarPiece();

    void resetChasingStarPiece();
    bool isResetChasingStarPiece();

    bool gotByPlayer();

    bool giftStarPieceToTarget(HitSensor*, u32);

    void incNumStarPieceGettable(s32);
    void incNumStarPieceNewed();

};  // namespace MR

class StarPieceDirector : public LiveActorGroup {
public:
    StarPieceDirector(const char*);

    virtual ~StarPieceDirector();

    virtual void movement();

    void declare(const NameObj*, s32);
    void declareReceiveNum(const NameObj*, s32);
    s32 getDeclareRemnantStarPieceCount(const NameObj*);
    s32 getDeclareRemnantStarPieceCountNotExist(const NameObj*);
    bool hopPiece(const NameObj*, const TVec3f&, const TVec3f&);
    bool appearPiece(const NameObj*, const TVec3f&, s32, f32, f32, bool, bool, bool = false);
    bool appearPieceToDirection(const NameObj*, const TVec3f&, const TVec3f&, s32, f32, f32, bool, bool, bool = false);
    bool appearPieceToDirectionRandom(const NameObj*, const TVec3f&, const TVec3f&, s32, f32, bool, bool, bool);
    static u32 getRandomColor();
    bool gotByPlayer();
    void updateSound();
    void initCSDelay();
    void initCSSound();
    bool giftToTarget(HitSensor*, u32);
    StarPiece* getDeadStarPiece();
    TVec3f calcPosCameraShoot(s32);
    void clearGotCountReceiver(const NameObj*);
    StarPieceHostInfo* findHostInfo(const NameObj*);
    StarPieceReceiverInfo* findReceiverInfo(const NameObj*);
    void calcShootGoalUsingPointingDepth();

    /* 0x20 */ u32 mNumStarPieceNewed;
    /* 0x24 */ u32 mNumStarPieceGettable;
    /* 0x28 */ s32 mColorIndex;
    /* 0x2C */ s32 mNumStarPiecesLaunched;

    /* 0x30 */ StarPieceHostInfo* mHostInfoArray[512];

    /* 0x830 */ u32 mNewHostInfoIndex;

    /* 0x834 */ StarPieceReceiverInfo* mReceiverInfoArray[64];
    /* 0x934 */ u32 mNewReceiverInfoIndex;
    /* 0x938 */ bool mResetChasingStarPiece;
    /* 0x93C */ StarPieceShooter* mStarPieceShooters[2];

    // Only the first 16 entries are used by the get-sound queue.
    /* 0x944 */ bool mGetSoundArray[0x20];

    /* 0x964 */ s32 mSoundIndex;
    /* 0x968 */ bool mQueueNewGetSound;
};

class StarPieceShooter : public LiveActor {
public:
    StarPieceShooter(s32, const char*);

    virtual ~StarPieceShooter();

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeLockOn();

    bool shoot();
    bool tryShoot();

    void calcShootGoalUsingPointingDepth();

#ifdef GALAXY_NATIVE
    void tryShootNearestEnemy();
    bool mNativeShotThisFrame = false;
#endif

    f32 mShootSpeed;
    HitSensor* mTargetSensor;
    f32 mTargetDistance;
    HitSensor* mPreviousTargetSensor;
    f32 mPreviousTargetDistance;
    s32 mPointerPort;
    s32 mChargeStep;
    TVec3f mChargeEffectPos;
    TVec3f mChargeEffectRotation;
};
