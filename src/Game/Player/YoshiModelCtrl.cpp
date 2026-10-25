#define JGEOMETRY_OUTLINE_MATRIX_ZERO_TRANS
#define JGEOMETRY_OUTLINE_MATRIX_QUAT
#define JGEOMETRY_OUTLINE_MATRIX_ROTATION
#include "Game/Player/YoshiModelCtrl.hpp"
#include "Game/Player/YoshiModelCtrlState.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>

namespace {
const char* sHeadJointName="Head";
const char* sSpineJointName="Spine";
}

YoshiModelCtrl::YoshiModelCtrl(Yoshi* host) : mHost(host),mHeadController(nullptr),mSpineController(nullptr),mImmediate(0),mBlendDuration(6),mAlignEnabled(false),mTargetEnabled(false),mResetEnabled(false),mSpineBlend(0) {
    mBlend=new YoshiModelBlend(this);
    mCurrentState=nullptr;
    mIdleState=new YoshiModelIdleState(this);
    mAlignState=new YoshiModelAlignState(this);
    mTargetState=new YoshiModelTargetState(this);
    mResetState=new YoshiModelResetState(this);
    mHeadController=createJointController(host,sHeadJointName,&YoshiModelCtrl::calcHead,nullptr);
    mSpineController=createJointController(host,sSpineJointName,nullptr,&YoshiModelCtrl::calcSpine);
    mCurrentState=mIdleState;
}
JointController* YoshiModelCtrl::createJointController(Yoshi* host,const char* name,JointFunc calc,JointFunc after) {
    JointController* controller=new JointControlDelegator<YoshiModelCtrl>(this,calc,after);
    MR::setJointControllerParam(controller,host,name);
    return controller;
}
void YoshiModelCtrl::fn_80417FE0() { updateState();updateSpineBlend(); }
void YoshiModelCtrl::fn_80418020() { mHeadController->registerCallBack();mSpineController->registerCallBack(); }
void YoshiModelCtrl::fn_80418060(const TVec3f* target,s32 duration,f32 offset) { mTargetState->mTarget=target;mTargetState->mOffset=offset;mTargetEnabled=true;mImmediate=0;mBlendDuration=duration; }
void YoshiModelCtrl::fn_80418090() { mTargetState->mTarget=nullptr;mTargetEnabled=false;mImmediate=0; }
void YoshiModelCtrl::fn_804180B0(const TVec3f* target,f32 offset) { mTargetState->mTarget=target;mTargetState->mOffset=offset;mTargetEnabled=true;mImmediate=1;mSpineBlend=10; }
void YoshiModelCtrl::fn_804180E0() { mTargetState->mTarget=nullptr;mTargetEnabled=false;mImmediate=1;mSpineBlend=0; }
void YoshiModelCtrl::fn_80418100() { mResetEnabled=true; }
void YoshiModelCtrl::fn_80418110() { mResetEnabled=false; }
void YoshiModelCtrl::fn_80418120() { mAlignEnabled=false; }
void YoshiModelCtrl::fn_80418130() { mAlignEnabled=true; }
void YoshiModelCtrl::updateState() {
    YoshiModelState* next=mCurrentState->next();
    if(next) {
        if(mImmediate) { YoshiModelBlend* blend=mBlend;blend->mMode=0;blend->mStep=0; }
        else mBlend->start(mCurrentState,next,mBlendDuration);
        mCurrentState->leave();mCurrentState=next;next->enter();
    }
    mCurrentState->update();mBlend->update();
}
void YoshiModelCtrl::updateSpineBlend() {
    if(mCurrentState==mIdleState) { if(mSpineBlend>0) mSpineBlend--; }
    else if(mSpineBlend<10) mSpineBlend++;
}
void YoshiModelState::leave() {}
void YoshiModelState::enter() {}
void YoshiModelState::update() {}
void YoshiModelIdleState::calc(const TPos3f&,TPos3f*) {}
YoshiModelState* YoshiModelIdleState::next() { if(mCtrl->mTargetEnabled) return mCtrl->mTargetState;if(mCtrl->mAlignEnabled) return mCtrl->mAlignState;return nullptr; }
YoshiModelAlignState::YoshiModelAlignState(YoshiModelCtrl* ctrl) : YoshiModelState(ctrl) {}
YoshiModelState* YoshiModelAlignState::next() { if(mCtrl->mTargetEnabled) return mCtrl->mTargetState;if(mCtrl->mAlignEnabled) return nullptr;return mCtrl->mIdleState; }
YoshiModelTargetState::YoshiModelTargetState(YoshiModelCtrl* ctrl) : YoshiModelState(ctrl),mTarget(nullptr),mOffset(0.0f),mReset(false) { mInitialMtx.identity(); }
YoshiModelState* YoshiModelTargetState::next() { if(mCtrl->mTargetEnabled) { if(mCtrl->mResetEnabled) return mCtrl->mResetState;return nullptr; }if(mCtrl->mAlignEnabled) return mCtrl->mAlignState;return mCtrl->mIdleState; }
void YoshiModelTargetState::enter() { mReset=true; }
YoshiModelBlend::YoshiModelBlend(YoshiModelCtrl* ctrl) : mCtrl(ctrl),mMode(0),mStep(0),mDuration(6) { mMtx.identity(); }
void YoshiModelBlend::update() { if(mMode) { mStep++;if(mStep>=mDuration) { mMode=0;mStep=0; } } }
f32 YoshiModelBlend::rate() const { if(mDuration<=0) return 1.0f;return static_cast<f32>(mStep)/static_cast<f32>(mDuration); }

