#include "Game/Player/YoshiSilhouette.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/DrawUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DMtxBuffer.hpp>
#include <JSystem/J3DGraphAnimator/J3DJointTree.hpp>
YoshiSilhouette::YoshiSilhouette(Yoshi* host) : LiveActor("ヨッシーシルエット"),mHost(host),mHidden(false) {
    mPosition.set<f32>(host->mPosition);
    mRotation.set<f32>(host->mRotation);
    mScale.set<f32>(host->mScale);
    MR::initActor(this,"YoshiShadow",false);
    MR::connectToScene(this,0x25,0xA,-1,0x2C);
    initEffectKeeper(0,nullptr,false);
    initSound(4,nullptr,nullptr,TVec3f(0.0f));
    MR::invalidateClipping(this);
}
YoshiSilhouette::~YoshiSilhouette() {}
void YoshiSilhouette::init(const JMapInfoIter&) { makeActorAppeared(); }
void YoshiSilhouette::makeActorAppeared() { LiveActor::makeActorAppeared();mHidden=false; }
void YoshiSilhouette::makeActorDead() { LiveActor::makeActorDead();mHidden=true; }
void YoshiSilhouette::movement() {
    if(!checkHidden()) { checkAppeared();LiveActor::movement(); }
}
// NONMATCHING: equivalent clipping branch folded into the common epilogue.
void YoshiSilhouette::calcAnim() {
    if(!mHidden && !MR::isDead(this) && !MR::isNoCalcAnim(this)) {
        if(!MR::isClipped(this)) {
        J3DModel* model=MR::getJ3DModel(this);
        J3DMtxBuffer* destination=model->getMtxBuffer();
        J3DMtxBuffer* source=MR::getJ3DModel(mHost)->getMtxBuffer();
        for(s32 i=0;i<source->mJointTree->getJointNum();i++) PSMTXCopy(source->mpAnmMtx[i],destination->mpAnmMtx[i]);
        model->calcWeightEnvelopeMtx();
        model->viewCalc();
        }
    }
}
void YoshiSilhouette::calcViewAndEntry() {}
void YoshiSilhouette::draw() const { if(!mHidden) MR::drawModelAlphaShadow(this); }
bool YoshiSilhouette::checkHidden() {
    if(MR::isDead(this) || MR::isDead(mHost) || MR::isClipped(mHost) || MR::isHiddenModel(mHost)) {
        if(!mHidden) {
            mHidden=true;MR::invalidateHitSensors(this);
            if(!MR::isHiddenModel(this)) MR::disconnectToDrawTemporarily(this);
        }
        return true;
    }
    return false;
}
void YoshiSilhouette::checkAppeared() {
    if(mHidden) {
        mHidden=false;MR::validateHitSensors(this);
        if(!MR::isHiddenModel(this)) MR::connectToDrawTemporarily(this);
    }
}
