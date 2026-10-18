#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"

class MapObjConnector;
class TogepinHole;
class TogepinScorerSub;

class TogepinHole : public LiveActor {
public:
    TogepinHole(const char*, LiveActor*);

    virtual ~TogepinHole();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    /* 0x90 */ MapObjConnector* mConnector;
};

class Togepin : public LiveActor {
public:
    Togepin(const char*, LiveActor*);

    virtual ~Togepin();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    bool tryBlow(HitSensor*, HitSensor*);
    void tryHide();
    void tryAppear();
    void reappear();

    void exeAppear();
    void exeWait();
    void exeReaction();
    void exePanic();
    void exeHide();
    void exeBlow();

    bool isNerveTypeHittable() const;
    bool isNerveTypeBlow() const NO_INLINE;

    /* 0x90 */ LiveActor* mParent;
    /* 0x94 */ MapObjConnector* mConnector;
    /* 0x98 */ TogepinHole* mHole;
    /* 0x9C */ TVec3f mInitPos;
    /* 0xA8 */ bool mIsRushRockMario;
};

class TogepinGroup : public LiveActorGroup {
public:
    TogepinGroup(const char* pName, int numMax) : LiveActorGroup(pName, numMax) {}

    virtual ~TogepinGroup();
};

class TogepinScorer : public LiveActor {
public:
    TogepinScorer(const char*);

    virtual ~TogepinScorer();
    virtual void init(const JMapInfoIter&);

    /* 0x90 */ NameObj* mScoreCounter;
    /* 0x94 */ NameObj* mSetLayout;
    /* 0x98 */ LiveActorGroup* mTogepinGroup;
    /* 0x9C */ TogepinScorerSub* _9C;
    /* 0xA0 */ s32 mCounter;
    /* 0xA4 */ bool mIsClear;
    /* 0xA5 */ bool mIsEnd;
};
