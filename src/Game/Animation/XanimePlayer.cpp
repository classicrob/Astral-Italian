#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DMtxCalc.hpp>
#include <revolution/types.h>

namespace {
    const char* dummy_name = "NULL";
};  // namespace

XanimePlayer::XanimePlayer(J3DModel* pModel, XanimeResourceTable* pRessource) {
    init();

    mModel = pModel;
    mModelData = pModel->mModelData;
    u8 flags = pModel->mModelData->mFlags & 0xF;
    mResourceTable = pRessource;

    mFrameBlendRatio = 0.0f;

    mDefaultAnimation = nullptr;
    mCurrentAnimation = nullptr;
    mNextAnimation = nullptr;

    mCore = new XanimeCore(mResourceTable->mMaxGroupInfoTableSize, mModelData->mJointTree.mJointNum, flags);

    mWeights[0] = 0.0f;
    for (u32 i = 1; i < mResourceTable->mMaxGroupInfoTableSize; i++) {
        mWeights[i] = 0.0f;
    }

    mHasAnimation = false;
    mHasDefaultAnimation = false;
    mNextAnimationReady = false;
    mAutoRunDefaultAnimation = true;

    mFrameCtrls[0].init(0);
    mFrameCtrls[1].init(0);

    mCurrentFrameCtrlIndex = 0;
    mNextFrameCtrlIndex = 0;

    mFrameCtrl = mFrameCtrls;

    mCore->initT(mModelData);
}

XanimePlayer::XanimePlayer(J3DModel* pModel, XanimeResourceTable* pRessource, XanimePlayer* pPlayer) {
    init();
    mModel = pModel;
    mModelData = pModel->mModelData;
    mResourceTable = pRessource;

    mFrameBlendRatio = 0.0f;

    mDefaultAnimation = nullptr;
    mCurrentAnimation = nullptr;
    mPrevAnimation = nullptr;

    mNextAnimation = nullptr;

    mCore = new XanimeCore(mResourceTable->mMaxGroupInfoTableSize, pPlayer->mCore);

    mWeights[0] = 0.0f;
    for (u32 i = 1; i < mResourceTable->mMaxGroupInfoTableSize; i++) {
        mWeights[i] = 0.0f;
    }

    mHasAnimation = false;
    mHasDefaultAnimation = false;
    mNextAnimationReady = false;
    mAutoRunDefaultAnimation = true;

    mFrameCtrls[0].init(0);
    mFrameCtrls[1].init(0);

    mCurrentFrameCtrlIndex = 0;
    mNextFrameCtrlIndex = 0;

    mFrameCtrl = mFrameCtrls;

    mCore->initT(mModelData);
}

void XanimePlayer::init() {
    mModel = nullptr;
    mModelData = nullptr;
    mCurrentBckName = nullptr;
    mNextAnimation = nullptr;
    mPrevAnimation = nullptr;
    mCurrentAnimation = nullptr;
    mCurrentAnimation = nullptr;
    mDefaultAnimation = nullptr;
    mCore = nullptr;
    mResourceTable = nullptr;
    mFrameCtrl = nullptr;
    mNextFrameCtrlIndex = 0;
    mCurrentFrameCtrlIndex = 0;

    f32 f1 = mCurrentFrameCtrlIndex;
    mFrameBlendRatio = f1;
    mInterpoleRatio = f1;

    mDefaultAnimationDelay = 0;
    mNextAnimationReady = false;
    mAutoRunDefaultAnimation = false;
    mHasDefaultAnimation = false;
    mHasAnimation = false;
    mHasUpdatedFrame = false;
    mAnimationChanged = false;
    mKeepFrameRatio = false;

    for (int i = 0; i < ARRAY_SIZE(mWeights); i++) {
        mWeights[i] = 0.0f;
    }

    mSimpleGroup = nullptr;
    mCachedFrame = 0.0f;
}

void XanimePlayer::setDefaultAnimation(const char* pAnimationName) {
    const XanimeGroupInfo* pDefaultAnimation = mResourceTable->getGroupInfo(pAnimationName);

    if (mCurrentAnimation == mDefaultAnimation) {
        changeAnimation(pDefaultAnimation);
    }

    mDefaultAnimation = pDefaultAnimation;
    mHasDefaultAnimation = true;
}

