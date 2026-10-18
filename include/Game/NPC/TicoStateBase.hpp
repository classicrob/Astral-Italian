#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class NPCActor;
class TicoStateBase : public ActorStateBaseInterface {
public:
    TicoStateBase(const char* name, NPCActor* host) : ActorStateBaseInterface(name), mHost(host) {}
    virtual ~TicoStateBase() NO_INLINE;
    /* 0x0C */ NPCActor* mHost;
};
