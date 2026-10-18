#pragma auto_inline off
#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace {
    const char* cPointingBounds = "BoxOut";
    const char* cButtonPane = "ZoomOutButton";
}
namespace NrvMapGalaxyZoomButton {
    class MapGalaxyZoomButtonNrvHidden : public Nerve { public: virtual void execute(Spine*) const {} static MapGalaxyZoomButtonNrvHidden sInstance; };
    class MapGalaxyZoomButtonNrvAppear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvAppear sInstance; };
    class MapGalaxyZoomButtonNrvWait : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvWait sInstance; };
    class MapGalaxyZoomButtonNrvDisappear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvDisappear sInstance; };
    class MapGalaxyZoomButtonNrvSelectIn : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvSelectIn sInstance; };
    class MapGalaxyZoomButtonNrvSelected : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvSelected sInstance; };
    class MapGalaxyZoomButtonNrvDecided : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyZoomButtonNrvDecided sInstance; };
    MapGalaxyZoomButtonNrvHidden MapGalaxyZoomButtonNrvHidden::sInstance;
    MapGalaxyZoomButtonNrvAppear MapGalaxyZoomButtonNrvAppear::sInstance;
    MapGalaxyZoomButtonNrvWait MapGalaxyZoomButtonNrvWait::sInstance;
    MapGalaxyZoomButtonNrvDisappear MapGalaxyZoomButtonNrvDisappear::sInstance;
    MapGalaxyZoomButtonNrvSelectIn MapGalaxyZoomButtonNrvSelectIn::sInstance;
    MapGalaxyZoomButtonNrvSelected MapGalaxyZoomButtonNrvSelected::sInstance;
    MapGalaxyZoomButtonNrvDecided MapGalaxyZoomButtonNrvDecided::sInstance;
}
using namespace NrvMapGalaxyZoomButton;
MapGalaxyZoomButton::MapGalaxyZoomButton() : SimpleLayout("ワールドマップズームボタン", "MapGalaxyZoomButton", 1, 78), mAppearKeepFrames(0), mPointingBounds(0), mPointingPane(0) { MR::registerDemoSimpleCastAll(this); }
void MapGalaxyZoomButton::requestAppear() {
    if (MR::isDead(this)) appear();
    mAppearKeepFrames = 4;
    if (isNerve(&MapGalaxyZoomButtonNrvHidden::sInstance) && mAppearKeepFrames > 0) {
        MR::startAnim(this, "Appear", 0);
        setNerve(&MapGalaxyZoomButtonNrvAppear::sInstance);
        appear();
    }
}
bool MapGalaxyZoomButton::isAppeared() const { return !isNerve(&MapGalaxyZoomButtonNrvHidden::sInstance) && !isNerve(&MapGalaxyZoomButtonNrvDisappear::sInstance); }
void MapGalaxyZoomButton::init(const JMapInfoIter&) {
    MR::createAndAddPaneCtrl(this, cButtonPane, 2);
    MR::startPaneAnim(this, cButtonPane, "ButtonShow", 1);
    initNerve(&MapGalaxyZoomButtonNrvHidden::sInstance);
    LayoutActor::kill();
}
void MapGalaxyZoomButton::appear() {
    LayoutActor::appear();
    MR::startPaneAnim(this, cButtonPane, "ButtonShow", 1);
}
void MapGalaxyZoomButton::kill() {
    mAppearKeepFrames = 0;
    LayoutActor::kill();
    setNerve(&MapGalaxyZoomButtonNrvHidden::sInstance);
}
void MapGalaxyZoomButton::control() {
    mAppearKeepFrames = MR::max(mAppearKeepFrames - 1, 0);
    if (mAppearKeepFrames == 0 && (isNerve(&MapGalaxyZoomButtonNrvWait::sInstance) || isNerve(&MapGalaxyZoomButtonNrvSelectIn::sInstance) || isNerve(&MapGalaxyZoomButtonNrvSelected::sInstance))) {
        if (mPointingPane) endPointing();
        MR::startAnim(this, "End", 0);
        setNerve(&MapGalaxyZoomButtonNrvDisappear::sInstance);
    }
}
#pragma dont_inline on
bool MapGalaxyZoomButton::tryDecide() {
    if (mPointingPane && MR::isEqualString(mPointingPane, cButtonPane) && isNerve(&MapGalaxyZoomButtonNrvSelected::sInstance)) { decide(); return true; }
    return false;
}
#pragma dont_inline reset
bool MapGalaxyZoomButton::isPointing() const { return isNerve(&MapGalaxyZoomButtonNrvSelected::sInstance) || isNerve(&MapGalaxyZoomButtonNrvSelectIn::sInstance) || isNerve(&MapGalaxyZoomButtonNrvDecided::sInstance); }
void MapGalaxyZoomButton::exeSelectIn() {
    if (tryNotPointing() && MR::isPaneAnimStopped(this, mPointingPane, 1)) setNerve(&MapGalaxyZoomButtonNrvSelected::sInstance);
}
void MapGalaxyZoomButton::exeDecided() {
    if (MR::isPaneAnimStopped(this, mPointingPane, 1)) {
        if (MR::isStarPointerPointingPane(this, mPointingBounds, 0, true, "弱")) { setNerve(&MapGalaxyZoomButtonNrvSelected::sInstance); return; }
        endPointing();
        setNerve(&MapGalaxyZoomButtonNrvWait::sInstance);
    }
}
bool MapGalaxyZoomButton::tryPointing(const char* bounds, const char* pane) {
    if (MR::isStarPointerPointingPane(this, bounds, 0, true, "弱")) { startPointing(bounds, pane); return true; }
    return false;
}
bool MapGalaxyZoomButton::tryNotPointing() {
    if (!MR::isStarPointerPointingPane(this, mPointingBounds, 0, true, "弱")) { endPointing(); return false; }
    return true;
}
void MapGalaxyZoomButton::startPointing(const char* bounds, const char* pane) {
    mPointingBounds = bounds;
    mPointingPane = pane;
    MR::startPaneAnim(this, pane, "ButtonSelectIn", 1);
    setNerve(&MapGalaxyZoomButtonNrvSelectIn::sInstance);
    MR::startActionSound(this, "PointOn");
}
void MapGalaxyZoomButton::endPointing() {
    MR::startPaneAnim(this, mPointingPane, "ButtonSelectOut", 1);
    mPointingPane = 0;
    setNerve(&MapGalaxyZoomButtonNrvWait::sInstance);
}
void MapGalaxyZoomButton::decide() {
    MR::startPaneAnim(this, mPointingPane, "ButtonDecide", 1);
    setNerve(&MapGalaxyZoomButtonNrvDecided::sInstance);
    MR::startActionSound(this, "Push");
}
MapGalaxyZoomButton::~MapGalaxyZoomButton() {}
namespace NrvMapGalaxyZoomButton {
    void MapGalaxyZoomButtonNrvDecided::execute(Spine* spine) const { static_cast<MapGalaxyZoomButton*>(spine->mExecutor)->exeDecided(); }
    void MapGalaxyZoomButtonNrvSelected::execute(Spine* spine) const { static_cast<MapGalaxyZoomButton*>(spine->mExecutor)->tryNotPointing(); }
    void MapGalaxyZoomButtonNrvSelectIn::execute(Spine* spine) const { static_cast<MapGalaxyZoomButton*>(spine->mExecutor)->exeSelectIn(); }
    void MapGalaxyZoomButtonNrvDisappear::execute(Spine* spine) const {
        MapGalaxyZoomButton* actor = static_cast<MapGalaxyZoomButton*>(spine->mExecutor);
        if (MR::isAnimStopped(actor, 0)) { actor->setNerve(&MapGalaxyZoomButtonNrvHidden::sInstance); actor->kill(); }
    }
    void MapGalaxyZoomButtonNrvWait::execute(Spine* spine) const { static_cast<MapGalaxyZoomButton*>(spine->mExecutor)->tryPointing(cPointingBounds, cButtonPane); }
    void MapGalaxyZoomButtonNrvAppear::execute(Spine* spine) const {
        MapGalaxyZoomButton* actor = static_cast<MapGalaxyZoomButton*>(spine->mExecutor);
        if (MR::isAnimStopped(actor, 0)) actor->setNerve(&MapGalaxyZoomButtonNrvWait::sInstance);
    }
}