void XanimePlayer::changeCurrentAnimation(const XanimeGroupInfo* pAnimation) {
    if (mCurrentAnimation == pAnimation) {
        return;
    }

    mPrevAnimation = mCurrentAnimation;
    mCurrentAnimation = pAnimation;

    updateAfterMovement();
}

void XanimePlayer::stopAnimation() {
    runDefaultAnimation();
}

void XanimePlayer::stopAnimation(const char* pName) {
    if (mResourceTable->getGroupInfo(pName) == mCurrentAnimation) {
        stopAnimation();
    }
}

void XanimePlayer::changeTrackAnimation(u8 arg1, const char* pName) {
    s32 index = mResourceTable->getSingleIndex(pName);
    if (index != -1) {
        changeTrackAnimation(arg1, index);
    }
}

void XanimePlayer::changeTrackAnimation(u8 arg1, u32 index) {
    if (arg1 > mCurrentAnimation->mBckTableVariant) {
        return;
    }

    J3DAnmTransform* resMotion = static_cast< J3DAnmTransform* >(mResourceTable->findResMotion(mResourceTable->mBckTables[index].mFileName));
    if (resMotion != nullptr) {
        mCore->setBck(arg1, resMotion);
    }
}

void XanimePlayer::changeAnimationBck(const char* pName) {
    const XanimeGroupInfo* info = mResourceTable->getGroupInfo(pName);
    if (info == nullptr) {
        changeSimpleBck(pName);
    } else {
        changeAnimation(info);
    }
}

bool XanimePlayer::changeSimpleBck(const char* pName) {
    J3DAnmTransform* resMotion = static_cast< J3DAnmTransform* >(mResourceTable->findResMotion(pName));
    if (resMotion == nullptr) {
        mCurrentBckName = nullptr;
        return false;
    }

    changeAnimationSimple(resMotion);
    mCurrentBckName = mResourceTable->findStringMotion(pName);

    return true;
}

void XanimePlayer::changeAnimation(const char* pName) {
    const XanimeGroupInfo* info = mResourceTable->getGroupInfo(pName);
    if (info == nullptr) {
        changeSimpleBck(pName);
        return;
    }

    changeAnimation(info);
}

void XanimePlayer::changeAnimationByHash(u32 hash) {
    const XanimeGroupInfo* info = mResourceTable->getGroupInfoFromHash(hash);
    if (info != nullptr) {
        changeAnimation(info);
    }
}

void XanimePlayer::swapFrameCtrl(const XanimeGroupInfo* pInfo) {
    if (pInfo == nullptr) {
        return;
    }

    if (pInfo->mAnimations[0] == nullptr) {
        return;
    }

    mNextFrameCtrlIndex = 1 - mCurrentFrameCtrlIndex;

    mFrameCtrl = &mFrameCtrls[mNextFrameCtrlIndex];
    mFrameCtrl->init(reinterpret_cast< J3DAnmTransform* >(pInfo->mAnimations[0])->getFrameMax());
    mFrameCtrl->setRate(pInfo->mRate);
    mFrameCtrl->setStart(pInfo->mStart);
    mFrameCtrl->setFrame(pInfo->mStart);
    mFrameCtrl->setLoop(pInfo->mLoop);
    mFrameCtrl->setEnd(pInfo->mEnd);
    mFrameCtrl->setAttribute(pInfo->mAttribute);
    mFrameCtrl->mInterpoleFrames = pInfo->mInterpoleFrame;

    mCachedFrame = pInfo->mStart;
    mHasUpdatedFrame = false;
}

void XanimePlayer::changeAnimation(const XanimeGroupInfo* pInfo) {
    if (pInfo == nullptr) {
        return;
    }

    if (mCurrentAnimation == pInfo) {
        return;
    }

    swapFrameCtrl(pInfo);
    mAnimationChanged = true;
    mNextAnimation = pInfo;
    changeCurrentAnimation(pInfo);
    mCurrentBckName = pInfo->mBckName;
}

