#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class MapObjConnector;
class SpinPullParticleCallBack;

class Candlestand : public MapObjActor {
public:
    Candlestand(const char* pName);

    virtual ~Candlestand();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool tryExtinguish();
    void emitEffectFire();
    void deleteEffectFire();
    void emitEffectExtinguishFire();
    void appearItem();
    void exeWaitFire();
    void exeFire();
    void exeExtinguish();
    void exeFlicker();
    inline void exeAttack();
    inline void exeBurn();

    /* 0xD0 */ s32 mItem;
    /* 0xD4 */ bool mHasItemAppear;
    /* 0xD8 */ SpinPullParticleCallBack* mSpinPtclCb;
    /* 0xDC */ MapObjConnector* mConnector;
};
