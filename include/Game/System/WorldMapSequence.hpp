#pragma once
#include "Game/System/NerveExecutor.hpp"
#include "Inline.hpp"

class WorldMapOwner;
class WorldMapResultStateWait;
class WorldMapResultStatePointEvent;
class WorldMapResultStateTicoRouteCreate;
class WorldMapResultStateWorldNews;
class WorldMapResultStateWorldWarp;
class WorldMapResultStateGalaxyMap;

// The world map's sequence (nerve executor "シーケンス管理") owned by WorldMapOwner; its states
// are WorldMapResultState* actor states. One definition: WorldMapModel.cpp allocates it from this
// header, so the layout must be the real one (a 0x20-byte stand-in under-allocated it natively).
class WorldMapSequence : public NerveExecutor {
public:
    WorldMapSequence(void*);

    virtual ~WorldMapSequence();

    void init(bool);
    void exeStart();
    void exeWait();
    void exePointMove();
    void exePointWarp();
    void exeGreenStarDemo();
    void exeNoMove();
    bool isGalaxyMap() const NO_INLINE;
    bool updatePoint(bool) NO_INLINE;
    bool checkWorldNews() NO_INLINE;

    /* 0x08 */ WorldMapOwner* mOwner;
    /* 0x0C */ WorldMapResultStateWait* mStateWait;
    /* 0x10 */ WorldMapResultStatePointEvent* mStatePointEvent;
    /* 0x14 */ WorldMapResultStateTicoRouteCreate* mStateTicoRoute;
    /* 0x18 */ WorldMapResultStateWorldNews* mStateWorldNews;
    /* 0x1C */ WorldMapResultStateWorldWarp* mStateWorldWarp;
    /* 0x20 */ WorldMapResultStateGalaxyMap* mStateGalaxyMap;
    /* 0x24 */ bool mFlag;
};

