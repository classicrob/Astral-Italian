#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class Monte;
class FixedPosition;
class ParabolicPath;
class ActorCameraInfo;
class MonteThrow : public LiveActor {
public:
    MonteThrow(Monte*);
    virtual ~MonteThrow();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void startThrow() NO_INLINE;
    bool tryBind(HitSensor*) NO_INLINE;
    void endThrow() NO_INLINE;
    void releasePlayer() NO_INLINE;
    void updatePlayerMtx() NO_INLINE;
    bool isPreparingThrow() const;
    void initThrowPath(const TVec3f&, const TVec3f&) NO_INLINE;
    void exeWait();
    void exePrepare();
    void exeThrow();
    void exeThrowUp();
    Monte* mHost;
    LiveActor* mBoundActor;
    FixedPosition* mBindPosition;
    ActorCameraInfo* mCameraInfo;
    ParabolicPath* mPath;
    f32 mThrowHeight;
    f32 mThrowSpeed;
    s32 mThrowTime;
    TVec3f mTargetPosition;
    bool mThrowUp;
};

extern const char* const sMonteThrowTargetPositionName;
extern const char* const sMonteThrowPlayerAnimation;
