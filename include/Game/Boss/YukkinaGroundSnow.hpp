#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BreakModel;

class YukkinaGroundSnow : public LiveActor {
public:
    /// @brief Creates a new `YukkinaGroundSnow`.
    /// @param pName A pointer to the null-terminated name of the object.
    YukkinaGroundSnow(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool tryBreak();
    void exeAppear();
    void exeWait();
    void exeBreak();

private:
    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ s32 mBreakCount;
    /* 0x98 */ s32 mItemIndex;
};
