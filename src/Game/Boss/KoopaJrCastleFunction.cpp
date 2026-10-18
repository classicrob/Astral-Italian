#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvA KoopaJrCastleHeadNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvB KoopaJrCastleHeadNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvC KoopaJrCastleHeadNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvD KoopaJrCastleHeadNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvE KoopaJrCastleHeadNrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvF KoopaJrCastleHeadNrvF::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvG KoopaJrCastleHeadNrvG::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvH KoopaJrCastleHeadNrvH::sInstance ATTRIBUTE_WEAK; }

namespace KoopaJrCastleFunction {
#define CASTLE() (MR::getSceneObj< KoopaJrCastleAccessor* >(0x8B)->mCastle)

    KoopaJrCastle* getCastle() {
        return CASTLE();
    }

    s32 getBattlePhase() {
        return CASTLE()->mStateBattle->mPhase;
    }

    const TVec3f& getStartPos() {
        return CASTLE()->mStartPos;
    }

    bool isCapsuleSensorHost(const KoopaJrCastle* pCastle, const HitSensor* pSensor) {
        LiveActor* pCapsule = pCastle->mCapsule;
        return pCapsule == MR::getSensorHost(pSensor);
    }

    bool isPlayerOnCapsule() {
        return MR::isOnPlayer(CASTLE()->mCapsule);
    }

    bool isPlayerOnHead() {
        return MR::isOnPlayer(CASTLE()->mHead);
    }

    bool isPlayerOnCastle() {
        return isPlayerOnCapsule() || isPlayerOnHead();
    }

    void windFn_800D4290() {
        CASTLE()->mWind->fn_800D9B40();
    }

    bool windIsDead() {
        return MR::isDead(CASTLE()->mWind);
    }

    bool windFn_800D4310() {
        return CASTLE()->mWind->fn_800D9C40();
    }

    void bodyStartMove0() {
        CASTLE()->mBody->startMove0();
    }

    void bodyStartMove1() {
        CASTLE()->mBody->startMove1();
    }

    void bodyStartMove2() {
        CASTLE()->mBody->startMove2();
    }

    bool bodyIsNotMove() {
        return !CASTLE()->mBody->isMove();
    }

    void bodyTryStop() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!bodyIsNotMove()) {
            pCastle->mBody->setNerveStop();
        }
    }

    bool bodyIsMoveState() {
        return CASTLE()->mBody->isMoveState();
    }

    u8 bodyGetStateFlag() {
        return CASTLE()->mBody->mState->getFlag();
    }

    void cannonFn_800D4590() {
        CASTLE()->mCannon->fn_800D0540();
    }

    void cannonFn_800D45D0() {
        CASTLE()->mCannon->fn_800D05C0();
    }

    void cannonFn_800D4610() {
        CASTLE()->mCannon->fn_800D0580();
    }

    void killCannon() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mCannon)) {
            pCastle->mCannon->kill();
        }
    }

    bool isCannonDead() {
        return MR::isDead(CASTLE()->mCannon);
    }

    bool isHeadFn_800D4790() {
        return !CASTLE()->mHead->fn_800D58D0();
    }

    void appearFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
    }

    void killFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->kill();
        }
    }

    void fireBarOff() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 0;
        }
    }

    void fireBarOn() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 1;
        }
    }

    void appearFireBarAndStart() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
        pCastle->mFireBar->fn_800D2C50();
    }

    f32 getFireBarB0() {
        KoopaJrCastleFireBar* pFireBar = CASTLE()->mFireBar;
        if (pFireBar != nullptr) {
            return pFireBar->_B0;
        }
        return 0.0f;
    }

    void killRingBeam() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mRingBeam)) {
            pCastle->mRingBeam->kill();
        }
    }

    void appearElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->appear();
        }
    }

    void killElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->kill();
        }
    }

    bool isElectricDead() {
        return MR::isDead(CASTLE()->mElectric);
    }

    bool isElectricActive() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric) && pCastle->mElectric->fn_800D2250()) {
            return true;
        }
        return false;
    }
};  // namespace KoopaJrCastleFunction

KoopaJrCastleHead::KoopaJrCastleHead(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleHead", 0, 0x13, false) {
    _A0 = nullptr;
    _D4 = true;
    PSMTXIdentity(_A4);
}

KoopaJrCastleHead::~KoopaJrCastleHead() {}

void KoopaJrCastleHead::setNerveA() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

void KoopaJrCastleHead::setNerveB() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance);
}

void KoopaJrCastleHead::setNerveC() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance);
}

void KoopaJrCastleHead::setNerveD() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
}

void KoopaJrCastleHead::fn_800D57D0() {
    if (!isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance) && !isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
    }
}

bool KoopaJrCastleHead::fn_800D5830() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D5880() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D58D0() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance)) {
        ret = true;
    }
    return ret;
}
