#include "Game/Camera/CameraCover.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Util.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>

namespace {
    const f32 sAngleThreshold = (JGeometry::TUtil< f32 >::PI() * 60.0f) / 180.0f;
};  // namespace

CameraCover::CameraCover(const char* pName) : NameObj(pName) {
    mCoverFrames = 0;
    mIsCoverDisabled = 0;
    mIsCoverRequested = false;
    mCoverDuration = 1;
    mActor = new CaptureScreenActor(MR::DrawType_CaptureScreenCamera, "Camera");
    MR::connectToScene(this, MR::MovementType_CameraCover, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_CameraCover);
    MR::joinToNameObjGroup(this, "IgnorePauseNameObj");
    mInvViewMtx.identity();
    mActor->initWithoutIter();
}

void CameraCover::movement() {
    if (isCameraHopping()) {
        if (!mCoverFrames) {
            MR::startToCaptureScreen("Camera");
            CategoryList::execute(MR::DrawType_CaptureScreenCamera);
            mCoverFrames = mCoverDuration + 1;
        }
    }

    if (mCoverFrames && !--mCoverFrames) {
        MR::endToCaptureScreen("Camera");
    }

    copyCamera();
    mIsCoverDisabled = 0;
    mIsCoverRequested = false;
}

void CameraCover::draw() const {
    if (mCoverFrames) {
        JUTTexture texture(MR::getScreenResTIMG(), 0);

        ImageEffectLocalUtil::setupDrawTexture();
        ImageEffectLocalUtil::drawTexture(&texture, 1, 0, 255, (ImageEffectLocalUtil::ETexDrawType)0);
    }
}

void CameraCover::cover(u32 a1) {
    mCoverDuration = a1;
    mIsCoverRequested = true;
}

bool CameraCover::isCameraHopping() const {
    if (mIsCoverDisabled) {
        return false;
    }

    return mIsCoverRequested;
}

void CameraCover::copyCamera() {
    mInvViewMtx.set(MR::getCameraInvViewMtx());
}

CameraCover::~CameraCover() {
}
