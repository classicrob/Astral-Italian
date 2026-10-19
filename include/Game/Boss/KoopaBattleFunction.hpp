#pragma once

#include <JSystem/JGeometry.hpp>
#include <revolution.h>

class HitSensor;
class KoopaBattle;
class LiveActor;
class NameObj;

// Helper functions of the SMG2 final Bowser battle (KoopaFinalBattlePosTU01).
namespace MR {
    KoopaBattle* getKoopaBattle();
    void setKoopaBattleNo(s32);
    void getKoopaBattleBasePos(TVec3f*);
    f32 getKoopaBattleRadius();
    void getKoopaBattlePoseB(TQuat4f*, TVec3f*);
    void calcKoopaBattleDirFromBase(TVec3f*, const TVec3f&);
    void calcKoopaBattleEdgePos(TVec3f*, const TVec3f&);
    void calcKoopaBattleEdgePos(TVec3f*, const TVec3f&, f32);
    void getKoopaBattlePos(TVec3f*);
    void getKoopaBattleUp(TVec3f*);
    void calcKoopaBattleDirToActor(TVec3f*);
    void calcKoopaBattleActorEdgePos(TVec3f*);
    void calcKoopaBattleWorldPos(TVec3f*, const TVec3f&);
    f32 calcKoopaBattleFrontDot(const TVec3f&);
    f32 calcKoopaBattleDistance(const TVec3f&);
    void getKoopaBattleTargetPos(TVec3f*);
    bool turnKoopaBattleToPlayer(KoopaBattle*, f32, f32);
    void appearCoinPopKoopaBattle(const TVec3f&);
    void addKoopaBattleReceiverSensor(HitSensor*);
    void resetKoopaBattleDemo(KoopaBattle*);
    void resetKoopaBattleToPoseA(KoopaBattle*);
    void resetKoopaBattleToDemoChamber(KoopaBattle*);
    void resetKoopaBattleToFinalStar(KoopaBattle*);
    void resetKoopaBattlePosition(KoopaBattle*);
    void invalidateKoopaBattleCollision(KoopaBattle*);
    void validateKoopaBattleCollision(KoopaBattle*);
    void calcKoopaBattleFacePos(TVec3f*);
    void startKoopaBattleFaceCtrl(KoopaBattle*, s32);
    void endKoopaBattleFaceCtrl(KoopaBattle*, s32);
    void startKoopaBattleDynamicCtrl(KoopaBattle*, s32);
    void setKoopaBattleVolumeScaleUpRate(KoopaBattle*, s32);
    void setKoopaBattleVolumeScaleDownRate(KoopaBattle*, s32);
    void appearKoopaBattlePlanet();
    void killKoopaBattlePlanet();
    void appearKoopaDemoChamber();
    void killKoopaDemoChamber();
    void startBreakKoopaDemoChamber();
    void appearKoopaFinalStarPlanet();
    LiveActor* createKoopaBattleSky(KoopaBattle*);
    LiveActor* createKoopaLastBattleSky(KoopaBattle*);
    void calcKoopaNearestAttackRockPos(TVec3f*);
    void setPlayerToKoopaDemoChamberEnd();
    void setKoopaBattleNo0();
    void setKoopaBattleNo1();
    void setKoopaBattleNo2();
    void setKoopaBattleNoA();
    void setKoopaBattleNoB();
    void setKoopaBattleNoC();
    s32 sendMsgToKoopaReceivers(u32);
    s32 sendMsgKoopaFinalBattleStart();
    s32 sendMsgKoopaC6();
    s32 sendMsgKoopaC8();
    s32 sendMsgKoopaCastAttackRockEnd();
    s32 sendMsgKoopaReleaseAttackRock();
    s32 sendMsgKoopaBB();
    bool trySendMsgToKoopaReceivers(u32);
    bool trySendMsgKoopaCastAttackRockStart();
    bool sendMsgAttackRockReflect(HitSensor*, HitSensor*);
    bool sendMsgKoopaBD(HitSensor*, HitSensor*);
    bool sendMsgKoopaPunchAttack(HitSensor*, HitSensor*);
    bool sendMsgToBindedAttackRockReflect(LiveActor*, HitSensor*);
    bool sendMsgToBindedKoopaC7(LiveActor*, HitSensor*);
    bool isMsgAttackRockReflect(u32);
    bool isMsgKoopaC7(u32);
    bool isMsgKoopaPunchAttack(u32);
    bool isMsgKoopaC8(u32);
    bool isMsgKoopaBA(u32);
    bool isMsgKoopaBB(u32);
    bool isMsgKoopaFinalBattleStart(u32);
    bool isMsgKoopaC6(u32);
    bool isMsgKoopaCastAttackRockStart(u32);
    bool isMsgKoopaCastAttackRockEnd(u32);
    bool isMsgKoopaReleaseAttackRock(u32);
    bool isKoopaHeadOrBodySensor(LiveActor*, HitSensor*);
    bool isKoopaBody2OrLegSensor(LiveActor*, HitSensor*);
    bool isKoopaArmLSensor(LiveActor*, HitSensor*);
    bool isKoopaArmRSensor(LiveActor*, HitSensor*);
};  // namespace MR
