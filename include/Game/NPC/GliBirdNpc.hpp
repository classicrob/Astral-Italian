#pragma once
#include "Game/NPC/NPCActor.hpp"
class GliBirdNpc : public NPCActor {
public:
    GliBirdNpc(const char*);
    virtual ~GliBirdNpc();
    virtual void init(const JMapInfoIter&);
};
