#pragma auto_inline off
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/HitSensorInfo.hpp"
#include "Game/LiveActor/HitSensorKeeper.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/LiveActor/MessageSensorHolder.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/Binder.hpp"

namespace MR {
    void setSensorFollowPos(LiveActor* pActor, const char* pSensorName, const TVec3f* pPos) {
        pActor->mSensorKeeper->getSensorInfo(pSensorName)->setFollowPos(pPos);
    }

    void setSensorFollowMtx(LiveActor* pActor, const char* pSensorName, MtxPtr mtx) {
        pActor->mSensorKeeper->getSensorInfo(pSensorName)->setFollowMtx(mtx);
    }

    HitSensor* addHitSensor(LiveActor* pActor, const char* pName, u32 type, u16 a4, f32 radius, const TVec3f& rOffset) {
        return pActor->mSensorKeeper->add(pName, type, a4, radius, pActor, rOffset)->mSensor;
    }

    HitSensor* addHitSensorBinder(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x5B, a4, radius, rOffset);
    }

    HitSensor* addHitSensorPriorBinder(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x5D, a4, radius, rOffset);
    }

    HitSensor* addHitSensorRide(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0xC, a4, radius, rOffset);
    }

    HitSensor* addHitSensorMapObj(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x40, a4, radius, rOffset);
    }

    HitSensor* addHitSensorMapObjSimple(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x41, a4, radius, rOffset);
    }

    HitSensor* addHitSensorMapObjMoveCollision(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x42, a4, radius, rOffset);
    }

    HitSensor* addHitSensorEnemy(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x18, a4, radius, rOffset);
    }

    HitSensor* addHitSensorEnemyAttack(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x1A, a4, radius, rOffset);
    }

    HitSensor* addHitSensorNpc(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x8, a4, radius, rOffset);
    }

    HitSensor* addHitSensorEye(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x7A, a4, radius, rOffset);
    }

    HitSensor* addHitSensorPush(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f& rOffset) {
        return MR::addHitSensor(pActor, pName, 0x7B, a4, radius, rOffset);
    }

    HitSensor* addHitSensorPosRide(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f* pFollowPos, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0xC, a4, radius, pActor, rOffset);
        info->setFollowPos(pFollowPos);
        return info->mSensor;
    }

    HitSensor* addHitSensorPosMapObj(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f* pFollowPos, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0x40, a4, radius, pActor, rOffset);
        info->setFollowPos(pFollowPos);
        return info->mSensor;
    }

    HitSensor* addHitSensorPosEnemyAttack(LiveActor* pActor, const char* pName, u16 a4, f32 radius, const TVec3f* pFollowPos, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0x1A, a4, radius, pActor, rOffset);
        info->setFollowPos(pFollowPos);
        return info->mSensor;
    }

    HitSensor* addHitSensorMtx(LiveActor* pActor, const char* pName, u32 type, u16 a4, f32 radius, MtxPtr mtx, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, type, a4, radius, pActor, rOffset);
        info->setFollowMtx(mtx);
        return info->mSensor;
    }

    HitSensor* addHitSensorMtxRide(LiveActor* pActor, const char* pName, u16 a4, f32 radius, MtxPtr mtx, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0xC, a4, radius, pActor, rOffset);
        info->setFollowMtx(mtx);
        return info->mSensor;
    }

    HitSensor* addHitSensorMtxEnemy(LiveActor* pActor, const char* pName, u16 a4, f32 radius, MtxPtr mtx, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0x18, a4, radius, pActor, rOffset);
        info->setFollowMtx(mtx);
        return info->mSensor;
    }

    HitSensor* addHitSensorMtxEnemyAttack(LiveActor* pActor, const char* pName, u16 a4, f32 radius, MtxPtr mtx, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0x1A, a4, radius, pActor, rOffset);
        info->setFollowMtx(mtx);
        return info->mSensor;
    }

    HitSensor* addHitSensorMtxAnimal(LiveActor* pActor, const char* pName, u16 a4, f32 radius, MtxPtr mtx, const TVec3f& rOffset) {
        HitSensorInfo* info = pActor->mSensorKeeper->add(pName, 0xA, a4, radius, pActor, rOffset);
        info->setFollowMtx(mtx);
        return info->mSensor;
    }

    HitSensor* addHitSensorAtJoint(LiveActor* pActor, const char* pName, const char* pJointName, u32 type, u16 a5, f32 radius,
                                   const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, type, a5, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorAtJointMapObj(LiveActor* pActor, const char* pName, const char* pJointName, u16 a4, f32 radius, const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, 0x40, a4, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorAtJointMapObjSimple(LiveActor* pActor, const char* pName, const char* pJointName, u16 a4, f32 radius,
                                               const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, 0x41, a4, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorAtJointEnemy(LiveActor* pActor, const char* pName, const char* pJointName, u16 a4, f32 radius, const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, 0x18, a4, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorAtJointEnemyAttack(LiveActor* pActor, const char* pName, const char* pJointName, u16 a4, f32 radius,
                                              const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, 0x1A, a4, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorAtJointNpc(LiveActor* pActor, const char* pName, const char* pJointName, u16 a4, f32 radius, const TVec3f& rOffset) {
        MtxPtr jointMtx = MR::getJointMtx(pActor, pJointName);
        return addHitSensorMtx(pActor, pName, 0x8, a4, radius, jointMtx, rOffset);
    }

    HitSensor* addHitSensorCallback(LiveActor* pActor, const char* pName, u32 type, u16 a4, f32 radius) {
        return pActor->mSensorKeeper->addCallback(pName, type, a4, radius, pActor)->mSensor;
    }

    HitSensor* addHitSensorCallbackBinder(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0x5B, a4, radius);
    }

    HitSensor* addHitSensorCallbackPriorBinder(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0x5D, a4, radius);
    }

    HitSensor* addHitSensorCallbackRide(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0xC, a4, radius);
    }

    HitSensor* addHitSensorCallbackMapObj(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0x40, a4, radius);
    }

    HitSensor* addHitSensorCallbackEnemyAttack(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0x1A, a4, radius);
    }

    HitSensor* addHitSensorCallbackEye(LiveActor* pActor, const char* pName, u16 a4, f32 radius) {
        return MR::addHitSensorCallback(pActor, pName, 0x7A, a4, radius);
    }

