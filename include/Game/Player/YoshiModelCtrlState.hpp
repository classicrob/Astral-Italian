#pragma once
#include "Game/Player/YoshiModelCtrl.hpp"
class YoshiModelState {
public:
    explicit YoshiModelState(YoshiModelCtrl* ctrl) : mCtrl(ctrl) {}
    virtual ~YoshiModelState() {}
    virtual YoshiModelState* next() = 0;
    virtual void enter();
    virtual void update();
    virtual void leave();
    virtual void calc(const TPos3f&, TPos3f*) = 0;
    void calcJoint(TPos3f*, const JointControllerInfo&) NO_INLINE;
    YoshiModelCtrl* mCtrl;
};
class YoshiModelIdleState : public YoshiModelState {
public:
    explicit YoshiModelIdleState(YoshiModelCtrl* ctrl) : YoshiModelState(ctrl) {}
    virtual ~YoshiModelIdleState() {}
    virtual YoshiModelState* next();
    virtual void calc(const TPos3f&, TPos3f*);
};
class YoshiModelAlignState : public YoshiModelState {
public:
    explicit YoshiModelAlignState(YoshiModelCtrl*);
    virtual ~YoshiModelAlignState() {}
    virtual YoshiModelState* next();
    virtual void calc(const TPos3f&, TPos3f*);
};
class YoshiModelTargetState : public YoshiModelState {
public:
    explicit YoshiModelTargetState(YoshiModelCtrl*);
    virtual ~YoshiModelTargetState() {}
    virtual YoshiModelState* next();
    virtual void enter();
    virtual void calc(const TPos3f&, TPos3f*);
    void calcTargetRotation(const TPos3f&, TQuat4f*);
    const TVec3f* mTarget;
    f32 mOffset;
    TPos3f mInitialMtx;
    bool mReset;
};
class YoshiModelResetState : public YoshiModelState {
public:
    explicit YoshiModelResetState(YoshiModelCtrl* ctrl) : YoshiModelState(ctrl) {}
    virtual ~YoshiModelResetState() {}
    virtual YoshiModelState* next();
    virtual void enter();
    virtual void calc(const TPos3f&, TPos3f*);
    TPos3f mMtx;
};
class YoshiModelBlend {
public:
    explicit YoshiModelBlend(YoshiModelCtrl*);
    void start(YoshiModelState*, YoshiModelState*, s32);
    void update();
    void calc(TPos3f*, const TPos3f&);
    void capture();
    void calcCaptured(TPos3f*, const TPos3f&);
    f32 rate() const;
    YoshiModelCtrl* mCtrl;
    s32 mMode;
    TPos3f mMtx;
    s32 mStep;
    s32 mDuration;
};
bool decomposeYoshiModelMtx(const TPos3f&, TVec3f*, TVec3f*, TQuat4f*);
void blendYoshiModelMtx(TPos3f*, const TPos3f&, f32);
