#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "JSystem/JGeometry/TVec.hpp"

class LiveActor;
class HitSensor;
class KoopaBattle;
class KoopaDemoChamber;
class KoopaBattlePlanet;
class KoopaFinalStarPlanet;
class AttackRockFinal;

// SMG2-only scene object (SceneObj 0x67) shared by the Bowser final battle pieces.
class KoopaBattleAccessor : public NameObj {
public:
    KoopaBattleAccessor();

    virtual ~KoopaBattleAccessor() {}

    void registerKoopaBattle(KoopaBattle*);
    void registerKoopaDemoChamber(KoopaDemoChamber*);
    void registerKoopaBattlePlanet(KoopaBattlePlanet*);
    void registerKoopaFinalStarPlanet(KoopaFinalStarPlanet*);
    void setPoseA(const LiveActor*);
    void getPoseA(TQuat4f*, TVec3f*) const;
    void setPoseB(const LiveActor*);
    void getPoseB(TQuat4f*, TVec3f*) const;
    void addReceiverSensor(HitSensor*);
    HitSensor* getReceiverSensor(s32) const;
    void addAttackRockFinal(AttackRockFinal*);
    AttackRockFinal* getAttackRockFinal(s32) const;

    /* 0x14 */ KoopaBattle* mKoopaBattle;
    /* 0x18 */ KoopaDemoChamber* mDemoChamber;
    /* 0x1C */ KoopaBattlePlanet* mBattlePlanet;
    /* 0x20 */ KoopaFinalStarPlanet* mFinalStarPlanet;
    /* 0x24 */ HitSensor** mReceiverSensors;
    /* 0x28 */ s32 mReceiverSensorNum;
    /* 0x2C */ s32 mReceiverSensorMax;
    /* 0x30 */ AttackRockFinal** mAttackRocks;
    /* 0x34 */ s32 mAttackRockNum;
    /* 0x38 */ s32 mAttackRockMax;
    /* 0x3C */ TQuat4f mQuatA;
    /* 0x4C */ TVec3f mPosA;
    /* 0x58 */ TQuat4f mQuatB;
    /* 0x68 */ TVec3f mPosB;
};

namespace MR {
    void createKoopaBattleAccessor();
    KoopaBattleAccessor* getKoopaBattleAccessor();
};  // namespace MR
