#pragma once
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry.hpp>
class MoveBlockFunction : public NameObj {
public:
    MoveBlockFunction();
    virtual ~MoveBlockFunction();
    virtual void init(const JMapInfoIter&);
    void setMoveVec(const TVec3f&);
    void setRotateVec(const TVec3f&);
    void setMoveStep(s32);
    void setReturnStep(s32);
    void calcMoveVec(TVec3f*) const;
    void setForward(bool);
    bool update();
    bool isEndStep() const;
    bool isStartStep() const;
    bool canMove(bool) const;
    TQuat4f mRotation;
    TQuat4f mInitialRotation;
    TVec3f mPosition;
    TVec3f mInitialPosition;
    TVec3f mMoveVec;
    TVec3f mRotateVec;
    f32 mRate;
    s32 mMoveStep;
    s32 mReturnStep;
    s32 mStep;
    bool mForward;
};
