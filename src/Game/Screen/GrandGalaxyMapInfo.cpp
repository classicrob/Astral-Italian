#include "Game/Screen/GrandGalaxyMapInfo.hpp"
#include "Game/Screen/GalaxyStatusIcon.hpp"
#include "Game/System/GrandGalaxyMapGalaxy.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include <algorithm>

namespace {
    const char* cIconPaneNames[] = {"Position1", "Position2", "Position3", "Position4", "Position5", "Position6", "Position7"};
    bool compareEntries(const GrandGalaxyMapInfo::Entry* first, const GrandGalaxyMapInfo::Entry* second) {
        return first->index < second->index;
    }
}

namespace NrvGrandGalaxyMapInfo {
    NEW_NERVE(Wait, GrandGalaxyMapInfo, Wait);
    class Hide : public Nerve { public: virtual void execute(Spine*) const; static Hide sInstance; };
    Hide Hide::sInstance;
    class Appear : public Nerve { public: virtual void execute(Spine*) const; static Appear sInstance; };
    Appear Appear::sInstance;
    class NextAppear : public Nerve { public: virtual void execute(Spine*) const; static NextAppear sInstance; };
    NextAppear NextAppear::sInstance;
    class PreviousAppear : public Nerve { public: virtual void execute(Spine*) const; static PreviousAppear sInstance; };
    PreviousAppear PreviousAppear::sInstance;
    class End : public Nerve { public: virtual void execute(Spine*) const; static End sInstance; };
    End End::sInstance;
    NEW_NERVE(PreviousEnd, GrandGalaxyMapInfo, PreviousEnd);
    NEW_NERVE(NextEnd, GrandGalaxyMapInfo, NextEnd);
}

GrandGalaxyMapInfo::GrandGalaxyMapInfo() : LayoutActor("グランドギャラクシーマップワールド情報", true), mSelectedIcon(nullptr) {
    MR::connectToScene(this, 0x2E, 0xD, -1, 0x4F);
    initLayoutManager("MapGalaxyReport", 1);
    mEntries = new Entry*[7];
    for (s32 i = 0; i < 7; i++) {
        mEntries[i] = new Entry;
        mEntries[i]->icon = new GalaxyStatusIcon;
        mEntries[i]->actor = nullptr;
        mEntries[i]->_8 = 0;
        mEntries[i]->index = i;
    }
    initActionSound(4, nullptr, nullptr);
}

GrandGalaxyMapInfo::~GrandGalaxyMapInfo() {}

void GrandGalaxyMapInfo::init(const JMapInfoIter&) {
    initNerve(&NrvGrandGalaxyMapInfo::Wait::sInstance);
    kill();
}

void GrandGalaxyMapInfo::appear() {
    mSelectedIcon = nullptr;
    LayoutActor::appear();
    for (s32 i = 0; i < 7; i++) {
        mEntries[i]->icon->appear();
        if (mEntries[i]->actor) mEntries[i]->actor->appear();
    }
}

void GrandGalaxyMapInfo::kill() {
    mSelectedIcon = nullptr;
    LayoutActor::kill();
    for (s32 i = 0; i < 7; i++) {
        mEntries[i]->icon->kill();
        if (mEntries[i]->actor) mEntries[i]->actor->kill();
    }
}

void GrandGalaxyMapInfo::requestEnd() {
    if (!MR::isDead(this)) setNerve(&NrvGrandGalaxyMapInfo::End::sInstance);
}

void GrandGalaxyMapInfo::requestPreviousEnd() {
    if (!MR::isDead(this)) setNerve(&NrvGrandGalaxyMapInfo::PreviousEnd::sInstance);
}

void GrandGalaxyMapInfo::requestNextEnd() {
    if (!MR::isDead(this)) setNerve(&NrvGrandGalaxyMapInfo::NextEnd::sInstance);
}

void GrandGalaxyMapInfo::setIconState(s32 slot, const char* state) { mEntries[slot]->icon->mState = state; }
void GrandGalaxyMapInfo::setIconDefaultPanes(s32 slot) { mEntries[slot]->icon->setDefaultPanes(); }
void GrandGalaxyMapInfo::setIconNewPanes(s32 slot) { mEntries[slot]->icon->setNewPanes(); }
void GrandGalaxyMapInfo::setIconCometPanes(s32 slot, bool medal, s32 starCount, bool green, bool crown1, bool crown2) {
    mEntries[slot]->icon->setCometPanes(medal, starCount, green, crown1, crown2);
}
void GrandGalaxyMapInfo::setIconActor(s32 slot, GrandGalaxyMapGalaxy* actor) { mEntries[slot]->actor = actor; }
void GrandGalaxyMapInfo::setIconIndex(s32 slot, s32 index) { mEntries[slot]->index = index; }
void GrandGalaxyMapInfo::sortEntries() { std::sort(mEntries, mEntries + 7, compareEntries); }

void GrandGalaxyMapInfo::exeWait() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Wait", 0);
    for (s32 i = 0; i < 7; i++) mEntries[i]->icon->showForTime();
    updatePointing();
    if (mSelectedIcon) {
        if (!mSelectedIcon->isPointingBox()) mSelectedIcon = nullptr;
    } else {
        mSelectedIcon = nullptr;
        GalaxyStatusIcon* icon;
        for (s32 i = 0; i < 7; i++) {
            icon = mEntries[i]->icon;
            if (icon->isPointingBox()) { mSelectedIcon = icon; break; }
        }
    }
}

