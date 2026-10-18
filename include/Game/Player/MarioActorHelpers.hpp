#pragma once

#include <revolution.h>

class HashSortTable;
class LiveActor;
class MarioActor;
class MatrixValueGetter;

// Small helper classes owned by MarioActor in SMG2. Original names unknown; named by role/address.

/// @brief Name-keyed action table (size 8). Constructed by MarioActor::fn_803BFAE0.
class MarioActionHash {
public:
    MarioActionHash();  // fn_803DB5E0
    bool fn_803DB650(const char**, const char*, u16, u16);

    /* 0x00 */ HashSortTable* mTable;
    /* 0x04 */ s32 _4;
};

/// @brief Fixed array of 16 (value, actor) slots (MarioActor + 0x108C).
class MarioSensorSlots {
public:
    void fn_803DAB10(LiveActor*);

    /* 0x00 */ u8 _0[0x80];
    /* 0x80 */ s32 mCount;
};

class MarioTeresaModel {
public:
    static MarioTeresaModel* create(MarioActor*, bool);

    LiveActor* mActor;
    f32 _4;
    f32 _8;
    u8 _C[0x10];
};

class MarioNameMatrix {
public:
    u8 getValue(const char*, const char*) const;
    bool getValueOrNone(const char*, const char*, u8*) const;
};

class MarioActionTable {
public:
    MarioActionTable(MarioActor*);
    void initActions();
    void initAutoBind();
    void initEffects();
    bool isActionOk(const char*) const;
    u8 selectAction(const char*) const;
    bool selectAutoBind(const char*, u8*) const;
    bool testEffect(const char*, const char*) const;
    bool testEffectDisabled(const char*, const char*) const;
    f32 getFaceLookHeight(const char*) const;

    u8 _0[0xC];
    MatrixValueGetter* mHeights;
    MarioActor* mActor;
    MarioNameMatrix* mActions;
    MarioNameMatrix* mAutoBind;
    MarioNameMatrix* mEffects;
};