void XanimePlayer::prepareAnimation(const XanimeGroupInfo* pInfo) {
    if (pInfo == nullptr) {
        return;
    }

    mCore->doFreeze();
    changeCurrentAnimation(pInfo);
    mNextAnimationReady = true;
    runNextAnimation();
}

void XanimePlayer::setModel(J3DModel* pModel) {
    mModel = pModel;
    mModelData = pModel->mModelData;
    mCore->reconfigJointTransform(mModelData);
}

void XanimePlayer::runNextAnimation() {
    if (!mNextAnimationReady) {
        return;
    }

    mNextAnimationReady = false;

    const XanimeGroupInfo* curAnim = mCurrentAnimation;
    u32 i;
    for (i = 0; i < curAnim->mBckTableVariant; i++) {
        mCore->setBck(i & 0xFF, static_cast< J3DAnmTransform* >(curAnim->mAnimations[i]));
        mWeights[i] = curAnim->mWeights[i];
    }

    for (u32 j = i; j < mResourceTable->mMaxGroupInfoTableSize; j++) {
        mCore->setBck(j & 0xFF, nullptr);
        mWeights[j] = 0.0f;
    }

    const bool cond = mHasAnimation;

    mCurrentFrameCtrlIndex = mNextFrameCtrlIndex;

    mInterpoleRatio = 0.0f;
    mFrameBlendRatio = 1.0f;

    mDefaultAnimationDelay = 0;
    mHasAnimation = true;

    if (!cond) {
        mFrameCtrls[mNextFrameCtrlIndex].mInterpoleFrames = 0;
        mInterpoleRatio = 1.0f;
        updateBeforeMovement();
        calcAnm(0);
    }
}

void XanimePlayer::changeAnimationSimple(J3DAnmTransform* pAnm) {
    mNextAnimation = nullptr;

    if (mCurrentFrameCtrlIndex != mNextFrameCtrlIndex) {
        mCurrentFrameCtrlIndex = mNextFrameCtrlIndex;
    }

    mCore->doFreeze();
    changeCurrentAnimation(getSimpleGroup());
    mCore->setBck(0, pAnm);

    mWeights[0] = 1.0f;
    for (u32 i = 1; i < mResourceTable->mMaxGroupInfoTableSize; i++) {
        mCore->setBck(i & 0xFF, nullptr);
        mWeights[i] = 0.0f;
    }

    XanimeGroupInfo* info = getSimpleGroup();
    info->mAnimations[0] = pAnm;
    J3DAnmTransform* transform = static_cast< J3DAnmTransform* >(info->mAnimations[0]);
    mFrameCtrl->init(transform->getFrameMax());
    mFrameCtrl->setAttribute(transform->getAttribute());
    mFrameCtrl->setFrame(0.0f);

    mInterpoleRatio = 0.0f;
    mFrameBlendRatio = 1.0f;
    mDefaultAnimationDelay = 0;
    mFrameCtrl->mInterpoleFrames = 1;
    mHasAnimation = true;
    mNextAnimationReady = false;
    mNextAnimation = nullptr;
    mCachedFrame = 0.0f;
    mHasUpdatedFrame = false;
    mAnimationChanged = true;
}

void XanimePlayer::changeSpeed(f32 speed) {
    mFrameCtrl->mRate = speed;
}

void XanimePlayer::changeInterpoleFrame(s32 frame) {
    mFrameCtrl->mInterpoleFrames = frame;
    if (frame == 0) {
        mInterpoleRatio = 1.0f;
        mCore->mInterpoleRatio = 1.0f;
        return;
    }

    mInterpoleRatio = 0.0f;
    mCore->mInterpoleRatio = 0.0f;
    updateInterpoleRatio();
}

bool XanimePlayer::changeTrackWeight(u32 track, f32 weight) {
    if (mCurrentAnimation == nullptr) {
        return false;
    }

    if (track >= mCurrentAnimation->mBckTableVariant) {
        return false;
    }

    mWeights[track] = weight;
    return true;
}

void XanimePlayer::calcAnm(u16 arg) {
    if (!mKeepFrameRatio) updateFrameRatio();

    mCore->updateFrame();
    mModelData->mJointTree.mJointNodePointer[arg]->mMtxCalc = mCore;
    mHasUpdatedFrame = false;
}