void GrandGalaxyMapInfo::exePreviousEnd() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "PreviousEnd", 0);
    if (MR::isAnimStopped(this, 0)) {
        MR::setAnimFrameAndStopAtEnd(this, 0);
        setNerve(&NrvGrandGalaxyMapInfo::Hide::sInstance);
        kill();
    }
}

void GrandGalaxyMapInfo::exeNextEnd() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "NextEnd", 0);
    if (MR::isAnimStopped(this, 0)) {
        MR::setAnimFrameAndStopAtEnd(this, 0);
        setNerve(&NrvGrandGalaxyMapInfo::Hide::sInstance);
        kill();
    }
}

// NONMATCHING: the equivalent early-return blocks are emitted in reverse order.
const char* GrandGalaxyMapInfo::getSelectedIconState() {
    if (!mSelectedIcon) return 0;
    return mSelectedIcon->mState;
}

bool GrandGalaxyMapInfo::isPointingBoxBase() { return MR::isStarPointerPointingPane(this, "BoxBase", 0, false, "弱"); }

namespace NrvGrandGalaxyMapInfo {
    void End::execute(Spine* spine) const {
        GrandGalaxyMapInfo* actor = static_cast<GrandGalaxyMapInfo*>(spine->mExecutor);
        if (MR::isFirstStep(actor)) MR::startAnim(actor, "End", 0);
        MR::killAtAnimStopped(actor, 0);
    }
    void PreviousAppear::execute(Spine* spine) const {
        GrandGalaxyMapInfo* actor = static_cast<GrandGalaxyMapInfo*>(spine->mExecutor);
        if (MR::isFirstStep(actor)) MR::startAnim(actor, "PreviousAppear", 0);
        MR::setNerveAtAnimStopped(actor, &Wait::sInstance, 0);
    }
    void NextAppear::execute(Spine* spine) const {
        GrandGalaxyMapInfo* actor = static_cast<GrandGalaxyMapInfo*>(spine->mExecutor);
        if (MR::isFirstStep(actor)) MR::startAnim(actor, "NextAppear", 0);
        MR::setNerveAtAnimStopped(actor, &Wait::sInstance, 0);
    }
    void Appear::execute(Spine* spine) const {
        GrandGalaxyMapInfo* actor = static_cast<GrandGalaxyMapInfo*>(spine->mExecutor);
        if (MR::isFirstStep(actor)) MR::startAnim(actor, "Appear", 0);
        MR::setNerveAtAnimStopped(actor, &Wait::sInstance, 0);
    }
    void Hide::execute(Spine*) const {}
}

void GrandGalaxyMapInfo::startAppear() {
    appear();
    setNerve(&NrvGrandGalaxyMapInfo::Appear::sInstance);
    MR::startAnim(this, "Appear", 0);
    calcAnim();
    control();
    MR::startActionSound(this, "Open", -1, -1);
}

void GrandGalaxyMapInfo::startNextAppear() {
    appear();
    setNerve(&NrvGrandGalaxyMapInfo::NextAppear::sInstance);
    MR::startActionSound(this, "Open", -1, -1);
}

void GrandGalaxyMapInfo::startPreviousAppear() {
    appear();
    setNerve(&NrvGrandGalaxyMapInfo::PreviousAppear::sInstance);
    MR::startActionSound(this, "Open", -1, -1);
}

void GrandGalaxyMapInfo::updatePointing() {
    if (StarPointerUtil::beginMovePositionRegistration(this, false)) {
        const char* previous = nullptr;
        for (s32 i = 0; i < 7; i++) {
            if (mEntries[i]->icon->mIsPointable) {
                const char* pane = cIconPaneNames[i];
                TVec2f offset(0.0f, 10.0f);
                StarPointerUtil::addStarPointerMovePositionFromPane(this, pane, &offset);
                if (previous) StarPointerUtil::setConnectionMovePositionRight2Way(previous, pane);
                previous = pane;
            }
        }
        const char* first = nullptr;
        const char* last = nullptr;
        for (s32 i = 0; i < 7; i++) {
            if (mEntries[i]->icon->mIsPointable) {
                last = cIconPaneNames[i];
                if (!first) first = last;
            }
        }
        StarPointerUtil::setConnectionMovePositionRight2Way(last, first);
        StarPointerUtil::setLastMovePosition(last);
        StarPointerUtil::setFirstMovePosition(first);
    }
    StarPointerUtil::endMovePositionRegistration(this);
}

void GrandGalaxyMapInfo::setIconEventPane(s32 slot, const char* galaxyName) {
    WorldMapFunction::setStatusIconKind(mEntries[slot]->icon, galaxyName);
}

void GrandGalaxyMapInfo::control() {
    for (s32 i = 0; i < 7; i++) {
        MR::setLayoutPosAtPaneTrans(mEntries[i]->icon, this, cIconPaneNames[i]);
        if (mEntries[i]->actor) {
            MR::setActorPositionFromLayoutPane(mEntries[i]->actor, this, cIconPaneNames[i], mEntries[i]->actor->mLayoutOffset, 10000.0f);
        }
    }
}
