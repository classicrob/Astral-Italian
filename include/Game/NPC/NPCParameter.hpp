#pragma once

#include "Game/NPC/NPCActor.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class JMapInfo;
class NPCActorItem;

class NPCParameterBase {
public:
    NPCParameterBase(const char* pName) {
        mName = pName;
    };

    /* 0x00 */ const char* mName;

    virtual void read(JMapInfo*, s32);
};

class NPCParameterEdit : public NPCParameterBase {
public:
    NPCParameterEdit(const char*, const char**, bool);

    virtual void read(JMapInfo*, s32);

    /* 0x08 */ const char** mValue;
    /* 0x0C */ bool mIsResourceName;
};

class NPCParameterBool : public NPCParameterBase {
public:
    NPCParameterBool(const char*, bool*);

    virtual void read(JMapInfo*, s32);

    /* 0x08 */ bool* mValue;
};

class NPCParameterV3f : public NPCParameterBase {
public:
    NPCParameterV3f(const char*, TVec3f*, f32, f32);

    virtual void read(JMapInfo*, s32);

    /* 0x08 */ TVec3f* mValue;
    /* 0x0C */ f32 mMinValue;
    /* 0x10 */ f32 mMaxValue;
};

class NPCParameterJoint : public NPCParameterBase {
public:
    NPCParameterJoint(const char*, const char*, const char**);

    virtual void read(JMapInfo*, s32);

    /* 0x08 */ const char** mValue;
    /* 0x0C */ const char* mModelName;
    /* 0x10 */ s32 mJointIndex;
};

class NPCParameterReader : public NPCParameterBase, public MR::Vector< MR::FixedArray< NPCParameterBase*, 48 > > {
public:
    NPCParameterReader(const char*);

    virtual void read(JMapInfo*, s32);
};

class NPCCapsParameterReader : public NPCParameterReader {
public:
    NPCCapsParameterReader(const char*);

    void init();

    /* 0xCC */ NPCActorCaps mActorCaps;
};

class NPCItemParameterReader : public NPCParameterReader {
public:
    NPCItemParameterReader(const char*);

    void copy(const NPCActorItem*);
    /* 0xCC */ NPCActorItem mItem;
};

template < typename T >
class NPCParameterRange : public NPCParameterBase {
public:
    NPCParameterRange< T >(const char*, T*, T, T);

    virtual void read(JMapInfo*, s32);

    /* 0x08 */ T* mValue;
    /* 0x0C */ T mMinValue;
    /* 0x10 */ T mMaxValue;
};
