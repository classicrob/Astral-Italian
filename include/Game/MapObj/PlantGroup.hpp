#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class PlantMember;

class PlantGroup : public LiveActor {
public:
    PlantGroup(const char*);

    virtual ~PlantGroup();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void makeModelName(char*, u32) const;  // fn_802DB950
    bool isBloomEnd() const;               // fn_802DB9B0
    void initPlantType(s32*, s32*, const JMapInfoIter&);  // fn_802DC790
    void initMember(s32, const JMapInfoIter&);
    s32 placeOnCollisionFormCircle(TVec3f*, const TVec3f&, const TVec3f&, const TVec3f&);
    s32 placeOnRail(TVec3f*);  // fn_802DC190
    f32 calcBoundingSphereRadius(const TVec3f&);
    void updateBloom();  // fn_802DC900
    void emitHintEffect();

    /* 0x90 */ PlantMember** mMembers;
    /* 0x94 */ s32 mMemberCount;
    /* 0x98 */ s32 mItemCount;
    /* 0x9C */ s32 mPlantType;
    /* 0xA0 */ s32 mPlantShape;
    /* 0xA4 */ s32 mItemType;
    /* 0xA8 */ s32 mColorFrame;
    /* 0xAC */ s32 mUseGravity;
    /* 0xB0 */ s32 mRandomRotation;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ s32 mBloomTimer;
    /* 0xBC */ TVec3f mHintPosition;
    /* 0xC8 */ TVec3f mHintRotation;
    /* 0xD4 */ s32 mHintTimer;
    /* 0xD8 */ s32 mHintIndex;
};

class PlantMember : public LiveActor {
public:
    PlantMember(const char* pName, const char* pModelName, bool useLight, bool strongLight);

    virtual ~PlantMember();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void control();

    bool tryEmitHint();
    void exeWait();
    void exeHint();
    void exeShakeWeak();
    void exeShakeMiddle();
    void exeShakeStrong();
    bool generateItem(PlantGroup*);
    void initPosture(bool);
    bool tryShake(HitSensor*);
    bool tryPush(const TVec3f&, f32, s32);
    void animControl(PlantGroup*);

    bool isAppearing() const {
        return _98 >= 0.0f;
    }

    /* 0x90 */ s32 mPlantType;
    /* 0x94 */ bool mHasItem;
    /* 0x95 */ u8 _95;
    /* 0x98 */ f32 _98;
};

#pragma push
#pragma fp_contract off
template <typename T>
NO_INLINE T JMAHermiteInterpolation(T value, T start, T startValue, T startTangent, T end, T endValue, T endTangent) {
    T distance = value - start;
    T span = end - start;
    T difference = startValue - endValue;
    T t = distance / span;
    T square = t * t;
    T twiceT = t + t;
    T blend = square - t;
    T weight = twiceT * blend - square;
    T tangent = startTangent * blend + startTangent;
    T interpolated = weight * difference + startValue;
    tangent = endTangent * blend + tangent;
    tangent = t * startTangent - tangent;
    return interpolated - distance * tangent;
}
#pragma pop
