#pragma once

#include "Game/MapObj/ClipArea.hpp"

class ClipAreaShapeSphere;

class ClipAreaDrop : public ClipArea {
public:
    /// @brief Creates a new `ClipAreaDrop`.
    /// @param pName A pointer to the null-terminated name of the object.
    /// @param pShapeName A pointer to the null-terminated name of the volume model used for the sphere shape.
    ClipAreaDrop(const char* pName, const char* pShapeName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void control();

    void setBaseSize(f32);
    void setExtendsEndTime(s32);
    void setShrinkTime(s32, s32);
    void startHold();

    void exeWait();
    void exeHold();

    /* 0xC4 */ const char* mShapeName;
    /* 0xC8 */ ClipAreaShapeSphere* mShapeSphere;
    /* 0xCC */ f32 mBaseSize;
    /* 0xD0 */ s32 mExtendsEndTime;
    /* 0xD4 */ s32 mShrinkStartTime;
    /* 0xD8 */ s32 mShrinkEndTime;
    /* 0xDC */ s32 mStep;
    /* 0xE0 */ bool mCountUp;
};
