#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Screen/YoshiTargetMark.hpp"
#include "Game/Util.hpp"
#include <cstdio>
#ifdef GALAXY_NATIVE
using namespace YoshiLockOnParams;
#endif
class YoshiLockOnIterator {
public:
    YoshiLockOnIterator(MR::BothDirPtrLink* link) : mLink(link) {
    }
    MR::BothDirPtrLink* operator++(int) NO_INLINE;
    YoshiLockOnTargetControl* operator->() const {
        return static_cast<YoshiLockOnTargetControl*>(mLink->mValue);
    }
    MR::BothDirPtrLink* mLink;
};
MR::BothDirPtrLink* YoshiLockOnIterator::operator++(int) {
    MR::BothDirPtrLink* previous = mLink;
    mLink = previous->mNextLink;
    return previous;
}

void YoshiLockOnTargetControl::reset() {
    mTarget = nullptr;
    mFramesUntilExpiry = 0;
    mMark = nullptr;
}

YoshiLockOnManager::YoshiLockOnManager(Yoshi* yoshi) : NameObj("ターゲットロック管理"), mSelectionMode(0), mContextActor(nullptr), mContextActive(false), mHasUpdated(false), mFree(), mFading(), mActive(), mSoundLatch(0) {
    loadLockOnParams(yoshi);
}

