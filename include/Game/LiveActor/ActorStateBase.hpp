#pragma once

#include "Game/LiveActor/ActorStateKeeper.hpp"
#include "Game/System/NerveExecutor.hpp"

class ActorStateBaseInterface : public NerveExecutor {
public:
    ActorStateBaseInterface(const char* pName) : NerveExecutor(pName), mIsDead(true) {
    }

    virtual ~ActorStateBaseInterface() {}

    virtual void init() {
    }

    virtual void appear() {
        mIsDead = false;
    }

    virtual void kill() {
        mIsDead = true;
    }

    virtual bool update();

    virtual void control() {
    }

    /* 0x8 */ bool mIsDead;
};

template < typename T >
class ActorStateBase : public ActorStateBaseInterface {
public:
#ifdef ACTOR_STATE_BASE_CTOR_NOINLINE
    ActorStateBase(const char* pName, T* pHost) NO_INLINE :
#else
    inline ActorStateBase(const char* pName, T* pHost) :
#endif
        ActorStateBaseInterface(pName), mHost(pHost) {
    }

    T* getHost() const {
        return mHost;
    }

    /* 0xC */ T* mHost;
};
