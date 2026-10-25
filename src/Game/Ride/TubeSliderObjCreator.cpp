#pragma auto_inline off
#include "Game/Ride/TubeSliderObjCreator.hpp"
#include "Game/Ride/TubeSlider.hpp"
#include "Game/Ride/TubeSliderFunction.hpp"
#include "Game/Ride/TubeSliderJumpStep.hpp"
#include "Game/Ride/TubeSliderCrystal.hpp"
#include "Game/MapObj/TicoCoin.hpp"
#include "Game/Util.hpp"
TubeSliderObjCreator::TubeSliderObjCreator(TubeSlider* slider, const char* name) : NameObj("オブジェ生成（チューブスライダー）"), mSlider(slider), mObjName(name) {}
namespace {
ALWAYS_INLINE void createObj(TubeSlider* slider, const char* name, f32 offset, TVec3f& up, const JMapInfoIter& iter) {
    LiveActor* obj = 0;
    if (MR::isEqualString(name, "TubeSliderJumpStep")) obj = new TubeSliderJumpStep(-MR::getRailDirection(slider));
    else if (MR::isEqualString(name, "TubeSliderCrystal")) obj = new TubeSliderCrystal(-MR::getRailDirection(slider));
    else if (MR::isEqualString(name, "TubeSliderTicoCoin")) obj = new TicoCoin("チココイン（チューブスライダー）");
    TubeSliderFunction::getUnknown1(&obj->mPosition, slider, up, -offset);
    TVec3f savedPos(obj->mPosition);
    obj->mGravity.set(up);
    MR::setClippingFar300m(obj);
    obj->init(iter);
    obj->mGravity.set(up);
    MR::setPosition(obj, savedPos);
}
}
// NONMATCHING: call-argument load scheduling; JumpStep ctor symbol needs its erroneous _v suffix removed.
namespace {
    inline TubeSlider* getObjectTubeSlider(const TubeSliderObjCreator& creator) {
        return creator.mSlider;
    }
}

void TubeSliderObjCreator::init(const JMapInfoIter& iter) {
    TVec3f pos(0.0f);
    MR::getJMapInfoTrans(iter, &pos);
    MR::moveCoordToNearestPos(mSlider, pos);
    TubeSliderFunction::updateTubeSliderInfo(mSlider, true);
    f32 angle = -1.0f;
    MR::getJMapInfoArg0NoInit(iter, &angle);
    f32 offset = 0.0f;
    MR::getJMapInfoArg1NoInit(iter, &offset);
    TVec3f up(0.0f, 1.0f, 0.0f);
    if (angle == -1.0f) angle = TubeSliderFunction::getFloorAngle(mSlider);
    TubeSliderFunction::calcTubeVecDegree(angle, &up, getObjectTubeSlider(*this));
    createObj(mSlider, mObjName, offset, up, iter);
}
TubeSliderObjCreator::~TubeSliderObjCreator() {}