bool decomposeYoshiModelMtx(const TPos3f& mtx,TVec3f* scale,TVec3f* trans,TQuat4f* quat) {
    TVec3f x,y,z;mtx.getXDir(x);f32 sx=PSVECMag(&x);mtx.getYDir(y);f32 sy=PSVECMag(&y);mtx.getZDir(z);f32 sz=PSVECMag(&z);
    if(sx==0.0f) {
        if(sy==0.0f) return false;
        if(sz==0.0f) return false;
        PSVECCrossProduct(&y,&z,&x);
    }
    else if(sy==0.0f) {if(sz==0.0f) return false;PSVECCrossProduct(&z,&x,&y);}
    if(MR::normalizeOrZero(&x)) return false;
    if(MR::normalizeOrZero(&y)) return false;
    if(MR::normalizeOrZero(&z)) return false;
    scale->x=sx;scale->y=sy;scale->z=sz;
    TPos3f rot;rot.identity();rot.setXDir(x);rot.setYDir(y);rot.setZDir(z);rot.getQuat(*quat);mtx.getTrans(*trans);return true;
}
void setYoshiModelScale(TPos3f* mtx,const TVec3f& scale) {
    f32 zero = 0.0f;
    f32 x;
    f32 y;
    f32 z;
    z = scale.z;
    y = scale.y;
    x = scale.x;
    mtx->mMtx[0][0]=x;mtx->mMtx[0][1]=zero;mtx->mMtx[0][2]=zero;
    mtx->mMtx[1][0]=zero;mtx->mMtx[1][1]=y;mtx->mMtx[1][2]=zero;
    mtx->mMtx[2][0]=zero;mtx->mMtx[2][1]=zero;mtx->mMtx[2][2]=z;
}
void blendYoshiModelMtx(TPos3f* out,const TPos3f& input,f32 rate) {
    TQuat4f quat0;
    TVec3f scale0,trans0;
    if(!decomposeYoshiModelMtx(*out,&scale0,&trans0,&quat0)) {JMath::gekko_ps_copy12(out,&input);return;}
    TQuat4f quat1;
    TVec3f scale1,trans1;
    if(decomposeYoshiModelMtx(input,&scale1,&trans1,&quat1)) {
        TQuat4f quat;quat=quat0;quat.slerp(quat1,rate);
        TVec3f scale=scale0*(1.0f-rate)+scale1*rate;
        TVec3f trans=trans0*(1.0f-rate)+trans1*rate;
        TPos3f scaleMtx;setYoshiModelScale(&scaleMtx,scale);out->makeQuat(quat);out->concat(scaleMtx);out->setTrans(trans);
    }
}
void YoshiModelBlend::start(YoshiModelState* oldState,YoshiModelState* newState,s32 duration) {
    mDuration=duration;
    if(!mMode && oldState==mCtrl->mIdleState && newState==mCtrl->mAlignState) {mMode=1;mStep=0;return;}
    capture();
}
void YoshiModelBlend::calc(TPos3f* out,const TPos3f& input) {
    switch(mMode) {case 1:blendYoshiModelMtx(out,input,rate());break;case 2:calcCaptured(out,input);break;default:JMath::gekko_ps_copy12(out,&input);break;}
}
void YoshiModelBlend::capture() {
    mMode=2;mStep=0;
    TPos3f head,spine;head.set(MR::getJointMtx(mCtrl->mHost,sHeadJointName));spine.set(MR::getJointMtx(mCtrl->mHost,sSpineJointName));spine.invert(spine);mMtx.concat(spine,head);
}
void YoshiModelBlend::calcCaptured(TPos3f* out,const TPos3f& input) {
    TPos3f spine,mtx;spine.set(MR::getJointMtx(mCtrl->mHost,sSpineJointName));mtx.concat(spine,mMtx);blendYoshiModelMtx(&mtx,input,rate());JMath::gekko_ps_copy12(out,&mtx);
}
void YoshiModelAlignState::calc(const TPos3f&,TPos3f* out) {
    MtxPtr root=MR::getJointMtx(mCtrl->mHost,"AllRoot");TVec3f rootX(root[0][0],root[1][0],root[2][0]);if(MR::normalizeOrZero(&rootX)) return;
    TVec3f z(out->mMtx[0][2],out->mMtx[1][2],out->mMtx[2][2]);if(MR::normalizeOrZero(&z)) return;
    TQuat4f q;q.setRotate(z,rootX);
    TVec3f x(out->mMtx[0][0],out->mMtx[1][0],out->mMtx[2][0]);q.transform(x);
    TVec3f y(out->mMtx[0][1],out->mMtx[1][1],out->mMtx[2][1]);q.transform(y);
    TVec3f newZ(out->mMtx[0][2],out->mMtx[1][2],out->mMtx[2][2]);q.transform(newZ);
    TVec3f trans(out->mMtx[0][3],out->mMtx[1][3],out->mMtx[2][3]);
    out->identity();out->setXDir(x);out->setYDir(y);out->setZDir(newZ);out->setTrans(trans);
}
YoshiModelState* YoshiModelResetState::next() {
    if(mCtrl->mTargetEnabled) {if(mCtrl->mResetEnabled) return nullptr;return mCtrl->mTargetState;}
    if(mCtrl->mAlignEnabled) return mCtrl->mAlignState;return mCtrl->mIdleState;
}
void YoshiModelResetState::enter() {
    TPos3f spine;spine.set(MR::getJointMtx(mCtrl->mHost,sSpineJointName));spine.invert(spine);mMtx.set(MR::getJointMtx(mCtrl->mHost,sHeadJointName));mMtx.concat(spine,mMtx);
}
void YoshiModelResetState::calc(const TPos3f& input,TPos3f* out) {JMath::gekko_ps_copy12(out,&input);out->concat(mMtx);}

