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

namespace NrvKoopaJrCastleRingBeam { KoopaJrCastleRingBeamNrvPre KoopaJrCastleRingBeamNrvPre::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleRingBeam { KoopaJrCastleRingBeamNrvA KoopaJrCastleRingBeamNrvA::sInstance ATTRIBUTE_WEAK; }

#include "Game/Enemy/RingBeam.hpp"
#include "Game/Util/FixedPosition.hpp"

const volatile s32 sKoopaJrCastleRingBeamColor = 0;

KoopaJrCastleRingBeam::KoopaJrCastleRingBeam(const char* pName) : LiveActor(pName) {
    _90 = nullptr;
    _94 = nullptr;
}

void KoopaJrCastleRingBeam::init(const JMapInfoIter& iter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    _94 = new RingBeam*[5];
    for (s32 i = 0; i < 5; i++) _94[i] = nullptr;
    s32 color = sKoopaJrCastleRingBeamColor;
    for (s32 i = 0; i < 5; i++) {
        _94[i] = new RingBeam("\x83\x8a\x83\x93\x83\x4f\x83\x72\x81\x5b\x83\x80", this, false, false, true, true, nullptr);
        _94[i]->init(iter);
        _94[i]->setSpeed(18.0f);
        _94[i]->setLife(110);
        _94[i]->setColor(color);
    }
    initNerve(&NrvKoopaJrCastleRingBeam::KoopaJrCastleRingBeamNrvPre::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleRingBeam::appear() {
    LiveActor::appear();
    control();
    setNerve(&NrvKoopaJrCastleRingBeam::KoopaJrCastleRingBeamNrvA::sInstance);
    fn_800D7CE0();
}

MtxPtr KoopaJrCastleRingBeam::getBaseMtx() const {
    return (MtxPtr)&_90->mMtx;
}

void KoopaJrCastleRingBeam::control() {
    _90->calc();
    TVec3f pos(0.0f);
    TVec3f up(0.0f);
    MR::extractMtxTrans((MtxPtr)&_90->mMtx, &pos);
    MR::extractMtxYDir((MtxPtr)&_90->mMtx, &up);
    MR::setPosition(this, pos);
    MR::setQuatUpNoSupport(this, up);
}

void KoopaJrCastleRingBeam::setup(LiveActor* host, const TVec3f& trans, const TVec3f& rot, const char* joint) {
    if (joint) {
        _90 = new FixedPosition(host, joint, trans, rot);
        return;
    }
    _90 = new FixedPosition(host, trans, rot);
}

void KoopaJrCastleRingBeam::exe800D7C00() {
    for (s32 i = 0; i < 5; i++) {
        if (MR::isDead(_94[i])) {
            _94[i]->setSpeed(18.0f);
            _94[i]->setLife(110);
            _94[i]->setColor(sKoopaJrCastleRingBeamColor);
            _94[i]->appear();
            break;
        }
    }
    setNerve(&NrvKoopaJrCastleRingBeam::KoopaJrCastleRingBeamNrvPre::sInstance);
}

void KoopaJrCastleRingBeam::fn_800D7CE0() {
    for (s32 i = 0; i < 5; i++) {
        if (!MR::isDead(_94[i])) _94[i]->makeActorDead();
    }
}

KoopaJrCastleRingBeam::~KoopaJrCastleRingBeam() {}
