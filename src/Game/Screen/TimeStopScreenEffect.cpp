#include "Game/Screen/TimeStopScreenEffect.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/TimeStopScreenCaptureTexture.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRSolidHeap.hpp>
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

TimeStopScreenCaptureTexture::TimeStopScreenCaptureTexture(const char* name) : NameObj(name), mTexture(nullptr) {
    MR::CurrentHeapRestorer restorer(MR::getSceneHeapGDDR3());
    mTexture = new JUTTexture(JUTVideo::getManager()->getRenderMode()->fbWidth,
                              JUTVideo::getManager()->getRenderMode()->efbHeight, GX_TF_RGB565);
}
TimeStopScreenCaptureTexture::~TimeStopScreenCaptureTexture() {}
TimeStopScreenEffect::~TimeStopScreenEffect() {}

TVec3f TimeStopScreenEffect::sCameraOffset(0.0f, 0.0f, -200.0f);
namespace NrvTimeStopScreenEffect {
    TimeStopScreenEffectNrvWait::TimeStopScreenEffectNrvWait() {}
    TimeStopScreenEffectNrvWait TimeStopScreenEffectNrvWait::sInstance;
    TimeStopScreenEffectNrvStart::TimeStopScreenEffectNrvStart() {}
    TimeStopScreenEffectNrvStart TimeStopScreenEffectNrvStart::sInstance;
    TimeStopScreenEffectNrvActive::TimeStopScreenEffectNrvActive() {}
    TimeStopScreenEffectNrvActive TimeStopScreenEffectNrvActive::sInstance;
    TimeStopScreenEffectNrvEnd::TimeStopScreenEffectNrvEnd() {}
    TimeStopScreenEffectNrvEnd TimeStopScreenEffectNrvEnd::sInstance;
    void TimeStopScreenEffectNrvEnd::execute(Spine* spine) const { static_cast<TimeStopScreenEffect*>(spine->mExecutor)->exeEnd(); }
    void TimeStopScreenEffectNrvActive::execute(Spine* spine) const { MR::setAudioEffectType(3, 0x40, 0); MR::setStageBGMStateBit(0x40); }
    void TimeStopScreenEffectNrvStart::execute(Spine* spine) const { static_cast<TimeStopScreenEffect*>(spine->mExecutor)->exeStart(); }
    void TimeStopScreenEffectNrvWait::execute(Spine* spine) const {  }
}
namespace MR {
    void createTimeStopScreenEffect() { createSceneObj(0x30); createSceneObj(0x31); }
    void onTimeStopScreenEffect() {
        TimeStopScreenEffect* effect = getSceneObj<TimeStopScreenEffect>(0x31);
        effect->appear();
        effect->setNerve(&NrvTimeStopScreenEffect::TimeStopScreenEffectNrvStart::sInstance);
    }
    void offTimeStopScreenEffect() {
        getSceneObj<TimeStopScreenEffect>(0x31)->setNerve(&NrvTimeStopScreenEffect::TimeStopScreenEffectNrvEnd::sInstance);
    }
    bool isTimeStopScreenEffectActive() {
        TimeStopScreenEffect* effect = getSceneObj<TimeStopScreenEffect>(0x31);
        return effect->isNerve(&NrvTimeStopScreenEffect::TimeStopScreenEffectNrvStart::sInstance) ||
               effect->isNerve(&NrvTimeStopScreenEffect::TimeStopScreenEffectNrvActive::sInstance);
    }
    JUTTexture* getTimeStopScreenCaptureTexture() {
        if (!isExistSceneObj(0x30)) return nullptr;
        return getSceneObj<TimeStopScreenCaptureTexture>(0x30)->mTexture;
    }
}
