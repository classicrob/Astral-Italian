#include "Game/Screen/StarPointerMovePositionHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/MathUtil.hpp"

namespace NrvStarPointerMovePositionHolder {
    NEW_NERVE(StarPointerMovePositionHolderNrvWait, StarPointerMovePositionHolder, Wait);
    NEW_NERVE(StarPointerMovePositionHolderNrvActive, StarPointerMovePositionHolder, Active);
    NEW_NERVE(StarPointerMovePositionHolderNrvRelease, StarPointerMovePositionHolder, Release);
}

namespace {
    TVec2f sZeroVelocity(0.0f, 0.0f);
}


StarPointerMovePosition::StarPointerMovePosition() {
    reset();
}

void StarPointerMovePosition::reset() {
    mName = nullptr;
    mLeft = nullptr;
    mRight = nullptr;
    mUp = nullptr;
    mDown = nullptr;
    mPosition.set(0.0f, 0.0f);
    mOffset.set(0.0f, 0.0f);
}

StarPointerMovePositionHolder::StarPointerMovePositionHolder(s32 channel)
    : NerveExecutor("スターポインタ移動位置保持"), mOwner(nullptr), mPadChannel(channel), mMovementRequested(false), mRegistered(false),
      mUseSubPad(true), mSelected(nullptr), mPositions(nullptr), mPositionCount(0), mSelectionPhysicalInScreen(false), mPhysicalInScreen(false),
      mStickUp(false), mStickDown(false), mStickLeft(false), mStickRight(false), mStickUpFrames(0), mStickDownFrames(0),
      mStickLeftFrames(0), mStickRightFrames(0) {
    mRoot.mName = "Root";
    mScreenPosition.set(0.0f, 0.0f);
    mSelectionPhysicalPosition.set(0.0f, 0.0f);
    mPhysicalPosition.set(0.0f, 0.0f);
    mPositions = new StarPointerMovePosition[32];
    initNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
}

void StarPointerMovePositionHolder::beginRegistration(const void* owner, bool useSubPad) {
    mUseSubPad = useSubPad;
    mOwner = owner;
    mPositionCount = 0;
    mRoot.reset();
    mRegistered = true;
    mSelected = nullptr;
    setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
}

void StarPointerMovePositionHolder::resetSelection() {
    mSelected = nullptr;
    setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
}

void StarPointerMovePositionHolder::update(const TVec2f& position, bool inScreen) {
    mPhysicalPosition = position;
    mPhysicalInScreen = inScreen;
    updateStickRepeat();
    updateNerve();
    mMovementRequested = false;
}

void StarPointerMovePositionHolder::requestMovement() {
    mMovementRequested = true;
}

bool StarPointerMovePositionHolder::isActive() const {
    return !isNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
}

bool StarPointerMovePositionHolder::selectPosition(StarPointerMovePosition* position) {
    if (!position) return false;
    mSelectionPhysicalPosition = mPhysicalPosition;
    mSelectionPhysicalInScreen = mPhysicalInScreen;
    mSelected = position;
    mScreenPosition = position->mPosition + position->mOffset;
    setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvActive::sInstance);
    return true;
}

// NONMATCHING: negative threshold constants fold instead of emitting fneg.
void StarPointerMovePositionHolder::updateStickRepeat() {
#define UPDATE_REPEAT(axis, test, threshold, frames, flag) \
    if (MR::axis(0) test threshold) { \
        if (frames == 0) flag = true; \
        else if (frames - 20 > 0 && (frames - 20) % 15 == 0) flag = true; \
        else flag = false; \
        frames++; \
    } else { frames = 0; flag = false; }
    UPDATE_REPEAT(getSubPadStickY, >, 0.72f, mStickUpFrames, mStickUp)
    UPDATE_REPEAT(getSubPadStickY, <, -0.72f, mStickDownFrames, mStickDown)
    UPDATE_REPEAT(getSubPadStickX, <, -0.85f, mStickLeftFrames, mStickLeft)
    UPDATE_REPEAT(getSubPadStickX, >, 0.85f, mStickRightFrames, mStickRight)
#undef UPDATE_REPEAT
}

bool StarPointerMovePositionHolder::tryMove(StarPointerMovePosition* position) {
    mScreenPosition = position->mPosition + position->mOffset;
    if (MR::testCorePadTriggerLeft(mPadChannel) && selectPosition(position->mLeft)) return true;
    if (MR::testCorePadTriggerRight(mPadChannel) && selectPosition(position->mRight)) return true;
    if (MR::testCorePadTriggerUp(mPadChannel) && selectPosition(position->mUp)) return true;
    if (MR::testCorePadTriggerDown(mPadChannel) && selectPosition(position->mDown)) return true;
    if (!mUseSubPad) return false;
    if (mStickLeft && selectPosition(position->mLeft)) return true;
    if (mStickRight && selectPosition(position->mRight)) return true;
    if (mStickUp && selectPosition(position->mUp)) return true;
    if (mStickDown && selectPosition(position->mDown)) return true;
    return false;
}

