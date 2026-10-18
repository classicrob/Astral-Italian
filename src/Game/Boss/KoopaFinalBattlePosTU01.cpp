#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaDemoChamber.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Enemy/AttackRock.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Map/Sky.hpp"
#include "Game/Util.hpp"


// PLACEHOLDER(objects): unnamed core helper (dst = joint matrix of (actor, joint name) * local position); resolve when core names fn_8002D5C0
extern "C" void fn_8002D5C0(TVec3f*, const TVec3f*, const LiveActor*, const char*);

namespace {
    TVec3f sFaceOffset(100.0f, -50.0f, 0.0f);
};  // namespace

namespace MR {
    KoopaBattle* getKoopaBattle() {
        KoopaBattleAccessor* pAccessor = getKoopaBattleAccessor();
        if (pAccessor == nullptr) {
            return nullptr;
        }
        return pAccessor->mKoopaBattle;
    }

    void setKoopaBattleNo(s32 no) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle != nullptr) {
            pBattle->mBattleNo = no;
        }
    }

    void getKoopaBattleBasePos(TVec3f* pDst) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            pDst->zero();
            return;
        }
        pDst->set< f32 >(pBattle->mPos);
    }

    f32 getKoopaBattleRadius() {
        return 850.0f;
    }

    void getKoopaBattlePoseB(TQuat4f* pQuat, TVec3f* pPos) {
        getKoopaBattleAccessor()->getPoseB(pQuat, pPos);
    }

    void calcKoopaBattleDirFromBase(TVec3f* pDst, const TVec3f& rPos) {
        TVec3f base;
        getKoopaBattleBasePos(&base);
        TVec3f diff = rPos - base;
        pDst->set< f32 >(diff);
        MR::normalizeOrZero(pDst);
    }

    void calcKoopaBattleEdgePos(TVec3f* pDst, const TVec3f& rPos) {
        calcKoopaBattleEdgePos(pDst, rPos, 850.0f);
    }

    void calcKoopaBattleEdgePos(TVec3f* pDst, const TVec3f& rPos, f32 dist) {
        TVec3f base;
        getKoopaBattleBasePos(&base);
        TVec3f dir;
        dir.set< f32 >(rPos - base);
        MR::normalizeOrZero(&dir);
        pDst->set< f32 >(base + dir * dist);
    }

    void getKoopaBattlePos(TVec3f* pDst) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            pDst->zero();
            return;
        }
        pDst->set< f32 >(pBattle->mPosition);
    }

    void getKoopaBattleUp(TVec3f* pDst) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            pDst->set< f32 >(0.0f, 1.0f, 0.0f);
            return;
        }
        MR::calcUpVec(pDst, pBattle);
    }

    void calcKoopaBattleDirToActor(TVec3f* pDst) {
        TVec3f pos;
        getKoopaBattlePos(&pos);
        TVec3f base;
        getKoopaBattleBasePos(&base);
        TVec3f diff = pos - base;
        pDst->set< f32 >(diff);
        MR::normalizeOrZero(pDst);
    }

    void calcKoopaBattleActorEdgePos(TVec3f* pDst) {
        TVec3f pos;
        getKoopaBattlePos(&pos);
        TVec3f base;
        getKoopaBattleBasePos(&base);
        TVec3f diff = pos - base;
        diff *= 850.0f / PSVECMag(&diff);
        TVec3f res = base + diff;
        pDst->set< f32 >(res);
    }

    void calcKoopaBattleWorldPos(TVec3f* pDst, const TVec3f& rLocal) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            pDst->set< f32 >(rLocal);
            return;
        }
        TVec3f rotated;
        pBattle->mQuatB.transform(rLocal, rotated);
        TVec3f res = pBattle->mPos + rotated;
        pDst->set< f32 >(res);
    }

    f32 calcKoopaBattleFrontDot(const TVec3f& rPos) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            return 0.0f;
        }
        TVec3f diff = rPos - pBattle->mPos;
        TVec3f front;
        pBattle->mQuatB.getZDir(front);
        return diff.dot(front);
    }

    f32 calcKoopaBattleDistance(const TVec3f& rPos) {
        TVec3f base;
        getKoopaBattleBasePos(&base);
        return PSVECDistance(&base, &rPos) - 850.0f;
    }

    void getKoopaBattleTargetPos(TVec3f* pDst) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle == nullptr) {
            pDst->zero();
            return;
        }
        pDst->set< f32 >(pBattle->mPosition);
    }

    bool turnKoopaBattleToPlayer(KoopaBattle* pBattle, f32 speed, f32 limit) {
        TVec3f base;
        getKoopaBattleBasePos(&base);
        TVec3f toActor = pBattle->mPosition - base;
        TVec3f toPlayerDir;
        TVec3f toTarget;
        TVec3f up;
        MR::normalizeOrZero(&toActor);
        MR::normalizeOrZero(&toPlayerDir, *MR::getPlayerPos() - base);
        MR::setPosition(pBattle, base + toActor * 4000.0f);
        f32 angle = 57.29578f * MR::diffAngleAbs(toActor, toPlayerDir);
        f32 rate;
        if (angle < 100.0f) {
            rate = MR::normalize(angle, 45.0f, 100.0f);
        } else {
            rate = 1.0f - MR::normalize(angle, 100.0f, 180.0f);
        }
        MR::calcVecToTargetPosH(&toTarget, pBattle, *MR::getPlayerPos(), &toActor);
        MR::turnQuatYDirRad(&pBattle->mQuatA, pBattle->mQuatA, toTarget, 0.017453292f * (3.0f * rate));
        MR::turnQuatZDirRad(&pBattle->mQuatA, pBattle->mQuatA, -toActor, 0.10471976f);
        MR::calcUpVec(&up, pBattle);
        JMAVECScaleAdd(&toActor, &up, &up, -toActor.dot(up));
        MR::normalizeOrZero(&up);
        MR::fn_80010CB0(pBattle, up, 5.5f * speed);
        MR::turnVecToPlane(&pBattle->mVelocity, pBattle->mVelocity, toActor);
        MR::attenuateVelocity(pBattle, 0.95f);
        return angle < limit;
    }

    void appearCoinPopKoopaBattle(const TVec3f& rPos) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle != nullptr) {
            MR::appearCoinPop(pBattle, rPos, 1);
        }
    }

    void addKoopaBattleReceiverSensor(HitSensor* pSensor) {
        createKoopaBattleAccessor();
        KoopaBattleAccessor* pAccessor = getKoopaBattleAccessor();
        if (pAccessor != nullptr) {
            pAccessor->addReceiverSensor(pSensor);
        }
    }

    void resetKoopaBattleDemo(KoopaBattle* pBattle) {
        pBattle->mJointCtrl->endFaceCtrl(0);
        s32 frame;
        pBattle->mJointCtrl->endDynamicCtrl(nullptr, frame);
        MR::showModel(pBattle);
        MR::showPlayer();
        MR::resetPlayerStatus();
        MR::setPlayerStateWait();
        pBattle->mVolumeDrawer->setScaleDownRate(0);
        MR::invalidateCollisionParts(pBattle);
        sendMsgToKoopaReceivers(0xBA);
        MR::zeroVelocity(pBattle);
    }

    void resetKoopaBattleToPoseA(KoopaBattle* pBattle) {
        TPos3f mtx;
        TQuat4f quat;
        TVec3f pos;
        getKoopaBattleAccessor()->getPoseA(&quat, &pos);
        pBattle->setPose(quat, pos);
        resetKoopaBattlePosition(pBattle);
        TVec3f front;
        TVec3f playerPos;
        TVec3f up;
        pBattle->mQuatB.getZDir(front);
        playerPos.set< f32 >(pBattle->mPos + front * 1350.0f);
        mtx.identity();
        pBattle->mQuatB.getYDir(up);
        MR::makeMtxUpFrontPos(&mtx, front, up, playerPos);
        MR::setPlayerBaseMtx(mtx.toMtxPtr());
        MR::resetPlayerStatus();
        MR::setPlayerStateWait();
    }

    void resetKoopaBattleToDemoChamber(KoopaBattle* pBattle) {
        KoopaDemoChamber* pChamber = getKoopaBattleAccessor()->mDemoChamber;
        TPos3f mtx(pChamber->getBaseMtx());
        mtx.getQuat(pBattle->mQuatA);
        mtx.getTrans(pBattle->mPosition);
        MR::resetPosition(pBattle);
        TPos3f playerMtx(pChamber->getBaseMtx());
        MR::setPlayerBaseMtx(playerMtx.toMtxPtr());
        MR::resetPlayerStatus();
        MR::setPlayerStateWait();
    }

    void resetKoopaBattleToFinalStar(KoopaBattle* pBattle) {
        KoopaFinalStarPlanet* pPlanet = getKoopaBattleAccessor()->mFinalStarPlanet;
        if (pPlanet != nullptr) {
            TPos3f mtx(pPlanet->getBaseMtx());
            mtx.getQuat(pBattle->mQuatA);
            mtx.getTrans(pBattle->mPosition);
            MR::resetPosition(pBattle);
            TPos3f playerMtx(pPlanet->getBaseMtx());
            MR::setPlayerBaseMtx(playerMtx.toMtxPtr());
            MR::resetPlayerStatus();
            MR::setPlayerStateWait();
        }
    }

    void resetKoopaBattlePosition(KoopaBattle* pBattle) {
        MR::zeroVelocity(pBattle);
        pBattle->mQuatA = pBattle->mQuatB;
        TVec3f front;
        pBattle->mQuatB.getZDir(front);
        MR::resetPosition(pBattle, pBattle->mPos - front * 4000.0f);
    }

    void invalidateKoopaBattleCollision(KoopaBattle* pBattle) {
        pBattle->getDefenseCollision()->appear();
        MR::invalidateCollisionParts(pBattle);
        pBattle->_10C = true;
    }

    void validateKoopaBattleCollision(KoopaBattle* pBattle) {
        pBattle->getDefenseCollision()->kill();
        MR::validateCollisionParts(pBattle);
        pBattle->_10C = false;
    }

    void calcKoopaBattleFacePos(TVec3f* pDst) {
        KoopaBattle* pBattle = getKoopaBattle();
        if (pBattle != nullptr && pDst != nullptr) {
            fn_8002D5C0(pDst, &sFaceOffset, pBattle, "Face");
        }
    }

    void startKoopaBattleFaceCtrl(KoopaBattle* pBattle, s32 no) {
        pBattle->mJointCtrl->startFaceCtrl(no);
    }

    void endKoopaBattleFaceCtrl(KoopaBattle* pBattle, s32 no) {
        pBattle->mJointCtrl->endFaceCtrl(no);
    }

    void startKoopaBattleDynamicCtrl(KoopaBattle* pBattle, const char* pName, s32 frame) {
        pBattle->mJointCtrl->startDynamicCtrl(pName, frame);
    }

    void setKoopaBattleVolumeScaleUpRate(KoopaBattle* pBattle, s32 rate) {
        pBattle->mVolumeDrawer->setScaleUpRate(rate);
    }

    void setKoopaBattleVolumeScaleDownRate(KoopaBattle* pBattle, s32 rate) {
        pBattle->mVolumeDrawer->setScaleDownRate(rate);
    }

    void appearKoopaBattlePlanet() {
        getKoopaBattleAccessor()->mBattlePlanet->makeActorAppeared();
    }

    void killKoopaBattlePlanet() {
        getKoopaBattleAccessor()->mBattlePlanet->makeActorDead();
    }

    void appearKoopaDemoChamber() {
        getKoopaBattleAccessor()->mDemoChamber->makeActorAppeared();
    }

    void killKoopaDemoChamber() {
        getKoopaBattleAccessor()->mDemoChamber->makeActorDead();
    }

    void startBreakKoopaDemoChamber() {
        getKoopaBattleAccessor()->mDemoChamber->startBreak();
    }

    void appearKoopaFinalStarPlanet() {
        getKoopaBattleAccessor()->mFinalStarPlanet->makeActorAppeared();
    }

    LiveActor* createKoopaBattleSky(KoopaBattle* pBattle) {
        Sky* pSky = new Sky("クッパ戦闘中の空", "KoopaBattleSky");
        TVec3f up;
        pBattle->mQuatB.getYDir(up);
        MR::setQuatUpNoSupport(pSky, up);
        pSky->initWithoutIter();
        pSky->makeActorDead();
        return pSky;
    }

    LiveActor* createKoopaLastBattleSky(KoopaBattle* pBattle) {
        Sky* pSky = new Sky("クッパ最終戦空", "KoopaLastBattleSky");
        TVec3f front;
        pBattle->mQuatB.getZDir(front);
        MR::setQuatUpNoSupport(pSky, front);
        pSky->initWithoutIter();
        pSky->makeActorDead();
        return pSky;
    }

    void calcKoopaNearestAttackRockPos(TVec3f* pDst) {
        KoopaBattleAccessor* pAccessor = getKoopaBattleAccessor();
        if (pAccessor == nullptr) {
            pDst->set< f32 >(*MR::getPlayerPos());
            return;
        }
        s32 num = pAccessor->mAttackRockNum;
        TVec3f nearest(*MR::getPlayerPos());
        f32 minDist = 3.4028235e38f;
        for (s32 i = 0; i < num; i++) {
            AttackRockFinal* pRock = pAccessor->getAttackRockFinal(i);
            if (pRock->mIsOnPlayer) {
                pDst->set< f32 >(pRock->mStartPos);
                return;
            }
            f32 dist = PSVECDistance(&pRock->mStartPos, MR::getPlayerPos());
            if (dist <= minDist) {
                nearest.set< f32 >(pRock->mStartPos);
                minDist = dist;
            }
        }
        pDst->set< f32 >(nearest);
    }

    void setPlayerToKoopaDemoChamberEnd() {
        TPos3f mtx;
        getKoopaBattleAccessor()->mDemoChamber->getDemoEndPlayerPos(mtx.toMtxPtr());
        MR::setPlayerBaseMtx(mtx.toMtxPtr());
        MR::resetPlayerStatus();
        MR::setPlayerStateWait();
    }

    void setKoopaBattleNo0() {
        setKoopaBattleNo(0);
    }

    void setKoopaBattleNo1() {
        setKoopaBattleNo(1);
    }

    void setKoopaBattleNo2() {
        setKoopaBattleNo(2);
    }

    void setKoopaBattleNoA() {
        setKoopaBattleNo(10);
    }

    void setKoopaBattleNoB() {
        setKoopaBattleNo(11);
    }

    void setKoopaBattleNoC() {
        setKoopaBattleNo(12);
    }

    s32 sendMsgToKoopaReceivers(u32 msg) {
        KoopaBattleAccessor* pAccessor = getKoopaBattleAccessor();
        if (pAccessor == nullptr) {
            return 0;
        }
        s32 count = 0;
        s32 num = pAccessor->mReceiverSensorNum;
        for (s32 i = 0; i < num; i++) {
            if (MR::sendArbitraryMsg(msg, pAccessor->getReceiverSensor(i), MR::getMessageSensor())) {
                count++;
            }
        }
        return count;
    }

    s32 sendMsgKoopaFinalBattleStart() {
        return sendMsgToKoopaReceivers(0xC4);
    }

    s32 sendMsgKoopaC6() {
        return sendMsgToKoopaReceivers(0xC6);
    }

    s32 sendMsgKoopaC8() {
        return sendMsgToKoopaReceivers(0xC8);
    }

    s32 sendMsgKoopaCastAttackRockEnd() {
        return sendMsgToKoopaReceivers(0xC3);
    }

    s32 sendMsgKoopaReleaseAttackRock() {
        return sendMsgToKoopaReceivers(0xC1);
    }

    s32 sendMsgKoopaBB() {
        return sendMsgToKoopaReceivers(0xBB);
    }

    bool trySendMsgToKoopaReceivers(u32 msg) {
        KoopaBattleAccessor* pAccessor = getKoopaBattleAccessor();
        if (pAccessor == nullptr) {
            return false;
        }
        s32 num = pAccessor->mReceiverSensorNum;
        for (s32 i = 0; i < num; i++) {
            if (MR::sendArbitraryMsg(msg, pAccessor->getReceiverSensor(i), MR::getMessageSensor())) {
                return true;
            }
        }
        return false;
    }

    bool trySendMsgKoopaCastAttackRockStart() {
        return trySendMsgToKoopaReceivers(0xC2);
    }

    bool sendMsgAttackRockReflect(HitSensor* pReceiver, HitSensor* pSender) {
        return MR::sendArbitraryMsg(0xC9, pReceiver, pSender);
    }

    bool sendMsgKoopaBD(HitSensor* pReceiver, HitSensor* pSender) {
        return MR::sendArbitraryMsg(0xBD, pReceiver, pSender);
    }

    bool sendMsgKoopaPunchAttack(HitSensor* pReceiver, HitSensor* pSender) {
        return MR::sendArbitraryMsg(0xC0, pReceiver, pSender);
    }

    bool sendMsgToBindedAttackRockReflect(LiveActor* pActor, HitSensor* pSender) {
        return MR::sendMsgToBindedSensor(0xC9, pActor, pSender);
    }

    bool sendMsgToBindedKoopaC7(LiveActor* pActor, HitSensor* pSender) {
        return MR::sendMsgToBindedSensor(0xC7, pActor, pSender);
    }

    bool isMsgAttackRockReflect(u32 msg) {
        return msg == 0xC9;
    }

    bool isMsgKoopaC7(u32 msg) {
        return msg == 0xC7;
    }

    bool isMsgKoopaPunchAttack(u32 msg) {
        return msg == 0xC0;
    }

    bool isMsgKoopaC8(u32 msg) {
        return msg == 0xC8;
    }

    bool isMsgKoopaBA(u32 msg) {
        return msg == 0xBA;
    }

    bool isMsgKoopaBB(u32 msg) {
        return msg == 0xBB;
    }

    bool isMsgKoopaFinalBattleStart(u32 msg) {
        return msg == 0xC4;
    }

    bool isMsgKoopaC6(u32 msg) {
        return msg == 0xC6;
    }

    bool isMsgKoopaCastAttackRockStart(u32 msg) {
        return msg == 0xC2;
    }

    bool isMsgKoopaCastAttackRockEnd(u32 msg) {
        return msg == 0xC3;
    }

    bool isMsgKoopaReleaseAttackRock(u32 msg) {
        return msg == 0xC1;
    }

    bool isKoopaHeadOrBodySensor(LiveActor* pActor, HitSensor* pSensor) {
        if (MR::isEqualSensor(pSensor, pActor, "Head") || MR::isEqualSensor(pSensor, pActor, "Body")) {
            return true;
        }
        return false;
    }

    bool isKoopaBody2OrLegSensor(LiveActor* pActor, HitSensor* pSensor) {
        if (MR::isEqualSensor(pSensor, pActor, "Body2") || MR::isEqualSensor(pSensor, pActor, "LegL1") ||
            MR::isEqualSensor(pSensor, pActor, "LegL2") || MR::isEqualSensor(pSensor, pActor, "LegR1") ||
            MR::isEqualSensor(pSensor, pActor, "LegR2")) {
            return true;
        }
        return false;
    }

    bool isKoopaArmLSensor(LiveActor* pActor, HitSensor* pSensor) {
        if (MR::isEqualSensor(pSensor, pActor, "ArmL1") || MR::isEqualSensor(pSensor, pActor, "ArmL2")) {
            return true;
        }
        return false;
    }

    bool isKoopaArmRSensor(LiveActor* pActor, HitSensor* pSensor) {
        if (MR::isEqualSensor(pSensor, pActor, "ArmR1") || MR::isEqualSensor(pSensor, pActor, "ArmR2") ||
            MR::isEqualSensor(pSensor, pActor, "HandR")) {
            return true;
        }
        return false;
    }
};  // namespace MR
