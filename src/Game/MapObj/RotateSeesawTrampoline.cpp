#include "Game/MapObj/RotateSeesawTrampoline.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvRotateSeesawTrampoline {
NEW_NERVE(RotateSeesawTrampolineNrvWait, RotateSeesawTrampoline, Wait);
NEW_NERVE(RotateSeesawTrampolineNrvMove, RotateSeesawTrampoline, Move);
}
RotateSeesawTrampoline::RotateSeesawTrampoline(const char* name) : MapObjActor(name) { mEffectMtx.identity(); }
void RotateSeesawTrampoline::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoSeesaw(&info, iter, 0, 0.0f);
    info.setupNerve(&NrvRotateSeesawTrampoline::RotateSeesawTrampolineNrvWait::sInstance);
    initialize(iter, info);
    mEffectMtx.set(getBaseMtx());
    MR::setEffectHostMtx(this, "Wait", mEffectMtx);
    MR::setEffectHostMtx(this, "Move", mEffectMtx);
}
void RotateSeesawTrampoline::exeWait() {
    if (MR::isFirstStep(this)) MR::emitEffect(this, "Wait");
    if (MapObjActorUtil::isRotatorMoving(this)) { setNerve(&NrvRotateSeesawTrampoline::RotateSeesawTrampolineNrvMove::sInstance); MR::deleteEffect(this, "Wait"); }
}
void RotateSeesawTrampoline::exeMove() {
    if (MR::isFirstStep(this)) MR::emitEffect(this, "Move");
    if (!MapObjActorUtil::isRotatorMoving(this)) { setNerve(&NrvRotateSeesawTrampoline::RotateSeesawTrampolineNrvWait::sInstance); MR::deleteEffect(this, "Move"); }
}
RotateSeesawTrampoline::~RotateSeesawTrampoline() {}
