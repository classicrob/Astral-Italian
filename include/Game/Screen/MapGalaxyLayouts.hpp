#pragma once
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util/Array.hpp"
class MapGalaxyWorld : public SimpleLayout {
public:
    MapGalaxyWorld();
    virtual ~MapGalaxyWorld();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    void requestAppear();
    void setWorldName(const wchar_t*);

    /* 0x2C */ s32 mKeepFrames;
};


class MapGalaxyZoomButton : public SimpleLayout {
public:
    MapGalaxyZoomButton();
    virtual ~MapGalaxyZoomButton();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    void requestAppear();
    bool isAppeared() const;
    bool tryDecide();
    bool isPointing() const;
    void exeSelectIn();
    void exeDecided();
    bool tryPointing(const char*, const char*);
    bool tryNotPointing();
    void startPointing(const char*, const char*);
    void endPointing();
    void decide();
    s32 mAppearKeepFrames;
    const char* mPointingBounds;
    const char* mPointingPane;
};

class CoinCounter;
class StarPieceCounter;
class PlayerLeft;
class StarCounter;
class MapGalaxyCounters : public LayoutActor {
public:
    MapGalaxyCounters();
    virtual ~MapGalaxyCounters();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    void requestAppearCoinsAndLives();
    void requestAppearStars();
    void requestAppearStarPieces();
    CoinCounter* mCoinCounter;
    StarPieceCounter* mStarPieceCounter;
    PlayerLeft* mPlayerLeft;
    StarCounter* mStarCounter;
    s32 mCoinLifeKeepFrames;
    s32 mStarKeepFrames;
    s32 mStarPieceKeepFrames;
    s32 mLastStarPieceCount;
};

namespace nw4r { namespace lyt { class TexMap; } }
class MapGalaxyInfo : public SimpleLayout {
public:
    struct TextureInfo { const char* mName; nw4r::lyt::TexMap* mTexture; };
    MapGalaxyInfo(s32 drawType = -1);
    virtual ~MapGalaxyInfo();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    void showGalaxy(const char*, s32);
    bool isAppeared() const;
    void setGalaxyTexture(const char*);
    void startAppear();
    MR::Vector<MR::AssignableArray<TextureInfo> > mTextures;
    s32 mKeepFrames;
    const char* mGalaxyName;
    bool mPlaySound;
};
