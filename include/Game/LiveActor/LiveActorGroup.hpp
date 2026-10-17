#pragma once

#include "Game/NameObj/NameObjGroup.hpp"

class LiveActor;

class LiveActorGroup : public NameObjGroup {
public:
    LiveActorGroup(const char*, int);

    inline virtual ~LiveActorGroup() {
    }

    void registerActor(LiveActor*);
    LiveActor* getActor(int) const;
    LiveActor* getDeadActor() const;
    s32 getLivingActorNum() const;
    void killAll();
    void appearAll();
    void makeAllDead();

    bool hasActor(const LiveActor* pActor) const {
        for (int i = 0; i < getObjNum(); i++) {
            if (getActor(i) == pActor) {
                return true;
            }
        }

        return false;
    }
};

template < typename T >
class DeriveActorGroup : public LiveActorGroup {
public:
    DeriveActorGroup(const char* pName, int numMax) : LiveActorGroup(pName, numMax) {
    }

    T* getMember(int index) const {
        return static_cast< T* >(getActor(index));
    }

    T* getDeadMember() const {
        if (getDeadActor() != nullptr) {
            T* pActor = static_cast< T* >(getDeadActor());
            return pActor;
        }

        return nullptr;
    }
};
