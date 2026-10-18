#pragma once

#include "Game/MapObj/DiceBase.hpp"

class BenefitItemObj;

// SMG2-only unit (no Petari source). Four dice classes share this TU: ItemDice (two benefit items),
// MultipleChoiceDice (switch based, no items), PositiveDice (one star piece item), LuckyDice (five items).
// Virtual overrides fill the pure slots 0x70..0x84 of DiceBase.

class ItemDice : public DiceBase {
public:
    ItemDice(const char* pName);
    virtual ~ItemDice();
    virtual const char* getModelName();
    virtual void initItems(const JMapInfoIter&);
    virtual void setAnimFrame();
    virtual void setResultFrame(u32);
    virtual bool makeResult(u32);
    virtual bool isGoodResult(u32) const;

    // unnamed fn_802B4B50 / fn_802B4BB0 in the target
    void placeMainItem() NO_INLINE;
    void popSubItem() NO_INLINE;

    /* 0x10C */ BenefitItemObj* mMainItem;
    /* 0x110 */ BenefitItemObj* mSubItem;
};

class MultipleChoiceDice : public DiceBase {
public:
    virtual ~MultipleChoiceDice();
    virtual const char* getModelName();
    virtual void initItems(const JMapInfoIter&);
    virtual void setAnimFrame();
    virtual void setResultFrame(u32);
    virtual bool makeResult(u32);
    virtual bool isGoodResult(u32) const;
};

class PositiveDice : public DiceBase {
public:
    PositiveDice(const char* pName);
    virtual ~PositiveDice();
    virtual const char* getModelName();
    virtual void initItems(const JMapInfoIter&);
    virtual void setAnimFrame();
    virtual void setResultFrame(u32);
    virtual bool makeResult(u32);
    virtual bool isGoodResult(u32) const;

    // unnamed fn_802B4F90 in the target
    void popItem() NO_INLINE;

    /* 0x10C */ BenefitItemObj* mItem;
};

class LuckyDice : public DiceBase {
public:
    LuckyDice(const char* pName);
    virtual const char* getModelName();
    virtual void initItems(const JMapInfoIter&);
    virtual void setAnimFrame();
    virtual void setResultFrame(u32);
    virtual bool makeResult(u32);
    virtual bool isGoodResult(u32) const;

    // unnamed fn_802B5240 in the target: throws `count` items toward the player
    void shootItems(s32 count);

    /* 0x10C */ BenefitItemObj* mItems[5];
};
