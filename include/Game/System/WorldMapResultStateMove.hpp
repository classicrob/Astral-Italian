#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateMove {
    WMR_NERVE_DECL(WorldMapResultStateMoveMove);
};  // namespace NrvWorldMapResultStateMove

/// Result-map step that moves the cursor ("State[移動]").
class WorldMapResultStateMove : public ActorStateBaseInterface {
public:
    WorldMapResultStateMove(WorldMapOwner*);
    virtual ~WorldMapResultStateMove();

    virtual void init();
    virtual void appear();
    void exeMove();


    /* 0x0C */ WorldMapOwner* mHost;
    /* 0x10 */ u8 mFlagA;
    /* 0x11 */ bool mFlagB;
    /* 0x14 */ const char* mTarget;
};
