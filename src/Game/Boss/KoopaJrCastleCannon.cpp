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

namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvPre KoopaJrCastleCannonNrvPre::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvA KoopaJrCastleCannonNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvB KoopaJrCastleCannonNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvC KoopaJrCastleCannonNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvD KoopaJrCastleCannonNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvE KoopaJrCastleCannonNrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvA KoopaJrCastleCannonShockNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvB KoopaJrCastleCannonShockNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvC KoopaJrCastleCannonShockNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleCannon::KoopaJrCastleCannon(KoopaJrCastle* pHost, const char* pName)
    : LiveActor(pName), mHost(pHost), mShock(nullptr), _C8(1), _CC(0), _D0(0), _D4(false), _D8(0.0f) {
    for (int i = 0; i < 12; i++) {
        mBall[i] = nullptr;
    }
    MR::setPosition(this, pHost->mPosition);
}

KoopaJrCastleCannon::~KoopaJrCastleCannon() {}

void KoopaJrCastleCannon::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    mShock = new KoopaJrCastleCannonShock("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
    mShock->initWithoutIter();
    for (int i = 0; i < 12; i++) {
        mBall[i] = new KoopaJrCastleCannonBall("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
        MR::setScale(mBall[i], 1.5f);
        mBall[i]->initWithoutIter();
    }
    initNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleCannon::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance);
    _CC = 0;
    _D4 = false;
    _D8 = 0.0f;
}

void KoopaJrCastleCannon::fn_800D0540() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
}

void KoopaJrCastleCannon::fn_800D0580() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::fn_800D05C0() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvC::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::makeActorDead() {
    LiveActor::makeActorDead();
    mHost->mHead->fn_800D57D0();
}

void KoopaJrCastleCannon::killAllBalls() {
    for (int i = 0; i < 12; i++) {
        if (!MR::isDead(mBall[i])) {
            mBall[i]->makeActorDead();
        }
    }
}

KoopaJrCastleCannonBall* KoopaJrCastleCannon::findDeadBall() {
    for (int i = 0; i < 12; i++) {
        if (MR::isDead(mBall[i])) {
            return mBall[i];
        }
    }
    return nullptr;
}

void KoopaJrCastleCannon::updateGauge() {
    _D8 = _D8 + 0.02f;
    _D8 = MR::clamp(_D8, 0.0f, 0.55f);
}

KoopaJrCastleCannonShock::KoopaJrCastleCannonShock(const char* pName) : LiveActor(pName) {
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
    _9C = 0.0f;
    _A0 = 1.0f;
}

KoopaJrCastleCannonShock::~KoopaJrCastleCannonShock() {}
