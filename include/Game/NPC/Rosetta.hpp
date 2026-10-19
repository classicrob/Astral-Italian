#pragma once

#include "Game/NPC/NPCActor.hpp"
#include "Game/Util/NPCUtil.hpp"

class NameObjArchiveListCollector;
class NerveExecutor;
class TurnJointCtrl;
class TakeOutStar;

class Rosetta : public NPCActor {
public:
    /// @brief Creates a new `Rosetta`.
    /// @param pName The pointer to the null-terminated name of the object.
    Rosetta(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    bool canUpdateStarePos() const;

private:
    /* 0x164 */ TurnJointCtrl* mTurnJointCtrl;
    /* 0x168 */ bool mShouldGivePowerStar;
    /* 0x16C */ TakeOutStar* mTakeOutStar;
};