bool YoshiModelCtrl::calcHead(TPos3f* mtx,const JointControllerInfo& info) {
    if(MR::isPowerStarGetDemoActive()) return false;
    if(MR::fn_80050020()) return false;
    mCurrentState->calcJoint(mtx,info);return true;
}
bool YoshiModelCtrl::calcSpine(TPos3f* mtx,const JointControllerInfo&) {
    if(MR::isPowerStarGetDemoActive()) return false;
    if(MR::fn_80050020()) return false;
    MtxPtr head=MR::getJointMtx(mHost,sHeadJointName);TVec3f headZ(head[0][2],head[1][2],head[2][2]);TVec3f x,z,trans;mtx->getXDir(x);
    if(MR::normalizeOrZero(&x)) return false;
    JMAVECScaleAdd(&x,&headZ,&headZ,-x.dot(headZ));if(MR::normalizeOrZero(&headZ)) return false;
    mtx->getZDir(z);if(MR::normalizeOrZero(&z)) return false;
    mtx->getTrans(trans);f32 ratio=(0.5f*static_cast<f32>(mSpineBlend))/10.0f;
    TQuat4f quat;TVec3f axis;PSVECCrossProduct(&z,&headZ,&axis);f32 mag=axis.length();
    if(mag<=0.000003814697265625f) quat.set(0.0f,0.0f,0.0f,1.0f);
    else {
        f32 angle=ratio*(0.5f*JMath::sAtanTable.atan2_(mag,z.dot(headZ)));
        quat.toTvec()->scale(static_cast<f32>(sin(angle))/mag,axis);quat.w=cos(angle);
    }
    TPos3f rot;rot.makeQuat(quat);mtx->concat(rot,*mtx);mtx->setTrans(trans);return true;
}
static inline void makeYoshiModelJointMtx(TPos3f& local, const TVec3f& scale, const TVec3f& angles, const TVec3f& trans) {
    f32 cz, cy, cx, sz, sy, sx;
    double sxResult=sin(angles.x);f32 syArg=angles.y;sx=sxResult;
    double syResult=sin(syArg);f32 szArg=angles.z;sy=syResult;
    double szResult=sin(szArg);f32 cxArg=angles.x;sz=szResult;
    double cxResult=cos(cxArg);f32 cyArg=angles.y;cx=cxResult;
    double cyResult=cos(cyArg);f32 czArg=angles.z;cy=cyResult;
    cz=cos(czArg);
    f32 sxy=sx*sy,cxz=cx*sz,cxc=cx*cz;
    local.mMtx[0][0]=scale.x*(cy*cz);local.mMtx[1][0]=scale.x*(cy*sz);local.mMtx[2][0]=scale.x*-sy;
    local.mMtx[0][1]=scale.y*(sxy*cz-cxz);local.mMtx[1][1]=scale.y*(cxc+sxy*sz);local.mMtx[2][1]=scale.y*(sx*cy);
    local.mMtx[0][2]=scale.z*(cxc*sy+sx*sz);local.mMtx[2][2]=scale.z*(cx*cy);local.mMtx[1][2]=scale.z*(cxz*sy-sx*cz);
    local.setTrans(trans);local.invert(local);
}

