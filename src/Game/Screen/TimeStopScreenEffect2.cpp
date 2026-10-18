#include "Game/Screen/TimeStopScreenEffect.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>

using namespace NrvTimeStopScreenEffect;

TimeStopScreenEffect::TimeStopScreenEffect(const char* name) : LiveActor(name), mCaptureTexture(0) {
    MR::createSceneObj(0x30);
}

void TimeStopScreenEffect::init(const JMapInfoIter& iter) {
    mCaptureTexture = MR::getTimeStopScreenCaptureTexture();
    MR::initDefaultPos(this, iter);
    MR::initActor(this, iter, "TimeStopScreenEffect", false);
    MR::connectToScene(this, 0x21, 5, 0x25, 0x41);
    initSound(4, 0, 0, TVec3f(0.0f));
    initNerve(&TimeStopScreenEffectNrvWait::sInstance);
    MR::invalidateClipping(this);
    makeActorDead();
    MR::startBtk(this, "TimeStopScreenEffect");
    MR::startBrk(this, "Start");
}

void TimeStopScreenEffect::control() {}

void TimeStopScreenEffect::calcAndSetBaseMtx() {
    TPos3f matrix = MR::getCameraInvViewMtx();
    TPos3f translation;
    translation.identity();
    translation.setTrans(sCameraOffset);
    matrix.concat(matrix, translation);
    MR::setBaseTRMtx(this, matrix);
}

void TimeStopScreenEffect::draw() const {
    GXRenderModeObj* mode = JUTVideo::getManager()->getRenderMode();
    GXSetCopyFilter(false, mode->sample_pattern, false, mode->vfilter);
    mCaptureTexture->capture(0, 0, GX_TF_RGB565, false, 0);
    GXSetCopyFilter(false, mode->sample_pattern, true, mode->vfilter);
}

void TimeStopScreenEffect::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Start");
        MR::startActionSound(this, "TimeStopEffectStart", -1, -1, -1);
    }
    if (MR::isBrkStopped(this)) setNerve(&TimeStopScreenEffectNrvActive::sInstance);
}

void TimeStopScreenEffect::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "End");
        MR::startActionSound(this, "TimeStopEffectEnd", -1, -1, -1);
        MR::setAudioEffectType(0, 0x40, 0);
    }
    if (MR::isBrkStopped(this)) {
        setNerve(&TimeStopScreenEffectNrvWait::sInstance);
        kill();
    }
}