void StarPointerMovePositionHolder::exeWait() {
    if (!mMovementRequested) mOwner = nullptr;
    else if (mRegistered) tryMove(&mRoot);
}

void StarPointerMovePositionHolder::exeActive() {
    bool release = false;
    if (!mRegistered) release = true;
    else if (!mMovementRequested) {
        setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvRelease::sInstance);
        return;
    } else if (mSelectionPhysicalInScreen) {
        f32 x = mSelectionPhysicalPosition.x - mPhysicalPosition.x;
        f32 y = mSelectionPhysicalPosition.y - mPhysicalPosition.y;
        if (JGeometry::TUtil<f32>::sqrt(x * x + y * y) > 150.0f) release = true;
        else if (MR::isCorePadSwing(mPadChannel) || MR::isSubPadSwing(mPadChannel)) release = true;
        else if (MR::isGreaterStep(this, 120)) release = true;
    } else {
        if (MR::isCorePadSwing(mPadChannel) || MR::isSubPadSwing(mPadChannel)) release = true;
        else if (mPhysicalInScreen) release = true;
    }
    if (release) {
        mSelected = nullptr;
        setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
        return;
    }
    tryMove(mSelected);
}

void StarPointerMovePositionHolder::exeRelease() {
    if (MR::isGreaterEqualStep(this, 4)) {
        mSelected = nullptr;
        setNerve(&NrvStarPointerMovePositionHolder::StarPointerMovePositionHolderNrvWait::sInstance);
    }
}

StarPointerMovePosition* StarPointerMovePositionHolder::addPosition(const char* name, const TVec2f* position, const TVec2f& offset) {
    StarPointerMovePosition* entry = &mPositions[mPositionCount++];
    entry->reset();
    entry->mName = name;
    entry->mPosition.x = position->x;
    entry->mPosition.y = position->y;
    entry->mOffset = offset;
    return entry;
}

void StarPointerMovePositionHolder::addMovePosition(const char* name, const TVec2f* position, const TVec2f& offset) {
    addPosition(name, position, offset);
}

StarPointerMovePosition* StarPointerMovePositionHolder::getPositionByName(const char* name) {
    for (s32 i = 0; i < mPositionCount; i++) {
        if (MR::isEqualString(mPositions[i].mName, name)) return &mPositions[i];
    }
    return nullptr;
}

void StarPointerMovePositionHolder::setPosition(const char* name, const TVec2f* position) {
    StarPointerMovePosition* entry = getPositionByName(name);
    entry->mPosition.x = position->x;
    entry->mPosition.y = position->y;
}

void StarPointerMovePositionHolder::setDefaultLeft(const char* name) { mRoot.mLeft = getPositionByName(name); }
void StarPointerMovePositionHolder::setDefaultRight(const char* name) { mRoot.mRight = getPositionByName(name); }
void StarPointerMovePositionHolder::setDefaultUp(const char* name) { mRoot.mUp = getPositionByName(name); }
void StarPointerMovePositionHolder::setDefaultDown(const char* name) { mRoot.mDown = getPositionByName(name); }

void StarPointerMovePositionHolder::connectRight(const char* left, const char* right) {
    StarPointerMovePosition* a = getPositionByName(left);
    StarPointerMovePosition* b = getPositionByName(right);
    a->mRight = b;
    b->mLeft = a;
}

void StarPointerMovePositionHolder::connectDown(const char* up, const char* down) {
    StarPointerMovePosition* a = getPositionByName(up);
    StarPointerMovePosition* b = getPositionByName(down);
    a->mDown = b;
    b->mUp = a;
}

const char* StarPointerMovePositionHolder::getPositionName(s32 index) const { return mPositions[index].mName; }
const TVec2f& StarPointerMovePositionHolder::getScreenPosition() const { return mScreenPosition; }
const TVec2f& StarPointerMovePositionHolder::getScreenVelocity() const { return sZeroVelocity; }
const TVec3f& StarPointerMovePositionHolder::getWorldVelocity() const { return TVec3f(gZeroVec); }
f32 StarPointerMovePositionHolder::getScreenSpeed() const { return 0.0f; }
f32 StarPointerMovePositionHolder::getViewDistanceZ() const { return 0.0f; }
s32 StarPointerMovePositionHolder::getPastPointCount() const { return 1; }
s32 StarPointerMovePositionHolder::getNextPastPointCount() const { return 1; }
const TVec2f& StarPointerMovePositionHolder::getPastPosition(s32) const { return getScreenPosition(); }
const TVec2f& StarPointerMovePositionHolder::getNextPastPosition(s32) const { return getScreenPosition(); }
const TVec3f& StarPointerMovePositionHolder::getWorldPosition() const { return TVec3f(gZeroVec); }
bool StarPointerMovePositionHolder::isInScreen() const { return true; }
bool StarPointerMovePositionHolder::isOutScreenLong() const { return false; }
