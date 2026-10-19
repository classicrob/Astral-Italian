#include "Game/GameAudio/AudCameraWatcher.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include <JSystem/JMath/JMath.hpp>

AudCameraWatcher::AudCameraWatcher() : NameObj("AudCameraWatcher"), _C(0), _14(0.0f) {
}

void AudCameraWatcher::playAtmosphereSE() {
    if (MR::isCameraInWater() || (MR::isInWater(*MR::getPlayerPos()) && MR::getFovy() < 20.0f)) {
        MR::startAtmosphereLevelSE("SE_AT_LV_UNDER_WATER");
        AudWrap::getSystem()->mAudEffector->setCutoff(3);
        MR::setStageBGMStateBit(2);
        return;
    }
    AudWrap::getSystem()->mAudEffector->setCutoff(127);
}

void AudCameraWatcher::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, MR::MovementType_AudCameraWatcher, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

namespace {
    extern const f32 sTelephotoScale;
};  // namespace

// NONMATCHING: stack layout of the three matrices and saved-register allocation differ (instruction sequence and constants agree)
void AudCameraWatcher::movement() {
    if (MR::isDemoActive() || MR::isStageStateScenarioOpeningCamera() || MR::isExecScenarioStarter() || MR::isFirstPersonCamera()) {
        MR::setMicMtx(const_cast< TPos3f& >(MR::getCameraViewMtx()), 0);
        _14 = PSVECDistance(MR::getPlayerCenterPos(), &AudMicWrap::getMicPos());
        AudWrap::getSystem()->setCameraRate(1.0f);
    } else {
        TPos3f micMtx;
        TPos3f cameraMtx = MR::getCameraInvViewMtx();
        TVec3f playerPos(*MR::getPlayerCenterPos());
        f32 rate;
        if (MR::getFovy() < 20.0f) {
            rate = 0.45f * sTelephotoScale;
        } else {
            rate = 0.45f;
        }
        TPos3f invMtx;
        invMtx.set(cameraMtx);
        TVec3f cameraPos;
        cameraPos.set<f32>(cameraMtx(0, 3), cameraMtx(1, 3), cameraMtx(2, 3));
        TVec3f cameraRight;
        cameraRight.set<f32>(cameraMtx(0, 0), cameraMtx(1, 0), cameraMtx(2, 0));
        f32 along = (playerPos - cameraPos).dot(cameraRight);
        cameraRight *= along;
        TVec3f target = playerPos - cameraRight;
        TVec3f micPos;
        JMAVECLerp(&target, &cameraPos, &micPos, rate);
        invMtx.mMtx[0][3] = micPos.x;
        invMtx.mMtx[1][3] = micPos.y;
        invMtx.mMtx[2][3] = micPos.z;
        micMtx.invert(invMtx);
        _14 = PSVECDistance(&micPos, &playerPos);
        MR::setMicMtx(micMtx, 0);
        AudWrap::getSystem()->setCameraRate(_14 / 1300.0f);
    }

    playAtmosphereSE();
}

namespace {
    const f32 sTelephotoScale = 0.5f;
};  // namespace
