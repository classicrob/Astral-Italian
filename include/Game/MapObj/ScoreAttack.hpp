#pragma once
#include <revolution/types.h>
class ScoreAttackMan;
namespace MR {
    void startScoreAttack();
    void endScoreAttack();
    s32 getScoreAttackTargetScore();
    bool isScoreAttackFinished();
    bool isScoreAttackClear();
    bool isScoreAttackFirstClear();
    bool isScoreAttackNewRecord();
    void registerScoreAttackMan(ScoreAttackMan*);
    void killScoreAttackLayouts();
    void killScoreAttackResults();
}
