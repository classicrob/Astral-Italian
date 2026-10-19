#include "Game/Effect/SyncBckEffectChecker.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/LiveActor/ActorAnimKeeper.hpp"

SyncBckEffectChecker::SyncBckEffectChecker(XanimePlayer* pPlayer, ActorAnimKeeper* keeper)
    : mPlayer(pPlayer), mAnimKeeper(keeper), mPrevFrame(), mIsReset(), mIsCreateEnabled(), mCurrentBckName(), mPrevBckName(), mCurrentAnimName(), mPrevAnimName() {
}

// NONMATCHING: branch lowering/regalloc
void SyncBckEffectChecker::updateBefore() {
    bool enabled = false;
    if (mPlayer->isCurrentBckActive()) {
        enabled = 0.0f != mPlayer->getFrameCtrl()->getRate() || mPrevFrame != mPlayer->getFrameCtrl()->getFrame() || mIsReset;
    }
    mIsCreateEnabled = enabled;
    mCurrentBckName = mPlayer->getCurrentBckName();
    mCurrentAnimName = mAnimKeeper ? mAnimKeeper->getCurrentAnimName() : nullptr;
}

bool XanimePlayer::isCurrentBckActive() const {
    if (mFrameCtrls[mCurrentFrameCtrlIndex].checkState(1)) return false;
    return true;
}

void SyncBckEffectChecker::updateAfter() {
    mPrevBckName = mCurrentBckName;
    mPrevAnimName = mCurrentAnimName;
    mIsReset = false;
    mIsCreateEnabled = false;
    mPrevFrame = mPlayer->getFrameCtrl()->getFrame();
}

void SyncBckEffectChecker::reset() {
    mPrevFrame = 0.0f;
    mIsCreateEnabled = true;
    mIsReset = true;
}

void SyncBckEffectChecker::fn_8006F1F0() {
    if (mIsReset) mPrevFrame = mPlayer->getFrameCtrl()->getFrame();
}

// NONMATCHING: regalloc
bool SyncBckEffectChecker::isCreate(const SyncBckEffectInfo* info, bool checkFrame) const {
    if (!mIsCreateEnabled) return false;
    if (!info->isRegisteredBck(mCurrentBckName) && !info->isRegisteredBck(mCurrentAnimName)) return false;
    const J3DFrameCtrl* ctrl = mPlayer->getFrameCtrl();
    if (!checkFrame) {
        f32 end = info->mEndFrame;
        if (end < 0.0f) end = ctrl->getEnd();
        return mPlayer->isFrameInRange(info->mStartFrame, end);
    }
    f32 frame = info->mStartFrame;
    if (mIsReset) {
        if (ctrl->getRate() != 0.0f && MR::isNearZero(frame + ctrl->getRate() - ctrl->getFrame())) return true;
        f32 delta = frame - ctrl->getFrame();
        return 0.0f <= delta && delta < ctrl->getRate();
    }
    return checkPass(frame);
}

// NONMATCHING: equivalent branch layout/regalloc
bool SyncBckEffectChecker::isDelete(const SyncBckEffectInfo* info) const {
    bool registered = info->isRegisteredBck(mCurrentBckName) || info->isRegisteredBck(mCurrentAnimName);
    if (registered) {
        if (!mIsCreateEnabled) {
            if (info->mContinueAnimEnd) return false;
            return !mPlayer->isCurrentBckActive();
        }
        if (!MR::Effect::isExistSyncBckDeleteFrame(info)) return false;
        bool active = mPlayer->isFrameInRange(info->mStartFrame, info->mEndFrame) && mPlayer->isCurrentBckActive();
        return !active;
    }
    if (info->mContinueAnimEnd) {
        const char* name = mPlayer->getCurrentBckName();
        if (!name) return false;
        if (!info->isRegisteredBck(name)) return mPlayer->isTerminate(name);
        return MR::Effect::isExistSyncBckDeleteFrame(info);
    }
    if (mIsCreateEnabled) return mCurrentBckName != mPrevBckName || mCurrentAnimName != mPrevAnimName;
    return !mPlayer->isCurrentBckActive();
}

f32 SyncBckEffectChecker::fn_8006F510() const {
    return mPlayer->getFrameCtrl()->getRate();
}

bool SyncBckEffectChecker::checkPass(f32 frame) const {
    if (0.0f == mPlayer->getFrameCtrl()->getRate()) {
        return checkPassIfRate0(frame);
    }

    return mPlayer->checkPass(frame) == true;
}

bool SyncBckEffectChecker::checkPassIfRate0(f32 frame) const {
    const J3DFrameCtrl* pFrameCtrl = mPlayer->getFrameCtrl();
    const f32 currentFrame = pFrameCtrl->getFrame();

    if (pFrameCtrl->getAttribute() == J3DFrameCtrl::EMode_LOOP && currentFrame < mPrevFrame) {
        if ((mPrevFrame <= frame && frame < pFrameCtrl->getEnd()) || (pFrameCtrl->getLoop() <= frame && frame < currentFrame)) {
            return true;
        }
    } else if (mPrevFrame <= currentFrame) {
        if (mPrevFrame <= frame && frame < currentFrame) {
            return true;
        }
    } else {
        if (currentFrame <= frame && frame < mPrevFrame) {
            return true;
        }
    }

    return false;
}
