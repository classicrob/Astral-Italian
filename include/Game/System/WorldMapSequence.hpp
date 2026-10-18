#pragma once
#include "Game/System/NerveExecutor.hpp"
class WorldMapOwner;
class WorldMapSequence : public NerveExecutor {
public:
    WorldMapSequence(void*);
    void init(bool);
    bool isGalaxyMap() const;
    u8 mData[0x20];
};

class WorldMapMainLayout {
public:
    WorldMapMainLayout(WorldMapOwner*);
    void init(bool);
    u8 mData[0x18];
};
