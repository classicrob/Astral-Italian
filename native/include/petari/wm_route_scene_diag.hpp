#pragma once
#include <petari/wm_route_camera_diag.hpp>
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapSequence.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyEnd.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <cmath>
namespace PetariWmRouteDiag {
inline bool validCount(int n) { return n>=0 && n<=4096; }
inline const char* nameFor(const Spine* spine, const Nerve* nerve) {
    if (!nerve) return "null";
    const auto* k=spine?spine->mStateKeeper:nullptr;
    if (k && k->mStates && validCount(k->mLength) && k->mLength<=k->mStatesCapacity)
        for (int i=0;i<k->mLength;++i) if (k->mStates[i].mNerve==nerve) return k->mStates[i].mName?k->mStates[i].mName:"UNKNOWN/name-null";
    return "UNKNOWN/unregistered";
}
inline const char* resultName(const Nerve* p) {
    using namespace NrvWorldMapResultStateGalaxyMap;
    if (!p) return "null";
    if (p==&WorldMapResultStateGalaxyMapStart::sInstance) return "source-Start";
    if (p==&WorldMapResultStateGalaxyMapWait::sInstance) return "source-Wait";
    if (p==&WorldMapResultStateGalaxyMapEnd::sInstance) return "source-End";
    if(p==&NrvWorldMapResultStateGrandGalaxyEnd::WorldMapResultStateGrandGalaxyEndZoomOut::sInstance) return "source-GrandEnd-ZoomOut";
    if(p==&NrvWorldMapResultStateGrandGalaxyEnd::WorldMapResultStateGrandGalaxyEndWait::sInstance) return "source-GrandEnd-Wait";
    return "UNKNOWN";
}
inline void state(const char* label, const NerveExecutor* executor, bool result=false) {
    const auto* s=executor?executor->mSpine:nullptr;
    const auto* keeper=s?s->mStateKeeper:nullptr;
    const auto* current=keeper?keeper->mCurrentState:nullptr;
    const auto* actor=current?current->mInterface:nullptr;
    std::fprintf(stderr,"[wm-state] aurora_frame=%llu seq=%llu thread=%llu phase=%s label=%s executor=%p spine=%p current=%p/%s pending=%p/%s step=%d keeper-state=%s dead=%d actor-step=%d actor-current=%p/%s actor-pending=%p/%s\n",
        runtimeFrame(),++eventSequence,threadId(),phase,label,executor,s,s?s->mCurrNerve:nullptr,s?(result?resultName(s->mCurrNerve):nameFor(s,s->mCurrNerve)):"UNKNOWN",
        s?s->mNextNerve:nullptr,s?(result?resultName(s->mNextNerve):nameFor(s,s->mNextNerve)):"UNKNOWN",s?s->mStep:-1,
        current&&current->mName?current->mName:"UNKNOWN/null",actor?int(actor->mIsDead):-1,actor&&actor->mSpine?actor->mSpine->mStep:-1,
        actor&&actor->mSpine?actor->mSpine->mCurrNerve:nullptr,actor&&actor->mSpine?resultName(actor->mSpine->mCurrNerve):"UNKNOWN",
        actor&&actor->mSpine?actor->mSpine->mNextNerve:nullptr,actor&&actor->mSpine?resultName(actor->mSpine->mNextNerve):"UNKNOWN");
}
inline void bodyState(const char* label,const NerveExecutor* e) {
    Observation observation;
    if(!enabled() || !world) return;
    const int step=e&&e->mSpine?e->mSpine->mStep:-1;
    if((step==0 || step==6 || step==20 || step==60 || (step>0 && step%60==0)) && event()) {
        std::fprintf(stderr,"[wm-body] aurora_frame=%llu seq=%llu thread=%llu phase=%s actual-body=%s step=%d\n",runtimeFrame(),++eventSequence,threadId(),phase,label,step);
        state(label,e,true);
    }
}
inline std::uintptr_t mix(std::uintptr_t h, std::uintptr_t v) { return (h^v)*1099511628211ull; }
inline std::uintptr_t stateKey(const NerveExecutor* e) {
    const auto* s=e?e->mSpine:nullptr;
    return s?mix(reinterpret_cast<std::uintptr_t>(s->mCurrNerve),reinterpret_cast<std::uintptr_t>(s->mNextNerve)):0;
}
inline std::uintptr_t key(WorldMapOwner* o) {
    if (!o || o!=readyOwner) return 0;
    if(!o->mContext) return 0;
    auto* g=static_cast<GrandGalaxyMap*>(o->mMainLayout); auto* c=o->mContext->mCamera;
    std::uintptr_t h=stateKey(o->mSequence);
    h=mix(h,g?stateKey(g->mSequence):0); h=mix(h,g?g->mIsEnd:0);
    h=mix(h,o->mSequence?stateKey(o->mSequence->mStateGalaxyMap):0); h=mix(h,c?c->mCurrent+1:0);
    auto* d=MR::getCameraDirector(); auto* e=d?d->mCameraManEvent:nullptr;
    h=mix(h,reinterpret_cast<std::uintptr_t>(e?e->mChunk:nullptr));
    if (e) for (int i=0;i<NR_FIFO_ITEMS;++i) { h=mix(h,reinterpret_cast<std::uintptr_t>(e->mItems[i].mFirst.mChunk)); h=mix(h,reinterpret_cast<std::uintptr_t>(e->mItems[i].mSecond.mChunk)); }
    return h;
}
struct Counts { unsigned total{},alive{},hidden{},clipped{},nulls{}; };
inline void actor(const LiveActor* p,int domain,int node,Counts& n,bool join) {
    if (!p) { ++n.nulls; return; } ++n.total; n.alive+=!p->mFlags.mIsDead; n.hidden+=p->mFlags.mIsNoEntryDrawBuffer; n.clipped+=p->mFlags.mIsClipped;
    if (join && p->mModelManager) addModel(p->mModelManager->getJ3DModel(),domain,node);
}
inline void countLine(const char* label,int domain,const Counts& n) {
    std::fprintf(stderr,"[wm-actors] aurora_frame=%llu phase=%s domain=%d category=%s total=%u alive=%u hidden=%u clipped=%u null=%u\n",runtimeFrame(),phase,domain,label,n.total,n.alive,n.hidden,n.clipped,n.nulls);
}
inline void worldActors(WorldMapResultWorld* w,int domain,bool join,bool print) {
    if (!w) { if(print) std::fprintf(stderr,"[wm-world] aurora_frame=%llu domain=%d ready=0\n",runtimeFrame(),domain); return; }
    Counts parts,marks,galaxies,lines,attached;
    auto* p=w->mUnit14; if(p && p->mParts && validCount(p->mNum)) for(int i=0;i<p->mNum;++i) actor(p->mParts[i].mActor,domain,p->mParts[i].mPointIndex,parts,join);
    auto* m=w->mUnit18; if(m && m->mMarks && validCount(m->mNum)) for(int i=0;i<m->mNum;++i) if(auto* a=m->mMarks[i]) { actor(a,domain,a->mPointIndex,marks,join); actor(a->mShadow,domain,a->mPointIndex,attached,join); } else ++marks.nulls;
    auto* g=w->mWorldData; if(g && g->mInfos && validCount(g->mNum)) for(int i=0;i<g->mNum;++i) if(auto* a=g->mInfos[i]) { actor(a,domain,a->mPointIndex,galaxies,join); actor(a->mShadow,domain,a->mPointIndex,attached,join); actor(a->mBack,domain,a->mPointIndex,attached,join); actor(a->mFlag,domain,a->mPointIndex,attached,join); } else ++galaxies.nulls;
    auto* l=w->mLinks; if(l && l->mLines && validCount(l->mNum)) for(int i=0;i<l->mNum;++i) if(auto* a=l->mLines[i]) { actor(a,domain,-1,lines,join); actor(a->mEffectActor,domain,-1,attached,join); actor(a->mShadowActor,domain,-1,attached,join); } else ++lines.nulls;
    if(print) {
        std::fprintf(stderr,"[wm-world] aurora_frame=%llu phase=%s domain=%d world=%p id=%d nodes=%d arrays=(%p,%p,%p,%p) count-valid=(%d,%d,%d,%d)\n",runtimeFrame(),phase,domain,w,w->mWorldId,w->mMapData?w->mMapData->mNum:-1,p,m,g,l,p&&validCount(p->mNum),m&&validCount(m->mNum),g&&validCount(g->mNum),l&&validCount(l->mNum));
        countLine("parts",domain,parts);countLine("marks",domain,marks);countLine("galaxies",domain,galaxies);countLine("routes",domain,lines);countLine("attached",domain,attached);
    }
}
inline void backdrop(WorldMapOwner* o,bool join,bool print) {
    Counts n; const auto* model=o&&o->mContext?o->mContext->mModel:nullptr;
    actor(model,0,-1,n,join);
    if(model && model->mChildren.mArr && validCount(model->mChildren.mMaxSize)) for(int i=0;i<model->mChildren.mMaxSize;++i) actor(model->mChildren.mArr[i],0,-1,n,join);
    if(print) countLine("local-backdrop",0,n);
}
inline void node(WorldMapResultWorld* w,int domain,int current) {
    auto* data=w?w->mMapData:nullptr; bool valid=data&&data->mNodes&&validCount(data->mNum)&&current>=0&&current<data->mNum;
    if(!valid) { std::fprintf(stderr,"[wm-node] aurora_frame=%llu domain=%d node=%d valid=0\n",runtimeFrame(),domain,current); return; }
    const auto& n=data->mNodes[current]; const auto& v=MR::getCameraViewMtx();const auto& p=MR::getCameraProjectionMtx();TVec3f view;
    v.mult(n.mPos,view);
    float cw=p.get(3,0)*view.x+p.get(3,1)*view.y+p.get(3,2)*view.z+p.get(3,3);
    float cx=p.get(0,0)*view.x+p.get(0,1)*view.y+p.get(0,2)*view.z+p.get(0,3);
    float cy=p.get(1,0)*view.x+p.get(1,1)*view.y+p.get(1,2)*view.z+p.get(1,3);
    std::fprintf(stderr,"[wm-node] aurora_frame=%llu phase=%s domain=%d node=%d valid=1 layer=%d hidden=%d pos=(%.6g,%.6g,%.6g) view=(%.6g,%.6g,%.6g) clip=(%.6g,%.6g,%.6g) ndc-valid=%d\n",
        runtimeFrame(),phase,domain,current,n.mLayer,n.mIsHidden,n.mPos.x,n.mPos.y,n.mPos.z,view.x,view.y,view.z,cx,cy,cw,std::isfinite(cx)&&std::isfinite(cy)&&std::isfinite(cw)&&cw!=0);
}
inline void tuple(WorldMapOwner* o,const char* source,bool draw=false) {
    Observation observation;
    if(!o || o!=readyOwner) { std::fprintf(stderr,"[wm-unready] aurora_frame=%llu seq=%llu thread=%llu phase=%s source=%s owner=%p ready-owner=%p\n",runtimeFrame(),++eventSequence,threadId(),phase,source,o,readyOwner); return; }
    if(!o->mContext) return;
    auto* c=o->mContext->mCamera; auto* g=static_cast<GrandGalaxyMap*>(o->mMainLayout);
    auto* local=o->mContext->mWorld; auto* all=g&&g->mParts?g->mParts->mWorld:nullptr;
    auto* r=o->mSequence?o->mSequence->mStateGalaxyMap:nullptr;
    std::fprintf(stderr,"[wm-owner] aurora_frame=%llu seq=%llu phase=%s source=%s thread=%llu epoch=%u owner=%p context-world=%d local=%p grand=%p galaxy-map=%d grand-end=%d current=%d next=%d moving-left=%d saved-local=%d saved-grand=%d result-dead=%d logical=(%d,%d)\n",
        runtimeFrame(),++eventSequence,phase,source,threadId(),worldEpoch,o,o->mContext->mWorldId,local,all,o->mSequence?int(o->isGalaxyMap()):-1,g?int(g->mIsEnd):-1,c?c->mCurrent:-1,c?c->mNext:-1,c?int(c->mMovingLeft):-1,r?r->mInt10:-1,r?r->mInt14:-1,r?int(r->mIsDead):-1,MR::getScreenWidth(),MR::getScreenHeight());
    if(c && c->mCameraTarget) {const auto& t=c->mCameraTarget->mPosition; std::fprintf(stderr,"[wm-holder] aurora_frame=%llu phase=%s actor=%p target=(%.6g,%.6g,%.6g)\n",runtimeFrame(),phase,c->mCameraTarget,t.x,t.y,t.z);}
    state("owner-sequence",o->mSequence);state("grand-sequence",g?g->mSequence:nullptr);state("result-galaxy-map",r,true);
    if(draw) {node(local,0,c?c->mCurrent:-1);node(all,1,c?c->mCurrent:-1);backdrop(o,false,true);worldActors(local,0,false,true);worldActors(all,1,false,true);cameraPose();}
}
inline void checkpoint(WorldMapOwner* o,const char* source) {
    Observation observation;
    if(event()) tuple(o,source);
}
inline void retireScene() {
    Observation observation;
    if(!enabled()) return;
    if(world) std::fprintf(stderr,"[wm-retire] aurora_frame=%llu seq=%llu thread=%llu source=GameScene-destructor owner=%p graph-traversed=0\n",runtimeFrame(),++eventSequence,threadId(),readyOwner);
    readyOwner=nullptr;counting=false;modelCount=0;world=false;selected=false;
}
inline void retired(WorldMapOwner* o) {
    if(enabled() && readyOwner==o) { readyOwner=nullptr;counting=false;modelCount=0;world=false; }
}
inline void consumed(WorldMapOwner* o,bool after,bool eventFlag) {
    Observation observation;
    if(!enabled()) return;world=true;readyOwner=nullptr;++worldEpoch;phase="owner-init-consume";
    std::fprintf(stderr,"[wm-consume] aurora_frame=%llu seq=%llu thread=%llu epoch=%u owner=%p consumed-startAtMap=%d consumed-grand-event=%d consumed-after-valid=%d consumed-after=%d raw-after-now=%d current-world-before-stage-selection=%d current-node=%d enter=(%d,%d,%d,%d,%d,%d)\n",
        runtimeFrame(),++eventSequence,threadId(),worldEpoch,o,after,eventFlag,eventFlag,eventFlag?int(after):-1,GameSequenceFunction::isWorldMapAfterFileSelect(),GameDataFunction::getCurrentWorldId(),GameSequenceFunction::getWorldMapCurrentNode(),
        GameSequenceFunction::isWorldMapEnterType0(),GameSequenceFunction::isWorldMapEnterType1(),GameSequenceFunction::isWorldMapEnterType2(),GameSequenceFunction::isWorldMapEnterType3(),GameSequenceFunction::isWorldMapEnterType4(),GameSequenceFunction::isWorldMapEnterType5());
}
inline void flags(const char* source) {
    Observation observation;
    if(!enabled()) return;
    std::fprintf(stderr,"[wm-flags] aurora_frame=%llu seq=%llu phase=%s source=%s observed-after=%d current-world=%d current-node=%d enter=(%d,%d,%d,%d,%d,%d)\n",
        runtimeFrame(),++eventSequence,phase,source,GameSequenceFunction::isWorldMapAfterFileSelect(),GameDataFunction::getCurrentWorldId(),GameSequenceFunction::getWorldMapCurrentNode(),
        GameSequenceFunction::isWorldMapEnterType0(),GameSequenceFunction::isWorldMapEnterType1(),GameSequenceFunction::isWorldMapEnterType2(),GameSequenceFunction::isWorldMapEnterType3(),GameSequenceFunction::isWorldMapEnterType4(),GameSequenceFunction::isWorldMapEnterType5());
}
inline void postInit(WorldMapOwner* o) { Observation observation; if(enabled()) { readyOwner=o;phase="owner-init-before-clear";flags("before-clear");tuple(o,"owner-init-before-clear"); } }
inline void movement(WorldMapOwner* o) {
    Observation observation;
    if(!enabled()) return;phase="owner-update";static std::uintptr_t previous=0;auto k=key(o);
    if(k!=previous || runtimeFrame()%60==0) { previous=k;tuple(o,"after-owner-movement"); }
}
inline void beginDraw() {
    Observation observation;
    if(!enabled()) return;world=MR::isStageWorldMap();counting=false;phase="draw-main";
    if(!world || !MR::isExistSceneObj(0x33)) return;
    auto* o=MR::getSceneObj<WorldMapOwner>(0x33);
    if(o!=readyOwner) { tuple(o,"draw-before-owner-init-complete");return; }
    static std::uintptr_t previous=0;static unsigned draws=0, previousEpoch=0;auto k=key(o);
    if(previousEpoch!=worldEpoch) { previousEpoch=worldEpoch;draws=0;previous=0; }
    selected=draws++<4 || runtimeFrame()%60==0 || k!=previous; previous=k;
    modelCount=modelOverflow=0;pass=0;
    auto* g=o?static_cast<GrandGalaxyMap*>(o->mMainLayout):nullptr;
    backdrop(o,true,false);worldActors(o&&o->mContext?o->mContext->mWorld:nullptr,0,true,false);worldActors(g&&g->mParts?g->mParts->mWorld:nullptr,1,true,false);
    counting=true;
    if(selected) tuple(o,"before-main-draw",true);
}
inline void endDraw() {
    Observation observation;
    if(!enabled() || !world || !counting) return;phase="after-draw";counting=false;
    if(selected) for(int domain=0;domain<2;++domain) std::fprintf(stderr,"[wm-submit] aurora_frame=%llu phase=after-draw domain=%d main-models=%u mini-models=%u main-unique-nodes=%u mini-unique-nodes=%u tracked=%u overflow=%u meaning=nonhidden-shape-packet-submission-not-rasterization\n",
        runtimeFrame(),domain,submittedModels(domain,0),submittedModels(domain,1),submittedNodes(domain,0),submittedNodes(domain,1),modelCount,modelOverflow);
}
}
