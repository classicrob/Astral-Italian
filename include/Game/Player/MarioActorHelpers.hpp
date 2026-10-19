#pragma once

#include <revolution.h>
#include "Game/NameObj/NameObj.hpp"

class HashSortTable;
class LiveActor;
class MarioActor;
class MatrixValueGetter;
class HitSensor;

class MarioActorCondition {
public:
    MarioActorCondition(MarioActor* actor) : mActor(actor), mSensor(nullptr) {}
    virtual bool canGet();
    virtual void execute();
    virtual bool result();

    MarioActor* mActor;
    HitSensor* mSensor;
};

class MarioActorCondition1 : public MarioActorCondition {
public:
    MarioActorCondition1(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual bool result();
};

class MarioActorCondition2 : public MarioActorCondition {
public:
    MarioActorCondition2(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual void execute();
    virtual bool result();
};

class MarioActorCondition3 : public MarioActorCondition {
public:
    MarioActorCondition3(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual void execute();
    virtual bool result();
};

class MarioActorCondition4 : public MarioActorCondition {
public:
    MarioActorCondition4(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual void execute();
    virtual bool result();
};

class MarioActorCondition5 : public MarioActorCondition {
public:
    MarioActorCondition5(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual bool result();
};

class MarioActorCondition6 : public MarioActorCondition {
public:
    MarioActorCondition6(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual bool result();
};

class MarioActorCondition7 : public MarioActorCondition {
public:
    MarioActorCondition7(MarioActor*) NO_INLINE;
    virtual bool canGet();
    virtual void execute();
    virtual bool result();
};

class MarioActorConditionGroup {
public:
    MarioActorConditionGroup(MarioActor*);
    bool tryGetItem(HitSensor*);
    MarioActorCondition* getCondition(HitSensor*);

    MarioActor* mActor;
    MarioActorCondition* mConditions[8];
};

class MarioScreenStripOffset {
public:
    virtual f32 calcOffset(u32) = 0;
};

class MarioScreenStripWave : public MarioScreenStripOffset {
public:
    MarioScreenStripWave(f32, u32, s32, f32);
    virtual f32 calcOffset(u32);
    f32 mAmplitude;
    f32 mPhase;
    f32 mFrequency;
};

class MarioScreenStripCurve : public MarioScreenStripOffset {
public:
    MarioScreenStripCurve(u32, u32);
    virtual f32 calcOffset(u32);
    u32 mPosition;
    f32 mPhase;
};

class MarioScreenStrip {
public:
    MarioScreenStrip();
    void fn_803DB9D0(s32, s32, s32);
    void fn_803DBA40();
    void fn_803DBAA0(u32);
    void fn_803DBB10(u32, u32);
    void fn_803DBB50(MarioScreenStripOffset*);
    u8* mBuffers[2];
    bool mEnabled;
    f32 mProgress;
    s16 mMode;
    f32 mBlend;
};

class MarioActorEffectState {
public:
    MarioActorEffectState() : mActive(0), mAlpha(255), mScale(1.0f), mTimer(0) {}
    u8 mActive;
    u32 mAlpha;
    f32 mScale;
    u16 mTimer;
};

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
    MarioSensorSlots();
    void fn_803DAB10(LiveActor*);
    void fn_803DAB90();

    struct Slot {
        s32 mTimer;
        LiveActor* mActor;
    };

    /* 0x00 */ Slot mSlots[16];
    /* 0x80 */ s32 mCount;
};

class MarioParts;
class XanimePlayer;
class XanimeResourceTable;

class MarioTeresaModel {
public:
    static MarioTeresaModel* create(MarioActor*, bool);
    MarioTeresaModel(const char*, MarioActor*);
    void initAnimation();

    MarioParts* mActor;
    f32 _4;
    f32 _8;
    f32 _C;
    u16 _10;
    XanimePlayer* mPlayer;
    XanimeResourceTable* mResource;
};

class MarioNameMatrix : public NameObj {
public:
    MarioNameMatrix(const char*, u32, const char* const*, u32, const char* const*, const char* const*);
    virtual ~MarioNameMatrix();
    void load(const char*, const char*);
    bool setValue(const char*, const char*, u8);
    s32 findIndex(const char*, const char*) const;
    s32 findRow(const char*) const;
    s32 findColumn(const char*) const;
    u8 getValue(const char*, const char*) const;
    bool getValueOrNone(const char*, const char*, u8*) const;

    u32 mRowCount;
    const char* const* mRows;
    u32 mColumnCount;
    const char* const* mColumns;
    const char* const* mColumnKeys;
    u8* mValues;
    HashSortTable* mRowHash;
    HashSortTable* mColumnHash;
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