void XanimePlayer::overWriteMtxCalc(u16 arg) {
    mModelData->mJointTree.mJointNodePointer[arg]->mMtxCalc = mCore;
}

void XanimePlayer::clearMtxCalc(u16 arg) {
    mModelData->mJointTree.mJointNodePointer[arg]->mMtxCalc = nullptr;
}

void XanimePlayer::clearAnm(u16 arg) {
    if (!mHasAnimation) {
        return;
    }

    mModelData->mJointTree.mJointNodePointer[arg]->mMtxCalc = nullptr;
}

void XanimePlayer::updateBeforeMovement() {
    if (!mHasAnimation) {
        return;
    }

    if ((mFrameCtrl->getState() & 1) == 1 && mAutoRunDefaultAnimation && (mFrameCtrl->getAttribute() == 0 || mFrameCtrl->getAttribute() == 3)) {
        runDefaultAnimation();
    }

    if (mDefaultAnimationDelay != 0 && (mDefaultAnimationDelay -= 0x1) == 0) {
        runDefaultAnimation();
    }
}

void XanimePlayer::updateAfterMovement() {
    prepareAnimation(mNextAnimation);
    mNextAnimation = nullptr;

    if (!mHasAnimation) {
        return;
    }

    if (mFrameCtrl->checkState(1) != 0) {
        f32 unusedCast = mFrameCtrl->getEnd();
    } else if (!mHasUpdatedFrame && !mAnimationChanged) {
        f32 rate = mFrameCtrl->getRate();

        mFrameCtrl->update();
        mCachedFrame = mFrameCtrl->getFrame();
        mHasUpdatedFrame = true;

        if (mFrameCtrl->checkState(1) != 0) {
            mFrameCtrl->setRate(rate);
        }
    }

    updateInterpoleRatio();

    for (u32 i = 0; i < mCurrentAnimation->mBckTableVariant; i++) {
        mCore->setWeight(i, mWeights[i]);
    }

    mCore->mInterpoleRatio = mInterpoleRatio;
    mCore->mFrameBlendRatio = mFrameBlendRatio;
}

void XanimePlayer::updateInterpoleRatio() {
    if (mFrameCtrl->getRate() != 0.0f || mFrameCtrl->getAttribute() == 1) {
        if (mFrameCtrl->mInterpoleFrames != 0) {
            mInterpoleRatio += (1.0f - mInterpoleRatio) / mFrameCtrl->mInterpoleFrames;
            mFrameCtrl->mInterpoleFrames -= 1;
        } else {
            mInterpoleRatio = 1.0f;
        }

        return;
    }

    mInterpoleRatio = 1.0f;
    mFrameCtrl->mInterpoleFrames = 0;
}

void XanimePlayer::getMainAnimationTrans(u32 arg1, TVec3f* pOut) const {
    XanimeTrack* pTracks = mCore->mTrackList;
    const u8 trackCount = mCore->mTrackCount;
    u8 index = 0;
    f32 f2 = pTracks[0].getWeight() + 0.01f;
    for (u8 i = 1; i < trackCount; i++) {
        const XanimeTrack& rTrack = mCore->mTrackList[i];
        if (rTrack.mAnimation != nullptr && rTrack.getWeight() > f2) {
            f2 = rTrack.getWeight() + 0.01f;
            index = i;
        }
    }

    if (pTracks[index].mAnimation == nullptr) {
        pOut->zero();
        return;
    }

    J3DTransformInfo info;
    pTracks[index].mAnimation->getTransform(arg1, &info);
    pOut->set(info.mTranslate);
}

bool XanimePlayer::isExistAnimation(const char* name) const {
    if (mResourceTable->getGroupInfo(name)) return true;
    return mResourceTable->findResMotion(name) != nullptr;
}

bool XanimePlayer::isRun(const char* pName) const {
    if (mCurrentAnimation == nullptr) {
        return false;
    }

    XanimeGroupInfo* group = getSimpleGroup();
    if (mCurrentAnimation == group) {
        return mResourceTable->findResMotion(pName) == getSimpleGroup()->mAnimations[0];
    }

    return mCurrentAnimation == mResourceTable->getGroupInfo(pName);
}

