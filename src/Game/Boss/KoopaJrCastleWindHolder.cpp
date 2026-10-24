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
#include "Game/Util.hpp"

namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvA KoopaJrCastleWindNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvB KoopaJrCastleWindNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvC KoopaJrCastleWindNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvD KoopaJrCastleWindNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvE KoopaJrCastleWindNrvE::sInstance ATTRIBUTE_WEAK; }

namespace {
    KoopaJrCastleWindHolder* getWindHolder() {
        return MR::getSceneObj< KoopaJrCastleWindHolder >(SceneObj_KoopaJrCastleWindHolder);
    }
}  // namespace

bool KoopaJrCastleWindHolder::calcWindInfo(const TVec3f& rPos, TVec3f* pDir, f32* pPower) const {
    pDir->zero();
    *pPower = -1.0f;
    f32 zero = 0.0f;
    f32 minPower = 1000000.0f;
    TVec3f sum(0.0f, 0.0f, 0.0f);
    LiveActor* pActor;
    bool found = false;
    for (s32 i = 0; i < getObjNum(); i++) {
        pActor = getActor(i);
        if (!MR::isDead(pActor)) {
            TVec3f dir(0.0f, 0.0f, 0.0f);
            f32 power = zero;
            if (static_cast< KoopaJrCastleWindBase* >(pActor)->calcWindInfo(&rPos, &dir, &power)) {
                found = true;
                sum += dir;
                if (power < minPower) {
                    minPower = power;
                }
            }
        }
    }
    pDir->set(sum);
    *pPower = minPower;
    return found;
}

void KoopaJrCastleWindFunction::createWindHolder() {
    MR::createSceneObj(SceneObj_KoopaJrCastleWindHolder);
}

void KoopaJrCastleWindFunction::registerWind(LiveActor* pActor) {
    ::getWindHolder()->registerActor(pActor);
}

bool KoopaJrCastleWindFunction::calcWindInfo(const TVec3f* pPos, TVec3f* pDir, f32* pPower) {
    if (!MR::isExistSceneObj(SceneObj_KoopaJrCastleWindHolder)) {
        pDir->zero();
        *pPower = -1.0f;
        return false;
    }
    return ::getWindHolder()->calcWindInfo(*pPos, pDir, pPower);
}

KoopaJrCastleWindHolder::~KoopaJrCastleWindHolder() {}

KoopaJrCastleWind::KoopaJrCastleWind(KoopaJrCastle* pHost, const char* pName) : KoopaJrCastleWindBase(pName), mCastle(pHost), mPower(0.0f) {
}

KoopaJrCastleWindBase::~KoopaJrCastleWindBase() {}

void KoopaJrCastleWind::init(const JMapInfoIter&) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    MR::invalidateClipping(this);
    initNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvB::sInstance, 0);
    makeActorDead();
    KoopaJrCastleWindFunction::registerWind(this);
}

void KoopaJrCastleWind::control() {
    f32 scale = 1050.0f;
    if (KoopaJrCastleFunction::getBattlePhase() == 1) {
        scale = 1050.0f;
    }
    TVec3f pos(mCastle->mBody->mPosition - mGravity.scaleInline(scale));
    MR::setPosition(this, pos);
}

void KoopaJrCastleWind::appear() {
    LiveActor::appear();
    mPower = 0.0f;
    setNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvD::sInstance);
}

void KoopaJrCastleWind::appearFromZeroPower() {
    LiveActor::appear();
    mPower = 0.0f;
    setNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvA::sInstance);
}

void KoopaJrCastleWind::makeActorDead() {
    LiveActor::makeActorDead();
    MR::tryDeleteEffect(mCastle->mBody, "WindRing");
    MR::tryDeleteEffect(mCastle->mBody, "WindRingVacuum");
    if (mCastle->mBody->mModelManager != NULL) {
        if (MR::isBckPlaying(mCastle->mBody, "Blow") || MR::isBckPlaying(mCastle->mBody, "BlowStart") || MR::isBckPlaying(mCastle->mBody, "BlowEnd")) {
            MR::startAction(mCastle->mBody, "Wait");
        }
    }
}

bool KoopaJrCastleWind::isBlowing() {
    bool ret = false;
    if ((isNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvA::sInstance) && MR::isGreaterStep(this, 0x2B)) ||
        (isNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvD::sInstance) && MR::isGreaterStep(this, 0x2B)) ||
        isNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvE::sInstance) || isNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvB::sInstance)) {
        ret = true;
    }
    return ret;
}

void KoopaJrCastleWind::exeBlowStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mCastle->mBody, "BlowStart");
    }
    if (MR::isGreaterStep(this, 0x49)) {
        setNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvB::sInstance);
    }
}

void KoopaJrCastleWind::exeVacuum() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mCastle->mBody, "Blow");
        MR::emitEffect(mCastle->mBody, "WindRingVacuum");
    }
    mPower = -0.95f;
    if (MR::isGreaterStep(this, 0xF5)) {
        MR::tryDeleteEffect(mCastle->mBody, "WindRing");
        MR::tryDeleteEffect(mCastle->mBody, "WindRingVacuum");
    }
    if (MR::isGreaterStep(this, 0x12C)) {
        setNerve(&NrvKoopaJrCastleWind::KoopaJrCastleWindNrvC::sInstance);
    }
}

void KoopaJrCastleWind::exeBlowEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mCastle->mBody, "BlowEnd");
    }
    mPower = 0.0f;
    if (MR::isBckOneTimeAndStopped(mCastle->mBody)) {
        MR::startAction(mCastle->mBody, "Wait");
    }
    if (MR::isGreaterStep(this, 0x1E)) {
        kill();
    }
}

void KoopaJrCastleWind::exeBlow() {
    mPower = 1.6f;
    MR::emitEffect(mCastle->mBody, "WindRing");
    if (MR::isGreaterStep(this, 0x10E)) {
        kill();
    }
}

bool KoopaJrCastleWind::calcWindInfo(const TVec3f* pPos, TVec3f* pDir, f32* pPower) {
    f32 power;
    TVec3f toTarget(0.0f);
    MR::calcVecToTargetHorizon(&toTarget, this, pPos, NULL);
    f32 dist = toTarget.length();
    if (0.0f < dist && dist < 10000.0f && MR::calcDistanceVertical(this, *pPos) < 3000.0f) {
        f32 ease = dist >= 400.0f ? MR::getEaseInValueWithNormalize(dist, 400.0f, 10000.0f, 1.0f, 0.625f) :
                                    -MR::getEaseInValueWithNormalize(dist, 0.0f, 400.0f, 1.0f, 0.625f);
        power = mPower;
        *pDir = toTarget / dist * power * ease;
        *pPower = dist;
        return true;
    }
    return false;
}

KoopaJrCastleWind::~KoopaJrCastleWind() {}

// The splits place this ctor (and its name string) at the start of this unit, but its string lives at 0x80675640, before this unit's
// string pool (0x80675690): the ctor most likely belongs to the previous unit. It is kept last here so the pool offsets match.
KoopaJrCastleWindHolder::KoopaJrCastleWindHolder() : LiveActorGroup("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x95\x97\x83z\x83\x8b\x83_\x81[", 0x20) {}
