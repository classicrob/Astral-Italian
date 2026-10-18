#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class Yoshi;
class XanimePlayer;
class PartsModel;
class YoshiJointModelCtrl {
public:
    YoshiJointModelCtrl(Yoshi*,const char*,PartsModel*,u8);
    void fn_80386400(u8);
    void fn_80386480();
    void fn_80386490();
    Yoshi* mActor;
    const char* mJoint;
    PartsModel* mModel;
    u8 mJointCount;
    u8 mSelected;
};
class YoshiAnimationModelCtrlBase {
public:
    YoshiAnimationModelCtrlBase();
    virtual ~YoshiAnimationModelCtrlBase();
    virtual void calcAnim();
    virtual void appear();
    virtual void kill();
    void fn_8040B2A0(Yoshi*,XanimePlayer*,PartsModel*,PartsModel*,u8);
    Yoshi* mActor;
    PartsModel* mLeftModel;
    PartsModel* mRightModel;
    YoshiJointModelCtrl* mLeftJoint;
    YoshiJointModelCtrl* mRightJoint;
    XanimePlayer* mPlayer;
    TVec3f mAnimationTranslation;
    bool mAppeared;
};
class YoshiAnimationModelCtrl : public YoshiAnimationModelCtrlBase {
public:
    YoshiAnimationModelCtrl(Yoshi*,XanimePlayer*,const char*);
    virtual ~YoshiAnimationModelCtrl();
    virtual void appear();
    virtual void kill();
    void fn_8040B6D0(const char*);
};
