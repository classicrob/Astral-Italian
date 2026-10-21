#pragma once
#include <petari/wm_route_diag.hpp>
#include "Game/Camera/Camera.hpp"
#include "Game/Camera/CameraHolder.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraManEvent.hpp"
#include "Game/Camera/CameraParamChunk.hpp"
#include "Game/Camera/CameraParamChunkID.hpp"
#include "Game/Camera/CameraPoseParam.hpp"
#include "Game/Camera/CameraTargetHolder.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/Camera/CameraViewInterpolator.hpp"
#include "Game/Util/CameraUtil.hpp"
namespace PetariWmRouteDiag {
inline void chunk(const char* source, const char* slot, const CameraParamChunk* p, unsigned frame, const CameraTargetArg* arg, const CameraHolder* holder) {
    const int type=p?int(p->mCameraTypeIndex):-1;
    const char* typeName=holder && type>=0 && type<holder->getNum()?holder->getNameStrOf(type):"UNKNOWN/null";
    std::fprintf(stderr,"[wm-camera-chunk] aurora_frame=%llu seq=%llu thread=%llu phase=%s source=%s slot=%s ptr=%p name=%s zone-valid=%d zone=%d type=%d/%s frames=%u arg=(%p,%p,%p,%p)\n",
        runtimeFrame(),++eventSequence,threadId(),phase,source,slot,p,p && p->mParamChunkID && p->mParamChunkID->mName ? p->mParamChunkID->mName : "UNKNOWN/null",
        p && p->mParamChunkID,p && p->mParamChunkID ? p->mParamChunkID->mZoneID : 0,type,typeName,frame,
        arg?arg->mTargetObj:nullptr,arg?arg->mTargetMtx:nullptr,arg?arg->mLiveActor:nullptr,arg?arg->mMarioActor:nullptr);
}
inline void cameraQueue(const char* source, const CameraManEvent* e) {
    Observation observation;
    if (!enabled() || !world || !e) return;
    chunk(source,"effective",e->mChunk,0,nullptr,e->mHolder);
    for (int i=0;i<NR_FIFO_ITEMS;++i) {
        char slot[32]; std::snprintf(slot,sizeof(slot),"priority%d-current",i);
        chunk(source,slot,e->mItems[i].mFirst.mChunk,e->mItems[i].mFirst.mChunk?e->mItems[i].mFirst.mFrame:0,e->mItems[i].mFirst.mChunk?&e->mItems[i].mFirst.mTargetArg:nullptr,e->mHolder);
        std::snprintf(slot,sizeof(slot),"priority%d-queued",i);
        chunk(source,slot,e->mItems[i].mSecond.mChunk,e->mItems[i].mSecond.mChunk?e->mItems[i].mSecond.mFrame:0,e->mItems[i].mSecond.mChunk?&e->mItems[i].mSecond.mTargetArg:nullptr,e->mHolder);
    }
}
inline void applied(const CameraManEvent* e) {
    Observation observation;
    if (!enabled() || !world) return;
    static const CameraParamChunk* previous=nullptr;
    if (previous!=e->mChunk || runtimeFrame()%60==0) {
        previous=e->mChunk;
        if (event()) cameraQueue("CameraManEvent::changeCamera",e);
    }
}
inline void cameraPose() {
    Observation observation;
    auto* d=MR::getCameraDirector();
    if (!d) { std::fprintf(stderr,"[wm-camera] aurora_frame=%llu director=UNKNOWN/null\n",runtimeFrame()); return; }
    auto* man=d->getCurrentCameraMan();
    const TVec3f eye=MR::getCamPos(),up=MR::getCamYdir(),front=MR::getCamZdir();
    const auto* interp=d->mViewInterpolator;
    std::fprintf(stderr,"[wm-camera] aurora_frame=%llu seq=%llu phase=%s man=%p man-name=%s event-active=%d eye=(%.6g,%.6g,%.6g) up=(%.6g,%.6g,%.6g) forward=(%.6g,%.6g,%.6g) fovy=%.6g near=%.6g far=%.6g aspect=%.6g\n",
        runtimeFrame(),++eventSequence,phase,man,man&&man->getName()?man->getName():"UNKNOWN",man==d->mCameraManEvent,
        eye.x,eye.y,eye.z,up.x,up.y,up.z,front.x,front.y,front.z,MR::getFovy(),MR::getNearZ(),MR::getFarZ(),MR::getAspect());
    const auto* target=d->mTargetHolder?d->mTargetHolder->mTarget:nullptr;
    if(target) {
        const auto& t=target->getPosition();
        std::fprintf(stderr,"[wm-active-target] aurora_frame=%llu phase=%s target=%p pos=(%.6g,%.6g,%.6g)\n",runtimeFrame(),phase,target,t.x,t.y,t.z);
    } else std::fprintf(stderr,"[wm-active-target] aurora_frame=%llu phase=%s valid=0\n",runtimeFrame(),phase);
    if (interp) std::fprintf(stderr,"[wm-interp] aurora_frame=%llu phase=%s watch=(%.6g,%.6g,%.6g) timer=%u duration=%u fovy=%.6g target-fovy=%.6g force=%d\n",
        runtimeFrame(),phase,interp->mTargetPosition.x,interp->mTargetPosition.y,interp->mTargetPosition.z,interp->mInterpolateTimer,interp->mInterpolateTime,interp->mFovy,interp->mTargetFovy,interp->mIsForceCameraChange);
    cameraQueue("draw",d->mCameraManEvent);
    if (d->mCameraManEvent && d->mCameraManEvent->mCamera) {
        auto* cam=d->mCameraManEvent->mCamera; auto* pose=cam->mPoseParam;
        std::fprintf(stderr,"[wm-camera-param] aurora_frame=%llu phase=%s camera=%p name=%s pose-valid=%d pose-fovy=%.6g local=(%.6g,%.6g,%.6g) global=(%.6g,%.6g,%.6g)\n",
            runtimeFrame(),phase,cam,cam->getName()?cam->getName():"UNKNOWN",pose!=nullptr,pose?pose->mFovy:0.0f,
            pose?pose->mLocalOffset.x:0.0f,pose?pose->mLocalOffset.y:0.0f,pose?pose->mLocalOffset.z:0.0f,
            pose?pose->mGlobalOffset.x:0.0f,pose?pose->mGlobalOffset.y:0.0f,pose?pose->mGlobalOffset.z:0.0f);
    }
    const auto& v=MR::getCameraViewMtx(); const auto& p=MR::getCameraProjectionMtx();
    for (int i=0;i<4;++i) {
        std::fprintf(stderr,"[wm-proj] aurora_frame=%llu phase=%s row=%d values=(%.8g,%.8g,%.8g,%.8g)\n",runtimeFrame(),phase,i,p.get(i,0),p.get(i,1),p.get(i,2),p.get(i,3));
        if (i<3) std::fprintf(stderr,"[wm-view] aurora_frame=%llu phase=%s row=%d values=(%.8g,%.8g,%.8g,%.8g)\n",runtimeFrame(),phase,i,v.get(i,0),v.get(i,1),v.get(i,2),v.get(i,3));
    }
}
}
