#define JGEOMETRY_VEC2_SET2_NOINLINE
#define JGEOMETRY_VEC2_MI_NOINLINE
#include "Game/Screen/StaffRollPart.hpp"
#include "Game/Screen/StaffRollLine.hpp"
#include "Game/Screen/StaffRollUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <algorithm>
#ifndef GALAXY_NATIVE
#include <functional.hpp>
#endif

namespace NrvStaffRollPart {
    NEW_NERVE(StaffRollPartNrvWait, StaffRollPart, Wait);
    NEW_NERVE(StaffRollPartNrvPart, StaffRollPart, Part);
    NEW_NERVE(StaffRollPartNrvWaitBeforePart, StaffRollPart, WaitBeforePart);
    NEW_NERVE(StaffRollPartNrvEnd, StaffRollPart, End);
}

namespace {
    const TVec2f cDefaultPosition(406.0f, 64.0f);

    void getRectCenter(const JGeometry::TBox2<f32>& rect, TVec2f* center) {
        center->x = rect.f.x + 0.5f * (rect.i.x - rect.f.x);
        center->y = rect.f.y + 0.5f * (rect.i.y - rect.f.y);
    }

    TVec2f negatePosition(const TVec2f& position) NO_INLINE {
        TVec2f result;
        result.x = -position.x;
        result.y = -position.y;
        return result;
    }
}

StaffRollPart::StaffRollPart(const wchar_t* message, u32* line)
    : NerveExecutor("スタッフブロック"), mType(3), mPosition(TVec2f(0.0f, 0.0f)), mLineCount(0), mMessage(message),
      mScrollSpeed(30.0f), mWaitFrames(30), mHasEndMarker(false), mElapsedFrames(0), mDuration(0), mScrollFrames(0), mIsAlive(false) {
    u32 lineCount = MR::countMessageLine(message);
    u32 index = 0;
    StaffRollPartInfo info(0, cDefaultPosition);
    if (StaffRollUtil::readPartInfo(&info, mMessage, *line)) {
        index = 1;
        (*line)++;
    }
    mType = info.mType;
    mPosition.set(info.mPosition);
    while (*line < lineCount) {
        bool end = false;
        StaffRollLine* actor = StaffRollUtil::createLine(mMessage, *line, index, &end);
        if (!actor) {
            mHasEndMarker = end;
            (*line)++;
            break;
        }
        actor->mPosition.set(getPosition().x, index * mScrollSpeed + getPosition().y);
        s32 i = mLineCount++;
        mLines[i] = actor;
        index++;
        (*line)++;
    }
    initNerve(GET_NERVE(StaffRollPart, StaffRollPartNrvWait));
}

StaffRollPart::~StaffRollPart() {
}

// NONMATCHING: target keeps `this` in r3 across setNerve; ours emits an extra mr r3, r31.
void StaffRollPart::appear() {
    const Nerve* nerve = GET_NERVE(StaffRollPart, StaffRollPartNrvPart);
    if (mWaitFrames > 0) {
        nerve = GET_NERVE(StaffRollPart, StaffRollPartNrvWaitBeforePart);
    }
    setNerve(nerve);
    mScrollFrames = 0;
    mIsAlive = true;
}

void StaffRollPart::kill() {
#ifdef GALAXY_NATIVE
    std::for_each(mLines, mLines + mLineCount, [](StaffRollLine* item) { item->kill(); });
#else
    std::for_each(mLines, mLines + mLineCount, std::mem_func(&StaffRollLine::kill));
#endif
    mIsAlive = false;
}

void StaffRollPart::movement() {
    updateNerve();
    if (mIsAlive) {
#ifdef GALAXY_NATIVE
        std::for_each(mLines, mLines + mLineCount, [](StaffRollLine* item) { item->movement(); });
#else
        std::for_each(mLines, mLines + mLineCount, std::mem_func(&LayoutActor::movement));
#endif
    }
    mScrollFrames++;
}

void StaffRollPart::calcAnim() {
    if (mIsAlive) {
#ifdef GALAXY_NATIVE
        std::for_each(mLines, mLines + mLineCount, [](StaffRollLine* item) { item->calcAnim(); });
#else
        std::for_each(mLines, mLines + mLineCount, std::mem_func(&LayoutActor::calcAnim));
#endif
    }
}

void StaffRollPart::draw() const {
    if (mIsAlive) {
#ifdef GALAXY_NATIVE
        std::for_each(mLines, mLines + mLineCount, [](StaffRollLine* item) { item->draw(); });
#else
        std::for_each(mLines, mLines + mLineCount, std::mem_func(&LayoutActor::draw));
#endif
    }
}