#pragma dont_inline on
    HitSensor* addBodyMessageSensorReceiver(LiveActor* pActor) {
        return MR::addBodyMessageSensor(pActor, 0x7F);
    }

    HitSensor* addBodyMessageSensorMapObj(LiveActor* pActor) {
        return MR::addBodyMessageSensor(pActor, 0x40);
    }

    HitSensor* addBodyMessageSensorMapObjPress(LiveActor* pActor) {
        return MR::addBodyMessageSensor(pActor, 0x71);
    }

    HitSensor* addBodyMessageSensorEnemy(LiveActor* pActor) {
        return MR::addBodyMessageSensor(pActor, 0x18);
    }

    NO_INLINE HitSensor* addBodyMessageSensor(LiveActor* pActor, u32 type) {
        TVec3f offs(0.0f, 0.0f, 0.0f);
        return pActor->mSensorKeeper->add("Body", type, 0, 0.0f, pActor, offs)->mSensor;
    }

#pragma dont_inline reset

    HitSensor* addMessageSensorReceiver(LiveActor* pActor, const char* pName) {
        TVec3f offs(0.0f, 0.0f, 0.0f);
        return pActor->mSensorKeeper->add(pName, 0x7F, 0, 0.0f, pActor, offs)->mSensor;
    }

    HitSensor* addHitSensorMapObj(LiveActor* pActor, const char* pName) {
        TVec3f offs(0.0f, 0.0f, 0.0f);
        return pActor->mSensorKeeper->add(pName, 0x40, 0, 0.0f, pActor, offs)->mSensor;
    }

    bool tryUpdateHitSensorsAll(LiveActor* pActor) {
        if (pActor->mSensorKeeper != nullptr) {
            pActor->mSensorKeeper->update();
            return true;
        }

        return false;
    }

    void updateHitSensorsAll(LiveActor* pActor) {
        pActor->mSensorKeeper->update();
    }

    bool isSensorType(const HitSensor* pSensor, u32 type) {
        return pSensor->isType(type);
    }

    void setSensorType(LiveActor* pActor, const char* pName, u32 type) {
        pActor->getSensor(pName)->setType(type);
    }

    bool isSensorValid(const HitSensor* pSensor) {
        bool valid = false;
        if (pSensor->mValidByHost && pSensor->mValidBySystem) {
            valid = true;
        }

        return valid;
    }

    HitSensor* getSensorWithIndex(LiveActor* pActor, int idx) {
        return pActor->mSensorKeeper->getNthSensorInfo(idx)->mSensor;
    }

    void setSensorPos(HitSensor* pSensor, const TVec3f& rPos) {
        pSensor->mPosition.set(rPos);
    }

    void setSensorPos(LiveActor* pActor, const char* pName, const TVec3f& rPos) {
        setSensorPos(pActor->getSensor(pName), rPos);
    }

    void setSensorOffset(LiveActor* pActor, const char* pName, const TVec3f& rPos) {
        pActor->mSensorKeeper->getSensorInfo(pName)->_C = rPos;
    }

    f32 getSensorRadius(HitSensor* pSensor) {
        return pSensor->mRadius;
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    f32 getSensorRadius(LiveActor* pActor, const char* pName) {
        return getSensorRadius(pActor->mSensorKeeper->getSensorInfo(pName)->mSensor);
    }

    void scaleAllSensorRadius(LiveActor* pActor, f32 radius) {
        s32 sensorNum = MR::getSensorNum(pActor);

        for (s32 i = 0; i < sensorNum; i++) {
            HitSensorInfo* inf = pActor->mSensorKeeper->getNthSensorInfo(i);
            inf->mSensor->mRadius = radius * inf->mSensor->mRadius;
        }
    }

    TVec3f& getSensorHostPos(const HitSensor* pSensor) {
        return pSensor->mHost->mPosition;
    }

    TVec3f& getSensorHostGravity(const HitSensor* pSensor) {
        return pSensor->mHost->mGravity;
    }

    void validateHitSensors(LiveActor* pActor) {
        if (pActor->mSensorKeeper != nullptr) {
            pActor->mSensorKeeper->validate();
        }
    }

    void invalidateHitSensors(LiveActor* pActor) {
        if (pActor->mSensorKeeper != nullptr) {
            pActor->mSensorKeeper->invalidate();
        }
    }

    void validateHitSensor(HitSensor* pSensor) {
        pSensor->validate();
    }

    void invalidateHitSensor(HitSensor* pSensor) {
        pSensor->invalidate();
    }

    void validateHitSensor(LiveActor* pActor, const char* pName) {
        pActor->getSensor(pName)->validate();
    }

    void invalidateHitSensor(LiveActor* pActor, const char* pName) {
#ifdef GALAXY_NATIVE
        // The game calls this for sensors an actor does not have (Meramera::endRunaway invalidates "break", but
        // Meramera.arc only defines "Body" and "attack"). On the Wii HitSensor::invalidate() then runs on a null
        // `this`, which reads and clears bytes of low memory without effect; on the host it faults. No sensor
        // means nothing to invalidate.
        HitSensor* pSensor = pActor->getSensor(pName);
        if (pSensor == nullptr) {
            return;
        }
        pSensor->invalidate();
#else
        pActor->getSensor(pName)->invalidate();
#endif
    }

    bool tryValidateHitSensor(LiveActor* pActor, const char* pName) {
        HitSensor* sensor = pActor->mSensorKeeper->getSensor(pName);
        if (!sensor->mValidByHost) {
            pActor->getSensor(pName)->validate();
            return true;
        } else {
            return false;
        }
    }

    bool tryInvalidateHitSensor(LiveActor* pActor, const char* pName) {
        HitSensor* sensor = pActor->mSensorKeeper->getSensor(pName);
        if (sensor->mValidByHost) {
            pActor->getSensor(pName)->validate();
            return true;
        } else {
            return false;
        }
    }

    void clearHitSensors(LiveActor* pActor) {
        if (pActor->mSensorKeeper != nullptr) {
            pActor->mSensorKeeper->clear();
        }
    }

    s32 getSensorNum(const LiveActor* pActor) {
        if (pActor->mSensorKeeper == nullptr) {
            return 0;
        }

        return pActor->mSensorKeeper->mSensorCount;
    }

    HitSensor* getSensor(LiveActor* pActor, int idx) {
        return pActor->mSensorKeeper->getNthSensorInfo(idx)->mSensor;
    }

    LiveActor* getSensorHost(const HitSensor* pSensor) {
        return pSensor->mHost;
    }

    bool isEqualSensor(const HitSensor* pSensor, LiveActor* pActor, const char* pName) {
        return pSensor == pActor->getSensor(pName);
    }

    bool isSensorPlayer(const HitSensor* pSensor) {
        return pSensor->isType(1);
    }

    bool isSensorYoshi(const HitSensor* pSensor) {
        return pSensor->isType(5);
    }

    bool isSensorBinder(const HitSensor* pSensor) {
        return pSensor->isType(0x5B);
    }

    bool isSensorReceiver(const HitSensor* pSensor) {
        return pSensor->isType(0x7F);
    }

    // NONMATCHING: equivalent range-check lowering
    bool isSensorRide(const HitSensor* pSensor) {
        if (pSensor->mType <= 0xB) {
            return false;
        }
        if (pSensor->mType >= 0x16) {
            return false;
        }
        return true;
    }

    /* ... */

    bool isSensorEnemyAttack(const HitSensor* pSensor) {
        return pSensor->isType(0x1A);
    }

    bool isSensorNpc(const HitSensor* pSensor) {
        return pSensor->mType == 8;
    }

    // MR::isSensorMapObj
    // MR::isSensorAutoRush
    // MR::isSensorRush

    bool isSensorPressObj(const HitSensor* pSensor) {
        return pSensor->mType == 0x71;
    }

    bool isSensorEye(const HitSensor* pSensor) {
        return pSensor->isType(0x7A);
    }

    bool isSensorPush(const HitSensor* pSensor) {
        return pSensor->isType(0x7B);
    }

    bool isSensorItem(const HitSensor* pSensor) {
        if (pSensor->isType(0x44) || pSensor->isType(0x45) || pSensor->isType(0x47)) {
            return true;
        } else {
            return false;
        }
    }

    bool isSensorGhost(const HitSensor* pSensor) {
        return pSensor->isType(0x15);
    }

    bool tryGetItem(HitSensor* a1, HitSensor* a2) {
        return a2->receiveMessage(0x83, a1);
    }

    const TVec3f* getSensorPos(const HitSensor* pSensor) {
        return &pSensor->mPosition;
    }

    const TVec3f* getSensorPos(const LiveActor* pActor, const char* pName) {
        return &pActor->getSensor(pName)->mPosition;
    }

    /* ... */

    bool sendArbitraryMsg(u32 msg, HitSensor* a2, HitSensor* a3) {
        return a2->receiveMessage(msg, a3);
    }

    bool sendMsgPush(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x33, a2);
    }

    bool sendMsgReflect(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x48, a2);
    }

    bool sendMsgReflectV(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x49, a2);
    }

    bool sendMsgPlayerTrample(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(2, a2);
    }

    bool sendMsgPlayerPunch(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(1, a2);
    }

    bool sendMsgInvincibleAttack(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0xE, a2);
    }

    bool sendMsgDrillAttack(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x10, a2);
    }

    bool sendMsgRockAttack(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x11, a2);
    }

    bool sendMsgJump(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x36, a2);
    }

    bool sendMsgTouchJump(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x37, a2);
    }

    bool sendMsgTaken(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x2A, a2);
    }

    bool sendMsgAwayJump(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x39, a2);
    }

    bool sendMsgRushEnd(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x98, a2);
    }

    bool sendMsgIsRushEnable(HitSensor* a1, HitSensor* a2) {
        return a1->receiveMessage(0x99, a2);
    }

    HitSensor* getTaken(const LiveActor* pActor) {
        if (pActor->mSensorKeeper != nullptr) {
            return reinterpret_cast< HitSensor* >(pActor->mSensorKeeper->_10);
        }

        return nullptr;
    }

    void setSensorRadius(HitSensor* pSensor, f32 radius) {
        pSensor->mRadius = radius;
    }

    void setSensorRadius(LiveActor* pActor, const char* pName, f32 radius) {
        pActor->mSensorKeeper->getSensorInfo(pName)->mSensor->mRadius = radius;
    }

    bool isSensorPlayerOrRide(const HitSensor* pSensor) {
        return isSensorPlayer(pSensor) || isSensorRide(pSensor);
    }

    bool isSensorPlayerOrYoshi(const HitSensor* pSensor) {
        bool result = true;
        if (pSensor->isType(1) == 0) {
            if (pSensor->isType(5) == 0) {
                result = false;
            }
        }

        return result;
    }

    bool isSensorPlayerOrYoshiOrRide(const HitSensor* pSensor) {
        bool result = pSensor->isType(1);
        if (!result) {
            result = pSensor->isType(5);
        }

        if (!result) {
            result = isSensorRide(pSensor);
        }

        return result;
    }

    // NONMATCHING: equivalent range-check lowering
    bool isSensorEnemy(const HitSensor* pSensor) {
        if (pSensor->mType <= 0x17 /* ATYPE_ENEMY_START */) {
            return false;
        }

        if (pSensor->mType < 0x3e /* ATYPE_ENEMY_END */) {
            return true;
        }

        return false;
    }

    // NONMATCHING: equivalent range-check lowering
    bool isSensorMapObj(const HitSensor* pSensor) {
        if (pSensor->mType <= 0x3f /* ATYPE_MAPOBJ_START */) {
            return false;
        }

        if (pSensor->mType < 0x59 /* ATYPE_MAPOBJ_END */) {
            return true;
        }

        return false;
    }

    // NONMATCHING: equivalent range-check lowering
    bool isSensorAutoRush(const HitSensor* pSensor) {
        if (pSensor->mType <= 0x5a /* ATYPE_AUTO_RUSH_OBJ_START */) {
            return false;
        }

        if (pSensor->mType < 0x69 /* ATYPE_AUTO_RUSH_OBJ_END */) {
            return true;
        }

        return false;
    }

    // NONMATCHING: equivalent range-check lowering
    bool isSensorRush(const HitSensor* pSensor) {
        if (pSensor->mType <= 0x6a /* ATYPE_RUSH_OBJ_START */) {
            return false;
        }

        if (pSensor->mType < 0x6f /* ATYPE_RUSH_OBJ_END */) {
            return true;
        }

        return false;
    }

    TVec3f* getSensorPos2(const HitSensor* pSensor) {
        return const_cast< TVec3f* >(&pSensor->mPosition);
    }

    TVec3f* getSensorPos2(const LiveActor* pActor, const char* pSensorName) {
        return const_cast< TVec3f* >(&pActor->getSensor(pSensorName)->mPosition);
    }

    void calcSensorDirection(TVec3f* pDir, const HitSensor* pSensor1, const HitSensor* pSensor2) {
        TVec3f dir = pSensor2->mPosition - pSensor1->mPosition;
        pDir->set(dir);
    }

    void calcSensorDirectionNormalize(TVec3f* pDir, const HitSensor* pSensor1, const HitSensor* pSensor2) {
        calcSensorDirection(pDir, pSensor1, pSensor2);
        normalizeOrZero(pDir);
    }

    void calcSensorDistanceGravity(f32* pDistance, f32* pDot, const HitSensor* pSensorA, const HitSensor* pSensorB, TVec3f& rGravity) {
        func_8003DEF0(pDistance, pDot, &pSensorA->mPosition, &pSensorB->mPosition, &rGravity);
    }

    f32 calcSensorDistance(const HitSensor* pSensorA, const HitSensor* pSensorB) {
        return pSensorA->mPosition.distance(pSensorB->mPosition);
    }

    void calcSensorHorizonNormalize(TVec3f* pHorizon, const TVec3f& rGravity, const HitSensor* pSensor1, const HitSensor* pSensor2) {
        TVec3f horizon = pSensor2->mPosition - pSensor1->mPosition;
        pHorizon->killElement(horizon, rGravity);
        normalizeOrZero(pHorizon);
    }

    HitSensor* getMessageSensor() {
        return getSceneObj< MessageSensorHolder >(SceneObj_MessageSensorHolder)->getSensor(nullptr);
    }

    bool sendMsgEnemyAttackMsgToDir(u32 msg, HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        TVec3f senderPos = pSender->mPosition;
        pSender->mPosition.sub(pReceiver->mPosition, rDir);

        bool isSent = pReceiver->receiveMessage(msg, pSender);
        setSensorPos(pSender, senderPos);

        return isSent;
    }

    bool sendMsgEnemyAttackFlipWeak(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x5b /* ACTMES_ENEMY_ATTACK_FLIP_WEAK */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFlipWeakJump(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x5c /* ACTMES_ENEMY_ATTACK_FLIP_WEAK_JUMP */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFlip(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x5e /* ACTMES_ENEMY_ATTACK_FLIP */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFlipToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(0x5e /* ACTMES_ENEMY_ATTACK_FLIP */, pReceiver, pSender, rDir);
    }

    bool sendMsgEnemyAttackFlipJump(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x5d /* ACTMES_ENEMY_ATTACK_FLIP_JUMP */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFlipRot(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x5f /* ACTMES_ENEMY_ATTACK_FLIP_ROT */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFlipMaximumToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(0x60 /* ACTMES_ENEMY_ATTACK_FLIP_MAXIMUM */, pReceiver, pSender, rDir);
    }

    bool sendMsgEnemyAttack(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x61 /* ACTMES_ENEMY_ATTACK */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackOrPush(HitSensor* pReceiver, HitSensor* pSender) {
        if (pReceiver->receiveMessage(0x61 /* ACTMES_ENEMY_ATTACK */, pSender)) {
            return true;
        }

        pReceiver->receiveMessage(0x33, pSender);
        return false;
    }

    bool sendMsgEnemyAttackStrong(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x62 /* ACTMES_ENEMY_ATTACK_STRONG */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackStrongOrPush(HitSensor* pReceiver, HitSensor* pSender) {
        if (pReceiver->receiveMessage(0x62 /* ACTMES_ENEMY_ATTACK_STRONG */, pSender)) {
            return true;
        }

        pReceiver->receiveMessage(0x33, pSender);
        return false;
    }

    bool sendMsgEnemyAttackStrongToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(0x62 /* ACTMES_ENEMY_ATTACK_STRONG */, pReceiver, pSender, rDir);
    }

    bool sendMsgEnemyAttackFire(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x66 /* ACTMES_ENEMY_ATTACK_FIRERUN */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFireStrong(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x67 /* ACTMES_ENEMY_ATTACK_FIRERUN_STRONG */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackElectric(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x68 /* ACTMES_ENEMY_ATTACK_ELECTRIC */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackFreeze(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x6b /* ACTMES_ENEMY_ATTACK_FREEZE */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackExplosion(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x64 /* ACTMES_ENEMY_ATTACK_EXPLOSION */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackCounterSpin(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x6e /* ACTMES_ENEMY_ATTACK_COUNTER_SPIN */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackCounterHipDrop(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x6f /* ACTMES_ENEMY_ATTACK_COUNTER_HIPDROP */, pReceiver, pSender);
    }

    bool sendMsgLockOnStarPieceShoot(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x15 /* ACTMES_IS_LOCKON_STAR_PIECE_SHOOT */, pReceiver, pSender);
    }

    bool sendMsgStarPieceAttack(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0xf /* ACTMES_STAR_PIECE_ATTACK */, pReceiver, pSender);
    }

    bool sendMsgStarPieceGift(HitSensor* pReceiver, HitSensor* pSender, u32 num) {
        return sendArbitraryMsg((num + 0x16 /* ACTMES_STAR_PIECE_GIFT */) - 1, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackMaximum(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x63 /* ACTMES_ENEMY_ATTACK_MAXIMUM */, pReceiver, pSender);
    }

    bool sendMsgEnemyAttackMaximumToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(0x63 /* ACTMES_ENEMY_ATTACK_MAXIMUM */, pReceiver, pSender, rDir);
    }

    bool sendMsgEnemyAttackToBindedSensor(LiveActor* pActor, HitSensor* pSender) {
        return sendMsgToBindedSensor(0x61 /* ACTMES_ENEMY_ATTACK */, pActor, pSender);
    }

    bool sendMsgEnemyAttackExplosionToBindedSensor(LiveActor* pActor, HitSensor* pSender) {
        return sendMsgToBindedSensor(0x64 /* ACTMES_ENEMY_ATTACK_EXPLOSION */, pActor, pSender);
    }

    bool sendSimpleMsgToActor(u32 msg, LiveActor* pActor) {
        HitSensor* sender = getMessageSensor();
        HitSensor* receiver = getDefaultMessageOrActorSensor(pActor);
        return pActor->receiveMessage(msg, sender, receiver);
    }

    bool sendMsgToGroundSensor(u32 msg, HitSensor* pSender) {
        if (!isBindedGround(getSensorHost(pSender))) {
            return false;
        }

        return getGroundSensor(getSensorHost(pSender))->receiveMessage(msg, pSender);
    }

    bool sendMsgToWallSensor(u32 msg, HitSensor* pSender) {
        if (!isBindedWall(getSensorHost(pSender))) {
            return false;
        }

        return getWallSensor(getSensorHost(pSender))->receiveMessage(msg, pSender);
    }

#pragma dont_inline on
    bool sendMsgToRoofSensor(u32 msg, HitSensor* pSender) {
        if (isBindedRoof(pSender->mHost)) {
            HitSensor* pRoof = getRoofSensor(pSender->mHost);
            return pRoof->receiveMessage(msg, pSender);
        }

        return false;
    }
#pragma dont_inline reset

    bool sendMsgStartDemo(LiveActor* pActor) {
        return sendSimpleMsgToActor(0x7F /* ACTMES_START_DEMO */, pActor);
    }

    bool sendMsgToEnemyAttackBlow(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x70 /* ACTMES_TO_ENEMY_ATTACK_BLOW */, pReceiver, pSender);
    }

    bool sendMsgToEnemyAttackTrample(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x72 /* ACTMES_TO_ENEMY_ATTACK_TRAMPLE */, pReceiver, pSender);
    }

    bool sendMsgToEnemyAttackBlowOrTrample(HitSensor* pReceiver, HitSensor* pSender, f32 ratio) {
        TVec3f dir = pReceiver->mPosition - pSender->mPosition;
        normalizeOrZero(&dir);

        if (ratio < getSensorHost(pSender)->mGravity.dot(dir)) {
            return sendMsgToEnemyAttackTrample(pReceiver, pSender);
        } else {
            return sendMsgToEnemyAttackBlow(pReceiver, pSender);
        }
    }

    bool sendMsgToEnemyAttackShockWave(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(0x74 /* ACTMES_TO_ENEMY_ATTACK_SHOCK_WAVE */, pReceiver, pSender);
    }

    void sendMsgToAllLiveActor(u32 msg, LiveActor* pActor) {
        AllLiveActorGroup* pGroup = getAllLiveActorGroup();

        for (int i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pGroupActor = pGroup->getActor(i);

            if (isDead(pGroupActor)) {
                continue;
            }

            if (pGroupActor == pActor) {
                continue;
            }

            HitSensor* sender = getMessageSensor();
            HitSensor* receiver = getDefaultMessageOrActorSensor(pGroupActor);
            pGroupActor->receiveMessage(msg, sender, receiver);
        }
    }

    void sendMsgToGroupMember(u32 msg, LiveActor* pActor, HitSensor* pSender, const char* pName) {
        MsgSharedGroup* pGroup = static_cast< MsgSharedGroup* >(getGroupFromArray(pActor));

        if (pGroup != nullptr) {
            pGroup->sendMsgToGroupMember(msg, pSender, pName);
        } else {
            pActor->receiveMessage(msg, pSender, pActor->getSensor(pName));
        }
    }

    void sendMsgExplosionToNearActor(HitSensor* pSender, f32 distance) {
        HitSensor* pReceiver;

        for (u16 i = 0; i < pSender->mSensorCount; i++) {
            pReceiver = pSender->mSensors[i];

            if (distance < 0.0f || isNear(pSender, pReceiver, distance)) {
                sendArbitraryMsg(0x64 /* ACTMES_ENEMY_ATTACK_EXPLOSION */, pReceiver, pSender);
            }
        }
    }

    bool isInSpinStormRange(u32 msg, HitSensor* pSender, HitSensor* pReceiver, f32 range) {
        if (msg != 0x3c /* ACTMES_SPIN_STORM_RANGE */) {
            return false;
        }

        // NOTE: Double negative?
        bool isFar = !isNear(pSender, pReceiver, range);

        return !isFar;
    }

    bool receiveItemShowMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        if (msg == 0x86 /* ACTMES_ITEM_SHOW */ && isDead(getSensorHost(pReceiver))) {
            getSensorHost(pReceiver)->makeActorAppeared();

            return true;
        }

        return false;
    }

    bool receiveItemHideMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        if (msg == 0x87 /* ACTMES_ITEM_HIDE */ && !isDead(getSensorHost(pReceiver))) {
            getSensorHost(pReceiver)->makeActorDead();

            return true;
        }

        return false;
    }

#pragma dont_inline on
    HitSensor* getGroundSensor(const LiveActor* pActor) NO_INLINE {
        return pActor->mBinder->mGroundInfo.mParentTriangle.mSensor;
    }

#pragma dont_inline reset
    HitSensor* getRoofSensor(const LiveActor* pActor) {
        return pActor->mBinder->mRoofInfo.mParentTriangle.mSensor;
    }

#pragma dont_inline on
    HitSensor* getWallSensor(const LiveActor* pActor) NO_INLINE {
        return pActor->mBinder->mWallInfo.mParentTriangle.mSensor;
    }

#pragma dont_inline reset
    LiveActor* getGroundActor(const LiveActor* pActor) {
        return pActor->mBinder->mGroundInfo.mParentTriangle.getSensor()->mHost;
    }

    // NONMATCHING: equivalent bitset/branch lowering
    bool isMsgPlayerHitAll(u32 msg) {
        u32 index = msg - 1;
        if (index <= 18 && ((1u << index) & 0x5B5D1) != 0) return true;
        return false;
    }

    bool isMsgPlayerHitProjectiles(u32 msg) {
        bool result = false;
        u32 idx = msg - 7;
        if (idx <= 10) {
            if (((1 << idx) & 0x697) != 0) {
                result = true;
            }
        }

        return result;
    }

    bool isMsgPlayerSpinAttack(u32 msg) {
        return msg == 0x1 /* ACTMES_PLAYER_PUNCH */;
    }

    bool isMsgPlayerSpinAttackOrSupportTico(u32 msg) {
        bool result = false;
        if (msg == 0x1 || msg == 0x13) {
            result = true;
        }

        return result;
    }

    bool isMsgPlayerTrample(u32 msg) {
        return msg == 0x2 /* ACTMES_PLAYER_TRAMPLE */;
    }

    bool isMsgPlayerHipDrop(u32 msg) {
        return msg == 0x3 /* ACTMES_PLAYER_HIP_DROP */;
    }

    bool isMsgPlayerHipDropFloor(u32 msg) {
        return msg == 0x4 /* ACTMES_PLAYER_HIP_DROP_FLOOR */;
    }

    bool isMsgPlayerUpperPunch(u32 msg) {
        return msg == 0x5 /* ACTMES_PLAYER_UPPER_PUNCH */;
    }

    bool isMsgPlayerKick(u32 msg) {
        return msg == 0x35 /* ACTMES_KICK */;
    }

    bool isMsgJetTurtleAttack(u32 msg) {
        return msg == 0x7 /* ACTMES_JET_TURTLE_ATTACK */;
    }

    bool isMsgFireBallAttack(u32 msg) {
        return msg == 0xb /* ACTMES_FIREBALL_ATTACK */;
    }

    bool isMsgSearchlightAttack(u32 msg) {
        return msg == 0xc /* ACTMES_SEARCHLIGHT_ATTACK */;
    }

    bool isMsgFreezeAttack(u32 msg) {
        return msg == 0xd /* ACTMES_FREEZE_ATTACK */;
    }

    bool isMsgInvincibleAttack(u32 msg) {
        return msg == 0xe /* ACTMES_INVINCIBLE_ATTACK */;
    }

    bool isMsgAutoRushBegin(u32 msg) {
        return msg == 0x93 /* ACTMES_AUTORUSH_BEGIN */;
    }

    bool isMsgRushBegin(u32 msg) {
        return msg == 0x92 /* ACTMES_RUSH_BEGIN */;
    }

    bool isMsgUpdateBaseMtx(u32 msg) {
        return msg == 0xa1 /* ACTMES_UPDATE_BASEMTX */;
    }

    bool isMsgRushCancel(u32 msg) {
        return msg == 0x95 /* ACTMES_RUSH_CANCEL */;
    }

    bool isMsgIsRushTakeOver(u32 msg) {
        return msg == 0x9a /* ACTMES_IS_RUSH_TAKEOVER */;
    }

    bool isMsgFloorTouch(u32 msg) {
        return msg == 0xac /* ACTMES_FLOOR_TOUCH */;
    }

    bool isMsgWallTouch(u32 msg) {
        return msg - 0xAD <= 2;
    }

    bool isMsgCeilTouch(u32 msg) {
        return msg == 0xb0 /* ACTMES_CEIL_TOUCH */;
    }

    bool isMsgItemGet(u32 msg) {
        return msg == 0x83 /* ACTMES_ITEM_GET */;
    }

    bool isMsgItemPull(u32 msg) {
        return msg == 0x85 /* ACTMES_ITEM_PULL */;
    }

    bool isMsgItemShow(u32 msg) {
        return msg == 0x86 /* ACTMES_ITEM_SHOW */;
    }

    bool isMsgItemHide(u32 msg) {
        return msg == 0x87 /* ACTMES_ITEM_HIDE */;
    }

    bool isMsgItemStartMove(u32 msg) {
        return msg == 0x8b /* ACTMES_ITEM_START_MOVE */;
    }

    bool isMsgItemEndMoveOrDrop(u32 msg) {
        return (msg - 0x8c) <= 2;
    }

    bool isMsgItemEndMove(u32 msg) {
        return msg == 0x8c /* ACTMES_ITEM_END_MOVE */;
    }

    bool isMsgInhaleBlackHole(u32 msg) {
        return msg == 0x81 /* ACTMES_INHALE_BLACK_HOLE */;
    }

    bool isMsgEnemyAttack(u32 msg) {
        return msg == 0x61 /* ACTMES_ENEMY_ATTACK */;
    }

    bool isMsgEnemyAttackFire(u32 msg) {
        return msg == 0x66 /* ACTMES_ENEMY_ATTACK_FIRERUN */;
    }

    bool isMsgEnemyAttackFireStrong(u32 msg) {
        return msg == 0x67 /* ACTMES_ENEMY_ATTACK_FIRERUN_STRONG */;
    }

    bool isMsgEnemyAttackElectric(u32 msg) {
        return msg == 0x68 /* ACTMES_ENEMY_ATTACK_ELECTRIC */;
    }

    bool isMsgExplosionAttack(u32 msg) {
        return msg == 0x64 /* ACTMES_ENEMY_ATTACK_EXPLOSION */;
    }

    bool isMsgToEnemyAttackBlow(u32 msg) {
        return msg == 0x70 /* ACTMES_TO_ENEMY_ATTACK_BLOW */;
    }

    bool isMsgToEnemyAttackTrample(u32 msg) {
        return msg == 0x72 || msg == 0x73;
    }

    bool isMsgToEnemyAttackShockWave(u32 msg) {
        return msg == 0x74 /* ACTMES_TO_ENEMY_ATTACK_SHOCK_WAVE */;
    }

    bool isMsgSpinStormRange(u32 msg) {
        return msg == 0x3c /* ACTMES_SPIN_STORM_RANGE */;
    }

    bool isMsgRaceReady(u32 msg) {
        return msg == 0xee /* ACTMES_RACE_READY */;
    }

    bool isMsgRaceStart(u32 msg) {
        return msg == 0xef /* ACTMES_RACE_START */;
    }

    bool isMsgRaceReset(u32 msg) {
        return msg == 0xf1 /* ACTMES_RACE_RESET */;
    }

    bool isMsgLockOnStarPieceShoot(u32 msg) {
        return msg == 0x15 /* ACTMES_IS_LOCKON_STAR_PIECE_SHOOT */;
    }

    bool isMsgBallDashWall(u32 msg) {
        return msg == 0x41 /* ACTMES_BALL_DASH_WALL */;
    }

    bool isMsgBallDashGround(u32 msg) {
        return msg == 0x42 /* ACTMES_BALL_DASH_GROUND */;
    }

    bool isMsgStartPowerStarGet(u32 msg) {
        return msg == 0x82 /* ACTMES_START_POWER_STAR_GET */;
    }

    bool isMsgTouchPlantItem(u32 msg) {
        return msg == 0xf5 /* ACTMES_PLANT_GROUP_EMIT_ITEM */;
    }

    bool isMsgHitmarkEmit(u32 msg) {
        return msg == 0x27 /* ACTMES_HITMARK_EMIT */;
    }

    bool isMsgStarPieceAttack(u32 msg) {
        return msg == 0xf /* ACTMES_STAR_PIECE_ATTACK */;
    }

    bool isMsgStarPieceReflect(u32 msg) {
        return msg == 0x14 /* ACTMES_IS_STAR_PIECE_REFLECT */;
    }

    // NONMATCHING: equivalent range-check lowering
    bool isMsgStarPieceGift(u32 msg) {
        return msg >= 0x16 /* ACTMES_STAR_PIECE_GIFT */ && msg <= 0x25 /* ACTMES_STAR_PIECE_GIFT_MAX */;
    }

    s32 getStarPieceGiftCount(u32 msg) {
        return msg - 0x15;
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    void calcPosBetweenSensors(TVec3f* pPos, const HitSensor* pSensor1, const HitSensor* pSensor2, f32 offset) {
        TVec3f dir = pSensor2->mPosition - pSensor1->mPosition;
        normalizeOrZero(&dir);

        f32 dist = pSensor1->mPosition.distance(pSensor2->mPosition);
        f32 radius2 = pSensor2->mRadius;
        f32 radius1 = pSensor1->mRadius;
        f32 value = (radius1 + radius2 - dist) / 2.0f;

        pPos->set(dir);
        *pPos *= value + pSensor1->mRadius + offset;
        pPos->add(pSensor1->mPosition);
    }

    bool tryForceKillIfMsgStartPowerStarGet(LiveActor* pActor, u32 msg) {
        if (msg == 0x82 /* ACTMES_START_POWER_STAR_GET */) {
            if (isExistEffectKeeper(pActor)) {
                forceDeleteEffectAll(pActor);
            }

            pActor->makeActorDead();

            return true;
        }

        return false;
    }

    bool sendMsgToBindedSensor(u32 msg, LiveActor* pActor, HitSensor* pSender) {
        HitInfo* info[32];
        u32 size = pActor->mBinder->copyPlaneArrayAndSortingSensor(info, ARRAY_SIZE(info));

        if (size == 0) {
            return false;
        }

        bool isSent = false;
        HitSensor* pReceiver = info[0]->mParentTriangle.mSensor;
        isSent |= pReceiver->receiveMessage(msg, pSender);

        for (int i = 1; i < size; i++) {
            if (info[i]->mParentTriangle.mSensor == pReceiver) {
                continue;
            }

            pReceiver = info[i]->mParentTriangle.mSensor;
            isSent |= pReceiver->receiveMessage(msg, pSender);
        }

        return isSent;
    }

    bool sendMsgToBindedSensor(u32 msg, HitSensor* pSender) {
        return sendMsgToBindedSensor(msg, getSensorHost(pSender), pSender);
    }
#pragma dont_inline on
    bool sendMsgGorogoroReflect(HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(0x58, pSender);
    }

    bool sendMsgFireDance(HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(0x57, pSender);
    }

    bool sendMsgEnemyAttackFlipVeryWeak(HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(0x5a, pSender);
    }

    bool sendMsgEnemyAttackRollToBindedSensor(LiveActor* pActor, HitSensor* pSender) {
        return sendMsgToBindedSensor(0x71, pActor, pSender);
    }

    bool sendMsgEnemyAttackTrampleDeathToGroundSensor(HitSensor* pSender) {
        return sendMsgToGroundSensor(0x73, pSender);
    }

    HitSensor* getDefaultMessageOrActorSensor(LiveActor* pActor) {
        if (pActor->mSensorKeeper == nullptr) {
            return static_cast< LiveActor* >(getSceneObjHolder()->getObj(9))->getSensor(nullptr);
        }

        return pActor->mSensorKeeper->getNthSensorInfo(0)->mSensor;
    }

    bool sendMsgToEnemyAttackRoll(HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(0x71, pSender);
    }

    bool sendMsgToEnemyAttackBlowToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(0x70, pReceiver, pSender, rDir);
    }

    bool sendMsgToEnemyAttackTrampleDeath(HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(0x73, pSender);
    }

    void sendMsgGroupMoveStartToMember(LiveActor* pActor, HitSensor* pSender, const char* pName) {
        sendMsgToGroupMember(0x7a, pActor, pSender, pName);
    }

    void sendMsgGroupMoveStopToMember(LiveActor* pActor, HitSensor* pSender, const char* pName) {
        sendMsgToGroupMember(0x7b, pActor, pSender, pName);
    }

    void sendMsgGroupShowToMember(LiveActor* pActor, HitSensor* pSender, const char* pName) {
        sendMsgToGroupMember(0x7d, pActor, pSender, pName);
    }

    void sendMsgGroupHideToMember(LiveActor* pActor, HitSensor* pSender, const char* pName) {
        sendMsgToGroupMember(0x7e, pActor, pSender, pName);
    }

    bool isMsgPlayerJump(u32 msg) {
        return msg == 0x6;
    }

    bool isMsgTaken(u32 msg) {
        return msg == 0x2a;
    }

    bool isMsgThrow(u32 msg) {
        return msg == 0x2b;
    }

    bool isMsgDamageOrRushDrop(u32 msg) {
        return (msg - 0x2c) <= 2;
    }

    bool isMsgWaterDrop(u32 msg) {
        return msg == 0x2f;
    }

    bool isMsgYoshiProjectile(u32 msg) {
        return msg == 0x8;
    }

    bool isMsgYoshiEat(u32 msg) {
        return msg == 0x9;
    }

    // NONMATCHING: MWCC folds the two compares of the || into a range check; target keeps both cmplwi
    // NONMATCHING: equivalent range-check lowering
    bool isMsgYoshiProjectileOrEat(u32 msg) {
        bool result = false;
        if (msg == 0x8) {
            result = true;
        } else if (msg == 0x9) {
            result = true;
        }

        return result;
    }

    bool isMsgDrillAttack(u32 msg) {
        return msg == 0x10;
    }

    bool isMsgRockAttack(u32 msg) {
        return msg == 0x11;
    }

    bool isMsgSnowBallAttack(u32 msg) {
        return msg == 0x12;
    }

    bool isMsgSupportTicoSpin(u32 msg) {
        return msg == 0x13;
    }

    bool isMsgItemRushBegin(u32 msg) {
        return msg == 0x94;
    }

    bool isMsgRushForceCancel(u32 msg) {
        return msg == 0x96;
    }

    bool isMsgRushEnd(u32 msg) {
        return msg == 0x98;
    }

    bool isMsgIsRushEnable(u32 msg) {
        return msg == 0x99;
    }

    bool isMsgItemNigeroTake(u32 msg) {
        return msg == 0x8a;
    }

    bool isMsgItemNigeroDrop(u32 msg) {
        return msg == 0x8d;
    }

    bool isMsgItemSupportTicoDrop(u32 msg) {
        return msg == 0x8e;
    }

    bool isMsgStartDemo(u32 msg) {
        return msg == 0x7f;
    }

    bool isMsgToEnemyAttackRoll(u32 msg) {
        return msg == 0x71;
    }

    bool isMsgToEnemyAttackTrampleDeath(u32 msg) {
        return msg == 0x73;
    }

    bool isMsgSuccess(u32 msg) {
        return msg == 0x76;
    }

    bool isMsgStarLightGrow(u32 msg) {
        return msg == 0xcd;
    }

    bool isMsgStarLightShrink(u32 msg) {
        return msg == 0xce;
    }

    bool isMsgReflect(u32 msg) {
        return msg == 0x48;
    }

    bool isMsgReflectV(u32 msg) {
        return msg == 0x49;
    }

    bool isMsgGroupMoveStart(u32 msg) {
        return msg == 0x7a;
    }

    bool isMsgGroupMoveStop(u32 msg) {
        return msg == 0x7b;
    }

    bool isMsgGroupShow(u32 msg) {
        return msg == 0x7d;
    }

    bool isMsgGroupHide(u32 msg) {
        return msg == 0x7e;
    }

#pragma dont_inline reset
};  // namespace MR