// NONMATCHING: floating-point register allocation and scheduling differ.
void YoshiModelState::calcJoint(TPos3f* mtx,const JointControllerInfo& info) {
    const J3DTransformInfo& t=info.mJoint->getTransformInfo();TPos3f local;local.identity();
    makeYoshiModelJointMtx(local,
        TVec3f(t.mScale.x,t.mScale.y,t.mScale.z),
        TVec3f(static_cast<f32>(t.mRotation.x)/32768.0f,static_cast<f32>(t.mRotation.y)/32768.0f,static_cast<f32>(t.mRotation.z)/32768.0f),
        TVec3f(t.mTranslate.x,t.mTranslate.y,t.mTranslate.z));
    TPos3f base=*mtx;base.concat(local);TPos3f out=*mtx;calc(base,&out);mCtrl->mBlend->calc(mtx,out);
}
void YoshiModelTargetState::calcTargetRotation(const TPos3f& input,TQuat4f* quat) {
    TVec3f x,y,z;input.getXDir(x);input.getYDir(y);input.getZDir(z);MR::normalize(&x);MR::normalize(&y);MR::normalize(&z);
    TPos3f root;root.set(MR::getJointMtx(mCtrl->mHost,"AllRoot"));TVec3f rootX,rootY,rootZ;root.getXDir(rootX);root.getYDir(rootY);root.getZDir(rootZ);
    if(mCtrl->mAlignEnabled && !MR::normalizeOrZero(&rootY)) {TQuat4f q;q.setRotate(y,-rootY);q.transform(x);q.transform(y);q.transform(z);}
    if(!MR::normalizeOrZero(&rootZ)) {TQuat4f q;q.setRotate(x,rootZ);q.transform(x);q.transform(y);q.transform(z);}
    TPos3f rot;rot.identity();rot.setXDir(x);rot.setYDir(y);rot.setZDir(z);rot.getQuat(*quat);
}
namespace {
inline f32 calcYoshiTargetYaw(const TQuat4f& targetQuat, const TQuat4f& originalQuat, const TVec3f& targetDir, f32 factor) {
    f32 baseAngle,lower,upper,targetAngle;
    TVec3f targetX,targetY,targetZ,originalX,originalY,originalZ;
    targetQuat.getXDir(targetX);targetQuat.getYDir(targetY);targetQuat.getZDir(targetZ);
    originalQuat.getXDir(originalX);originalQuat.getYDir(originalY);originalQuat.getZDir(originalZ);
    TVec3f projected;JMAVECScaleAdd(&targetY,&originalX,&projected,-targetY.dot(originalX));
    if(MR::normalizeOrZero(&projected)) {
        TQuat4f q;if(originalX.dot(targetY)>0.0f) q.setRotate(originalZ,-1.5707963705062866f);else q.setRotate(originalZ,1.5707963705062866f);
        projected.x=originalX.x;projected.y=originalX.y;projected.z=originalX.z;q.transform(projected);JMAVECScaleAdd(&targetY,&projected,&projected,-targetY.dot(projected));MR::normalize(&projected);
    }
    baseAngle=projected.angle(targetX);if(targetZ.dot(projected)>0.0f) baseAngle=-baseAngle;
    f32 limit=1.919862151145935f;
    upper=limit-baseAngle;lower=-limit-baseAngle;
    TVec3f targetProjected;JMAVECScaleAdd(&targetY,&targetDir,&targetProjected,-targetY.dot(targetDir));TVec3f cross;PSVECCrossProduct(&projected,&targetY,&cross);MR::normalize(&cross);
    targetAngle=projected.angle(targetProjected);if(cross.dot(targetProjected)>0.0f) targetAngle=-targetAngle;
    if(targetAngle>upper) targetAngle=upper;if(targetAngle<lower) targetAngle=lower;
    return baseAngle+targetAngle*factor;
}

inline f32 calcYoshiTargetPitch(const TQuat4f& targetQuat, const TVec3f& targetDir) {
    f32 upper=2.356194496154785f;f32 lower=0.7853981852531433f;
    TVec3f up;targetQuat.getYDir(up);f32 elevation=up.angle(targetDir);if(elevation>upper) elevation=upper;else if(elevation<lower) elevation=lower;
    return 1.5707963705062866f + -elevation;
}
}

