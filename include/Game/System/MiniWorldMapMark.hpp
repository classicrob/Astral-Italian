#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Shared 0x98-byte child used by the Mini* world-map objects (MiniStarCheckPoint, MiniTicoMaster).
// Its SMG2 class name is not in the symbols; "MiniWorldMapMark" is a placeholder.
class MiniWorldMapMark : public LiveActor {
public:
    MiniWorldMapMark(LiveActor* pOwner, const char* pName);
    virtual ~MiniWorldMapMark();
    virtual void init(const JMapInfoIter&);

    LiveActor* mOwner;   // 0x90
    const char* mName;   // 0x94
};
