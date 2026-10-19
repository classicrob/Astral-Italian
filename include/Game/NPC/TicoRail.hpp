#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class LodCtrl;
class TalkMessageCtrl;

class TicoRail : public LiveActor {
public:
    /// @brief Creates a new `TicoRail`.
    /// @param pName A pointer to the null-terminated name of the object.
    TicoRail(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeLookAround();
    void exeMoveSign();
    void exeMove();
    void exeStop();
    void exeTalkStart();
    void exeTalk();
    void exeTalkCancel();
    void exeGoodBye();
    bool isGreaterEqualStepAndRandom(s32) const;
    bool isSameRailActor(const LiveActor*) const;

    /* 0x8C */ TVec3f mFrontDir;
    /* 0x98 */ LiveActor* mTalkPartner;
    /* 0x9C */ LodCtrl* mLodCtrl;
    /* 0xA4 */ TalkMessageCtrl* mMsgCtrl;
    /* 0xA8 */ bool mIsTalkForceAtEnd;
};
