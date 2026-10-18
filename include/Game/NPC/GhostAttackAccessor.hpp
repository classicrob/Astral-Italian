#pragma once

#include "Game/NameObj/NameObj.hpp"

class GhostAttackGhost;

class GhostAttackAccessor : public NameObj {
public:
    GhostAttackAccessor();
    virtual ~GhostAttackAccessor();

    GhostAttackGhost* mGhost;
};

namespace MR {
    GhostAttackAccessor* registerGhostAttackGhost(GhostAttackGhost*);
}
