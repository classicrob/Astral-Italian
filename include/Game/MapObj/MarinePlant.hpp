#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class HitSensor;
class MarinePlantMember;



class MarinePlant : public LiveActor {
public:
    MarinePlant(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual ~MarinePlant();

    void initMember(const JMapInfoIter&);
    void makeMemberName(char*, s32, const JMapInfoIter&);
    s32 calcMemberSpread(TVec3f*, const TVec3f&, const TVec3f&, const TVec3f&);
    f32 calcMemberBounds(const TVec3f&);

    /* 0x90 */ MarinePlantMember** mMembers;
    /* 0x94 */ s32 mNumMembers;
    /* 0x98 */ f32 mRadius;
};

class MarinePlantMember : public LiveActor {
public:
    MarinePlantMember(const char*, const char*);

    virtual void init(const JMapInfoIter&);
    virtual ~MarinePlantMember();

    bool tryReactToSensor(HitSensor*);
    void initAxisAndScale();

    void exeWait();
    void exeReact();

    /* 0x90 */ u8 _90[0x18];
    /* 0xA8 */ TVec3f mSpinAxis;
};
