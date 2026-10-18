#pragma once
#include "Game/Util/JointController.hpp"
class Yoshi;
class YoshiModelState;
class YoshiModelBlend;
class YoshiModelTargetState;
class YoshiModelCtrl {
public:
    typedef JointControlDelegator<YoshiModelCtrl>::func JointFunc;
    explicit YoshiModelCtrl(Yoshi*);
    JointController* createJointController(Yoshi*, const char*, JointFunc, JointFunc);
    void fn_80417FE0();
    void fn_80418020();
    void fn_80418060(const TVec3f*, s32, f32);
    void fn_80418090();
    void fn_804180B0(const TVec3f*, f32);
    void fn_804180E0();
    void fn_80418100();
    void fn_80418110();
    void fn_80418120();
    void fn_80418130();
    bool calcHead(TPos3f*, const JointControllerInfo&);
    bool calcSpine(TPos3f*, const JointControllerInfo&);
    void updateState() NO_INLINE;
    void updateSpineBlend() NO_INLINE;
    Yoshi* mHost;
    JointController* mHeadController;
    JointController* mSpineController;
    bool mImmediate;
    s32 mBlendDuration;
    bool mAlignEnabled;
    bool mTargetEnabled;
    bool mResetEnabled;
    s32 mSpineBlend;
    YoshiModelBlend* mBlend;
    YoshiModelState* mCurrentState;
    YoshiModelState* mIdleState;
    YoshiModelState* mAlignState;
    YoshiModelTargetState* mTargetState;
    YoshiModelState* mResetState;
};