bool XanimePlayer::isTerminate(const char* pName) const {
    if (mCurrentAnimation != mResourceTable->getGroupInfo(pName)) {
        return true;
    }

    if ((mFrameCtrls[mCurrentFrameCtrlIndex].checkState(0x1) != 0)) {
        return true;
    }

    return false;
}

bool XanimePlayer::isTerminate() const {
    if (mFrameCtrls[mCurrentFrameCtrlIndex].checkState(0x1) != 0) {
        return true;
    }

    return false;
}

void XanimePlayer::runDefaultAnimation() {
    if (mNextAnimation != nullptr) {
        mNextAnimation = nullptr;
    }

    if (!mHasDefaultAnimation) {
        return;
    }

    if (mResourceTable->mAmountOfGroupInfos == 0) {
        return;
    }

    swapFrameCtrl(mDefaultAnimation);
    mNextAnimation = mDefaultAnimation;
    changeCurrentAnimation(mDefaultAnimation);
    mCurrentBckName = mDefaultAnimation->mBckName;
}

bool XanimePlayer::isAnimationRunSimple() const {
    if (mCurrentAnimation == nullptr) {
        return false;
    }

    return mCurrentAnimation == getSimpleGroup();
}

const char* XanimePlayer::getCurrentAnimationName() const {
    if (mCurrentAnimation == nullptr) {
        return ::dummy_name;
    }

    return mCurrentAnimation->mParent.mAnimationName;
}

const char* XanimePlayer::getDefaultAnimationName() const {
    if (mDefaultAnimation == nullptr) {
        return ::dummy_name;
    }

    return mDefaultAnimation->mParent.mAnimationName;
}

const char* XanimePlayer::getCurrentBckName() const {
    return mCurrentBckName;
}

const char* XanimePlayer::getNameStringPointer(const char* pName) const {
    const XanimeGroupInfo* info = mResourceTable->getGroupInfo(pName);
    if (info != nullptr) {
        return info->mParent.mAnimationName;
    }

    return nullptr;
}

f32 XanimePlayer::getEffectiveAnimationFrame() const {
    if (mFrameCtrl->checkState(1)) return mFrameCtrl->getEnd();
    if (mHasUpdatedFrame) return mCachedFrame;
    return mFrameCtrl->getFrame();
}

bool XanimePlayer::isFrameInRange(f32 begin, f32 end) const {
    f32 frame = mFrameCtrl->getFrame();
    if (mAnimationChanged) frame = mCachedFrame;
    return begin <= frame && frame < end;
}

bool XanimePlayer::checkPass(f32 arg) const {
    if (!mAnimationChanged) {
        return mFrameCtrl->checkPass(arg);
    }

    f32 oldFrame = mFrameCtrl->getFrame();
    mFrameCtrl->setFrame(mCachedFrame);
    bool res = mFrameCtrl->checkPass(arg);
    mFrameCtrl->setFrame(oldFrame);

    return res;
}

XanimeGroupInfo* XanimePlayer::getSimpleGroup() const {
    if (mSimpleGroup != nullptr) {
        return mSimpleGroup;
    }

    return &mResourceTable->mDefaultGroup;
}

void XanimePlayer::duplicateSimpleGroup() {
    mSimpleGroup = new XanimeGroupInfo();
    mSimpleGroup->init();
    mSimpleGroup->mParent.mAnimationName = "dup-non-group";
    mSimpleGroup->mRate = 1.0f;
    mSimpleGroup->mInterpoleFrame = 1;
    mSimpleGroup->mBckTableVariant = 1;
}

XanimeFrameCtrl::XanimeFrameCtrl() : mInterpoleFrames(0) {
}

void XanimePlayer::updateFrameRatio() {
    f32 currentFrame = getEffectiveAnimationFrame();
    if (mCurrentAnimation && mCurrentAnimation->mAnimations[0]) {
        s16 maxFrame = static_cast<J3DAnmTransform*>(mCurrentAnimation->mAnimations[0])->getFrameMax();
        if (!maxFrame) maxFrame = 1;
        mCore->mFrameRatio = currentFrame / maxFrame;
    }
}
