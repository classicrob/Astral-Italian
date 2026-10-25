#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace NrvMapGalaxyInfo {
    class MapGalaxyInfoNrvAppear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyInfoNrvAppear sInstance; };
    class MapGalaxyInfoNrvWait : public Nerve { public: virtual void execute(Spine*) const {} static MapGalaxyInfoNrvWait sInstance; };
    class MapGalaxyInfoNrvDisappear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyInfoNrvDisappear sInstance; };
    MapGalaxyInfoNrvAppear MapGalaxyInfoNrvAppear::sInstance;
    MapGalaxyInfoNrvWait MapGalaxyInfoNrvWait::sInstance;
    MapGalaxyInfoNrvDisappear MapGalaxyInfoNrvDisappear::sInstance;
}
using namespace NrvMapGalaxyInfo;
MapGalaxyInfo::MapGalaxyInfo(s32 drawType) : SimpleLayout("ワールドマップギャラクシー情報", "GalaxyInfo", 3, drawType < 0 ? 71 : drawType), mTextures(), mKeepFrames(0), mGalaxyName(0), mPlaySound(true) {
    ResTable* resources = MR::createAndAddResourceHolder("GalaxyInfoTexture.arc")->mFileInfoTable;
    mTextures.mArray.init(resources->mCount);
    for (u32 i = 0; i < resources->mCount; ++i) {
        TextureInfo info;
        info.mName = resources->getResName(i);
        info.mTexture = MR::createLytTexMap("GalaxyInfoTexture.arc", resources->getResName(i));
        mTextures.push_back(info);
    }
    MR::registerDemoSimpleCastAll(this);
}
void MapGalaxyInfo::showGalaxy(const char* name, s32 keepFrames) {
    if (mGalaxyName && MR::isEqualString(mGalaxyName, name)) { mKeepFrames = keepFrames; return; }
    if (MR::isDead(this)) appear();
    mGalaxyName = name;
    mKeepFrames = keepFrames;
    setGalaxyTexture(name);
    MR::setTextBoxMessageRecursive(this, "GalaxyName", MR::getGalaxyNameOnCurrentLanguage(mGalaxyName));
    const char* galaxyName = mGalaxyName;
    MR::makeGalaxyStatusAccessor(galaxyName);
    wchar_t stars[35];
    wchar_t* next = stars + MR::makeGalaxyCompletionMark(stars, 70, galaxyName, false);
    MR::makeGalaxyStarList(next, 70 - (next - stars), galaxyName, false, false);
    MR::setTextBoxMessageRecursive(this, "StarIcon", stars);
    if (GameDataFunction::isOnGalaxyFlagTicoCoin(name)) MR::showPaneRecursive(this, "TxtMedal");
    else MR::hidePaneRecursive(this, "TxtMedal");
    startAppear();
}
bool MapGalaxyInfo::isAppeared() const { return !MR::isDead(this) && !isNerve(&MapGalaxyInfoNrvDisappear::sInstance); }
void MapGalaxyInfo::setGalaxyTexture(const char* name) {
    for (u32 i = 0; i < mTextures.size(); ++i) {
        if (MR::isEqualSubString(mTextures[i].mName, name)) { MR::replacePaneTexture(this, "PicDummy", mTextures[i].mTexture, 0); return; }
    }
}
void MapGalaxyInfo::init(const JMapInfoIter&) {
    initNerve(&MapGalaxyInfoNrvWait::sInstance);
    LayoutActor::kill();
}
void MapGalaxyInfo::appear() { LayoutActor::appear(); }
void MapGalaxyInfo::kill() { mGalaxyName = 0; mKeepFrames = 0; LayoutActor::kill(); }
void MapGalaxyInfo::control() {
    mKeepFrames = MR::max(mKeepFrames - 1, 0);
    if (isNerve(&MapGalaxyInfoNrvWait::sInstance) && mKeepFrames == 0) {
        mGalaxyName = 0;
        MR::startAnim(this, "End", 0);
        setNerve(&MapGalaxyInfoNrvDisappear::sInstance);
    }
}
void MapGalaxyInfo::startAppear() {
    MR::startAnim(this, "Appear", 0);
    setNerve(&MapGalaxyInfoNrvAppear::sInstance);
    appear();
    if (mPlaySound) MR::startActionSound(this, "Open");
}
MapGalaxyInfo::~MapGalaxyInfo() {}
namespace NrvMapGalaxyInfo {
    inline void MapGalaxyInfoNrvDisappear::execute(Spine* spine) const {
        MapGalaxyInfo* actor = static_cast<MapGalaxyInfo*>(spine->mExecutor);
        if (MR::isAnimStopped(actor, 0)) actor->kill();
    }
    inline void MapGalaxyInfoNrvAppear::execute(Spine* spine) const {
        MapGalaxyInfo* actor = static_cast<MapGalaxyInfo*>(spine->mExecutor);
        if (MR::isAnimStopped(actor, 0)) actor->setNerve(&MapGalaxyInfoNrvWait::sInstance);
    }
}
