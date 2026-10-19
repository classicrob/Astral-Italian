#include "Game/Animation/AnmPlayer.hpp"
#include "Game/Util.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

AnmPlayerBase::AnmPlayerBase(const ResTable* pResTable) : mResTable(pResTable), mAnmRes(), mFrameCtrl(), mIsUpdateSkipped() {
}

void AnmPlayerBase::update() {
    if (mAnmRes != nullptr && !mIsUpdateSkipped) {
        mFrameCtrl.update();
    }
}

void AnmPlayerBase::reflectFrame() {
    if (mAnmRes != nullptr) {
        mAnmRes->mFrame = mFrameCtrl.mFrame;
    }
}

void AnmPlayerBase::clearUpdateSkipFlag() {
    mIsUpdateSkipped = false;
}

void AnmPlayerBase::start(const char* pResName) {
    J3DAnmBase* pAnmRes = reinterpret_cast< J3DAnmBase* >(mResTable->getRes(pResName));

    if (pAnmRes != mAnmRes) {
        changeAnimation(pAnmRes);
        mAnmRes = pAnmRes;
    }

    mFrameCtrl.init(mAnmRes->mFrameMax);
    mFrameCtrl.setAttribute(mAnmRes->mAttribute);
    mFrameCtrl.setFrame(0.0f);
    mFrameCtrl.setRate(1.0f);
    mIsUpdateSkipped = true;
}

void AnmPlayerBase::stop() {
    stopAnimation();
    mFrameCtrl.setRate(0.0f);
}

void AnmPlayerBase::resume() {
    mFrameCtrl.setRate(1.0f);
}

bool AnmPlayerBase::isPlaying(const char* pAnimName) const {
    if (mAnmRes != nullptr) {
        if (MR::isEqualStringCase(pAnimName, mResTable->getResName(mAnmRes))) {
            return true;
        }
    }

    return false;
}

void AnmPlayerBase::changeAnimation(J3DAnmBase*) {
}

void AnmPlayerBase::stopAnimation() {
}
