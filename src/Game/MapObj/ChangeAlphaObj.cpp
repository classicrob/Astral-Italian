#pragma auto_inline off
#include "Game/MapObj/ChangeAlphaObj.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <cstdio>
namespace NrvChangeAlphaObj {
NEW_NERVE(ChangeAlphaObjNrvWait, ChangeAlphaObj, Wait);
NEW_NERVE(ChangeAlphaObjNrvAlpha, ChangeAlphaObj, Alpha);
}
ChangeAlphaObj::ChangeAlphaObj(const char* name) : MapObjActor(name), mAlphaModel(0) {}
void ChangeAlphaObj::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info, iter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info, iter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info, iter);
    info.setupNerve(&NrvChangeAlphaObj::ChangeAlphaObjNrvWait::sInstance);
    initialize(iter, info);
    char model[256];
    snprintf(model, sizeof(model), "%sAlpha", mObjectName);
    mAlphaModel = new ModelObj("半透明化オブジェ", model, getBaseMtx(), 26, 33, -2, false);
    mAlphaModel->initWithoutIter();
    MR::startBrk(mAlphaModel, "Normal");
    MR::invalidateClipping(mAlphaModel);
    mAlphaModel->makeActorDead();
}
void ChangeAlphaObj::exeWait() {
    if (!MR::isStageStateScenarioOpeningCamera() && isNeedForKill()) {
        MR::invalidateClipping(this);
        mAlphaModel->appear();
        setNerve(&NrvChangeAlphaObj::ChangeAlphaObjNrvAlpha::sInstance);
    }
}
void ChangeAlphaObj::exeAlpha() {
    if (MR::isFirstStep(this)) { MR::hideModel(this); MR::startBrk(mAlphaModel, "Alpha"); }
    if (MR::isBrkStopped(mAlphaModel) && !isNeedForKill()) { MR::validateClipping(this); mAlphaModel->kill(); }
}
bool ChangeAlphaObj::isNeedForKill() {
    TVec3f direction(MR::getCamPos() - mPosition);
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f planar(0.0f);
    MR::vecKillElement(direction, up, &planar);
    return PSVECMag(&planar) < 1500.0f;
}
ChangeAlphaObj::~ChangeAlphaObj() {}