YoshiLockOnManager::~YoshiLockOnManager() {
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::init(const JMapInfoIter&) {
    for (u32 index = 0; index<16; ++index) mFree.append(new YoshiLockOnTargetControl());
    MR::connectToScene(this, 0x27, -1, -1, 0x46);
    MR::registerDemoSimpleCastAll(this);
    s32 targetMax = sTargetMax;
    mMarkCount = targetMax*2;
    mMarks = new YoshiTargetMark*[targetMax*2];
    for (s32 index = 0; index<mMarkCount; ++index) {
        mMarks[index] = new YoshiTargetMark("ヨッシーターゲットマーク");
        mMarks[index]->initWithoutIter();
    }
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::movement() {
    if (mContextActive) {
        if (mSuspended) return;
        if (MR::isPlayerInRushOrDemoOrTalk()) clearActiveTargets();
        updateFadingTargets();
        updateActiveTargets();
        mHasUpdated = true;
    }
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::draw() const {
    if (mHasUpdated) {
        for (YoshiLockOnIterator iterator(mFading.mHead); iterator.mLink; iterator++) static_cast<YoshiLockOnTargetControl*>(iterator.mLink->mValue)->calcScreenPosition();
        for (YoshiLockOnIterator iterator(mActive.mHead); iterator.mLink; iterator++) static_cast<YoshiLockOnTargetControl*>(iterator.mLink->mValue)->calcScreenPosition();
    }
}

void YoshiLockOnManager::loadLockOnParams(Yoshi* yoshi) {
    char fileName[64];
    snprintf(fileName, sizeof(fileName), "%s.bcsv", sParamFile);
    JMapInfo* params = MR::tryCreateCsvParser(MR::getResourceHolder(yoshi), fileName);
    if (params) {
        MR::getCsvDataS32(&sTimer, params, "TimerCount", 0);
        MR::getCsvDataF32(&sDistance0, params, "EatTargetDistMax", 0);
        MR::getCsvDataF32(&sDistance2, params, "HookTargetDistMax", 0);
        MR::getCsvDataF32(&sDistance4, params, "PullTargetDistMax", 0);
        MR::getCsvDataS32(&sTargetMax, params, "TargetMax", 0);
    }
}

bool YoshiLockOnManager::tryLockOnEatTarget(YoshiLockOnTarget* target) {
    return tryLockOnMultipleTargets(target, YoshiLockOnTargetType::Eat, 1, 3, sDistance0);
}

bool YoshiLockOnManager::tryLockOnLickTarget(YoshiLockOnTarget* target) {
    return tryLockOnMultipleTargets(target, YoshiLockOnTargetType::Lick, 1, 3, sDistance0);
}

bool YoshiLockOnManager::tryLockOnCaptureTarget(YoshiLockOnTarget* target, bool alternate) {
    if (alternate) return tryLockOnSingleTarget(target, YoshiLockOnTargetType::CaptureAlternate, sDistance2);
    return tryLockOnSingleTarget(target, YoshiLockOnTargetType::Capture, sDistance2);
}

bool YoshiLockOnManager::tryLockOnPullTarget(YoshiLockOnTarget* target) {
    return tryLockOnSingleTarget(target, YoshiLockOnTargetType::Pull, sDistance4);
}

bool YoshiLockOnManager::tryLockOnProjectileTarget(YoshiLockOnTarget* target) {
    return tryLockOnSingleTarget(target, YoshiLockOnTargetType::Projectile, sDistance5);
}

u32 YoshiLockOnManager::getTargetCapacity() const {
    return 16;
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
u32 YoshiLockOnManager::takeActiveTargets(YoshiLockOnTargetDesc* out, u32) {
    u32 count = 0;
    for (YoshiLockOnIterator iterator(mActive.mHead); iterator.mLink; iterator++, count++) iterator->copyTargetInfo(out+count);
    clearActiveTargets();
    return count;
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::clearActiveTargets() {
    for (YoshiLockOnIterator iterator(mActive.mHead); iterator.mLink; ) {
        YoshiLockOnIterator current(iterator);
        iterator++;
        current->disappearMark();
        mActive.remove(static_cast<YoshiLockOnTargetControl*>(current->mValue));
        mFading.append(static_cast<YoshiLockOnTargetControl*>(current->mValue));
    }
}

bool YoshiLockOnManager::tryLockOnMultipleTargets(YoshiLockOnTarget* target, s32 targetType, s32 selectionMode, u32 maxActiveTargets, f32 maxDistance) {
    if (!mContextActive || mSuspended) return false;
    if (PSVECDistance(&mContextActor->mPosition, &target->mActor->mPosition) >= maxDistance) return false;
    if (static_cast<s32>(mSelectionMode) != selectionMode) clearActiveTargets();
    else if (tryRefreshActiveTarget(target)) return true;
    if (mActive.mCount >= maxActiveTargets) return false;
    if (tryReactivateFadingTarget(target)) return true;
    if (!tryActivateTarget(target, sTimer, targetType)) return false;
    mSelectionMode = selectionMode;
    if (!mSoundLatch && !MR::isDemoActive()) {
        MR::startActionSound(mContextActor, "CurMarking", -1, -1, -1);
        mSoundLatch = 1;
    }return true;
}

bool YoshiLockOnManager::tryLockOnSingleTarget(YoshiLockOnTarget* target, s32 targetType, f32 maxDistance) {
    if (!mContextActive || mSuspended) return false;
    if (PSVECDistance(&mContextActor->mPosition, &target->mActor->mPosition) >= maxDistance) return false;
    YoshiLockOnTargetControl* control = findActiveTarget(target);
    if (control) {
        control->mFramesUntilExpiry = sTimer;
        mSoundLatch = 0;
        return true;
    }
    clearActiveTargets();
    if (!mSoundLatch && !MR::isDemoActive()) {
        MR::startActionSound(mContextActor, "CurMarking", -1, -1, -1);
        mSoundLatch = 1;
    }
    if (tryReactivateFadingTarget(target)) return true;
    if (!tryActivateTarget(target, sTimer, targetType)) return false;
    mSelectionMode = 0;
    return true;
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiLockOnManager::tryActivateTarget(YoshiLockOnTarget* target, s32 framesUntilExpiry, s32 targetType) {
    if (!mFree.mCount) return false;
    YoshiLockOnIterator current(mFree.mHead);
    mFree.remove(static_cast<YoshiLockOnTargetControl*>(current->mValue));
    mActive.append(static_cast<YoshiLockOnTargetControl*>(current->mValue));
    YoshiTargetMark* mark = findUnusedMark();
    if (mark) mark->appear();
    YoshiLockOnTargetControl* control = current.operator->();
    control->mTarget = target;
    control->mFramesUntilExpiry = framesUntilExpiry;
    control->mMark = mark;
    control->mType = targetType;
    current->updateMarkPosition();
    return true;
}

YoshiLockOnTargetControl* YoshiLockOnManager::findActiveTarget(YoshiLockOnTarget* target) {
    for (YoshiLockOnIterator iterator(mActive.mHead); iterator.mLink; iterator++) {
        YoshiLockOnTargetControl* control = static_cast<YoshiLockOnTargetControl*>(iterator.mLink->mValue);
        if (control->mTarget == target) return static_cast<YoshiLockOnTargetControl*>(control->mValue);
    }return nullptr;
}

YoshiTargetMark* YoshiLockOnManager::findUnusedMark() {
    for (s32 index = 0; index<mMarkCount; ++index) if (MR::isDead(mMarks[index])) return mMarks[index];
    return nullptr;
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::updateFadingTargets() {
    for (YoshiLockOnIterator iterator(mFading.mHead); iterator.mLink; ) {
        YoshiLockOnIterator current(iterator);
        iterator++;
        YoshiLockOnTargetControl* control = current.operator->();
        const TVec3f& position = mContextActor->mPosition;
        control->update(position, getYoshiLockOnDistance(control->mType));
        if (current->isMarkDead()) {
            YoshiLockOnTargetControl* fading = current.operator->();
            if (fading->mMark) fading->mMark->disappear();
            fading->reset();
            mFading.remove(static_cast<YoshiLockOnTargetControl*>(current->mValue));
            mFree.append(static_cast<YoshiLockOnTargetControl*>(current->mValue));
        }
    }
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiLockOnManager::updateActiveTargets() {
    for (YoshiLockOnIterator iterator(mActive.mHead); iterator.mLink; ) {
        YoshiLockOnIterator current(iterator);
        iterator++;
        YoshiLockOnTargetControl* control = current.operator->();
        const TVec3f& position = mContextActor->mPosition;
        control->update(position, getYoshiLockOnDistance(control->mType));
        if (current->isInvalid()) {
            current->disappearMark();
            mActive.remove(static_cast<YoshiLockOnTargetControl*>(current->mValue));
            mFading.append(static_cast<YoshiLockOnTargetControl*>(current->mValue));
        }current->invalidate();
    }
}

bool YoshiLockOnManager::tryRefreshActiveTarget(YoshiLockOnTarget* target) {
    YoshiLockOnTargetControl* control = findActiveTarget(target);
    if (control) {
        control->mFramesUntilExpiry = sTimer;
        mSoundLatch = 0;
        return true;
    }return false;
}

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiLockOnManager::tryReactivateFadingTarget(YoshiLockOnTarget* target) {
    for (YoshiLockOnIterator iterator(mFading.mHead); iterator.mLink; ) {
        YoshiLockOnIterator current(iterator);
        iterator++;
        if (current->mTarget == target) {
            current->appearMark();
            current->mFramesUntilExpiry = sTimer;
            mFading.remove(static_cast<YoshiLockOnTargetControl*>(current->mValue));
            mActive.append(static_cast<YoshiLockOnTargetControl*>(current->mValue));
            return true;
        }
    }return false;
}
