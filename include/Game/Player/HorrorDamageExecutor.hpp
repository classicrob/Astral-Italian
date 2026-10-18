#pragma once

#include "Game/System/NerveExecutor.hpp"

class MarioDamage;

class HorrorDamageExecutor : public NerveExecutor {
public:
    HorrorDamageExecutor(MarioDamage*);

    virtual ~HorrorDamageExecutor();

    void startBreak();
    bool isBreakEnd() const;
    void exeBoneBreakStart();
    void exeBoneBreakWait();
    void exeBoneBreakEnd();

    MarioDamage* mDamage;
    s32 mBreakDuration;
    bool mBreakEnded;
};