void YoshiModelTargetState::calc(const TPos3f& input,TPos3f* out) {
    if(mReset) {JMath::gekko_ps_copy12(&mInitialMtx,&input);mInitialMtx.invert(mInitialMtx);mInitialMtx.concat(*out);mReset=false;}
    TQuat4f oldQuat;TVec3f scale,trans;
    if(!decomposeYoshiModelMtx(*out,&scale,&trans,&oldQuat)) {JMath::gekko_ps_copy12(out,&input);out->concat(mInitialMtx);return;}
    if(!mTarget) {JMath::gekko_ps_copy12(out,&input);out->concat(mInitialMtx);return;}
    TVec3f y;out->getYDir(y);TVec3f targetDir=*mTarget-(trans-y*mOffset);
    if(MR::normalizeOrZero(&targetDir)) {JMath::gekko_ps_copy12(out,&input);out->concat(mInitialMtx);return;}
    TQuat4f targetQuat;calcTargetRotation(input,&targetQuat);
    TPos3f original;original.concat(input,mInitialMtx);TQuat4f originalQuat;TVec3f originalScale,originalTrans;decomposeYoshiModelMtx(original,&originalScale,&originalTrans,&originalQuat);
    f32 factor=0.1f;if(mCtrl->mBlend->mMode) factor=1.0f;
    f32 yaw=calcYoshiTargetYaw(targetQuat,originalQuat,targetDir,factor);
    f32 pitch=calcYoshiTargetPitch(targetQuat,targetDir);
    TQuat4f pitchQuat,yawQuat,result;
    pitchQuat.setRotate(TVec3f(0.0f,0.0f,1.0f),pitch);yawQuat.setRotate(TVec3f(0.0f,1.0f,0.0f),yaw);
    PSQUATMultiply(&targetQuat,&yawQuat,&result);PSQUATMultiply(&result,&pitchQuat,&result);out->makeQuat(result);
    TPos3f scaleMtx;scaleMtx.zeroTrans();setYoshiModelScale(&scaleMtx,scale);out->concat(scaleMtx);out->setTrans(trans);
    JMath::gekko_ps_copy12(&mInitialMtx,&input);mInitialMtx.invert(mInitialMtx);mInitialMtx.concat(*out);
}
