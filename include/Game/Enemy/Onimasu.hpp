#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorStateParamScale;

/// @brief Tox Box
class Onimasu : public LiveActor {
public:
    /// @brief Creates a new `Onimasu`.
    /// @param pName A pointer to the null-terminated name of the object.
    Onimasu(const char* pName);

    /// @brief Destroys the `Onimasu`.
    virtual ~Onimasu();

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void movement();
    virtual void calcAndSetBaseMtx();

    virtual void initFromRailPoint() = 0;
    virtual void incrementNextPoint() = 0;
    virtual void collectRailPointInfo() = 0;
    virtual s32 getNextPointNo() const = 0;
    virtual s32 getLastPointNo() const = 0;
    virtual const TVec3f& getLastPointNormal() const = 0;
    virtual const TVec3f& getNextPointNormal() const = 0;
    virtual void startMoveInner() {
    }
    virtual void updatePoseInner() {
    }

    void calcTargetPose();
    void updatePose();
    void land();
    bool calcTurnDirection(f32* pTurnDir) const;
    void updateStompVelocity();
    void calcGravityDir(TVec3f* pDir) const;
    f32 getGravityScalar() const;
    s32 getTimeToNextPoint() const;
    s32 getWaitTime() const;
    void emitEffectLand();

    void exeWaitForSwitchOn();
    void exeWait();
    void exeMove();
    void exeStamp();

    /* 0x90 */ ActorStateParamScale* mStateParam;
    /* 0x94 */ s32 mWaitTime;
    /* 0x98 */ s32 mTimeToNextPoint;
    /* 0x9C */ s32 mWaitTimeQuick;
    /* 0xA0 */ s32 mTimeToNextPointQuick;
    /* 0xA4 */ TPos3f mPointMtx;
    /* 0xD4 */ TVec3f mRotateAxis;
    /* 0xE0 */ TQuat4f mStartQuat;
    /* 0xF0 */ TQuat4f mTargetQuat;
    /* 0x100 */ TQuat4f mCurrentQuat;
    /* 0x110 */ TVec3f mRailClipPos;
};

namespace OnimasuFunction {
    bool getPolygonOnRailPoint(TVec3f* pPosition, TVec3f* pNormal, const LiveActor* pActor, int pointNo);
};  // namespace OnimasuFunction