void StaffRollPart::addLinePosition(const TVec2f& offset) {
    for (StaffRollLine** it = mLines; it != mLines + mLineCount; ++it) {
        StaffRollLine* line = static_cast<StaffRollLine*>(*it);
        line->mPosition.x += offset.x;
        line->mPosition.y += offset.y;
    }
}

void StaffRollPart::alignLines(s32 type) {
    if (type == 1) {
        JGeometry::TBox2<f32> rect;
        calcTextRect(&rect);
        TVec2f center;
        getRectCenter(rect, &center);
        TVec2f diff = center - getPosition();
        addLinePosition(negatePosition(diff));
    } else if (type == 0) {
        JGeometry::TBox2<f32> rect;
        calcTextRect(&rect);
        TVec2f center;
        getRectCenter(rect, &center);
        f32 x = center.x - getPosition().x;
        f32 y = rect.i.y - getPosition().y;
        TVec2f offset;
        offset.x = x;
        offset.y = y;
        addLinePosition(negatePosition(offset));
    } else if (type == 2) {
        JGeometry::TBox2<f32> rect;
        calcTextRect(&rect);
        TVec2f center;
        getRectCenter(rect, &center);
        f32 x = center.x - getPosition().x;
        f32 y = rect.f.y - getPosition().y;
        TVec2f offset;
        offset.x = x;
        offset.y = y;
        addLinePosition(negatePosition(offset));
    }
}

TVec2f StaffRollPart::getPosition() const {
    if (MR::isScreen16Per9()) {
        return mPosition;
    }
    TVec2f position;
    position.x = 0.7487684488296509f * mPosition.x;
    position.y = mPosition.y;
    return position;
}

void StaffRollPart::setDuration(s32 duration) {
    mDuration = duration;
    setLineDuration(duration - mWaitFrames - 3);
}

void StaffRollPart::setLineDuration(s32 duration) {
    mElapsedFrames = duration;
#ifdef GALAXY_NATIVE
    std::for_each(mLines, mLines + mLineCount, [duration](StaffRollLine* item) { item->setDuration(duration); });
#else
    std::for_each(mLines, mLines + mLineCount,
        std::binder2nd<std::mem_fun1_t<void, StaffRollLine, s32>, s32>(std::mem_func(&StaffRollLine::setDuration), duration));
#endif
}

bool StaffRollPart::isEnd() const {
    if (mIsAlive) {
        return false;
    }
    return isNerve(GET_NERVE(StaffRollPart, StaffRollPartNrvEnd));
}

bool StaffRollPart::isAllLinesDead() const {
    for (StaffRollLine* const* it = mLines; it != mLines + mLineCount; ++it) {
        if (!MR::isDead(*it)) {
            return false;
        }
    }
    return true;
}

// NONMATCHING: begin/it iterator registers are swapped (r30/r31).
void StaffRollPart::calcTextRect(JGeometry::TBox2<f32>* result) const {
    JGeometry::TBox2<f32> rect;
    StaffRollLine* const* begin;
    StaffRollLine* const* it = mLines;
    begin = it;
    for (; it != mLines + mLineCount; ++it) {
        if (it == begin) {
            MR::calcTextBoxRectRecursive(&rect, *it, (*it)->getPaneName());
        } else {
            JGeometry::TBox2<f32> part;
            MR::calcTextBoxRectRecursive(&part, *it, (*it)->getPaneName());
            if (rect.i.x >= part.i.x) rect.i.x = part.i.x;
            if (rect.i.y >= part.i.y) rect.i.y = part.i.y;
            if (rect.f.x <= part.f.x) rect.f.x = part.f.x;
            if (rect.f.y <= part.f.y) rect.f.y = part.f.y;
        }
    }
    result->i.x = rect.i.x;
    result->i.y = rect.i.y;
    result->f.set(rect.f);
}

void StaffRollPart::exeWait() {
}

void StaffRollPart::exeWaitBeforePart() {
    if (MR::isGreaterEqualStep(this, mWaitFrames)) {
        setNerve(GET_NERVE(StaffRollPart, StaffRollPartNrvPart));
    }
}

void StaffRollPart::exePart() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        std::for_each(mLines, mLines + mLineCount, [](StaffRollLine* item) { item->appear(); });
#else
        std::for_each(mLines, mLines + mLineCount, std::mem_func(&StaffRollLine::appear));
#endif
        alignLines(mType);
    } else if (isAllLinesDead()) {
        kill();
        setNerve(GET_NERVE(StaffRollPart, StaffRollPartNrvEnd));
    }
}

void StaffRollPart::exeEnd() {
}
