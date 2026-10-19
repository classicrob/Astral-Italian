#include "Game/Screen/GalaxyStatusIcon.hpp"
#include "Game/Util.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

GalaxyStatusIcon::GalaxyStatusIcon() : LayoutActor("ギャラクシー状態アイコン", true), mState(0), mIsGreen(false), mIsAppearing(false), mIsEnabled(false), mIsPointable(false), mShowTimer(0) {
    MR::connectToSceneLayout(this);
    initLayoutManager("MapGalaxyState", 1);
    initEffectKeeper(0, nullptr, nullptr);
    MR::createAndAddPaneCtrl(this, "Unknown", 1);
    MR::createAndAddPaneCtrl(this, "New", 1);
    MR::createAndAddPaneCtrl(this, "Medal", 1);
    MR::createAndAddPaneCtrl(this, "Comet", 1);
    MR::createAndAddPaneCtrl(this, "Crown1", 1);
    MR::createAndAddPaneCtrl(this, "Crown2", 1);
    MR::createAndAddPaneCtrl(this, "EventRace", 1);
    MR::createAndAddPaneCtrl(this, "EventHunter", 1);
    MR::createAndAddPaneCtrl(this, "EventScore", 1);
    initActionSound(4, nullptr, nullptr);
}

void GalaxyStatusIcon::init(const JMapInfoIter&) {
    kill();
}

void GalaxyStatusIcon::control() {
    updateCometAppear();
    s32 timer = mShowTimer - 1;
    mShowTimer = timer & ((static_cast<u32>(timer) >> 31) - 1);
    if (mShowTimer == 0) {
        MR::hideLayout(this);
    }
}

// fn_80472AF0
void GalaxyStatusIcon::hideAllPanes() {
    mIsEnabled = false;
    mIsPointable = false;
    MR::startPaneAnim(this, "Medal", "Hide", 0);
    MR::startPaneAnim(this, "Comet", "Hide", 0);
    MR::startPaneAnim(this, "Crown1", "Hide", 0);
    MR::startPaneAnim(this, "Crown2", "Hide", 0);
    MR::startPaneAnim(this, "Unknown", "Hide", 0);
    MR::startPaneAnim(this, "New", "Hide", 0);
    setEventPane(-1);
}

// fn_80472BC0
void GalaxyStatusIcon::setDefaultPanes() {
    mIsEnabled = false;
    mIsPointable = false;
    MR::startPaneAnim(this, "Medal", "Hide", 0);
    MR::startPaneAnim(this, "Comet", "Hide", 0);
    MR::startPaneAnim(this, "Crown1", "Hide", 0);
    MR::startPaneAnim(this, "Crown2", "Hide", 0);
    MR::startPaneAnim(this, "Unknown", "Unknown", 0);
    MR::startPaneAnim(this, "New", "Hide", 0);
    setEventPane(-1);
}

// fn_80472C90
void GalaxyStatusIcon::setNewPanes() {
    mIsEnabled = false;
    mIsPointable = true;
    MR::startPaneAnim(this, "Medal", "Hide", 0);
    MR::startPaneAnim(this, "Comet", "Hide", 0);
    MR::startPaneAnim(this, "Crown1", "Hide", 0);
    MR::startPaneAnim(this, "Crown2", "Hide", 0);
    MR::startPaneAnim(this, "Unknown", "Hide", 0);
    MR::startPaneAnim(this, "New", "New", 0);
    setEventPane(-1);
}

// fn_80472D60: (isMedal, starCount, isGreen, isCrown1, isCrown2) select which pane anims play
void GalaxyStatusIcon::setCometPanes(bool isMedal, s32 starCount, bool isGreen, bool isCrown1, bool isCrown2) {
    mIsEnabled = true;
    mIsPointable = true;
    mIsGreen = isGreen;
    MR::startPaneAnim(this, "Unknown", "Hide", 0);
    MR::startPaneAnim(this, "New", "Hide", 0);
    if (starCount) {
        MR::startPaneAnim(this, "Comet", "Comet", 0);
    } else if (isGreen) {
        MR::startPaneAnim(this, "Comet", "GreenComet", 0);
    } else {
        MR::startPaneAnim(this, "Comet", "Hide", 0);
    }
    if (isCrown2) {
        MR::startPaneAnim(this, "Medal", "Hide", 0);
        MR::startPaneAnim(this, "Crown1", "Hide", 0);
        MR::startPaneAnim(this, "Crown2", "Crown2", 0);
        return;
    }
    if (isCrown1) {
        MR::startPaneAnim(this, "Medal", "Hide", 0);
        MR::startPaneAnim(this, "Crown1", "Crown1", 0);
        MR::startPaneAnim(this, "Crown2", "Hide", 0);
        return;
    }
    if (isMedal) {
        MR::startPaneAnim(this, "Medal", "Medal", 0);
        MR::startPaneAnim(this, "Crown1", "Hide", 0);
        MR::startPaneAnim(this, "Crown2", "Hide", 0);
        return;
    }
    MR::startPaneAnim(this, "Medal", "Hide", 0);
    MR::startPaneAnim(this, "Crown1", "Hide", 0);
    MR::startPaneAnim(this, "Crown2", "Hide", 0);
}

// fn_80472F50
void GalaxyStatusIcon::setEventPane(s32 which) {
    MR::hidePane(this, "EventRace");
    MR::hidePane(this, "EventHunter");
    MR::hidePane(this, "EventScore");
    switch (which) {
    case 0:
        MR::showPane(this, "EventRace");
        return;
    case 1:
        MR::showPane(this, "EventHunter");
        return;
    case 2:
        MR::showPane(this, "EventScore");
        return;
    }
}

// fn_80473000
void GalaxyStatusIcon::startCometHideAnim() {
    MR::startPaneAnim(this, "Comet", "Hide", 0);
}

// fn_80473020
void GalaxyStatusIcon::startCometAppear() {
    mIsAppearing = true;
    MR::startPaneAnim(this, "Comet", mIsGreen ? "GreenCometAppear" : "CometAppear", 0);
}

// fn_80473050
bool GalaxyStatusIcon::updateCometAppear() {
    if (!mIsAppearing) {
        return false;
    }
    if (MR::isPaneAnimStopped(this, "Comet", 0)) {
        mIsAppearing = false;
        MR::startPaneAnim(this, "Comet", mIsGreen ? "GreenComet" : "Comet", 0);
        return true;
    }
    return false;
}

// fn_804730F0
bool GalaxyStatusIcon::isPointingBox() {
    bool pointing = false;
    if (mIsPointable) {
        pointing = MR::isStarPointerPointingPane(this, "BoxIcon", 0, true, "");
    }
    return mIsEnabled && pointing;
}

// fn_80473160
void GalaxyStatusIcon::showForTime() {
    mShowTimer = 4;
    if (MR::isHiddenLayout(this)) {
        MR::showLayout(this);
    }
}

GalaxyStatusIcon::~GalaxyStatusIcon() {
}
